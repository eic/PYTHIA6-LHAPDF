"""Stream PYTHIA6-eic text output and fill fixed-binned histograms.

usage: python3 histo.py out.npz maxevents file1.txt [file2.txt ...]
"""
import math
import sys
import numpy as np

EVT = {
    "y": np.linspace(0, 1, 51),
    "Q2": np.logspace(0, 1, 41),
    "x": np.logspace(-4, 0, 41),
    "W2": np.logspace(0, np.log10(3700), 41),
    "nu": np.logspace(0, np.log10(2000), 41),
    "nFinal": np.arange(-0.5, 80.5, 1),
    "nCharged": np.arange(-0.5, 50.5, 1),
}
PART = {
    "pT_charged": np.linspace(0, 5, 51),
    "eta_charged": np.linspace(-8, 8, 65),
    "p_charged": np.logspace(-2, 2.1, 42),
    "pT_pi": np.linspace(0, 3, 31),
    "eta_pi": np.linspace(-8, 8, 65),
    "eta_K": np.linspace(-8, 8, 65),
    "eta_proton": np.linspace(-8, 8, 65),
    "eta_gamma": np.linspace(-8, 8, 65),
    "eta_e_scat": np.linspace(-5, 1, 61),
    "p_e_scat": np.linspace(0, 12, 61),
}
CAT = {
    "subprocess": [91, 92, 93, 94, 95, 96, 99, 131, 132, 135, 136],
    "nucleon": [2212, 2112],
    "pid": [11, 22, 211, -211, 111, 321, -321, 130, 310, 2212, -2212, 2112, -2112, 3122],
}
CHARGED = {11, -11, 13, -13, 211, -211, 321, -321, 2212, -2212,
           3222, -3222, 3112, -3112, 3312, -3312, 3334, -3334}


def main():
    out, nmax, files = sys.argv[1], int(sys.argv[2]), sys.argv[3:]
    ev = {k: [] for k in EVT}
    pa = {k: [] for k in PART}
    cats = {k: {**dict.fromkeys(v, 0), "other": 0} for k, v in CAT.items()}
    nev = nfin = nch = 0
    escat = False
    done = False

    def bump(cat, key):
        c = cats[cat]
        c[key if key in c else "other"] += 1

    def close_event():
        ev["nFinal"].append(nfin)
        ev["nCharged"].append(nch)

    for fn in files:
        with open(fn) as f:
            for line in f:
                s = line.split()
                if len(s) == 30 and s[0] == "0":  # event header
                    if nev:
                        close_event()
                    if nev >= nmax:
                        done = True
                        break
                    nev += 1
                    bump("subprocess", int(s[3]))
                    bump("nucleon", int(s[4]))
                    ev["y"].append(float(s[10])); ev["Q2"].append(float(s[11]))
                    ev["x"].append(float(s[12])); ev["W2"].append(float(s[13]))
                    ev["nu"].append(float(s[14]))
                    nfin = nch = 0
                    escat = True
                elif len(s) == 14 and s[1] == "1" and s[0].isdigit():  # final-state particle
                    pid = int(s[2])
                    px, py, pz = float(s[6]), float(s[7]), float(s[8])
                    pt = math.hypot(px, py)
                    p = math.hypot(pt, pz)
                    eta = math.asinh(pz / pt) if pt > 0 else math.copysign(20.0, pz)
                    bump("pid", pid)
                    nfin += 1
                    if pid in CHARGED:
                        nch += 1
                        pa["pT_charged"].append(pt); pa["eta_charged"].append(eta); pa["p_charged"].append(p)
                    a = abs(pid)
                    if a == 211:
                        pa["pT_pi"].append(pt); pa["eta_pi"].append(eta)
                    elif a == 321:
                        pa["eta_K"].append(eta)
                    elif pid == 2212:
                        pa["eta_proton"].append(eta)
                    elif pid == 22:
                        pa["eta_gamma"].append(eta)
                    if escat and pid == 11 and s[3] == "3":  # scattered lepton
                        pa["eta_e_scat"].append(eta); pa["p_e_scat"].append(p)
                        escat = False
        if done:
            break
    if not done and nev:
        close_event()

    res = {"nev": nev}
    for k, edges in {**EVT, **PART}.items():
        src = ev if k in EVT else pa
        res[k] = np.histogram(np.asarray(src[k]), bins=edges)[0]
        res[k + "_edges"] = edges
    for k, c in cats.items():
        res["cat_" + k] = np.array(list(c.values()))
        res["cat_" + k + "_labels"] = np.array([str(l) for l in c])
    np.savez(out, **res)
    print(out, "events:", nev, "multiplicities:", len(ev["nFinal"]))


main()
