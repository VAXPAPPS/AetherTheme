#!/bin/bash
set -e

# Package details
APP_NAME="aethertheme"
VERSION="0.1.0"
ARCH="amd64"
PKG_DIR="${APP_NAME}_${VERSION}_${ARCH}"

echo "Cleaning previous builds..."
rm -rf build
rm -rf "$PKG_DIR"
rm -f "${PKG_DIR}.deb"

echo "Configuring and building..."
meson setup build --prefix=/usr
ninja -C build

echo "Installing to temporary directory..."
DESTDIR="$(pwd)/$PKG_DIR" ninja -C build install

echo "Creating DEBIAN control file..."
mkdir -p "$PKG_DIR/DEBIAN"
cat <<EOF > "$PKG_DIR/DEBIAN/control"
Package: $APP_NAME
Version: $VERSION
Section: utils
Priority: optional
Architecture: $ARCH
Maintainer: Aether <aether@example.com>
Description: AetherTheme Manager
 A custom GTK3 theme manager for Aether Desktop.
EOF

echo "Building Debian package..."
dpkg-deb --build "$PKG_DIR"

echo "Package successfully created: ${PKG_DIR}.deb"
