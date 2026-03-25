# NaI Detector — Co-60 Gamma Spectroscopy Simulation

A Geant4 simulation of a **3″ × 3″ NaI(Tl) scintillation detector** measuring the gamma-ray energy spectrum from a **Co-60 point source**. Runs in batch mode (no GUI) on macOS. Includes a Python analysis script with realistic detector resolution smearing.

---

## Physics Overview

**Co-60** decays to **Ni-60** via beta emission, releasing two coincident gamma rays:

| Line | Energy | Intensity |
|------|--------|-----------|
| γ₁   | 1173.2 keV | ~99.85% |
| γ₂   | 1332.5 keV | ~99.98% |

Features visible in the simulated spectrum:

- **Two photopeaks** at 1173.2 keV and 1332.5 keV (full energy deposition)
- **Compton continuum** — electrons scattered within the crystal
- **Compton edges** at ~963 keV (γ₁) and ~1118 keV (γ₂)
- **Photoelectric region** at low energies
- **NaI(Tl) resolution smearing** — R(E) = a/√E, calibrated to R = 7% at 662 keV

---

## Project Structure

```
NaI_Detector/
├── CMakeLists.txt                  # Build configuration (batch mode, no GUI)
├── main.cc                         # Entry point — batch mode only
├── run1_co60.mac                   # Full simulation: 100,000 events (50k per line)
├── run2_co60.mac                   # Quick test: 200 events
├── plot_co60_spectrum.py           # Python analysis + plot script
│
├── include/
│   ├── action_initialization.hh
│   ├── detector_construction.hh    # 3"x3" NaI(Tl) cylinder geometry
│   ├── event_action.hh
│   ├── primary_generator_action.hh # GPS-based gamma source
│   ├── run_action.hh               # Histogram: 800 bins, 0–1.6 MeV
│   └── stepping_action.hh
│
├── src/
│   ├── action_initialization.cc
│   ├── detector_construction.cc
│   ├── event_action.cc
│   ├── primary_generator_action.cc
│   ├── run_action.cc
│   └── stepping_action.cc
│
└── results/                        # Created at runtime
    ├── energy_spectrum_co60.txt    # Raw Geant4 histogram output
    ├── co60_spectrum.png           # Annotated plot
    └── co60_peaks.txt              # Fitted peak summary
```

---

## Detector Geometry

| Parameter | Value |
|-----------|-------|
| Crystal material | NaI(Tl) — `G4_SODIUM_IODIDE` |
| Shape | Cylinder |
| Diameter | 7.62 cm (3 inches) |
| Length | 7.62 cm (3 inches) |
| Source–detector distance | 7.5 cm (front face) |
| World volume | Air box, 30 cm cube |
| Physics list | QBBC |

---

## Requirements

### Geant4
- Geant4 **11.x** installed and sourced
- Built with `GEANT4_USE_SYSTEM_EXPAT` and standard physics

### Build tools
- CMake ≥ 3.16
- Ninja (`brew install ninja` — required on macOS with CMake 4.x)
- AppleClang or GCC

### Python (for analysis)
- Python 3.8+
- `numpy`, `matplotlib`, `scipy`

```bash
pip3 install numpy matplotlib scipy
```

---

## Build Instructions (macOS)

```bash
# 1. Source your Geant4 environment (adjust path to your installation)
source /usr/local/bin/geant4.sh

# 2. Clone or navigate to the project
cd NaI_Detector

# 3. Create a fresh build directory
mkdir build && cd build

# 4. Configure with Ninja
cmake -GNinja ..

# 5. Compile
ninja

# 6. Create the results directory
mkdir -p results
```

> **Note:** Always use `cmake -GNinja ..` on macOS with CMake 4.x. The standard `make` generator has a known dependency-tracking bug with this CMake version.

---

## Running the Simulation

All commands are run from inside the `build/` directory.

### Full run — 100,000 events (recommended)

```bash
./nai_detector run1_co60.mac
```

Fires 50,000 gammas at 1173.2 keV, then 50,000 at 1332.5 keV. Takes ~1–5 minutes depending on your machine. Output written to `results/energy_spectrum_co60.txt`.

### Quick test — 200 events

```bash
./nai_detector run2_co60.mac
```

Useful to verify the simulation runs before committing to a long job.

---

## Analysing Results

Run from the **project root** (not `build/`):

```bash
cd ..
python3 plot_co60_spectrum.py build/results/energy_spectrum_co60.txt
```

Output files written to `results/`:

| File | Contents |
|------|----------|
| `co60_spectrum.png` | Annotated dark-theme spectrum plot |
| `co60_peaks.txt` | Fitted peak positions, FWHM, resolution % |

### What the plot shows

- **Faint cyan line** — ideal Geant4 spectrum (no detector broadening)
- **Bright cyan line** — NaI(Tl) smeared spectrum (realistic)
- **Orange fill** — Compton continuum
- **Green fill** — Photopeak 1 (1173.2 keV)
- **Purple fill** — Photopeak 2 (1332.5 keV)
- **Red dashed lines** — Compton edges (CE₁ ≈ 963 keV, CE₂ ≈ 1118 keV)
- **Annotations** — fitted μ, FWHM, energy resolution R for each peak

---

## Energy Resolution Model

The `plot_co60_spectrum.py` script applies a post-processing Gaussian smear to simulate the finite energy resolution of a real NaI(Tl) detector:

```
R(E) = a / sqrt(E)          (fractional FWHM)

sigma(E) = R(E) * E / 2.355 = a * sqrt(E) / 2.355

Calibration: R = 7% at 662 keV  =>  a = 0.07 * sqrt(662) ≈ 1.801 sqrt-keV
```

Expected resolution at Co-60 energies:

| Peak | Expected R |
|------|-----------|
| 1173.2 keV | ~4.5% |
| 1332.5 keV | ~4.2% |

---

## Histogram Settings

Defined in `include/run_action.hh`:

```cpp
static const G4int        fNbins = 800;   // number of bins
static constexpr G4double fEmax  = 1.6;   // MeV — upper edge
```

Bin width = 1.6 MeV / 800 = **2 keV per bin**.

To change range or resolution, edit these two constants and recompile.

---

## Adapting for Other Sources

To simulate a different gamma source, edit the `.mac` file:

```
# Example: Cs-137 (661.7 keV single line)
/gps/ene/type Mono
/gps/ene/mono 0.6617 MeV
/run/beamOn 100000
```

Also update `fEmax` in `run_action.hh` if your source has gammas above 1.6 MeV (e.g. set to `2.8` for a Na-22 annihilation + 1274 keV study).

---

## Troubleshooting

| Problem | Fix |
|---------|-----|
| `No rule to make target depend` | Use `cmake -GNinja ..` instead of plain `cmake ..` |
| `FileNotFoundError: energy_spectrum_co60.txt` | Run simulation first; create `build/results/` directory |
| Very low count rate (~2.5%) | Normal — GPS is isotropic; most gammas miss the crystal |
| FWHM unrealistically narrow | Run `plot_co60_spectrum.py` — it applies resolution smearing |
| `Zone.Identifier` files present | Run `find . -name "*Zone.Identifier" -delete` (Windows transfer artefacts) |

---

## Author Notes

- Simulation uses **G4GeneralParticleSource (GPS)** for flexible source configuration
- Threading: **multi-threaded by default** via `G4RunManagerFactory`; histograms are merged thread-safely with `std::mutex`
- No GUI libraries are linked — `find_package(Geant4 REQUIRED)` with no `ui_all`/`vis_all`; the executable is purely batch-mode
