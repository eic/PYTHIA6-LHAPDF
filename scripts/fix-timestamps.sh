#!/bin/bash
# Fixes autotools timestamp ordering after a fresh git clone. Git doesn't
# preserve original file mtimes, which makes autotools think configure.ac/
# aclocal.m4 need regeneration. Setting every file to an identical fixed
# timestamp guarantees no file is ever seen as "newer" than another.

find "$(dirname "$0")/../LHAPDF-6.5.5" -type f -exec touch -t 202001010000 {} +
echo "Timestamps normalized — LHAPDF-6.5.5 ready to build."
