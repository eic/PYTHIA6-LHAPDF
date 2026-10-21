#!/bin/bash
# Generate PYTHIA6 runs from a steering card, the way the published samples
# were made: one card copy per run, identical except for the output file
# name on line 1. Each run gets a fresh seed from /dev/urandom, printed as
# "SEED =" in its log.
#
# Usage (inside eic-shell, after `source scripts/load-env.sh`):
#   run_generation.sh <card> <run> [<run> ...]
#   e.g. run_generation.sh STEERING/card_ep_9x130 0 1 2 3 4 5 6 7 8 9
#
# Card line 1 names the output, e.g. "ep_9x130_0.txt"; run N writes
# <stem>_N.txt and logs/<stem>_N.log in the current directory.
#   PYTHIA6_EIC=<path>  generator binary (default: this repo's build)
set -euo pipefail

if [ $# -lt 2 ]; then
  echo "Usage: $0 <card> <run> [<run> ...]"
  exit 1
fi
: "${LHAPDF:?source scripts/load-env.sh first}"

REPO_ROOT=$(dirname "$(dirname "$(realpath -e "$0")")")
PYTHIA6_EIC=${PYTHIA6_EIC:-${REPO_ROOT}/PYTHIA-RAD-CORR/build/pythia6-eic}
CARD=$(realpath -e "$1")
shift

NAME=$(awk 'NR==1 {print $1}' "$CARD")
STEM=${NAME%_[0-9]*.txt}
mkdir -p cards logs

for RUN in "$@"; do
  RUNCARD=cards/card_${STEM}_${RUN}
  sed "1s/^[^ ]*/${STEM}_${RUN}.txt/" "$CARD" > "$RUNCARD"
  echo "run ${RUN}: ${RUNCARD} -> ${STEM}_${RUN}.txt"
  "$PYTHIA6_EIC" < "$RUNCARD" > "logs/${STEM}_${RUN}.log"
  grep -h "SEED =" "logs/${STEM}_${RUN}.log"
done
