#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${ROOT_DIR}/build"
KERNEL_BIN="${BUILD_DIR}/mykernel.bin"
LOG_DIR="${ROOT_DIR}/build"
LOG_FILE="${LOG_DIR}/qemu_test.log"

mkdir -p "${BUILD_DIR}"

timeout_cmd="timeout"
if ! command -v timeout >/dev/null 2>&1; then
  timeout_cmd=""
fi

if [[ ! -f "${KERNEL_BIN}" ]]; then
  echo "kernel binary not found: ${KERNEL_BIN}" >&2
  exit 1
fi

rm -f "${LOG_FILE}"

if [[ -n "${timeout_cmd}" ]]; then
  set +e
  "${timeout_cmd}" 3s qemu-system-i386 -nographic -serial mon:stdio -no-reboot -kernel "${KERNEL_BIN}" \
    2>&1 | tee "${LOG_FILE}" >/dev/null
  status=$?
  set -e
  if [[ "${status}" -ne 0 && "${status}" -ne 124 ]]; then
    exit "${status}"
  fi
else
  qemu-system-i386 -nographic -serial mon:stdio -no-reboot -kernel "${KERNEL_BIN}" \
    2>&1 | tee "${LOG_FILE}" >/dev/null &
  qemu_pid=$!
  sleep 3
  kill "${qemu_pid}" >/dev/null 2>&1 || true
fi

grep -q "Kernel basladi" "${LOG_FILE}"
grep -q "Paging enabled" "${LOG_FILE}"

echo "QEMU boot smoke test: OK"
