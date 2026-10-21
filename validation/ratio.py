"""Ratio of one distribution to a reference, for several samples from histo.py.

usage: python3 ratio.py <distribution> out.png ref.npz "ref label" new.npz "label" [new2.npz "label2" ...]
"""
import sys
import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt


def main():
    dist, out, refpath, reflabel = sys.argv[1:5]
    samples = list(zip(sys.argv[5::2], sys.argv[6::2]))
    ref = np.load(refpath)
    e = ref[dist + "_edges"]
    ctr = 0.5 * (e[1:] + e[:-1])
    a = ref[dist].astype(float)
    fig, ax = plt.subplots(figsize=(7, 4.2))
    width = 0.8 * (e[1] - e[0]) / len(samples)
    for i, (path, label) in enumerate(samples):
        b = np.load(path)[dist].astype(float)
        with np.errstate(divide="ignore", invalid="ignore"):
            r = (b / b.sum()) / (a / a.sum())
            err = r * np.sqrt(1 / a + 1 / b)
        ok = (a > 0) & (b > 0)
        x = ctr + (i - (len(samples) - 1) / 2) * width
        ax.errorbar(x[ok], r[ok], err[ok], fmt="o", ms=3, lw=0.8, label=label)
    ax.axhline(1, color="k", lw=0.6)
    ax.set_ylim(0.85, 1.3)
    ax.set_xlabel(dist)
    ax.set_ylabel(f"ratio to {reflabel}")
    ax.legend(fontsize=8, loc="upper left")
    fig.savefig(out, dpi=100, bbox_inches="tight")


main()
