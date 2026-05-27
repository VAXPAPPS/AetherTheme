#!/usr/bin/env bash
# ==============================================================================
#  AetherTheme — Professional Debian Packaging Script
#  Version: 1.0.0
#  Description: Builds AetherTheme from source using Meson and packages it
#               into a standards-compliant .deb file.
# ==============================================================================

set -euo pipefail

# ──────────────────────────────────────────────
#  Color output helpers
# ──────────────────────────────────────────────
RESET="\033[0m"
BOLD="\033[1m"
RED="\033[1;31m"
GREEN="\033[1;32m"
YELLOW="\033[1;33m"
CYAN="\033[1;36m"
BLUE="\033[1;34m"

info()    { echo -e "${CYAN}${BOLD}[INFO]${RESET}  $*"; }
success() { echo -e "${GREEN}${BOLD}[OK]${RESET}    $*"; }
warn()    { echo -e "${YELLOW}${BOLD}[WARN]${RESET}  $*"; }
error()   { echo -e "${RED}${BOLD}[ERROR]${RESET} $*" >&2; }
step()    { echo -e "\n${BLUE}${BOLD}━━━  $*  ━━━${RESET}"; }

# ──────────────────────────────────────────────
#  Project metadata  (single source of truth)
# ──────────────────────────────────────────────
PKG_NAME="aethertheme"
APP_NAME="AetherTheme"
VERSION="0.1.0"
ARCH="amd64"
MAINTAINER="Venom Project <venom@example.com>"
DESCRIPTION="GTK3 Theme Manager — Customizes GTK3 look and feel settings"
LONG_DESCRIPTION="AetherTheme is a GTK3 theme manager that integrates with the\n Venom desktop environment. It reads color settings from\n ~/.config/venom/settings.vaxp and applies them in real-time\n without requiring an application restart."
SECTION="gnome"
PRIORITY="optional"
HOMEPAGE="https://github.com/VAXPAPPS/AetherTheme"

# Runtime dependencies
DEPENDS="libgtk-3-0 (>= 3.22.0), libglib2.0-0 (>= 2.50.0), libjson-glib-1.0-0, libxcursor1"

# ──────────────────────────────────────────────
#  Paths
# ──────────────────────────────────────────────
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"
DIST_DIR="${SCRIPT_DIR}/dist"
PKG_ROOT="${DIST_DIR}/${PKG_NAME}_${VERSION}_${ARCH}"
DEB_OUT="${DIST_DIR}/${PKG_NAME}_${VERSION}_${ARCH}.deb"

# Installation prefix inside the package staging root
PREFIX="/usr"
BIN_DIR="${PKG_ROOT}${PREFIX}/bin"
DATA_DIR="${PKG_ROOT}${PREFIX}/share/${APP_NAME}"
DESKTOP_DIR="${PKG_ROOT}${PREFIX}/share/applications"
PIXMAP_DIR="${PKG_ROOT}${PREFIX}/share/pixmaps"
ICON_DIR="${PKG_ROOT}${PREFIX}/share/icons/hicolor"
DOC_DIR="${PKG_ROOT}${PREFIX}/share/doc/${PKG_NAME}"
CONTROL_DIR="${PKG_ROOT}/DEBIAN"

# ──────────────────────────────────────────────
#  Banner
# ──────────────────────────────────────────────
print_banner() {
    echo -e "${BLUE}${BOLD}"
    echo "  ╔══════════════════════════════════════════════════════╗"
    echo "  ║          AetherTheme  —  Packaging Script            ║"
    echo "  ║                    v${VERSION}                             ║"
    echo "  ╚══════════════════════════════════════════════════════╝"
    echo -e "${RESET}"
}

