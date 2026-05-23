#!/bin/bash

# If a command fails, make the whole script exit
set -e
set -o pipefail

# =========================================================
# 🔧 BUILD ENVIRONMENT FIX (IMPORTANT for dconf/dbus)
# =========================================================
export DEBIAN_FRONTEND=noninteractive
unset LD_PRELOAD
unset DBUS_SESSION_BUS_ADDRESS
unset DBUS_SYSTEM_BUS_ADDRESS

# =========================================================
# Kali's default values
# =========================================================
KALI_DIST="kali-rolling"
KALI_VERSION=""
KALI_VARIANT="default"
TARGET_DIR="$(dirname "$0")/images"   # FIX #7: quoted dirname
TARGET_SUBDIR=""
SUDO="sudo"
VERBOSE=""
DEBUG=""
HOST_ARCH=$(dpkg --print-architecture)

FORCE_PURGE=""   # FIX #4: use empty/non-empty instead of "0"/"1"

# =========================================================
# Trap for better error reporting
# FIX (suggestion): ERR trap shows the failing line number
# =========================================================
trap 'echo "ERROR: Build failed at line $LINENO — check $BUILD_LOG" >&2' ERR

# =========================================================
# Image naming
# FIX #1 & #2: image_name now uses its argument ($1),
#              falling back to $KALI_ARCH if not provided
# =========================================================
image_name() {
  local arch="${1:-$KALI_ARCH}"
  case "$arch" in
    i386|amd64|arm64)
      echo "live-image-$arch.hybrid.iso"
    ;;
    armhf)
      echo "live-image-$arch.img"
    ;;
    *)
      echo "ERROR: Unsupported architecture: $arch" >&2
      exit 1
    ;;
  esac
}

target_image_name() {
  local arch="$1"

  IMAGE_NAME="$(image_name "$arch")"
  IMAGE_EXT="${IMAGE_NAME##*.}"

  if [ "$IMAGE_EXT" = "$IMAGE_NAME" ]; then
    IMAGE_EXT="img"
  fi

  if [ "$KALI_VARIANT" = "default" ]; then
    echo "${TARGET_SUBDIR:+$TARGET_SUBDIR/}zyphor-os-$KALI_VERSION-live-$KALI_ARCH.$IMAGE_EXT"
  else
    echo "${TARGET_SUBDIR:+$TARGET_SUBDIR/}zyphor-os-$KALI_VERSION-live-$KALI_VARIANT-$KALI_ARCH.$IMAGE_EXT"
  fi
}

target_build_log() {
  TARGET_IMAGE_NAME=$(target_image_name "$1")
  echo "${TARGET_IMAGE_NAME%.*}.log"
}

default_version() {
  case "$1" in
    kali-*)
      echo "${1#kali-}"
    ;;
    *)
      echo "$1"
    ;;
  esac
}

failure() {
  echo "Build failed: $KALI_DIST/$KALI_VARIANT/$KALI_ARCH" >&2
  echo "See log: $BUILD_LOG" >&2
  exit 2
}

# =========================================================
# Logging wrapper
# =========================================================
run_and_log() {
  if [ -n "$VERBOSE" ] || [ -n "$DEBUG" ]; then
    "$@" 2>&1 | tee -a "$BUILD_LOG"
  else
    "$@" >>"$BUILD_LOG" 2>&1
  fi
}

debug() {
  if [ -n "$DEBUG" ]; then
    echo "DEBUG: $*" >&2
  fi
}

# =========================================================
# Clean function (SAFE + optional purge)
# =========================================================
clean() {
  debug "Cleaning build environment"

  # FIX #4: check non-empty rather than string "1"
  if [ -n "$FORCE_PURGE" ]; then
    run_and_log $SUDO lb clean --purge
  else
    run_and_log $SUDO lb clean
  fi
}

# =========================================================
# Help
# =========================================================
print_help() {
  echo "Usage: $0 [options]"
  echo "  -d | --distribution <dist>   Kali distribution (default: kali-rolling)"
  echo "  -a | --arch <arch>           Target architecture (default: host arch)"
  echo "  -v | --verbose               Verbose output"
  echo "  -D | --debug                 Debug output"
  echo "  -h | --help                  Show this help"
  echo "       --variant <variant>     Image variant (default: default)"
  echo "       --version <version>     Override image version string"
  echo "       --subdir <subdir>       Output subdirectory under images/"
  echo "       --clean                 Clean only, do not build"
  echo "       --no-clean              Skip clean stage before building"
  echo "       --purge                 Use lb clean --purge (implies clean)"
  echo "  -p | --proposed-updates      Enable proposed-updates"
  exit 0
}

