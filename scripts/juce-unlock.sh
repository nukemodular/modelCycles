#!/usr/bin/env bash
set -euo pipefail

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

# Re-enable write permissions on JUCE so it can be updated.
chmod -R u+w "$root/JUCE"

echo "JUCE is now writable: $root/JUCE"
