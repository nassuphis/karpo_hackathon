# Lambda Compute Patterns

Using AWS Lambda for embarrassingly parallel numerical workloads — Monte Carlo, frame rendering, fan-out patterns, cross-compilation, debugging.

## Why Lambda for numerical compute

- **Pay-per-use**: billed per request + per GB-second of compute, zero idle cost
- **No infrastructure**: no AMIs, no SSH, no Docker images, no cluster management, no patching
- **Elastic**: 1000 concurrent instances by default (can request more)
- **Fire and forget**: coordinator kicks off jobs and walks away, Lambdas keep running

Replaces the old pattern of provisioning EC2 workers (e.g. 20x 96-core Graviton instances), managing them, feeding them work, monitoring, paying for idle time.

## Key limits

| Limit | Value |
|-------|-------|
| Max memory | 10 GB (gives ~6 vCPUs) |
| Max timeout | 15 minutes |
| Max response payload | 6 MB (via API Gateway) |
| Max deployment package | 50 MB zipped / 250 MB unzipped |
| Max `/tmp` storage | 10 GB |
| Default concurrency | 1000 (can request increase) |
| API Gateway timeout | 30 seconds (hard limit) |

## Fan-out pattern (embarrassingly parallel)

For independent work units (Monte Carlo batches, frame rendering, parameter sweeps):

```python
# coordinator.py — runs on your laptop, fire-and-forget
import boto3, json

client = boto3.client("lambda")
for i in range(10000):
    client.invoke(
        FunctionName="frame-renderer",
        InvocationType="Event",  # async, returns immediately
        Payload=json.dumps({"frame": i, "t": i/10000, "params": {...}})
    )
```

Each Lambda writes its result to S3. Monitor progress:
```bash
aws s3 ls s3://bucket/frames/ | wc -l
```

Your machine doesn't stay connected — close laptop, Lambdas keep running.

## Writing results to S3

Better than returning results via API Gateway (no 6MB limit, no 30s timeout):

```python
import boto3, json

s3 = boto3.client("s3")

def handler(event, context):
    params = json.loads(event.get("body", "{}") if "body" in event else json.dumps(event))
    result = compute(params)
    s3.put_object(
        Bucket="my-bucket",
        Key=f"results/{params['job_id']}.json",
        Body=json.dumps(result)
    )
    return {"statusCode": 200, "body": "ok"}
```

## S3 query cache (immutable data)

If inputs are immutable (e.g. result sets), cache query results by content hash:

```python
import hashlib

cache_key = hashlib.sha256(f"{result_name}:{sql}".encode()).hexdigest()
s3_path = f"query_cache/{cache_key}.json"

# Check cache first, skip compute if hit
try:
    obj = s3.get_object(Bucket=BUCKET, Key=s3_path)
    return json.loads(obj["Body"].read())
except s3.exceptions.NoSuchKey:
    result = expensive_compute(...)
    s3.put_object(Bucket=BUCKET, Key=s3_path, Body=json.dumps(result))
    return result
```

## Python numerical libraries on Lambda

- **NumPy** — vectorized array ops, random number generation. ~30MB. Handles most Monte Carlo without loops.
- **Numba** — JIT compiles Python to machine code. ~150MB with llvmlite. First call on cold instance pays ~3-5s compile time, cached in `/tmp` for warm invocations. Worth it for path-dependent logic that can't vectorize.
- **SciPy** — distributions, optimization, interpolation.
- All installable via pip into the deployment package.

## Native binaries (C/C++/Rust)

For maximum performance, compile a native binary and call from Python:

```python
import subprocess, json, os

def handler(event, context):
    params = json.loads(event["body"])
    binary = os.path.join(os.path.dirname(__file__), "solver")
    result = subprocess.run(
        [binary, json.dumps(params)],
        capture_output=True, text=True
    )
    return {"statusCode": 200, "body": result.stdout}
```

Binary reads JSON from argv/stdin, computes, prints JSON to stdout.

## Cross-compiling for Graviton (ARM64)

Graviton Lambdas are 20% cheaper per GB-second. Cross-compile using AWS Docker images:

```bash
# ARM64 build using Amazon Linux 2023 (exact Lambda environment)
docker run --rm --platform linux/arm64 \
  -v $(pwd):/src -w /src \
  public.ecr.aws/amazonlinux/amazonlinux:2023 \
  bash -c "yum install -y gcc gcc-c++ make -q && gcc -O3 -static solver.c -o solver -lm"
```

On Apple Silicon Mac, `--platform linux/arm64` runs natively (no emulation).

For Python packages with C extensions:
```bash
docker run --rm --platform linux/arm64 \
  -v $(pwd):/src -w /src \
  public.ecr.aws/sam/build-python3.12:latest \
  bash -c "pip install numpy numba -t /src/package/"
```

**Rust** (easiest cross-compile):
```bash
rustup target add aarch64-unknown-linux-musl
brew install filosottile/musl-cross/musl-cross
CARGO_TARGET_AARCH64_UNKNOWN_LINUX_MUSL_LINKER=aarch64-linux-musl-gcc \
  cargo build --release --target aarch64-unknown-linux-musl
```

## Creating a Graviton Lambda

```bash
aws lambda create-function --function-name frame-renderer \
  --runtime python3.12 --architectures arm64 \
  --handler handler.handler --zip-file fileb://deploy.zip \
  --role arn:aws:iam::710848990594:role/lambda-role \
  --memory-size 1024 --timeout 30 --region us-east-1
```

## Debugging

**Print to CloudWatch** (simplest):
```bash
aws logs tail /aws/lambda/my-function --follow
```

**Test locally** — handler is just a Python function:
```python
from handler import handler
result = handler({"body": '{"param": 42}'}, None)
print(result)
```

**Docker local emulation** (catches environment issues):
```bash
docker run --rm -p 9000:8080 -v $(pwd):/var/task \
  public.ecr.aws/lambda/python:3.12 handler.handler

curl -X POST "http://localhost:9000/2015-03-31/functions/function/invocations" \
  -d '{"body": "{\"param\": 42}"}'
```

**CloudWatch error search**:
```bash
aws logs filter-log-events \
  --log-group-name /aws/lambda/my-function \
  --start-time $(date -v-10M +%s000) \
  --filter-pattern "ERROR" --region us-east-1
```

## Cost examples

| Workload | Config | Cost |
|----------|--------|------|
| 10K frames x 5s each | 10GB Lambda | ~$8.35 |
| 10K frames x 5s each | 10GB Graviton | ~$6.68 (20% cheaper) |
| 1M Monte Carlo paths x 2s | 3GB Lambda | ~$0.10 |
| Light API queries (100/day) | 10GB Lambda | ~$0.05/day |

## Frame rendering movie workflow

1. Coordinator script on laptop fires 10K async Lambda invocations
2. Each Lambda renders one frame, saves `frame_XXXXX.png` to S3
3. With 1000 concurrency limit, completes in ~10 waves (~50s wall clock)
4. Download frames locally, stitch with ffmpeg:
   ```bash
   aws s3 sync s3://bucket/frames/ ./frames/
   ffmpeg -framerate 30 -i frames/frame_%05d.png -c:v libx264 -pix_fmt yuv420p movie.mp4
   ```
