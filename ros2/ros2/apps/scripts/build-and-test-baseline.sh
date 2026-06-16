#!/bin/bash
# Build and optionally smoke-test cat_apps baseline pub/sub inside GuestOS-Base.
#
# Usage (from repo host, after launch-guest-os.sh):
#   docker exec -it GuestOS-Base-${USER} bash /app/ros2_ws/scripts/build-and-test-baseline.sh
#
# Or inside the container:
#   cd /app/ros2_ws && ./scripts/build-and-test-baseline.sh
#
# Build only (skip smoke test):
#   RUN_SMOKE_TEST=0 ./scripts/build-and-test-baseline.sh

set -euo pipefail

WORKSPACE="/app/ros2_ws"
PACKAGES="cat_msgs pugixml cat_apps process_launcher"
RUN_SMOKE_TEST="${RUN_SMOKE_TEST:-1}"

cd "${WORKSPACE}"

echo "=== Sourcing ROS2 Jazzy ==="
source /opt/ros/jazzy/setup.bash

echo "=== Building packages: ${PACKAGES} ==="
colcon build --packages-select ${PACKAGES} --symlink-install

echo "=== Sourcing workspace ==="
source install/setup.bash

CONFIG="$(ros2 pkg prefix cat_apps)/share/cat_apps/config-example.xml"
PUBLISHER="$(ros2 pkg prefix cat_apps)/lib/cat_apps/BaselinePublisher"
SUBSCRIBER="$(ros2 pkg prefix cat_apps)/lib/cat_apps/BaselineSubscriber"

echo "=== Installed paths ==="
echo "Config:      ${CONFIG}"
echo "Publisher:   ${PUBLISHER}"
echo "Subscriber:  ${SUBSCRIBER}"

if [[ ! -f "${CONFIG}" ]]; then
  echo "ERROR: config file not found at ${CONFIG}" >&2
  exit 1
fi

if [[ "${RUN_SMOKE_TEST}" != "1" ]]; then
  echo "=== Build complete (smoke test skipped, RUN_SMOKE_TEST=${RUN_SMOKE_TEST}) ==="
  exit 0
fi

echo "=== Smoke test: subscriber + publisher for 5 seconds ==="
LOG_DIR="$(mktemp -d)"
SUB_LOG="${LOG_DIR}/subscriber.log"
PUB_LOG="${LOG_DIR}/publisher.log"

"${SUBSCRIBER}" "${CONFIG}" > "${SUB_LOG}" 2>&1 &
SUB_PID=$!
sleep 2

"${PUBLISHER}" 0 "${CONFIG}" > "${PUB_LOG}" 2>&1 &
PUB_PID=$!
sleep 5

kill "${PUB_PID}" 2>/dev/null || true
kill "${SUB_PID}" 2>/dev/null || true
wait "${PUB_PID}" 2>/dev/null || true
wait "${SUB_PID}" 2>/dev/null || true

echo "--- Subscriber log ---"
cat "${SUB_LOG}"
echo "--- Publisher log ---"
cat "${PUB_LOG}"

if grep -q "received \[Parameter1\]=" "${SUB_LOG}"; then
  echo "=== PASS: subscriber received Parameter1 data ==="
else
  echo "=== FAIL: expected received [Parameter1]= in subscriber log ===" >&2
  exit 1
fi

if grep -q "Publisher created" "${PUB_LOG}"; then
  echo "=== PASS: publisher started ==="
else
  echo "=== FAIL: publisher did not start correctly ===" >&2
  exit 1
fi

echo "=== Smoke test: ros2 launch baseline_pubsub.launch.yaml for 8 seconds ==="
LAUNCH_LOG="${LOG_DIR}/launch.log"
timeout 8 ros2 launch cat_apps baseline_pubsub.launch.yaml > "${LAUNCH_LOG}" 2>&1 || true

if grep -q "received \[Parameter1\]=" "${LAUNCH_LOG}"; then
  echo "=== PASS: launch file delivered Parameter1 data ==="
else
  echo "=== FAIL: launch file smoke test — expected received [Parameter1]= in log ===" >&2
  echo "--- Launch log ---"
  cat "${LAUNCH_LOG}"
  exit 1
fi

echo "=== All checks passed ==="
rm -rf "${LOG_DIR}"
