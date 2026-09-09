#!/usr/bin/env bash
set -Eeuo pipefail

lab_root="${NODE_RUNTIME_LAB_HOME:-/data/node-runtime-lab}"
sources_dir="${NODE_RUNTIME_SOURCES:-${lab_root}/sources}"
component="${1:-libuv}"
mkdir -p "${sources_dir}"

clone_tag() {
  local name="$1"
  local url="$2"
  local tag="$3"
  local target="${sources_dir}/${name}"

  if [[ ! -d "${target}/.git" ]]; then
    echo "Cloning ${name} at ${tag}..."
    git clone --depth 1 --branch "${tag}" "${url}" "${target}"
  else
    echo "Refreshing ${name} at ${tag}..."
    git -C "${target}" fetch --depth 1 origin "refs/tags/${tag}:refs/tags/${tag}"
    git -C "${target}" checkout --detach "${tag}"
  fi

  printf '%-8s %s  %s\n' "${name}" "$(git -C "${target}" rev-parse --short HEAD)" "$(git -C "${target}" describe --tags --exact-match 2>/dev/null || true)"

  if [[ ${EUID} -eq 0 ]] && id node-lab >/dev/null 2>&1; then
    chown -R node-lab:node-lab "${target}"
  fi
}

case "${component}" in
  libuv)
    clone_tag libuv https://github.com/libuv/libuv.git v1.52.1
    ;;
  node)
    clone_tag node https://github.com/nodejs/node.git v24.20.0
    ;;
  all)
    clone_tag libuv https://github.com/libuv/libuv.git v1.52.1
    clone_tag node https://github.com/nodejs/node.git v24.20.0
    ;;
  *)
    echo "Usage: $0 [libuv|node|all]" >&2
    exit 2
    ;;
esac

cat <<EOF

Sources are ready under ${sources_dir}
Requested component: ${component}

Known locations:
  libuv API/source:  ${sources_dir}/libuv
  Node source:        ${sources_dir}/node
  Node-aligned V8:    ${sources_dir}/node/deps/v8

Standalone upstream V8 is intentionally fetched later with depot_tools.
See docs/ENVIRONMENT.md.
EOF
