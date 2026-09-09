#!/usr/bin/env bash
set -Eeuo pipefail

lab_root="${NODE_RUNTIME_LAB_HOME:-/data/node-runtime-lab}"
sources_dir="${NODE_RUNTIME_SOURCES:-${lab_root}/sources}"
build_dir="${NODE_RUNTIME_BUILD:-${lab_root}/build}/libuv-debug"

if [[ ! -x "${build_dir}/uv_run_tests" ]]; then
  echo "libuv test binary not found: ${build_dir}/uv_run_tests" >&2
  echo "Build libuv first; see docs/ENVIRONMENT.md." >&2
  exit 1
fi

if [[ ${EUID} -eq 0 ]]; then
  if ! id node-lab >/dev/null 2>&1; then
    echo "node-lab user is missing; run scripts/bootstrap-rocky9.sh first." >&2
    exit 1
  fi

  chown -R node-lab:node-lab "${sources_dir}/libuv" "${build_dir}"
  exec runuser -u node-lab -- \
    ctest --test-dir "${build_dir}" --output-on-failure
fi

exec ctest --test-dir "${build_dir}" --output-on-failure
