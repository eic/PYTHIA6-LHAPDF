#!/bin/bash
# Full chain for one input file, inside the eic_xl container:
#   name_N.txt  -> BuildTree        -> name_N.root, name_N_1.root, ...
#               -> hadd             -> name_runN.root  (other names -> name_merged.root)
#               -> TreeToHepMC      -> name_runN.hepmc
#               -> abconv           -> name_runN_ab.hepmc  (afterburner, preset $AB_PRESET)
#               -> hepmc3ascii2root -> name_runN_ab.hepmc3.tree.root
# Given a .root or .hepmc file instead, the chain starts at that step.
# Intermediate files the chain creates are deleted once the next step
# succeeds, leaving only the _ab.hepmc3.tree.root (set KEEP=1 to keep them).
# Usage: AB_PRESET=<preset> run_chain.sh <input.txt|input.root|input.hepmc> [output dir = ./output_eic-smear]
# Requires HEPMC2ROOT=<path to hepmc3ascii2root binary> (https://github.com/eic/hepmc3ascii2root)
#   e.g. AB_PRESET=ip6_ep_130x9 (ep), AB_PRESET=ip6_eAu_100x9 (eAg, eAu)
set -euo pipefail

: "${AB_PRESET:?set AB_PRESET to the afterburner preset, e.g. ip6_ep_130x9}"
: "${HEPMC2ROOT:?set HEPMC2ROOT to the hepmc3ascii2root binary}"

INPUT=$(realpath -e "$1")
SCRIPTS_DIR=$(dirname "$(realpath -e "$0")")
mkdir -p "${2:-output_eic-smear}"
OUTDIR=$(realpath -e "${2:-output_eic-smear}")
IMAGE=${IMAGE:-/cvmfs/singularity.opensciencegrid.org/eicweb/eic_xl:nightly}
RUN="singularity exec -B /gpfs,/gpfs02,/cvmfs $IMAGE"

cleanup() { [ -n "${KEEP:-}" ] || rm -f "$@"; }

case "$INPUT" in
  *.txt)
    STEM=$(basename "$INPUT" .txt)
    $RUN root -l -b -q -e "gSystem->Load(\"libeicsmear\"); BuildTree(\"${INPUT}\", \"${OUTDIR}\", -1, \"${OUTDIR}/${STEM}.log\");"

    if [[ $STEM =~ ^(.*)_([0-9]+)$ ]]; then
      MERGED=${OUTDIR}/${BASH_REMATCH[1]}_run${BASH_REMATCH[2]}.root
    else
      MERGED=${OUTDIR}/${STEM}_merged.root
    fi
    PARTS=("${OUTDIR}/${STEM}.root")
    mapfile -t -O 1 PARTS < <(ls -v "${OUTDIR}/${STEM}"_[0-9]*.root 2>/dev/null)
    $RUN hadd -f "$MERGED" "${PARTS[@]}"
    cleanup "${PARTS[@]}"
    ;;
  *.root)
    MERGED=$INPUT
    ;;
  *.hepmc)
    HEPMC=$INPUT
    ;;
  *)
    echo "Unsupported input: $INPUT (expected .txt, .root or .hepmc)" >&2
    exit 1
    ;;
esac

if [ -z "${HEPMC:-}" ]; then
  $RUN root -l -b -q -e "gSystem->Load(\"libeicsmear\"); TreeToHepMC(\"${MERGED}\", \"${OUTDIR}\");"
  HEPMC=${OUTDIR}/$(basename "$MERGED" .root).hepmc
  [ "$MERGED" = "$INPUT" ] || cleanup "$MERGED"
fi

AB=${OUTDIR}/$(basename "$HEPMC" .hepmc)_ab
$RUN abconv -p "$AB_PRESET" -f hepmc3 -o "$AB" "$HEPMC"
[ "$HEPMC" = "$INPUT" ] || cleanup "$HEPMC"

$RUN "$(realpath -e "$HEPMC2ROOT")" "${AB}.hepmc" "${AB}.hepmc3.tree.root"
cleanup "${AB}.hepmc" "${AB}.hist.root"