# =========================================================
# Require package
# FIX #6: quoted $pkg variable
# =========================================================
require_package() {
  local pkg="$1"
  local required_version="$2"
  local pkg_version

  pkg_version=$(dpkg-query -f '${Version}' -W "$pkg" 2>/dev/null || true)

  if [ -z "$pkg_version" ]; then
    echo "ERROR: Missing package $pkg" >&2
    exit 1
  fi

  if dpkg --compare-versions "$pkg_version" lt "$required_version"; then
    echo "ERROR: $pkg version too old ($pkg_version < $required_version)" >&2
    exit 1
  fi
}

# =========================================================
# Setup
# =========================================================
cd "$(dirname "$0")/"

# FIX #5: check .getopt.sh exists before sourcing
if [ ! -f .getopt.sh ]; then
  echo "ERROR: .getopt.sh not found in $(pwd)" >&2
  exit 1
fi
source .getopt.sh

temp=$(getopt -o "$BUILD_OPTS_SHORT" -l "$BUILD_OPTS_LONG" -- "$@")
eval set -- "$temp"

while true; do
  case "$1" in
    -d|--distribution) KALI_DIST="$2"; shift 2 ;;
    -p|--proposed-updates) OPT_pu="1"; shift ;;
    -a|--arch) KALI_ARCH="$2"; shift 2 ;;
    -v|--verbose) VERBOSE="1"; shift ;;
    -D|--debug) DEBUG="1"; shift ;;
    -h|--help) print_help ;;
    --variant) KALI_VARIANT="$2"; shift 2 ;;
    --version) KALI_VERSION="$2"; shift 2 ;;
    --subdir) TARGET_SUBDIR="$2"; shift 2 ;;
    --clean) ACTION="clean"; shift ;;
    --no-clean) NO_CLEAN="1"; shift ;;
    --purge) FORCE_PURGE="1"; shift ;;   # sets non-empty
    --) shift; break ;;
    *) echo "ERROR: Invalid option: $1" >&2; exit 1 ;;
  esac
done

# FIX #4: warn if --purge is combined with --no-clean (purge would be silently ignored)
if [ -n "$FORCE_PURGE" ] && [ "$NO_CLEAN" = "1" ]; then
  echo "WARNING: --purge has no effect when combined with --no-clean" >&2
fi

# =========================================================
# Build log — FIX #3: initialized BEFORE clean() is called
# =========================================================
BUILD_LOG="$(pwd)/build.log"
: > "$BUILD_LOG"

# =========================================================
# Arch normalisation
# =========================================================
KALI_ARCH=${KALI_ARCH:-$HOST_ARCH}

if [ "$KALI_ARCH" = "x64" ]; then
  KALI_ARCH="amd64"
elif [ "$KALI_ARCH" = "x86" ]; then
  KALI_ARCH="i386"
fi

if [ -z "$KALI_VERSION" ]; then
  KALI_VERSION="$(default_version "$KALI_DIST")"
fi

# =========================================================
# Check OS
# =========================================================
export PATH="/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin"

require_package live-build "1:20250814+kali2"

# FIX #8: dbus check is now a hard requirement, not just a warning
require_package dbus-user-session "0"

# =========================================================
# Root check
# =========================================================
if [ "$(whoami)" != "root" ]; then
  if ! which $SUDO >/dev/null; then
    echo "ERROR: sudo not found" >&2
    exit 1
  fi
else
  SUDO=""
fi

# =========================================================
# Clean stage
# =========================================================
if [ "$NO_CLEAN" != "1" ]; then
  clean
fi

if [ "$ACTION" = "clean" ]; then
  echo "Clean complete."
  exit 0
fi

# =========================================================
# Output directory
# =========================================================
mkdir -pv "$TARGET_DIR/$TARGET_SUBDIR"

# =========================================================
# BUILD STAGE
# Note: remaining $@ args are forwarded to lb config
#       intentionally, to allow caller pass-through flags
# =========================================================
set -e
set -o pipefail

debug "Stage 1: config"
run_and_log lb config -a "$KALI_ARCH" --distribution "$KALI_DIST" -- --variant "$KALI_VARIANT" "$@"

debug "Stage 2: build"
run_and_log $SUDO lb build

IMAGE_NAME="$(image_name "$KALI_ARCH")"

if [ ! -e "$IMAGE_NAME" ]; then
  failure
fi

# =========================================================
# Move outputs
# =========================================================
run_and_log mv -f "$IMAGE_NAME" "$TARGET_DIR/$(target_image_name "$KALI_ARCH")"
run_and_log mv -f "$BUILD_LOG"  "$TARGET_DIR/$(target_build_log  "$KALI_ARCH")"

echo ""
echo "*** BUILD SUCCESSFUL ***"
readlink -f "$TARGET_DIR/$(target_image_name "$KALI_ARCH")"