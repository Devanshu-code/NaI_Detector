#!/usr/bin/env python3
"""
plot_co60_spectrum.py
=====================
Reads the Geant4 output  results/energy_spectrum_co60.txt  and produces
an annotated, realistically-broadened Co-60 spectrum for a 3"x3" NaI(Tl)
detector.

Physics included
----------------
  • Gaussian energy smearing  →  R(E) = a / sqrt(E),  calibrated to 7% at 662 keV
  • Compton continuum shading
  • Compton edge markers
  • Photopeak annotations with fitted mu, FWHM, resolution %
  • Photoelectric region label

Output
------
  results/co60_spectrum.png
  results/co60_peaks.txt

Usage
-----
  python3 plot_co60_spectrum.py [path/to/energy_spectrum_co60.txt]
"""

import sys
import os
import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from scipy.optimize import curve_fit

# ── 0. Paths ────────────────────────────────────────────────────────────────
DEFAULT_INPUT  = os.path.join("results", "energy_spectrum_co60.txt")
OUTPUT_DIR     = "results"
OUTPUT_PNG     = os.path.join(OUTPUT_DIR, "co60_spectrum.png")
OUTPUT_SUMMARY = os.path.join(OUTPUT_DIR, "co60_peaks.txt")

input_file = sys.argv[1] if len(sys.argv) > 1 else DEFAULT_INPUT
os.makedirs(OUTPUT_DIR, exist_ok=True)

# ── 1. Load raw Geant4 data ──────────────────────────────────────────────────
data       = np.loadtxt(input_file, comments="#")
energy_MeV = data[:, 0]
counts_raw = data[:, 1].astype(float)
energy_keV = energy_MeV * 1000.0

print(f"Loaded {len(energy_keV)} bins  |  raw total counts = {int(counts_raw.sum())}")

# ── 2. NaI(Tl) resolution smearing ──────────────────────────────────────────
# Real NaI(Tl): R(E) = a / sqrt(E)
# At 662 keV (Cs-137 photopeak), R ≈ 7%
#   => a = 0.07 * sqrt(662) = 1.801  [sqrt-keV units]
# sigma(E) = R(E)*E / 2.355 = a*sqrt(E) / 2.355

NaI_a = 0.07 * np.sqrt(662.0)   # ≈ 1.801

def smear_spectrum(e_arr, c_arr, a):
    smeared = np.zeros_like(c_arr)
    for i, (e0, cnt) in enumerate(zip(e_arr, c_arr)):
        if cnt == 0:
            continue
        sigma = a * np.sqrt(max(e0, 1.0)) / 2.355   # keV
        gauss = np.exp(-0.5 * ((e_arr - e0) / sigma) ** 2)
        norm  = gauss.sum()
        if norm > 0:
            smeared += cnt * gauss / norm
    return smeared

print("Applying NaI(Tl) smearing  (R = 7% @ 662 keV) — takes ~10 s ...")
counts = smear_spectrum(energy_keV, counts_raw, NaI_a)
print(f"Smearing done  |  smeared total counts = {counts.sum():.1f}")

# ── 3. Physics reference values ──────────────────────────────────────────────
PEAK1_KEV = 1173.2
PEAK2_KEV = 1332.5

def compton_edge(e):          # e in keV
    a = e / 511.0
    return e * 2*a / (1 + 2*a)

CE1_KEV = compton_edge(PEAK1_KEV)   # ~963 keV
CE2_KEV = compton_edge(PEAK2_KEV)   # ~1118 keV
print(f"Compton edge 1: {CE1_KEV:.1f} keV  |  Compton edge 2: {CE2_KEV:.1f} keV")

# ── 4. Gaussian fit helper ───────────────────────────────────────────────────
def gaussian(x, amp, mu, sigma):
    return amp * np.exp(-0.5 * ((x - mu) / sigma) ** 2)

def fit_peak(e_arr, c_arr, centre, window=60.0):
    mask = np.abs(e_arr - centre) < window
    if mask.sum() < 5:
        return None
    x, y = e_arr[mask], c_arr[mask]
    try:
        popt, pcov = curve_fit(gaussian, x, y,
                               p0=[y.max(), centre, 20.0], maxfev=8000)
        return popt, np.sqrt(np.diag(pcov)), mask
    except Exception:
        return None

# ── 5. Plot ──────────────────────────────────────────────────────────────────
fig, ax = plt.subplots(figsize=(13, 6))
fig.patch.set_facecolor("#0f1117")
ax.set_facecolor("#0f1117")
for sp in ax.spines.values():
    sp.set_edgecolor("#444")
ax.tick_params(colors="white")
ax.xaxis.label.set_color("white")
ax.yaxis.label.set_color("white")
ax.title.set_color("white")

# Raw (ideal) spectrum — faint reference
ax.step(energy_keV, counts_raw, where="mid",
        color="#005f6b", linewidth=0.6, alpha=0.5, label="Ideal (no smearing)")

# Smeared spectrum
ax.step(energy_keV, counts, where="mid",
        color="#00e5ff", linewidth=1.1, label="NaI(Tl) smeared (R=7% @ 662 keV)")

