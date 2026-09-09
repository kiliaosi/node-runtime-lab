#!/usr/bin/env bash

if [[ "${BASH_SOURCE[0]}" == "$0" ]]; then
  echo "Please source this file: source scripts/activate-lab.sh" >&2
  exit 1
fi

if [[ -f /opt/rh/gcc-toolset-14/enable ]]; then
  # shellcheck disable=SC1091
  source /opt/rh/gcc-toolset-14/enable
fi

export NODE_RUNTIME_LAB_HOME="${NODE_RUNTIME_LAB_HOME:-/data/node-runtime-lab}"
export NODE_RUNTIME_SOURCES="${NODE_RUNTIME_SOURCES:-${NODE_RUNTIME_LAB_HOME}/sources}"
export NODE_RUNTIME_BUILD="${NODE_RUNTIME_BUILD:-${NODE_RUNTIME_LAB_HOME}/build}"
export NODE_RUNTIME_TRACES="${NODE_RUNTIME_TRACES:-${NODE_RUNTIME_LAB_HOME}/traces}"

if [[ -d /opt/node-v24.20.0-linux-x64/bin ]]; then
  export PATH="/opt/node-v24.20.0-linux-x64/bin:${PATH}"
fi

export CC="${CC:-gcc}"
export CXX="${CXX:-g++}"

echo "Node Runtime Lab environment activated"
echo "  sources: ${NODE_RUNTIME_SOURCES}"
echo "  build:   ${NODE_RUNTIME_BUILD}"
echo "  traces:  ${NODE_RUNTIME_TRACES}"
