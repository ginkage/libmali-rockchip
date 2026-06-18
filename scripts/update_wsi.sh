#!/bin/sh -e
#
# update_wsi.sh - refresh the bundled Vulkan WSI layer (64-bit) from a prebuilt
# vulkan-wsi-layer release artifact.
#
# Downloads (or takes a local) libvulkan-wsi-layer .deb, extracts the layer .so
# and its manifest, and drops them where meson.build installs them from:
#
#   data/vulkan/lib/aarch64/libVkLayer_window_system_integration.so  (the layer .so)
#   data/vulkan/VkLayer_window_system_integration.json.in            (manifest template)
#
# The manifest's "library_path" is rewritten to @LIB@ so meson's existing sed step
# turns it into the bare soname. libmali installs the .so into libdir (resolved via
# ldconfig), not next to the manifest, so the artifact's relative "./..." path would
# not resolve.
#
# Note: this only refreshes the 64-bit (arm64) layer; the 32-bit blob under
# data/vulkan/lib/arm/ is left untouched. The .json.in is shared by both arches.
#
# Usage:
#   ./scripts/update_wsi.sh                 # pinned release below
#   ./scripts/update_wsi.sh <url|deb-path>  # override source
#   WSI_DEB_URL=<url> ./scripts/update_wsi.sh
#   WSI_DEB_SHA256=<hex> ./scripts/update_wsi.sh   # optional integrity check

set -u

# Pinned source artifact (override with $WSI_DEB_URL or the first argument).
DEFAULT_WSI_DEB_URL="https://github.com/ginkage/vulkan-wsi-layer/releases/download/v1.3.276-1-3636ca6/libvulkan-wsi-layer_1.3.276-1_arm64.deb"

SRC="${1:-${WSI_DEB_URL:-$DEFAULT_WSI_DEB_URL}}"
WSI_DEB_SHA256="${WSI_DEB_SHA256:-}"

# Resolve repo root from this script's location, so CWD does not matter.
SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
REPO_ROOT=$(cd -- "$SCRIPT_DIR/.." && pwd)

ARCH_DIR="$REPO_ROOT/data/vulkan/lib/aarch64"
DEST_SO="$ARCH_DIR/libVkLayer_window_system_integration.so"
DEST_JSON_IN="$REPO_ROOT/data/vulkan/VkLayer_window_system_integration.json.in"

WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT INT TERM
DEB="$WORK/wsi.deb"

download() {
	if command -v curl >/dev/null 2>&1; then
		curl -fSL --retry 3 -o "$2" "$1"
	elif command -v wget >/dev/null 2>&1; then
		wget -O "$2" "$1"
	else
		echo "ERROR: need curl or wget to download $1" >&2
		exit 1
	fi
}

# Obtain the .deb (local file or download).
if [ -f "$SRC" ]; then
	echo "Using local artifact: $SRC"
	cp -- "$SRC" "$DEB"
else
	echo "Downloading: $SRC"
	download "$SRC" "$DEB"
fi

# Optional integrity check.
if [ -n "$WSI_DEB_SHA256" ]; then
	echo "$WSI_DEB_SHA256  $DEB" | sha256sum -c -
fi

# Extract the package payload.
dpkg-deb -x "$DEB" "$WORK/root"

# Locate the layer .so and manifest within the payload (path-independent).
SO=$(find "$WORK/root" -type f -name 'libVkLayer_window_system_integration.so' | head -1)
JSON=$(find "$WORK/root" -type f -name 'VkLayer_window_system_integration.json' | head -1)

[ -n "$SO" ]   || { echo "ERROR: layer .so not found in $SRC" >&2; exit 1; }
[ -n "$JSON" ] || { echo "ERROR: manifest .json not found in $SRC" >&2; exit 1; }

# Install the layer .so (64-bit).
mkdir -p "$ARCH_DIR"
cp -- "$SO" "$DEST_SO"
chmod 0755 "$DEST_SO"

# Install the manifest as a .in template: rewrite "library_path" to @LIB@ so meson
# substitutes the bare soname at build time.
sed -E 's#("library_path"[[:space:]]*:[[:space:]]*)"[^"]*"#\1"@LIB@"#' \
	"$JSON" > "$DEST_JSON_IN"

grep -q '@LIB@' "$DEST_JSON_IN" || {
	echo "ERROR: library_path rewrite failed (no @LIB@ in manifest)" >&2
	exit 1
}

echo "Updated WSI layer from $(basename "$SRC"):"
echo "  $DEST_SO ($(stat -c %s "$DEST_SO") bytes)"
echo "  $DEST_JSON_IN"
