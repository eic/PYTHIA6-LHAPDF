# Production: PYTHIA6-LHAPDF ep / eAg / eAu samples (2026)

This page records how the published `pythia6-lhapdf` samples were made, from
the steering cards to the afterburned HepMC3 ROOT trees used by ePIC
simulation, and how to make them again.

| Dataset | Beams (e × ion, GeV/nucleon) | Target PDF | Q² (GeV²) | Runs × events | σ (µb) |
|---|---|---|---|---|---|
| ep 9x130 | 9 × 130 | cteq6l1 (LHAPDF 10042) | 1 – 10 | 10 × 5,000,000 | 0.4194 |
| eAg 9x115 | 9 × 115 | EPPS16nlo_CT14nlo_Ag108 (900800) | 1 – 10 | 10 × 5,000,000 | 0.3138 |
| eAu 9x100 | 9 × 100 | EPPS16nlo_CT14nlo_Au197 (901200) | 1 – 10 | 10 × 5,000,000 | 0.3024 |

σ is PYTHIA's total cross-section normalisation, averaged over the 10 runs
(the runs agree to about 0.03%). It is 1.0 / 1.5 / 1.7% higher than the events
support; see [Known issue: cross section](#known-issue-cross-section).

**1.1.0** is the same events split into three Bjorken-x decades; see
[1.1.0: the 1.0.0 samples split into x decades](#110-the-100-samples-split-into-x-decades).

The pipeline has two stages:

```
STEERING/card_*  ──pythia6-eic──▶  name_N.txt          (1. generation, this repo)
name_N.txt  ──BuildTree──▶ ──hadd──▶ ──TreeToHepMC──▶ ──abconv──▶ ──hepmc3ascii2root──▶  name_runN_ab.hepmc3.tree.root
            (eic-smear)                (eic-smear)     (afterburner)                       (2. conversion, run_chain.sh)
```

## Caveats for the eA samples

**The struck nucleon is always a bound proton.** The cards set the PYTHIA
target to a single nucleon (`A-Tar,Z-Tar = 1,1`). The nucleus enters only
through the nuclear PDF (`MSTP(195)`, EPPS16), whose LHAPDF grids are
bound-proton PDFs. This is the convention of the eA cards that ship with
PYTHIA-RAD-CORR (`PYTHIA-RAD-CORR/STEER-FILES-eA/`). As a result:

- Every event is e + bound proton in Ag or Au. There is no scattering off
  neutrons (61 of 108 nucleons in Ag, 118 of 197 in Au), so isospin effects
  are missing.
- The ion beam in the event record is a proton with the per-nucleon beam
  energy (e.g. `2212` at 115 GeV for eAg), not a nucleus.
- There are no nuclear remnants: no spectator nucleons, evaporation
  neutrons or fragments. Far-forward studies relying on them (ZDC, off-momentum
  and Roman-pot nuclear tagging) can't use these samples for that.
- Kinematics and hadronization are those of a single nucleon; nuclear effects
  are limited to the modified PDFs. There is no nuclear-medium modification
  of the final state.

**Ag is A = 108.** EPPS16 provides one silver set, `EPPS16nlo_CT14nlo_Ag108`
(A = 108, Z = 47), representing natural silver (about 52% ¹⁰⁷Ag and 48%
¹⁰⁹Ag; average mass 107.87). There is no ¹⁰⁷Ag set. The nuclear modification
varies slowly with A, so the difference from ¹⁰⁷Ag is far below the EPPS16
uncertainties. For ePIC simulation with a ¹⁰⁷Ag beamline the mismatch has no
practical effect, since no particle in these events carries nuclear rigidity.

**Afterburner.** eAg 9x115 uses the eAu 100x9 beam-effects preset (no Ag
preset exists); see [Afterburner presets](#afterburner-presets).

## Software used

| Component | Version | Notes |
|---|---|---|
| PYTHIA-RAD-CORR | upstream [`eic/mceg/PYTHIA-RAD-CORR`](https://gitlab.com/eic/mceg/PYTHIA-RAD-CORR) `578c892` + the nPDF fix in `src/pythia/pypdfu.f` | Same source as this repo at `7b57871` (checked file by file) |
| LHAPDF | 6.5.5 + the `LHAGlue.cc` patch | Same source as `LHAPDF-6.5.5/` here |
| PDF sets | cteq6l1 (data v4), EPPS16nlo_CT14nlo_Ag108 / Au197 (data v1) | Same as `pdfsets/` |
| Compiler | gcc / gfortran 8.3.0, CentOS 7 Fun4All stack on cvmfs (`/cvmfs/eic.opensciencegrid.org/gcc-8.3/opt/fun4all/core/gcc/8.3.0.1-0a5ad/x86_64-centos7`) | **Not eic-shell**, see below |
| CERNLIB | `/cvmfs/eic.opensciencegrid.org/gcc-8.3/opt/fun4all/core/cern64/pro` (`packlib mathlib kernlib pdflib804`) | Also used by eic-shell builds |
| Generator binary | `pythia6-eic` built 2025-05-28, run 2026-07-21 to 07-24 | |
| Conversion container | `eic_xl:nightly`, image `20c9f267aa84…` (cvmfs, updated 2026-10-14), run 2026-10-20 | eic-smear 1.2.2, afterburner 0.2.1 (`abconv --version`: 0.1.3), ROOT 6.40.04, HepMC3 3.3.0 |
| hepmc3ascii2root | [`eic/hepmc3ascii2root`](https://github.com/eic/hepmc3ascii2root) `bba0b3c`, binary built March 2025 | |

### Generator build: read this before regenerating

The samples were generated with a **gcc 8.3** build. A build made in
eic-shell (gcc 14) following the top-level README **without
`-fno-automatic`** gives the same cross section and event kinematics but
**different hadron pT spectra** (pion pT up to about 20% harder above
2 GeV). This branch adds `-fno-automatic` to
`PYTHIA-RAD-CORR/CMakeLists.txt`. With that flag, an eic-shell build
reproduces the production samples within statistics. The tests and the
cause are described in [`validation/README.md`](../validation/README.md).

## 1. Generation

### Steering cards

| Dataset | Card |
|---|---|
| ep 9x130 | `STEERING/card_ep_9x130` |
| eAg 9x115 | `STEERING/card_eAg_9x115_EPPS16` |
| eAu 9x100 | `STEERING/card_eAu_9x100_EPPS16` |

The three cards are the ones in the production area
(`/gpfs02/eic/gabrielg/Saturation_v6/STEERING/`), copied unchanged. Their
settings:

- Electron beam (type 11), 9 GeV; ion beam energy per nucleon as above.
- 1e-4 < x < 0.9, 0.01 < y < 0.99, 1 < Q² < 10 GeV²; radiative corrections
  off; GVMD generated in y and Q² (`Pythia-Model = 1`); `MSEL=2`.
- PDFs come from LHAPDF6 (`MSTP(51)=10042`, `MSTP(52)=2`). For eA the
  nuclear PDF is selected by `MSTP(195)` (= LHAPDF ID), which is the
  purpose of this repo. The target in PYTHIA is still a single nucleon
  (`A-Tar,Z-Tar = 1,1`); the nucleus enters only through the nPDF (see
  [Caveats](#caveats-for-the-ea-samples)).
- HERMES-style tune (`MSTP(17)=6`, `PARP(18)=0.17`, `PARJ(21)=0.40`, …),
  which differs from the `pythia6-eic` 1.3.0 campaign cards.
- 5,000,000 events per run.

Each run used its own copy of the card, identical except for the output
file name on line 1 (`ep_9x130_0.txt`, `ep_9x130_1.txt`, …).
`run_generation.sh` makes those copies and runs them. Its per-run cards
are byte-identical to the ones used in production.

### Running

Inside eic-shell, after building (top-level README) and
`source scripts/load-env.sh`:

```bash
mkdir -p gen && cd gen
../production/run_generation.sh ../STEERING/card_ep_9x130 0 1 2 3 4 5 6 7 8 9
```

This writes `ep_9x130_N.txt` and `logs/ep_9x130_N.log`. Runs are
independent, so they can be split across jobs (one run index per job).
Each 5M-event run takes about 2 hours and writes about 37 GB of text.

### Seeds

`pythia6-eic` takes its seed from `/dev/urandom` (falling back to the clock)
and prints it as `SEED =`. There is no input to set it, so production runs
cannot be repeated event for event; a new generation is statistically
equivalent, not identical. The production seeds, for the record:

| Run | ep 9x130 | eAg 9x115 | eAu 9x100 |
|---|---|---|---|
| 0 | 969671477 | 925407239 | 1013419522 |
| 1 | 1925349884 | 40284032 | 1096540481 |
| 2 | 183773467 | 1593946993 | 920052100 |
| 3 | 1196336252 | 957597977 | 1381327812 |
| 4 | 385100633 | 1057354154 | 1600310440 |
| 5 | 656626341 | 11834629 | 2133140119 |
| 6 | 1764896587 | 512923923 | 431316603 |
| 7 | 355715844 | 1017065507 | 1319896922 |
| 8 | 14034772 | 262374064 | 779936588 |
| 9 | 249406902 | 1885594891 | 918517291 |

Production logs: `/gpfs02/eic/gabrielg/Saturation_v6/LOGS/`.

## 2. Conversion to afterburned HepMC3 ROOT trees

`run_chain.sh` runs the whole chain for one generator file inside the
`eic_xl` container. `submit.sh` submits one BNL condor job per file.

| Step | Tool | Output |
|---|---|---|
| 1 | eic-smear `BuildTree(file, outdir, -1, log)` | `name_N.root`, `name_N_1.root`, … (ROOT splits large trees) |
| 2 | `hadd` | `name_runN.root` |
| 3 | eic-smear `TreeToHepMC(file, outdir)` | `name_runN.hepmc` (HepMC3 ASCII) |
| 4 | `abconv -p <preset> -f hepmc3` | `name_runN_ab.hepmc` (+ `.hist.root`) |
| 5 | `hepmc3ascii2root` | `name_runN_ab.hepmc3.tree.root` |

Intermediate files are deleted once the next step succeeds (`KEEP=1` keeps
them). Given a `.root` or `.hepmc` file, the chain starts at step 3 or 4.

### Afterburner presets

| Dataset | `AB_PRESET` |
|---|---|
| ep 9x130 | `ip6_ep_130x9` |
| eAg 9x115 | `ip6_eAu_100x9` |
| eAu 9x100 | `ip6_eAu_100x9` |

There is no 115 GeV eAg preset; eAg uses the eAu 100x9 preset by choice.
Neither preset is listed in `abconv --help`, but both are accepted (an
unknown name aborts with "Invalid configuration name").

### Running

```bash
export HEPMC2ROOT=/path/to/hepmc3ascii2root    # build from eic/hepmc3ascii2root
AB_PRESET=ip6_ep_130x9  ./production/submit.sh gen/ep_9x130_*.txt
AB_PRESET=ip6_eAu_100x9 ./production/submit.sh gen/eAg_9x115_EPPS16_*.txt
AB_PRESET=ip6_eAu_100x9 ./production/submit.sh gen/eAu_9x100_EPPS16_*.txt
```

Outputs go to `./output_eic-smear/` (or `OUTDIR`), condor files to
`./condor_logs/`. One file by hand:
`AB_PRESET=ip6_ep_130x9 production/run_chain.sh gen/ep_9x130_0.txt`.

| Variable | Effect |
|---|---|
| `AB_PRESET` | Afterburner preset (required) |
| `HEPMC2ROOT` | `hepmc3ascii2root` binary (required) |
| `OUTDIR` | Output directory (default `./output_eic-smear`) |
| `KEEP=1` | Keep intermediate files |
| `IMAGE` | Container (default `eic_xl:nightly`; pin a dated tag to reproduce exactly) |

Disk: about 66 GB peak per job (step 4, when both `.hepmc` files exist),
about 10.2–10.6 GB per final file.

Notes:
- Don't edit `run_chain.sh` in place while jobs are running: bash reads
  scripts as it goes, so running jobs can pick up half-edited lines.
- `TreeToHepMC` is called with two arguments; passing `maxEvent` is
  ambiguous between its overloads.
- Compared with the copies used in production, these scripts differ only in
  their path defaults (`HEPMC2ROOT` must be set; outputs and logs go under
  the current directory). The processing steps are unchanged.

### Production check

Every one of the 30 conversion jobs (condor clusters 277240 ep, 277257 eAg,
277258 eAu) logged `Events parsed and written: 5000000`.

## 1.1.0: the 1.0.0 samples split into x decades

1.1.0 contains no new generation. Each 1.0.0 file is split by its `x` event
attribute into x ∈ [1e-4,1e-3), [1e-3,1e-2) and [1e-2,1e-1). Every event is
written unchanged to the file of its decade. Events with x outside [1e-4,1e-1)
are dropped: 0.9% (ep), 1.7% (eAg) and 2.5% (eAu). The generator code is that of
1.0.0, and the tag adds only the filter.

The split serves analyses that need fixed statistics per x decade, such as the
truth-level dihadron Δφ correlations (about 400 pairs per decade). It replaces
random subsampling of 1.0.0. Generating directly in an x window with the
Bjorken-x cut of PR #2 is not equivalent to 1.0.0 until issues #3 and #5 are
fixed (PRs #4, #6).

### Making the files

```bash
production/filter_1.0.0.sh ep_noradcor/9x130 1 /path/to/out     # runs 1 ... 10, likewise
production/filter_1.0.0.sh eAg_EPPS16_noradcor/9x115 1 /path/to/out
production/filter_1.0.0.sh eAu_EPPS16_noradcor/9x100 1 /path/to/out
```

Each call reads one 1.0.0 file over XRootD and runs `filter_decades.C` in the
`eic_xl` container, about 40 minutes per run. Output:

```
<out>/<dataset>/q2_1to10/<decade>/pythia6-lhapdf_1.1.0_<dataset_>_q2_1to10_<decade>_run<NNN>.ab.hepmc3.tree.root
<out>/<dataset>/q2_1to10/logs/run<NNN>.log
```

where `<decade>` is `x1e-4to1e-3`, `x1e-3to1e-2` or `x1e-2to1e-1`. Each log
ends with the events kept per decade.

### Contents

All 10 runs per dataset. σ is the 1.0.0 value times the decade's share of the
5 × 10⁷ events. The last column uses the event-consistent total (see below).

| Dataset | Decade | Events (10 runs) | Share | σ (µb) | σ, event-consistent (µb) |
|---|---|---|---|---|---|
| ep 9x130 | x1e-4to1e-3 | 8,288,775 | 0.16578 | 0.06953 | 0.06883 |
| | x1e-3to1e-2 | 25,852,913 | 0.51706 | 0.21685 | 0.21468 |
| | x1e-2to1e-1 | 15,425,734 | 0.30851 | 0.12939 | 0.12810 |
| eAg 9x115 | x1e-4to1e-3 | 5,373,078 | 0.10746 | 0.03372 | 0.03321 |
| | x1e-3to1e-2 | 23,528,655 | 0.47057 | 0.14767 | 0.14541 |
| | x1e-2to1e-1 | 20,245,406 | 0.40491 | 0.12706 | 0.12512 |
| eAu 9x100 | x1e-4to1e-3 | 4,264,110 | 0.08528 | 0.02579 | 0.02535 |
| | x1e-3to1e-2 | 22,509,970 | 0.45020 | 0.13614 | 0.13384 |
| | x1e-2to1e-1 | 21,984,536 | 0.43969 | 0.13296 | 0.13072 |

The trees store `GenCrossSection = 1`, as in 1.0.0, so σ is taken from this table.

### Dihadron subset (first N events per decade)

The Δφ analysis needs about 400 truth pairs per decade. That is N = 400 /
(pairs per qualifying event) / 0.985 events, where 0.985 accounts for the
events with 9 < Q² < 10 GeV² that the analysis drops. It takes the first N
events in run order:

| Dataset | Decade | N | Runs used | Expected pairs |
|---|---|---|---|---|
| ep 9x130 | x1e-4to1e-3 | 7.54M | 001–010 (81,289 of 830,064 in 010) | ~400 |
| | x1e-3to1e-2 | 7.94M | 001–004 (184,269 of 2,585,251 in 004) | ~400 |
| | x1e-2to1e-1 | 18.55M | all 10 runs, 15.43M (**short**) | ~330 |
| eAg 9x115 | x1e-4to1e-3 | 4.98M | 001–010 (144,304 of 537,382 in 010) | ~400 |
| | x1e-3to1e-2 | 5.68M | 001–003 (973,386 of 2,353,050 in 003) | ~400 |
| | x1e-2to1e-1 | 14.51M | 001–008 (337,472 of 2,025,342 in 008) | ~400 |
| eAu 9x100 | x1e-4to1e-3 | 6.11M | all 10 runs, 4.26M (**short**) | ~283 |
| | x1e-3to1e-2 | 5.43M | 001–003 (928,682 of 2,251,614 in 003) | ~400 |
| | x1e-2to1e-1 | 14.56M | 001–007 (1,369,823 of 2,199,669 in 007) | ~400 |

The two short decades use all of 1.0.0. This was accepted rather than adding new runs.

### Checks

The files were made with an earlier copy of `filter_decades.C`. It applies the
same selection and differs only in how the output names are passed. The
outputs were then renamed to the scheme above. Checks, all passed:

- **Counts:** each of the 30 runs read 5,000,000 events. The entries of all 90
  trees equal the kept counts in the filter logs.
- **x range:** every event (147,473,177 in total) has its x attribute inside its
  file's decade.
- **Events unchanged:** for run 001 of each dataset, every 1.0.0 event with x in
  [1e-4,1e-1) was compared with the 1.1.0 event at the same position. All are
  identical in attributes, particles (pid, status, momentum, mass) and vertices:
  4,956,773 (ep), 4,914,792 (eAg), 4,875,896 (eAu). None differ, and no file
  holds extra events.

### Known issue: cross section

PYTHIA's low-pT/hard (non-diffractive) cross section counts events that
`PYREMN` fails to complete and that `PYEVNT` therefore regenerates (issue #5).
This is standard PYTHIA 6.4 bookkeeping. The quoted σ is therefore higher than
the events support, by 1.0% (ep), 1.5% (eAg) and 1.7% (eAu). The loss grows with
x, so the effect per decade differs.

- Per-event observables are not affected; they do not use σ.
- For absolute yields, use the event-consistent values (ep 0.4152, eAg 0.3090,
  eAu 0.2973 µb in total, or the last column above).
- The fix (PR #6) changes the quoted σ only; events stay byte-identical. It will
  come with the next physics version, so that 1.1.0 stays consistent with the
  1.0.0 records.

The process-95 rate in 1.0.0 also depends on the kinematics of earlier
attempts in the run (issue #3, PR #4). This is a physics change for a later
version. For the dihadron analysis it shifts pairs per event by 0.2–0.6%,
well below its statistical precision.

