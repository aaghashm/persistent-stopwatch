#!/bin/sh
# SPDX-FileCopyrightText: 2026 Aaghash Murugesan <aaghashm@gmail.com>
# SPDX-License-Identifier: CC0-1.0
#
# Installs the latest Persistent Stopwatch release with the system's
# package manager, so updates and removal work the usual way afterwards.
#
#   curl -fsSL https://aaghashm.github.io/persistent-stopwatch/install.sh | sh

set -eu

repo=https://github.com/aaghashm/persistent-stopwatch
base=${PERSISTENT_STOPWATCH_DOWNLOADS:-$repo/releases/latest/download}

say() {
    printf '%s\n' "$*"
}

die() {
    printf 'install.sh: %s\n' "$*" >&2
    exit 1
}

as_root() {
    if [ "$(id -u)" -eq 0 ]; then
        "$@"
    elif command -v sudo >/dev/null 2>&1; then
        sudo "$@"
    else
        die "need root for: $*"
    fi
}

if command -v dnf >/dev/null 2>&1; then
    arch=$(uname -m)
    say "Installing persistent-stopwatch ($arch RPM) with dnf"
    as_root dnf install -y "$base/persistent-stopwatch.$arch.rpm"
elif command -v apt-get >/dev/null 2>&1; then
    command -v curl >/dev/null 2>&1 || die "curl is needed to download the package"
    arch=$(dpkg --print-architecture)
    tmp=$(mktemp -d)
    trap 'rm -rf "$tmp"' EXIT
    # apt drops privileges to read local packages; let it into the directory.
    chmod 755 "$tmp"
    say "Downloading persistent-stopwatch ($arch .deb)"
    curl -fsSL -o "$tmp/persistent-stopwatch_$arch.deb" "$base/persistent-stopwatch_$arch.deb"
    as_root apt-get install -y "$tmp/persistent-stopwatch_$arch.deb"
elif command -v pacman >/dev/null 2>&1; then
    [ "$(id -u)" -ne 0 ] || die "makepkg refuses to run as root; run this as your normal user"
    command -v git >/dev/null 2>&1 || die "git is needed (pacman -S git base-devel)"
    command -v makepkg >/dev/null 2>&1 || die "makepkg is needed (pacman -S base-devel)"
    tmp=$(mktemp -d)
    trap 'rm -rf "$tmp"' EXIT
    say "Building persistent-stopwatch with makepkg"
    git clone --quiet --depth 1 "$repo" "$tmp/src"
    (cd "$tmp/src/packaging/arch" && makepkg -si --noconfirm)
else
    die "no dnf, apt-get or pacman found; see $repo#building-from-source"
fi

say "Done. Open it from the app menu, or try: persistent-stopwatch --help"
