#!/bin/bash
# Submit one BNL condor job per input file, each running run_chain.sh
# (.txt -> tree -> hadd -> HepMC -> afterburner -> hepmc3.tree.root;
#  .root or .hepmc start later).
# Usage: AB_PRESET=<preset> submit.sh <input.txt|.root|.hepmc> [...]
#   AB_PRESET=<name> afterburner preset (required): ip6_ep_130x9 for ep,
#                    ip6_eAu_100x9 for eAg and eAu
#   HEPMC2ROOT=<path> hepmc3ascii2root binary (required)
#   OUTDIR=<dir>  write outputs there instead of ./output_eic-smear
#   IMAGE=<path>  container image (default eic_xl:nightly)
set -euo pipefail

if [ $# -lt 1 ] || [ -z "${AB_PRESET:-}" ] || [ -z "${HEPMC2ROOT:-}" ]; then
  echo "Usage: AB_PRESET=<preset> HEPMC2ROOT=<binary> $0 <input.txt|.root|.hepmc> [...]"
  exit 1
fi

SCRIPTS_DIR=$(dirname "$(realpath -e "$0")")
OUTDIR=$(mkdir -p "${OUTDIR:-output_eic-smear}" && realpath -e "${OUTDIR:-output_eic-smear}")
LOGDIR=${PWD}/condor_logs
mkdir -p "$LOGDIR"

SUBMIT=${LOGDIR}/chain-$(date +%Y-%m-%dT%H%M%S).submit
cat > "$SUBMIT" <<EOF
getenv              = True
environment         = "AB_PRESET=${AB_PRESET} HEPMC2ROOT=$(realpath -e "$HEPMC2ROOT")"
universe            = vanilla
executable          = ${SCRIPTS_DIR}/run_chain.sh
transfer_executable = False
arguments           = \$(infile) ${OUTDIR}

error  = ${LOGDIR}/\$(Cluster).\$(Process).err
output = ${LOGDIR}/\$(Cluster).\$(Process).out
log    = ${LOGDIR}/\$(Cluster).log

request_cpus   = 1
request_memory = 4 GB

queue infile from (
$(for f in "$@"; do realpath -e "$f"; done)
)
EOF

condor_submit "$SUBMIT"
