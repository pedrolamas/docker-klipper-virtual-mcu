#!/bin/bash
# Start the image and check that the printer works through Moonraker.
#
# usage: smoke-test.sh IMAGE
#
# CONTAINER_CLI       container tool to use (default: docker)
# CONTAINER_RUN_ARGS  extra arguments for the "run" command
# SMOKE_PORT          host port for Moonraker (default: 7125)
# PYTHON              python interpreter (default: python3)
set -euo pipefail

IMAGE=${1:?usage: smoke-test.sh IMAGE}
CLI=${CONTAINER_CLI:-docker}
PORT=${SMOKE_PORT:-7125}
PYTHON=${PYTHON:-python3}
NAME=klipper-smoke-test-$$
URL=http://localhost:$PORT
LOG=/printer/printer_data/logs/klippy.log
CONFIG=/printer/printer_data/config/printer.cfg

step() { echo "==> $*"; }
fail() { echo "FAILED: $*" >&2; exit 1; }

cleanup() {
    local rc=$?
    trap - EXIT
    if [ "$rc" -ne 0 ]; then
        echo "--- container log" >&2
        "$CLI" logs "$NAME" 2>&1 | tail -n 50 >&2 || true
        echo "--- klippy.log" >&2
        "$CLI" exec "$NAME" sh -c "tail -n 100 $LOG" >&2 || true
    fi
    "$CLI" container remove -f "$NAME" >/dev/null 2>&1 || true
    exit "$rc"
}

# Evaluate a python expression on the "result" of a Moonraker response
result() {
    "$PYTHON" -c '
import json, sys
r = json.load(sys.stdin)["result"]
print(eval(sys.argv[1]))' "$1"
}

state() {
    curl -fsS -m 5 "$URL/printer/info" 2>/dev/null | result 'r["state"]' 2>/dev/null
}

wait_ready() {
    local current=
    for _ in $(seq 1 90); do
        current=$(state || true)
        [ "$current" = ready ] && return 0
        [ "$current" = shutdown ] && break
        sleep 2
    done
    fail "printer is not ready (state: ${current:-unavailable})"
}

gcode() {
    step "$1"
    curl -fsS -m 300 -X POST -G --data-urlencode "script=$1" \
        "$URL/printer/gcode/script" >/dev/null || fail "gcode: $1"
}

# check DESCRIPTION OBJECTS EXPRESSION - the expression sees the status as "s"
check() {
    local value
    value=$(curl -fsS -m 10 "$URL/printer/objects/query?$2" \
        | result "(lambda s: $3)(r['status'])") || fail "$1: query failed"
    [ "$value" = True ] || fail "$1"
    echo "    ok: $1"
}

check_endstops() {
    gcode QUERY_ENDSTOPS
    check "endstops are $1" query_endstops=last_query \
        "all(v == $2 for v in s['query_endstops']['last_query'].values())"
}

use_kinematics() {
    step "Switching to $1"
    "$CLI" exec "$NAME" sh -c "sed -i 's|^\[include printer/kinematics-.*\.cfg\]|[include printer/$1]|' $CONFIG"
    curl -fsS -m 10 -X POST "$URL/printer/restart" >/dev/null
    sleep 5
    wait_ready
}

trap cleanup EXIT

step "Starting $IMAGE"
# shellcheck disable=SC2086
"$CLI" run -d --name "$NAME" -p "$PORT:7125" ${CONTAINER_RUN_ARGS:-} "$IMAGE" >/dev/null
wait_ready
check "main mcu is the virtual mcu" mcu=mcu_constants \
    "s['mcu']['mcu_constants']['MCU'] == 'virtual'"

step "Cartesian homing and moves"
check_endstops open 0
gcode G28
check "all axes are homed" toolhead=homed_axes "s['toolhead']['homed_axes'] == 'xyz'"
check_endstops triggered 1
gcode "G1 X50 Y50 Z10 F6000"
gcode M400
check "toolhead reached the target" toolhead=position \
    "[round(v, 3) for v in s['toolhead']['position'][:3]] == [50, 50, 10]"
check_endstops open 0

step "Heaters"
gcode "M109 S100"
check "extruder is hot" extruder=temperature "s['extruder']['temperature'] > 95"
gcode "M190 S30"
check "bed is warm" heater_bed=temperature "s['heater_bed']['temperature'] > 28"
gcode "M104 S0"
gcode "M140 S0"

step "Firmware restart"
curl -fsS -m 10 -X POST "$URL/printer/firmware_restart" >/dev/null
sleep 5
wait_ready
check "axes are no longer homed" toolhead=homed_axes "s['toolhead']['homed_axes'] == ''"

use_kinematics kinematics-cartesian-probe.cfg
gcode G28
gcode "G1 Z5 F600"
gcode PROBE
check "probe triggers at Z=0" probe=last_z_result "abs(s['probe']['last_z_result']) < 0.05"

use_kinematics kinematics-delta.cfg
gcode G28
check "delta homes to the top" toolhead=position \
    "round(s['toolhead']['position'][2], 3) == 250"

step "Checking klippy.log"
if "$CLI" exec "$NAME" sh -c "grep -E 'Transition to shutdown|Timer too close|Stepper too far' $LOG"; then
    fail "klippy.log reports an mcu problem"
fi

step "Smoke test passed"
