"""Compare two histogram sets from histo.py.

usage: python3 compare.py ref.npz new.npz out.png [ref_label new_label]
Two-sample chi2 (Pearson, unequal totals), bins with n1+n2 < 10 dropped.
"""
import sys
import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from scipy.stats import chi2 as chi2dist

LOGX = {"Q2", "x", "W2", "nu", "p_charged"}


def chi2test(a, b):
    a, b = a.astype(float), b.astype(float)
    m = (a + b) >= 10
    a, b = a[m], b[m]
    A, B = a.sum(), b.sum()
    c2 = np.sum((np.sqrt(B / A) * a - np.sqrt(A / B) * b) ** 2 / (a + b))
    ndf = m.sum() - 1
    return c2, ndf, chi2dist.sf(c2, ndf)


def main():
    ref, new, out = np.load(sys.argv[1]), np.load(sys.argv[2]), sys.argv[3]
    lref, lnew = sys.argv[4:6] if len(sys.argv) > 5 else ("ref", "new")
    names = [k for k in ref.files if not k.endswith(("_edges", "_labels")) and k != "nev" and not k.startswith("cat_")]
    cats = [k for k in ref.files if k.startswith("cat_") and not k.endswith("_labels")]
    print(f"events: ref {int(ref['nev'])}  new {int(new['nev'])}")
    print(f"{'distribution':16s} {'chi2/ndf':>14s} {'p-value':>9s}")
    rows = []
    for k in names + cats:
        c2, ndf, p = chi2test(ref[k], new[k])
        rows.append((k, c2, ndf, p))
        print(f"{k:16s} {c2:8.1f}/{ndf:<5d} {p:9.3f}")

    n = len(names)
    ncol = 4
    nrow = -(-n // ncol)
    fig = plt.figure(figsize=(4.2 * ncol, 4.4 * nrow))
    gs = fig.add_gridspec(2 * nrow, ncol, height_ratios=[3, 1] * nrow, hspace=0.08)
    for i, k in enumerate(names):
        r, c = divmod(i, ncol)
        ax = fig.add_subplot(gs[2 * r, c])
        rx = fig.add_subplot(gs[2 * r + 1, c], sharex=ax)
        e = ref[k + "_edges"]
        a, b = ref[k].astype(float), new[k].astype(float)
        fa, fb = a / a.sum(), b / b.sum()
        ax.stairs(fa, e, label=lref, color="k")
        ax.stairs(fb, e, label=lnew, color="tab:red", ls="--")
        ax.set_yscale("log")
        if k in LOGX:
            ax.set_xscale("log")
        _, _, _, p = next(x for x in rows if x[0] == k)
        ax.set_title(f"{k}   p={p:.2f}", fontsize=10)
        ax.tick_params(labelbottom=False)
        with np.errstate(divide="ignore", invalid="ignore"):
            ratio = fb / fa
            err = ratio * np.sqrt(1 / b + 1 / a)
        ctr = 0.5 * (e[1:] + e[:-1]) if k not in LOGX else np.sqrt(e[1:] * e[:-1])
        ok = (a > 0) & (b > 0)
        rx.errorbar(ctr[ok], ratio[ok], err[ok], fmt=".", ms=3, color="tab:red", lw=0.8)
        rx.axhline(1, color="k", lw=0.6)
        rx.set_ylim(0.8, 1.2)
        rx.set_ylabel("new/ref", fontsize=8)
        if i == 0:
            ax.legend(fontsize=8)
    fig.suptitle(f"{lref} ({int(ref['nev']):,} events) vs {lnew} ({int(new['nev']):,} events), unit-normalized", y=0.995)
    fig.savefig(out, dpi=80, bbox_inches="tight")

    for k in cats:
        labels = ref[k + "_labels"]
        a, b = ref[k], new[k]
        print(f"\n{k[4:]} fractions (ref / new):")
        for l, x, y in zip(labels, a, b):
            if x + y:
                print(f"  {l:>6s}  {x / a.sum():.5f}  {y / b.sum():.5f}")


main()
