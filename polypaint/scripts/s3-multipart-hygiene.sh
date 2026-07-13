#!/bin/bash
# S3 multipart hygiene (CR34-streaming review F4).
#
# Code-level abort is best effort and cannot run after a Lambda timeout or
# process kill, so incomplete multipart uploads accrue invisible storage
# forever without a bucket lifecycle backstop (2026-07-13: eight orphans
# from 2026-03-24 were found, predating the streaming uploader entirely).
#
# READ-ONLY by default: lists incomplete uploads. Mutations are explicit:
#   ./scripts/s3-multipart-hygiene.sh                        # list only
#   ./scripts/s3-multipart-hygiene.sh --apply-lifecycle 7    # abort-after-7-days rule
#   ./scripts/s3-multipart-hygiene.sh --abort-before 2026-07-01  # abort old uploads
set -euo pipefail

BUCKET="${BUCKET:-polypaint}"
REGION="${REGION:-us-east-1}"

list_uploads() {
    aws s3api list-multipart-uploads --bucket "$BUCKET" --region "$REGION" \
        --query 'Uploads[].{Key:Key,UploadId:UploadId,Initiated:Initiated}' \
        --output json 2>/dev/null || echo "[]"
}

case "${1:-list}" in
list)
    UPLOADS=$(list_uploads)
    COUNT=$(echo "$UPLOADS" | python3 -c 'import json,sys; print(len(json.load(sys.stdin) or []))')
    echo "Incomplete multipart uploads in s3://$BUCKET: $COUNT"
    echo "$UPLOADS" | python3 -c '
import json, sys
for u in (json.load(sys.stdin) or []):
    print("  %s  %s  %s" % (u["Initiated"], u["Key"], u["UploadId"][:20] + "..."))'
    echo
    echo "Lifecycle configuration:"
    aws s3api get-bucket-lifecycle-configuration --bucket "$BUCKET" --region "$REGION" 2>&1 \
        | head -20 || true
    ;;
--apply-lifecycle)
    DAYS="${2:?usage: --apply-lifecycle <days>}"
    aws s3api put-bucket-lifecycle-configuration --bucket "$BUCKET" --region "$REGION" \
        --lifecycle-configuration "{
            \"Rules\": [{
                \"ID\": \"abort-incomplete-multipart\",
                \"Status\": \"Enabled\",
                \"Filter\": {},
                \"AbortIncompleteMultipartUpload\": {\"DaysAfterInitiation\": ${DAYS}}
            }]
        }"
    echo "Applied: incomplete multipart uploads abort after ${DAYS} days."
    ;;
--abort-before)
    CUTOFF="${2:?usage: --abort-before YYYY-MM-DD}"
    list_uploads | python3 -c "
import json, subprocess, sys
cutoff = '${CUTOFF}'
uploads = json.load(sys.stdin) or []
stale = [u for u in uploads if u['Initiated'][:10] < cutoff]
print('Aborting %d of %d incomplete uploads (initiated before %s)' % (len(stale), len(uploads), cutoff))
for u in stale:
    subprocess.run(['aws', 's3api', 'abort-multipart-upload',
                    '--bucket', '${BUCKET}', '--region', '${REGION}',
                    '--key', u['Key'], '--upload-id', u['UploadId']], check=True)
    print('  aborted %s (%s)' % (u['Key'], u['Initiated']))
"
    ;;
*)
    echo "usage: $0 [list | --apply-lifecycle <days> | --abort-before <YYYY-MM-DD>]" >&2
    exit 2
    ;;
esac
