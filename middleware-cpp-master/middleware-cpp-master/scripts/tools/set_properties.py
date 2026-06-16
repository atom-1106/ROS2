#!/usr/bin/env python3
# Copyright (C) Caterpillar Inc. All Rights Reserved.
# File: set_properties.py
# Description: Tags an Artifactory package version with arbitrary properties.

from __future__ import annotations

import argparse
import os
import sys
import time
from pathlib import Path

import requests

ARTIFACTORY_URL = "https://stuff.ecorp.cat.com/artifactory/api/storage"
ARTIFACTORY_REPO = "cat-e-conan-dev-local"


def build_session(token: str) -> requests.Session:
    """Create an authenticated requests session."""
    session = requests.Session()
    session.headers.update({"Authorization": f"Bearer {token}"})
    return session


def set_properties(session: requests.Session, url: str, properties: dict[str, str]) -> None:
    """
    Set properties on an Artifactory item, deleting any existing values first
    so the PUT overwrites rather than appends.

    :param session: Authenticated requests session.
    :param url: Base item URL (without query string).
    :param properties: Mapping of property name to value.
    """
    prop_names = ",".join(properties.keys())
    prop_string = ";".join(f"{k}={v}" for k, v in properties.items()) + ";"

    session.delete(f"{url}?properties={prop_names}")
    response = session.put(f"{url}?properties={prop_string}")
    response.raise_for_status()


def wait_for_version(session: requests.Session, url: str, version: str, max_retries: int = 3) -> None:
    """
    Poll until the given version URL returns HTTP 200, retrying up to max_retries times.

    :param session: Authenticated requests session.
    :param url: Full URL for the package version.
    :param version: Version string (used in error messages).
    :param max_retries: Number of attempts before giving up.
    """
    for attempt in range(1, max_retries + 1):
        if session.get(url).status_code == 200:
            return
        print(
            f"Version '{version}' not found (attempt {attempt}/{max_retries}), retrying in 1s...",
            file=sys.stderr,
        )
        if attempt == max_retries:
            print(f"Error: version '{version}' does not exist after {max_retries} attempts.", file=sys.stderr)
            sys.exit(1)
        time.sleep(1)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Set properties on an Artifactory package version.",
        epilog="Example: set_properties.py --package cate_middleware --version 1.0.0-98765 "
               "--property BUILD_ID --value 98765 --property BUILD_URL --value https://example.com "
               "--token /path/to/token",
    )
    parser.add_argument("--package", required=True, help="Conan package name (e.g. cate_middleware)")
    parser.add_argument("--version", required=True, help="Package version string (e.g. 1.0.0-98765)")
    parser.add_argument(
        "--property",
        dest="properties",
        metavar="KEY",
        action="append",
        required=True,
        help="Property name to set; may be repeated alongside --value",
    )
    parser.add_argument(
        "--value",
        dest="values",
        metavar="VALUE",
        action="append",
        required=True,
        help="Property value to set; must be given once per --property",
    )
    parser.add_argument("--token", required=True, type=lambda p: Path(p).resolve(), help="Path to the Artifactory bearer token file (resolved to absolute)")
    return parser.parse_args()


def load_token(token_file: Path) -> str:
    """Read and return the bearer token from an absolute path file."""
    if not token_file.is_file() or not os.access(token_file, os.R_OK):
        print(f"Error: Token file '{token_file}' does not exist or is not readable.", file=sys.stderr)
        sys.exit(1)
    return token_file.read_text().strip()


def main() -> None:
    args = parse_args()

    if len(args.properties) != len(args.values):
        print("Error: --property and --value must be provided in matching pairs.", file=sys.stderr)
        sys.exit(1)

    token = load_token(args.token)
    session = build_session(token)

    properties = dict(zip(args.properties, args.values))
    api_url = f"{ARTIFACTORY_URL}/{ARTIFACTORY_REPO}/_/{args.package}/{args.version}"

    wait_for_version(session, api_url, args.version)

    set_properties(session, api_url, properties)

    for key, value in properties.items():
        print(f"{key} = {value}")


if __name__ == "__main__":
    main()
