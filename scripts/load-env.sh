#!/bin/bash
# Sets up PATH / LD_LIBRARY_PATH / LHAPDF_DATA_PATH for the LHAPDF install
# built into this repo's install-lhapdf/ directory, plus the bundled
# pdfsets/ folder. No manual editing required - paths are derived from
# this script's own location.
#
# Usage (from inside eic-shell, at the repo root):
#   source scripts/load-env.sh

# Resolve this script's real path (works even if sourced via a relative
# path, and avoids relying on `pwd`, which can behave inconsistently
# across automounted filesystems on SDCC).
SCRIPT_DIR="$(cd "$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")" && pwd)"
REPO_ROOT="$(dirname "$SCRIPT_DIR")"

export LHAPDF="$REPO_ROOT/install-lhapdf"
export LD_LIBRARY_PATH="$LHAPDF/lib:$LD_LIBRARY_PATH"
export LHAPDF_DATA_PATH="$LHAPDF/share/LHAPDF:$REPO_ROOT/pdfsets"
export PATH="$LHAPDF/bin:$PATH"

echo "LHAPDF environment loaded from: $LHAPDF"

#Set path to cernlib
export CERN_ROOT=/cvmfs/eic.opensciencegrid.org/gcc-8.3/opt/fun4all/core/cern64/pro
echo "CERNLIB enenvironment loaded from: $CERN_ROOT"
