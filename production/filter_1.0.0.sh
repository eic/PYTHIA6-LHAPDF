#!/bin/bash
# Make the 1.1.0 files of one 1.0.0 run: read the 1.0.0 file over XRootD and split it
# into the three x decades with filter_decades.C (see README.md, "1.1.0").
#
# Usage: filter_1.0.0.sh <dataset> <run> [outdir]
#   <dataset>: ep_noradcor/9x130, eAg_EPPS16_noradcor/9x115 or eAu_EPPS16_noradcor/9x100
#   <run>:     1 ... 10
#   e.g. filter_1.0.0.sh eAu_EPPS16_noradcor/9x100 7 /path/to/out
# Output: <outdir>/<dataset>/q2_1to10/<decade>/
#           pythia6-lhapdf_1.1.0_<dataset_>_q2_1to10_<decade>_run<NNN>.ab.hepmc3.tree.root
#         and <outdir>/<dataset>/q2_1to10/logs/run<NNN>.log (events kept per decade)
set -euo pipefail

if [ $# -lt 2 ]; then
  echo "Usage: $0 <dataset> <run> [outdir]"
  exit 1
fi
REF=$1
RUN=$(printf %03d "$2")
OUTDIR=${3:-./output_1.1.0}
HERE=$(dirname "$(realpath -e "$0")")
IMAGE=${IMAGE:-/cvmfs/singularity.opensciencegrid.org/eicweb/eic_xl:nightly}
XRD=${XRD:-root://dtn2304.jlab.org:8443//jlab-osdf-ro/eic/EPIC/volatile//EVGEN/SIDIS/pythia6-lhapdf}

case "$REF" in
  ep_noradcor/9x130|eAg_EPPS16_noradcor/9x115|eAu_EPPS16_noradcor/9x100) ;;
  *) echo "Unknown dataset $REF" >&2; exit 1 ;;
esac
NAME=${REF/\//_}_q2_1to10                        # e.g. ep_noradcor_9x130_q2_1to10
URL="${XRD}/1.0.0/${REF}/q2_1to10/pythia6-lhapdf_1.0.0_${NAME}_run${RUN}.ab.hepmc3.tree.root"

DIR=${OUTDIR}/${REF}/q2_1to10
mkdir -p "${DIR}/logs" "${DIR}"/x1e-4to1e-3 "${DIR}"/x1e-3to1e-2 "${DIR}"/x1e-2to1e-1
DIR=$(realpath -e "$DIR")
PATTERN="${DIR}/{x}/pythia6-lhapdf_1.1.0_${NAME}_{x}_run${RUN}.ab.hepmc3.tree.root"
LOG=${DIR}/logs/run${RUN}.log

echo "input $URL" | tee "$LOG"
singularity exec -B /gpfs,/gpfs02,/cvmfs "$IMAGE" \
  root -l -b -q "${HERE}/filter_decades.C+(\"${URL}\",\"${PATTERN}\")" 2>&1 | tee -a "$LOG"
grep -q "^x in \[0.01, 0.1): kept" "$LOG"