# Compton continuum shading
cm = energy_keV < CE2_KEV - 15
ax.fill_between(energy_keV[cm], counts[cm],
                alpha=0.20, color="#ff9800",
                label=f"Compton continuum (< {CE2_KEV:.0f} keV)")

# Photopeak shading
for pk, col, lbl in [
    (PEAK1_KEV, "#4caf50", f"Photopeak 1  {PEAK1_KEV:.1f} keV"),
    (PEAK2_KEV, "#ce93d8", f"Photopeak 2  {PEAK2_KEV:.1f} keV"),
]:
    pm = np.abs(energy_keV - pk) < 60
    ax.fill_between(energy_keV[pm], counts[pm],
                    alpha=0.45, color=col, label=lbl)

# Compton edge lines
ymax = counts.max()
for ce, lbl in [(CE1_KEV, f"CE₁ {CE1_KEV:.0f} keV"),
                (CE2_KEV, f"CE₂ {CE2_KEV:.0f} keV")]:
    ax.axvline(ce, color="#ff5722", linestyle="--", linewidth=1.1, alpha=0.9)
    ax.text(ce + 6, ymax * 0.05, lbl,
            color="#ff5722", fontsize=7.5, rotation=90, va="bottom")

# Photopeak annotations with Gaussian fit results
results_text = []
for pk, col in [(PEAK1_KEV, "#4caf50"), (PEAK2_KEV, "#ce93d8")]:
    res = fit_peak(energy_keV, counts, pk)
    if res:
        popt, _, _ = res
        amp, mu, sigma = popt
        fwhm = 2.355 * abs(sigma)
        R    = fwhm / mu * 100
        txt  = f"{mu:.1f} keV\nFWHM={fwhm:.1f} keV\nR={R:.1f}%"
        ax.annotate(txt,
                    xy=(mu, amp),
                    xytext=(mu - 160, amp * 0.78),
                    color=col, fontsize=8,
                    arrowprops=dict(arrowstyle="->", color=col, lw=1.2),
                    bbox=dict(boxstyle="round,pad=0.3",
                              facecolor="#1a1a2e", edgecolor=col, alpha=0.85))
        line = (f"Peak {pk:.1f} keV | fitted μ={mu:.2f} keV | "
                f"FWHM={fwhm:.2f} keV | R={R:.2f}%")
        results_text.append(line)
        print(line)
    else:
        results_text.append(f"Peak {pk:.1f} keV | fit failed")

# Photoelectric label (low-E activity)
pe_mask = (energy_keV > 10) & (energy_keV < 200)
if pe_mask.any() and counts[pe_mask].max() > 0:
    pe_idx = np.where(pe_mask)[0][counts[pe_mask].argmax()]
    ax.annotate("Photoelectric\nregion",
                xy=(energy_keV[pe_idx], counts[pe_idx]),
                xytext=(energy_keV[pe_idx] + 120, counts[pe_idx] + ymax * 0.06),
                color="#ffd54f", fontsize=8,
                arrowprops=dict(arrowstyle="->", color="#ffd54f", lw=1.0),
                bbox=dict(boxstyle="round,pad=0.3",
                          facecolor="#1a1a2e", edgecolor="#ffd54f", alpha=0.85))

# Labels / legend / grid
ax.set_xlabel("Energy Deposited (keV)", fontsize=11)
ax.set_ylabel("Counts / bin", fontsize=11)
ax.set_title('Co-60  |  3"×3" NaI(Tl) Detector  |  Geant4 + Resolution Smearing',
             fontsize=13, fontweight="bold")
ax.set_xlim(0, energy_keV.max())
ax.set_ylim(bottom=0)
ax.legend(loc="upper left", facecolor="#1a1a2e", edgecolor="#555",
          labelcolor="white", fontsize=8)
ax.grid(True, which="major", linestyle="--", linewidth=0.4, color="#333", alpha=0.7)
ax.grid(True, which="minor", linestyle=":",  linewidth=0.3, color="#222", alpha=0.5)
ax.minorticks_on()

plt.tight_layout()
plt.savefig(OUTPUT_PNG, dpi=150, bbox_inches="tight",
            facecolor=fig.get_facecolor())
print(f"\nPlot saved  →  {OUTPUT_PNG}")

# ── 6. Summary text ──────────────────────────────────────────────────────────
with open(OUTPUT_SUMMARY, "w") as f:
    f.write("# Co-60 Spectrum Analysis Summary\n")
    f.write(f"# Input          : {input_file}\n")
    f.write(f"# Raw counts     : {int(counts_raw.sum())}\n")
    f.write(f"# NaI resolution : R = 7% @ 662 keV  (a={NaI_a:.4f} sqrt-keV)\n\n")
    f.write(f"Compton edge 1 (1173.2 keV line) : {CE1_KEV:.2f} keV\n")
    f.write(f"Compton edge 2 (1332.5 keV line) : {CE2_KEV:.2f} keV\n\n")
    for line in results_text:
        f.write(line + "\n")
print(f"Summary saved  →  {OUTPUT_SUMMARY}")
