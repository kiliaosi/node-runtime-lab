#!/usr/bin/env bash
set -u

failures=0
warnings=0

pass() { printf '[PASS] %s\n' "$1"; }
warn() { printf '[WARN] %s\n' "$1"; warnings=$((warnings + 1)); }
fail() { printf '[FAIL] %s\n' "$1"; failures=$((failures + 1)); }

version_line() {
  "$1" --version 2>/dev/null | head -n 1
}

echo '== Platform =='
if [[ "$(uname -s)" == "Linux" ]]; then
  pass "Linux $(uname -r) $(uname -m)"
else
  fail "Linux is required for the main labs"
fi

if command -v getconf >/dev/null 2>&1; then
  printf '       %s\n' "$(getconf GNU_LIBC_VERSION 2>/dev/null || true)"
fi

echo
echo '== Required tools =='
required=(gcc g++ make cmake git python3 node npm gdb strace perf valgrind)
for tool in "${required[@]}"; do
  if command -v "${tool}" >/dev/null 2>&1; then
    pass "${tool}: $(command -v "${tool}")"
  else
    fail "${tool} is missing"
  fi
done

if command -v gcc >/dev/null 2>&1; then
  gcc_major="$(gcc -dumpfullversion -dumpversion | cut -d. -f1)"
  if [[ "${gcc_major}" =~ ^[0-9]+$ ]] && (( gcc_major >= 12 )); then
    pass "$(version_line gcc)"
  else
    fail "Node 24 source build needs GCC/G++ 12.2+; found $(version_line gcc)"
  fi
fi

for tool in clangd; do
  if command -v "${tool}" >/dev/null 2>&1; then
    pass "optional ${tool}: $(command -v "${tool}")"
  else
    warn "optional ${tool} is missing"
  fi
done

echo
echo '== Runtime baseline =='
if command -v node >/dev/null 2>&1; then
  node_version="$(node -p 'process.version')"
  uv_version="$(node -p 'process.versions.uv')"
  v8_version="$(node -p 'process.versions.v8')"
  printf '       Node %s\n' "${node_version}"
  printf '       libuv %s\n' "${uv_version}"
  printf '       V8 %s\n' "${v8_version}"

  [[ "${node_version}" == "v24.20.0" ]] && pass 'Node baseline matches' || warn "expected Node v24.20.0"
  [[ "${uv_version}" == "1.52.1" ]] && pass 'libuv baseline matches' || warn "expected libuv 1.52.1"
  [[ "${v8_version}" == 13.6.233.17-node.53 ]] && pass 'V8 baseline matches' || warn "expected V8 13.6.233.17-node.53"
fi

echo
echo '== Lab directories =='
lab_root="${NODE_RUNTIME_LAB_HOME:-/data/node-runtime-lab}"
for dir in sources build traces tools; do
  path="${lab_root}/${dir}"
  if [[ -d "${path}" && -w "${path}" ]]; then
    pass "writable: ${path}"
  else
    fail "missing or not writable: ${path}"
  fi
done

if id node-lab >/dev/null 2>&1; then
  pass 'unprivileged libuv test user: node-lab'
else
  fail 'node-lab user is missing; libuv tests must not run as root'
fi

echo
printf 'Open-file limit: %s\n' "$(ulimit -n)"
if [[ -r /proc/sys/kernel/perf_event_paranoid ]]; then
  printf 'perf_event_paranoid: %s\n' "$(< /proc/sys/kernel/perf_event_paranoid)"
fi

echo
printf 'Summary: %d failure(s), %d warning(s)\n' "${failures}" "${warnings}"
(( failures == 0 ))