# ──────────────────────────────────────────────
#  Dependency check
# ──────────────────────────────────────────────
check_dependencies() {
    step "Checking build dependencies"

    local missing=()
    local tools=(meson ninja-build gcc pkg-config dpkg-deb fakeroot)

    for tool in "${tools[@]}"; do
        if ! command -v "${tool}" &>/dev/null; then
            # ninja-build ships the binary as 'ninja'
            if [[ "${tool}" == "ninja-build" ]] && command -v ninja &>/dev/null; then
                continue
            fi
            missing+=("${tool}")
        fi
    done

    # Check pkg-config libraries
    local libs=(gtk+-3.0 glib-2.0 gio-2.0 gio-unix-2.0 json-glib-1.0 xcursor)
    for lib in "${libs[@]}"; do
        if ! pkg-config --exists "${lib}" 2>/dev/null; then
            missing+=("lib:${lib}")
        fi
    done

    if [[ ${#missing[@]} -gt 0 ]]; then
        error "Missing required build tools / libraries:"
        for m in "${missing[@]}"; do
            echo -e "    ${RED}✗${RESET}  ${m}"
        done
        echo
        warn "Install missing packages with:"
        echo -e "    sudo apt install meson ninja-build gcc pkg-config fakeroot \\"
        echo -e "        libgtk-3-dev libglib2.0-dev libgirepository1.0-dev \\"
        echo -e "        libjson-glib-dev libxcursor-dev"
        exit 1
    fi

    success "All build dependencies satisfied"
}

# ──────────────────────────────────────────────
#  Build with Meson
# ──────────────────────────────────────────────
build_project() {
    step "Building ${APP_NAME} with Meson"

    # Configure
    if [[ -d "${BUILD_DIR}" ]]; then
        info "Existing build directory found — reconfiguring…"
        meson setup --reconfigure \
            --prefix "${PREFIX}" \
            "${BUILD_DIR}" \
            2>&1 | sed 's/^/  /'
    else
        info "Configuring fresh build…"
        meson setup \
            --prefix "${PREFIX}" \
            "${BUILD_DIR}" \
            2>&1 | sed 's/^/  /'
    fi

    # Compile
    info "Compiling sources…"
    meson compile -C "${BUILD_DIR}" 2>&1 | sed 's/^/  /'

    success "Build complete → ${BUILD_DIR}/${APP_NAME}"
}

# ──────────────────────────────────────────────
#  Stage the package tree
# ──────────────────────────────────────────────
stage_package() {
    step "Staging package filesystem"

    # Clean previous staging
    rm -rf "${PKG_ROOT}"

    # Create directory tree
    mkdir -p \
        "${BIN_DIR}" \
        "${DATA_DIR}" \
        "${DESKTOP_DIR}" \
        "${PIXMAP_DIR}" \
        "${DOC_DIR}" \
        "${CONTROL_DIR}" \
        "${ICON_DIR}/16x16/apps" \
        "${ICON_DIR}/32x32/apps" \
        "${ICON_DIR}/48x48/apps" \
        "${ICON_DIR}/64x64/apps" \
        "${ICON_DIR}/128x128/apps" \
        "${ICON_DIR}/scalable/apps"

    # ── Binary ──────────────────────────────────
    info "Installing binary…"
    install -m 755 "${BUILD_DIR}/${APP_NAME}" "${BIN_DIR}/${APP_NAME}"

    # ── Data files ──────────────────────────────
    info "Installing data files…"
    install -m 644 "${SCRIPT_DIR}/stuff/main.glade" "${DATA_DIR}/main.glade"

    # ── Desktop entry ───────────────────────────
    info "Installing desktop entry…"
    install -m 644 "${SCRIPT_DIR}/stuff/${APP_NAME}.desktop" \
        "${DESKTOP_DIR}/${APP_NAME}.desktop"

    # ── SVG icon (scalable + legacy pixmap) ─────
    info "Installing icons…"
    install -m 644 "${SCRIPT_DIR}/stuff/${APP_NAME}.svg" \
        "${ICON_DIR}/scalable/apps/${APP_NAME}.svg"
    install -m 644 "${SCRIPT_DIR}/stuff/${APP_NAME}.svg" \
        "${PIXMAP_DIR}/${APP_NAME}.svg"

    # Rasterise PNG icons if rsvg-convert or inkscape is available
    if command -v rsvg-convert &>/dev/null; then
        for size in 16 32 48 64 128; do
            rsvg-convert -w "${size}" -h "${size}" \
                "${SCRIPT_DIR}/stuff/${APP_NAME}.svg" \
                -o "${ICON_DIR}/${size}x${size}/apps/${APP_NAME}.png" 2>/dev/null \
                && info "  PNG ${size}×${size} generated" \
                || warn "  PNG ${size}×${size} failed (skipped)"
        done
    elif command -v inkscape &>/dev/null; then
        for size in 16 32 48 64 128; do
            inkscape --export-type=png \
                --export-filename="${ICON_DIR}/${size}x${size}/apps/${APP_NAME}.png" \
                -w "${size}" -h "${size}" \
                "${SCRIPT_DIR}/stuff/${APP_NAME}.svg" &>/dev/null \
                && info "  PNG ${size}×${size} generated" \
                || warn "  PNG ${size}×${size} failed (skipped)"
        done
    else
        warn "rsvg-convert / inkscape not found — PNG icon sizes skipped (SVG only)"
    fi

    # ── Docs ────────────────────────────────────
    info "Installing documentation…"
    cat > "${DOC_DIR}/copyright" <<EOF
Format: https://www.debian.org/doc/packaging-manuals/copyright-format/1.0/
Upstream-Name: ${APP_NAME}
Upstream-Contact: ${MAINTAINER}
Source: ${HOMEPAGE}

Files: *
Copyright: $(date +%Y) Venom Project
License: GPL-3.0+

License: GPL-3.0+
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 .
 The full text is available at /usr/share/common-licenses/GPL-3.
EOF

    # Changelog (gzip-compressed, as required by policy)
    {
        echo "${PKG_NAME} (${VERSION}) unstable; urgency=medium"
        echo
        echo "  * Initial release."
        echo
        echo " -- ${MAINTAINER}  $(date -R)"
    } | gzip -9 -c > "${DOC_DIR}/changelog.gz"

    success "Staging complete → ${PKG_ROOT}"
}

# ──────────────────────────────────────────────
#  Write DEBIAN/control
# ──────────────────────────────────────────────
write_control() {
    step "Writing DEBIAN/control"

    # Compute installed size (in KiB)
    local installed_size
    installed_size=$(du -sk "${PKG_ROOT}" | awk '{print $1}')

    cat > "${CONTROL_DIR}/control" <<EOF
Package: ${PKG_NAME}
Version: ${VERSION}
Architecture: ${ARCH}
Maintainer: ${MAINTAINER}
Installed-Size: ${installed_size}
Depends: ${DEPENDS}
Section: ${SECTION}
Priority: ${PRIORITY}
Homepage: ${HOMEPAGE}
Description: ${DESCRIPTION}
 ${LONG_DESCRIPTION}
EOF

    success "DEBIAN/control written"
}

# ──────────────────────────────────────────────
#  Write DEBIAN/postinst & postrm  (icon cache)
# ──────────────────────────────────────────────
write_maintainer_scripts() {
    step "Writing maintainer scripts"

    cat > "${CONTROL_DIR}/postinst" <<'EOF'
#!/bin/sh
set -e
if command -v gtk-update-icon-cache >/dev/null 2>&1; then
    gtk-update-icon-cache -f -t /usr/share/icons/hicolor || true
fi
if command -v update-desktop-database >/dev/null 2>&1; then
    update-desktop-database /usr/share/applications || true
fi
EOF
    chmod 0755 "${CONTROL_DIR}/postinst"

    cat > "${CONTROL_DIR}/postrm" <<'EOF'
#!/bin/sh
set -e
if command -v gtk-update-icon-cache >/dev/null 2>&1; then
    gtk-update-icon-cache -f -t /usr/share/icons/hicolor || true
fi
if command -v update-desktop-database >/dev/null 2>&1; then
    update-desktop-database /usr/share/applications || true
fi
EOF
    chmod 0755 "${CONTROL_DIR}/postrm"

    success "Maintainer scripts written"
}

# ──────────────────────────────────────────────
#  Fix permissions & build .deb
# ──────────────────────────────────────────────
build_deb() {
    step "Building .deb package"

    # Enforce standard Debian filesystem permissions
    find "${PKG_ROOT}" -type d -exec chmod 755 {} \;
    find "${PKG_ROOT}" -type f ! -name "postinst" ! -name "postrm" -exec chmod 644 {} \;
    chmod 755 "${BIN_DIR}/${APP_NAME}"
    chmod 755 "${CONTROL_DIR}/postinst" "${CONTROL_DIR}/postrm"

    # Create output directory
    mkdir -p "${DIST_DIR}"

    info "Running dpkg-deb…"
    fakeroot dpkg-deb --build "${PKG_ROOT}" "${DEB_OUT}"

    success "Package created → ${DEB_OUT}"
}

# ──────────────────────────────────────────────
#  Package validation
# ──────────────────────────────────────────────
validate_deb() {
    step "Validating package"

    echo -e "\n${BOLD}Package info:${RESET}"
    dpkg-deb --info "${DEB_OUT}" | sed 's/^/  /'

    echo -e "\n${BOLD}Package contents:${RESET}"
    dpkg-deb --contents "${DEB_OUT}" | awk '{print "  " $0}'

    if command -v lintian &>/dev/null; then
        echo
        info "Running lintian checks…"
        lintian --no-tag-display-limit "${DEB_OUT}" 2>&1 | sed 's/^/  /' || true
    else
        warn "lintian not installed — skipping lint checks (sudo apt install lintian)"
    fi

    local SIZE
    SIZE=$(du -sh "${DEB_OUT}" | cut -f1)
    success "Validation done  [Package size: ${SIZE}]"
}

# ──────────────────────────────────────────────
#  Usage / Help
# ──────────────────────────────────────────────
print_usage() {
    echo -e "
${BOLD}Usage:${RESET}  $0 [COMMAND]

${BOLD}Commands:${RESET}
  ${GREEN}build${RESET}      Compile the project only (no packaging)
  ${GREEN}package${RESET}    Build + create .deb (default)
  ${GREEN}clean${RESET}      Remove build/ and dist/ directories
  ${GREEN}install${RESET}    Install the generated .deb with apt
  ${GREEN}uninstall${RESET}  Remove the installed package
  ${GREEN}help${RESET}       Show this message

${BOLD}Examples:${RESET}
  $0               # full build + package
  $0 build         # compile only
  $0 clean         # clean all artifacts
  $0 install       # install produced .deb
"
}

# ──────────────────────────────────────────────
#  Individual actions
# ──────────────────────────────────────────────
cmd_clean() {
    step "Cleaning build artifacts"
    rm -rf "${BUILD_DIR}" "${DIST_DIR}"
    success "Clean complete"
}

cmd_install() {
    if [[ ! -f "${DEB_OUT}" ]]; then
        error "Package not found: ${DEB_OUT}"
        info  "Run '$0 package' first."
        exit 1
    fi
    step "Installing ${DEB_OUT}"
    sudo apt install --reinstall "${DEB_OUT}"
    success "Installed successfully"
}

cmd_uninstall() {
    step "Removing ${PKG_NAME}"
    sudo apt remove "${PKG_NAME}"
    success "Package removed"
}

cmd_build() {
    print_banner
    check_dependencies
    build_project
    echo -e "\n${GREEN}${BOLD}✔  Build finished.${RESET}\n"
}

cmd_package() {
    print_banner
    check_dependencies
    build_project
    stage_package
    write_control
    write_maintainer_scripts
    build_deb
    validate_deb

    echo -e "\n${GREEN}${BOLD}╔══════════════════════════════════════════════╗"
    echo    "║  ✔  Package ready!                           ║"
    printf  "║     %-44s ║\n" "${DEB_OUT}"
    echo -e "╚══════════════════════════════════════════════╝${RESET}\n"
    echo -e "  Install with:  ${BOLD}sudo apt install ./${DEB_OUT##*/}${RESET}"
    echo -e "             or: ${BOLD}$0 install${RESET}\n"
}

# ──────────────────────────────────────────────
#  Entry point
# ──────────────────────────────────────────────
main() {
    local cmd="${1:-package}"
    cd "${SCRIPT_DIR}"

    case "${cmd}" in
        build)     cmd_build     ;;
        package)   cmd_package   ;;
        clean)     cmd_clean     ;;
        install)   cmd_install   ;;
        uninstall) cmd_uninstall ;;
        help|-h|--help) print_usage ;;
        *)
            error "Unknown command: '${cmd}'"
            print_usage
            exit 1
            ;;
    esac
}

main "$@"
