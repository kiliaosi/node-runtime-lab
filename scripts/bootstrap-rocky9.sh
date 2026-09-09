#!/usr/bin/env bash
set -Eeuo pipefail

if [[ ${EUID} -ne 0 ]]; then
  echo "Run as root or with sudo: sudo bash scripts/bootstrap-rocky9.sh" >&2
  exit 1
fi

# shellcheck disable=SC1091
source /etc/os-release
if [[ "${ID_LIKE:-}" != *rhel* && "${ID:-}" != "rocky" ]]; then
  echo "This script targets Rocky/RHEL-family Linux. Found: ${PRETTY_NAME:-unknown}" >&2
  exit 1
fi

base_packages=(
  gcc-toolset-14-gcc \
  gcc-toolset-14-gcc-c++ \
  gcc-toolset-14-binutils \
  make \
  cmake \
  git \
  python3 \
  python3-pip \
  gdb \
  strace \
  perf \
  valgrind \
  pkgconf-pkg-config \
  autoconf \
  automake \
  libtool \
  openssl-devel \
  zlib-devel \
  curl \
  xz \
  tar \
  unzip \
  patch \
  which \
  file \
  procps-ng \
  rsync
)

dnf install -y --setopt=max_parallel_downloads=10 "${base_packages[@]}"

if [[ "${1:-}" == "--full" ]]; then
  # These tools are valuable later, but pull a much larger dependency set.
  dnf install -y --setopt=max_parallel_downloads=10 \
    clang \
    clang-tools-extra \
    llvm \
    lld
else
  echo "Skipping the large diagnostics/indexing toolset."
  echo "Install it later with: sudo bash scripts/bootstrap-rocky9.sh --full"
fi

lab_root="${NODE_RUNTIME_LAB_HOME:-/data/node-runtime-lab}"
lab_user="${NODE_RUNTIME_LAB_USER:-node-lab}"
if ! id "${lab_user}" >/dev/null 2>&1; then
  useradd --system --create-home --shell /bin/bash "${lab_user}"
fi

install -d -m 0755 -o "${lab_user}" -g "${lab_user}" \
  "${lab_root}/sources" \
  "${lab_root}/build" \
  "${lab_root}/traces" \
  "${lab_root}/tools"

echo
echo "Toolchain installed. Next:"
echo "  source scripts/activate-lab.sh"
echo "  bash scripts/check-env.sh"
echo "  libuv test user: ${lab_user}"
