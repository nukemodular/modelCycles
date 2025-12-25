#!/usr/bin/env bash
set -euo pipefail

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

# Make the JUCE submodule read-only to prevent accidental edits.
chmod -R a-w "$root/JUCE"

echo "JUCE is now read-only: $root/JUCE"
