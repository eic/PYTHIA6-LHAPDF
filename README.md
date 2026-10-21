# PYTHIA6-LHAPDF: nPDF selection fix

This repository documents a fix for a bug that silently overrides the user's chosen nuclear PDF (nPDF) when running PYTHIA6 (via `PYTHIA-RAD-CORR`) with LHAPDF6, and CERNLIB in the mix.

## The problem

`PYTHIA-RAD-CORR` depends on CERNLIB to run. When PYTHIA6 calls CERNLIB's `structa` routine, `structa` makes an internal call that overwrites the user's PDF selection — always forcing PDF ID **3089**, regardless of what was requested in the steering card. This happens silently: there's no error, no warning, just the wrong PDF being used.

## The fix

`LHAGlue` (LHAPDF's PDFLIB/CERNLIB compatibility interface) has been patched to detect and ignore initialization attempts using PDF ID 3089, so CERNLIB's unwanted override has no effect and the user's actual steering-card choice is respected.

This means **both** source trees in this repo carry modifications relative to their upstream releases:

- **[`PYTHIA-RAD-CORR/`](./PYTHIA-RAD-CORR)** — PYTHIA6 with radiative corrections, adapted for EIC physics ([upstream](https://gitlab.com/eic/mceg/PYTHIA-RAD-CORR); see below)
- **[`LHAPDF-6.5.5/`](./LHAPDF-6.5.5)** — LHAPDF 6.5.5, patched in `LHAGlue` to ignore PDF ID 3089 init calls ([upstream release](https://lhapdf.hepforge.org/))

## Selecting your nPDF

nPDF selection is done via a parameter in the steering card: `[PARAMETER NAME — MSTP(195)]`. See `STEERING` for example steering cards that demonstrates nPDF selection.

---

## Repository structure

```
PYTHIA6-LHAPDF/
├── PYTHIA-RAD-CORR/     # PYTHIA6 source, build system, and steering files (see nPDF fix note)
├── LHAPDF-6.5.5/        # LHAPDF source, patched in LHAGlue for the nPDF fix (see below)
├── pdfsets/             # A few bundled PDF sets for quick testing (see below)
├── STEERING/            # Example input cards, incl. the production cards
├── production/          # How the published samples were made: generation, eic-smear, afterburner
├── validation/          # Reproducibility tests (why -fno-automatic is required) and scripts
├── install-lhapdf/      # LHAPDF build output (git-ignored, created when you build)
├── scripts/             # general setup scripts
│   └── load-env.sh      # sets important paths (*required everytime you enter eic-shell*)
└── .gitignore
```

---

## Environment setup

This project is built and run inside **`eic-shell`**, the containerized EIC software environment.

```bash
eic-shell
```

> **Note:** even if your default login shell is `tcsh`, `eic-shell` runs `bash` internally. Verify with `ps -p $$ -o comm=` if unsure. All commands below assume bash, run from inside `eic-shell`.

---

## 1. Building LHAPDF

Before building (only needed once, right after cloning):

```bash
bash scripts/fix-timestamps.sh
```

LHAPDF uses an Autotools build system. **Always build out-of-source** — never run `./configure` directly inside `LHAPDF-6.5.5/`. This keeps the tracked source tree clean.

```bash
mkdir -p build-lhapdf install-lhapdf
cd build-lhapdf

../LHAPDF-6.5.5/configure --prefix=`readlink -f ../install-lhapdf` --disable-python
```

> Python bindings are disabled since they're not needed for this project's
> C++/Fortran workflow, and the pre-generated Python wrapper has a known
> path issue with out-of-source builds.

```bash
make -C src -j4
make -C src install
make -C include install
make -C bin install

mkdir -p ../install-lhapdf/share/LHAPDF
cp ../LHAPDF-6.5.5/pdfsets.index ../install-lhapdf/share/LHAPDF/
cp ../LHAPDF-6.5.5/lhapdf.conf ../install-lhapdf/share/LHAPDF/

cd ..
```

> Only src/include/bin are built and installed — examples and tests are
> skipped since they're not needed here, and have a known out-of-source
> build issue with generated headers.
> PDFs important information is copyed to relevant directory

### Loading the environment

Once built, source the environment script from the repo root:

```bash
source scripts/load-env.sh
```
Verify it worked:
```bash
lhapdf-config --datadir
lhapdf list --installed
```
This works out of the box for anyone who clones the repo and follows the build step above exactly — no path editing required.

### PDF sets

A few PDF sets are bundled directly in [`pdfsets/`](./pdfsets) for convenience.

To install additional PDF sets:

```bash
lhapdf install <pdf-set-name>
```
See the [LHAPDF PDF set list](https://lhapdf.hepforge.org/pdfsets.html) for available sets.

---

## 2. Building PYTHIA6 (PYTHIA-RAD-CORR)

```bash
cd PYTHIA-RAD-CORR
mkdir -p build
cd build

CC=gcc CXX=g++ FC=gfortran cmake ..
make -j4
```
The build output (executables, object files) lands in `build/` and is not tracked in git.

> **Compiler note.** `CMakeLists.txt` builds the Fortran with `-fno-automatic`.
> Without it, a gfortran 14 build (eic-shell) runs fine but gives different
> hadron pT spectra from the gcc 8.3 build that produced the published
> samples. See [`validation/README.md`](validation/README.md). Keep the
> flag if you change the build.

Verify it worked, from build/:
```bash
./pythia6-eic < ../../STEERING/card_test_eAg
```
---

## What was changed, and where

| File | Change |
|---|---|
| `LHAPDF-6.5.5/src/LHAGlue.cc` (Lines 1167 to 1172) | Ignores PDF init calls using ID 3089 (CERNLIB's `structa` override) |
| `PYTHIA-RAD-CORR/src/pythia/pypdfu.f` (Lines 385 to 395) | Set nPDF from steering-card input MSTP(195) |
| `PYTHIA-RAD-CORR/CMakeLists.txt` | Builds the Fortran with `-fno-automatic`, needed for modern gfortran (see [`validation/`](validation/README.md)) |

## Reproducing the published samples

[`production/README.md`](production/README.md) describes the full chain used
for the ep 9x130, eAg 9x115 and eAu 9x100 samples: steering cards, generation,
seeds, eic-smear conversion, afterburner presets, HepMC3 ROOT trees and software
versions.
Read its [caveats for the eA samples](production/README.md#caveats-for-the-ea-samples)
before using them: the struck nucleon is always a bound proton.

The published samples were generated with a gcc 8.3 build (CentOS 7 Fun4All
stack on cvmfs), not in eic-shell. An eic-shell build of this repository
reproduces them statistically, with the `-fno-automatic` flag. The tests
are in [`validation/README.md`](validation/README.md). The build also links
CERNLIB from `/cvmfs/eic.opensciencegrid.org/gcc-8.3/opt/fun4all/core/cern64/pro`
(set in `scripts/load-env.sh`), so it depends on that cvmfs area staying
available.

## Source history

`PYTHIA-RAD-CORR/` was originally developed at:
**https://gitlab.com/eic/mceg/PYTHIA-RAD-CORR**

The full, detailed commit history (including prior contributors' work) lives there. This repository tracks that source, plus the nPDF fix described above, alongside a patched LHAPDF for convenience of building both together. For the authoritative upstream history, issue tracking, and unmodified PYTHIA6 source, use the GitLab repository.

---

## License

[Add license info — LHAPDF is GPL-licensed (see `LHAPDF-6.5.5/COPYING`); confirm PYTHIA6/PYTHIA-RAD-CORR licensing before publishing this repo publicly.]
