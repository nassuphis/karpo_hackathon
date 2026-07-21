#!/usr/bin/env python3
"""Build nested JSON values passed to AWS CLI options."""

from __future__ import annotations

import argparse
import json


def poly_sheet_gc_attributes(dlq_arn: str) -> dict[str, str]:
    arn = str(dlq_arn or "").strip()
    if not arn or arn == "None":
        raise ValueError("Poly-Sheet GC dead-letter queue ARN is empty")
    redrive_policy = {
        "deadLetterTargetArn": arn,
        "maxReceiveCount": "5",
    }
    return {
        "VisibilityTimeout": "6000",
        "MessageRetentionPeriod": "345600",
        "RedrivePolicy": json.dumps(redrive_policy, separators=(",", ":")),
    }


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("command", choices=("poly-sheet-gc-attributes",))
    parser.add_argument("dlq_arn")
    args = parser.parse_args()

    if args.command == "poly-sheet-gc-attributes":
        value = poly_sheet_gc_attributes(args.dlq_arn)
        print(json.dumps(value, separators=(",", ":")))


if __name__ == "__main__":
    main()
