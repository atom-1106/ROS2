#!/bin/bash

SRC_DIR="$(pwd)/../apps"

pushd "${SRC_DIR}"

docker compose -f docker-compose.yml up -d --remove-orphans

popd