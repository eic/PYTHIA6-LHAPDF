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
(the runs agree to about 0.03%).

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
