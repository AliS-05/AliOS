#!/usr/bin/env bash
set -euo pipefail

tmp=$(mktemp)
trap 'rm -f "$tmp"' EXIT

find include -type f -name '*.h' | while read -r hdr; do
    rel="${hdr#include/}"
    base="$(basename "$hdr")"

    cat >> "$tmp" <<EOF
s@^[[:space:]]*#include[[:space:]]*[<"]${base}[>"]@#include <${rel}>@
EOF
done

find src include \
    -type f \( -name '*.c' -o -name '*.h' \) \
    -exec sed -Ei -f "$tmp" {} +
