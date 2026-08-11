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
├── STEERING/            # Example input cards
├── env/
│   └── lhapdf-env.sh    # sets up PATH/LD_LIBRARY_PATH/LHAPDF_DATA_PATH after building
├── install-lhapdf/      # LHAPDF build output (git-ignored, created when you build)
└── .gitignore
```

---

## Prerequisites

- CMake ≥ 3.x
- A Fortran compiler (gfortran) and C++ compiler
- Autotools (`autoconf`, `automake`, `libtool`) — only needed if reconfiguring LHAPDF
- Python 3 (optional, for LHAPDF's Python wrapper)

---

## Environment setup

This project is built and run inside **`eic-shell`**, the containerized EIC software environment.

```bash
eic-shell
```

> **Note:** even if your default login shell is `tcsh`, `eic-shell` runs `bash` internally. Verify with `ps -p $$ -o comm=` if unsure. All commands below assume bash, run from inside `eic-shell`.

---

## 1. Building LHAPDF

LHAPDF uses an Autotools build system. **Always build out-of-source** — never run `./configure` directly inside `LHAPDF-6.5.5/`. This keeps the tracked source tree clean.

```bash
mkdir -p build-lhapdf install-lhapdf
cd build-lhapdf

../LHAPDF-6.5.5/configure --prefix=`readlink -f ../install-lhapdf`
make -j4
make install
```
### Loading the environment

Once built, source the environment script from the repo root:

```bash
source env/lhapdf-env.sh
```

This works out of the box for anyone who clones the repo and follows the build step above exactly — no path editing required.

Verify it worked:
```bash
lhapdf-config --datadir
lhapdf list --installed
```

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

cmake .. -DLHAPDF_ROOT=`readlink -f ../../install-lhapdf`
make -j4
```

> Adjust the `LHAPDF_ROOT` flag/variable name to match whatever your `CMakeLists.txt` actually expects — check `PYTHIA-RAD-CORR/CMakeLists.txt` for the exact variable if this doesn't work out of the box.

The build output (executables, object files) lands in `build/` and is not tracked in git.

---

## What was changed, and where

| File | Change |
|---|---|
| `LHAPDF-6.5.5/src/LHAGlue.cc` (Lines 1167 to 1172) | Ignores PDF init calls using ID 3089 (CERNLIB's `structa` override) |
| `PYTHIA-RAD-CORR/src/pythia/pypdfu.f` (Lines 385 to 395) | Set nPDF from steering-card input MSTP(195) |

## Source history

`PYTHIA-RAD-CORR/` was originally developed at:
**https://gitlab.com/eic/mceg/PYTHIA-RAD-CORR**

The full, detailed commit history (including prior contributors' work) lives there. This repository tracks that source, plus the nPDF fix described above, alongside a patched LHAPDF for convenience of building both together. For the authoritative upstream history, issue tracking, and unmodified PYTHIA6 source, use the GitLab repository.

---

## License

[Add license info — LHAPDF is GPL-licensed (see `LHAPDF-6.5.5/COPYING`); confirm PYTHIA6/PYTHIA-RAD-CORR licensing before publishing this repo publicly.]
