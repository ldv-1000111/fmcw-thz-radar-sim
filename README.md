# FMCW & Terahertz Radar Simulation — Theory, Equations, and Bibliography

[![Phase 1 CI](https://github.com/ldv-1000111/fmcw-thz-radar-sim/actions/workflows/phase1-ci.yml/badge.svg?branch=main)](https://github.com/ldv-1000111/fmcw-thz-radar-sim/actions/workflows/phase1-ci.yml)
[![Phase 2 CI](https://github.com/ldv-1000111/fmcw-thz-radar-sim/actions/workflows/phase2-ci.yml/badge.svg?branch=main)](https://github.com/ldv-1000111/fmcw-thz-radar-sim/actions/workflows/phase2-ci.yml)
[![Phase 3 CI](https://github.com/ldv-1000111/fmcw-thz-radar-sim/actions/workflows/phase3-ci.yml/badge.svg?branch=main)](https://github.com/ldv-1000111/fmcw-thz-radar-sim/actions/workflows/phase3-ci.yml)
![C++17](https://img.shields.io/badge/C%2B%2B-17-blue)
![CMake](https://img.shields.io/badge/CMake-3.24%2B-blue)
![License](https://img.shields.io/badge/license-MIT-lightgrey)
![Docs](https://readthedocs.org/projects/fmcw-terahertz-radar-simulation/badge/?version=latest)

**Author:** Luis Viveros  
**Repository:** [fmcw-thz-radar-sim](https://github.com/ldv-1000111/fmcw-thz-radar-sim)  
**Docs:** [fmcw-terahertz-radar-simulation.readthedocs.io](https://fmcw-terahertz-radar-simulation.readthedocs.io)  
**Version:** v0.1.0 — May 2026  
**License:** MIT

---

## Introduction and rationale

Modern automotive sensing, autonomous vehicle perception, and advanced security
screening all converge on a shared problem: existing sensor modalities —
cameras, conventional radar, and lidar — degrade or fail under adverse weather,
low visibility, or high-resolution classification demands. Terahertz (THz)
imaging, occupying the electromagnetic band between 100 GHz and 10 THz, offers
a physically compelling answer. Its short wavelengths (~1 mm at 300 GHz) yield
sub-millimetre spatial resolution; its photon energy is non-ionising and safe;
and unlike optical sensors it propagates through fog, dust, and rain with
manageable attenuation.

This repository is a structured engineering tutorial. Its goal is to close the
gap between the physics of FMCW THz radar and production-grade embedded
implementation. It does so in three concrete phases:

- **Phase 1 — Physics Engine:** A standalone, dependency-free C++17 FMCW IF
  signal generator operating from mm-wave through 10 THz, with THz
  micro-Doppler vibration modelling and Catch2 test validation.
- **Phase 2 — Signal Processing:** A 2D Range-Doppler pipeline using FFTW3 and
  CA-CFAR detection, producing the detection maps used in real ADAS perception
  stacks.
- **Phase 3 — Yocto Deployment:** Cross-compilation and deployment of the
  simulator to embedded automotive Linux targets (NXP S32G, Renesas R-Car,
  Raspberry Pi 5) via a custom Yocto meta-layer.

### Why simulate before you fabricate?

THz hardware remains expensive and difficult to source. A physics-accurate
software simulator lets an engineer validate signal processing algorithms,
tune chirp parameters, and stress-test detection logic against known ground
truth — all before committing to hardware. The simulator's `fmcw_generator.hpp`
writes its own arithmetic data types in standard C++17 with no third-party
dependencies, making it directly portable to bare-metal and RTOS targets.

> **Note on theory provenance.** The signal model, equations, and physical
> derivations in this repository are drawn from the peer-reviewed radar and
> signal-processing literature listed in the bibliography below. They are
> **not** sourced from the Teradar commercial white papers cited in the
> application-layer documentation (ADAS, defense, healthcare, security). Those
> white papers provide validated performance metrics and application context;
> the foundational physics comes from the academic sources listed here.

---

## Simulator equations with source annotations

All equations below are implemented in `src/fmcw_generator.cpp` and declared
in `include/fmcw_generator.hpp`. Each equation is annotated with its primary
literature source from the bibliography.

### 1. Chirp instantaneous phase

$$\phi(t) = 2\pi \left[ f_0\, t + \frac{B}{2\,T_c}\, t^2 \right]$$

| Symbol | Meaning |
|--------|---------|
| $f_0$ | Start (centre) frequency, Hz — e.g. 300 × 10⁹ for a 300 GHz THz radar |
| $B$ | Sweep bandwidth, Hz — governs range resolution |
| $T_c$ | Chirp duration, s |
| $t$ | Fast-time index within the chirp |

**Source:** Stove (1992) §2; Richards, Scheer & Holm (2010) Ch. 17.

> **Implementation note.** The raw GHz/THz carrier is never simulated directly.
> At 300 GHz, Nyquist requires a sampling rate exceeding 600 GHz —
> computationally intractable. The simulator generates the **IF beat signal**
> instead. See Schasler et al. (2021) `[schasler2021]` for justification in
> an automotive MIMO context.

---

### 2. Round-trip delay

$$\tau(t) = \frac{2\,(R + v\,t)}{c}$$

| Symbol | Meaning |
|--------|---------|
| $R$ | Initial target range, m |
| $v$ | Radial velocity, m/s (positive = approaching) |
| $c$ | Speed of light, 3 × 10⁸ m/s |

**Source:** Skolnik (2001) Ch. 3; Stove (1992) §3.

---

### 3. IF beat frequency

$$f_\text{beat} = \frac{B}{T_c}\,\tau + \frac{2\,f_0\,v}{c}$$

The first term is the **range component** (proportional to round-trip delay);
the second is the **Doppler component** (proportional to radial velocity and
carrier frequency).

**Source:** Stove (1992) §3; Rohling & Meinecke (2001); Richards et al. (2010) Ch. 17.

---

### 4. Analytic IF signal

$$s_\text{IF}[i] = e^{\,j\,2\pi\,f_\text{beat}\,t_i}, \quad t_i = i / f_s$$

The signal is generated as a complex exponential (analytic signal), giving
both in-phase (I) and quadrature (Q) channels. This is the direct output of
`generate_chirp_if()`.

**Source:** Meta, Hoogeboom & Ligthart (2007); Rohling & Meinecke (2001).

---

### 5. Micro-Doppler vibration model

$$R(t) = R_0 + v\,t + A_\text{vib}\,\sin(2\pi\,f_\text{vib}\,t)$$

Substituting this time-varying range into $\tau(t)$ produces sinusoidal
phase modulation across slow-time chirps. At THz wavelengths (~1 mm at
300 GHz), a vibration amplitude of $A_\text{vib} = 0.2$ mm produces a
Doppler sideband amplitude comparable to the main target return — a
physically meaningful and measurable effect that is negligible at 77 GHz
mmWave ($\lambda = 3.9$ mm).

| Target model | $f_\text{vib}$ | $A_\text{vib}$ |
|---|---|---|
| Engine idle | ~200 Hz | 0.1–0.5 mm |
| Pedestrian limb | ~2.5 Hz | 5–20 mm |
| Drone rotor | ~80 Hz | 0.5–2 mm |
| Cyclist pedal | ~1.5 Hz | 10–30 mm |

**Source:** Chen (2011) Ch. 2–3; Chen, Li, Ho & Wechsler (2006); Han (2026)
`[han2026]` for measured drone micro-Doppler datasets.

---

### 6. Range resolution

$$\Delta r = \frac{c}{2B}$$

Wider bandwidth gives finer range resolution. At $B = 4$ GHz (300 GHz
system default): $\Delta r = 3.75$ cm. At $B = 50$ GHz: $\Delta r = 3$ mm.

**Source:** Richards et al. (2010) Ch. 17; Stove (1992) §4.

---

### 7. Velocity (Doppler) resolution

$$\Delta v = \frac{\lambda}{2\,N_c\,T_c}$$

where $N_c$ is the number of chirps per frame. At 300 GHz with
$N_c = 256$ chirps and $T_c = 100$ µs: $\Delta v = 1.95$ mm/s —
sufficient to resolve pedestrian micro-motion.

**Source:** Rohling & Meinecke (2001); Richards et al. (2010) Ch. 17.

---

### 8. Maximum unambiguous range

$$R_\text{max} = \frac{f_s \cdot c}{2\,(B / T_c)}$$

**Source:** Skolnik (2001) Ch. 3.

---

### 9. Maximum unambiguous velocity

$$v_\text{max} = \frac{\lambda}{4\,T_c}$$

**Source:** Rohling & Meinecke (2001).

---

### 10. Range bin index

$$k_r = \operatorname{round}\!\left(\frac{f_\text{beat}\,N_s}{f_s}\right)$$

Used in `scripts/plot_if.py` to validate the FFT peak against expected
range. Default parameters: $f_\text{beat} \approx 13.34$ kHz for $R = 50$ m,
giving bin $k_r = 1334$ of 5000.

**Source:** Meta et al. (2007); Richards et al. (2010) Ch. 17.

---

### 11. Atmospheric path loss (THz channel model)

$$L_\text{atm}(R) = 2\,\alpha\,R \quad \text{[dB]}$$

where $\alpha$ is the one-way attenuation coefficient in dB/km from
ITU-R P.676. Key values at operating bands:

| Condition | $\alpha$ (dB/km) |
|---|---|
| Clear air (300 GHz) | ~0 |
| Light fog | ~0.4 |
| Heavy fog | ~2 |
| Water vapour absorption peak (~557 GHz) | ~10 |

This term is added to the SNR estimate in the simulator GUI and in
`src/fmcw_generator.cpp`.

**Source:** Federici et al. (2005); Kleine-Ostmann & Nagatsuma (2011);
ITU-R Recommendation P.676.

---

### 12. SNR estimate (simplified radar range equation)

$$\text{SNR} \approx P_\text{tx} - F_n - 20\log_{10}(R) - L_\text{atm}(R)$$

where $P_\text{tx}$ is transmit power in dBm and $F_n$ is the receiver
noise figure in dB. This is a scalar approximation of the full Friis
transmission equation; a complete implementation for Phase 2 will add
antenna gain, RCS, and system losses.

**Source:** Skolnik (2001) Ch. 2 (radar range equation).

---

## Default system parameters (300 GHz THz system)

| Parameter | Value | Derived |
|---|---|---|
| $f_0$ | 300 GHz | $\lambda = 1$ mm |
| $B$ | 4 GHz | $\Delta r = 3.75$ cm |
| $T_c$ | 100 µs | — |
| $f_s$ | 50 MHz | 5000 samples/chirp |
| $N_c$ | 256 chirps | $\Delta v = 1.95$ mm/s |
| Target range | 50 m | Expected bin 1334 |
| Vibration | 0.2 mm @ 200 Hz | Engine idle model |

---

## Bibliography

### A — Foundational FMCW radar theory

These are the primary sources for the signal model and equations implemented
in `fmcw_generator.hpp` / `fmcw_generator.cpp`.

**[stove1992]**  
Stove, A. G. (1992). Linear FMCW radar techniques. *IEE Proceedings F —
Radar and Signal Processing*, 139(5), 343–350.
https://doi.org/10.1049/ip-f-2.1992.0048  
*Canonical derivation of the IF beat frequency equation $f_\text{beat} = (B/T_c)\tau + 2f_0 v/c$ and chirp slope $\mu = B/T_c$. Primary source for equations 1–3.*

**[richards2010]**  
Richards, M. A., Scheer, J. A., & Holm, W. A. (Eds.). (2010).
*Principles of Modern Radar: Basic Principles*. SciTech Publishing.  
*Comprehensive reference for range resolution ($c/2B$), range-Doppler map
construction, and pulse compression. Primary source for equations 1, 3, 6,
7, 10.*

**[skolnik2001]**  
Skolnik, M. I. (2001). *Introduction to Radar Systems* (3rd ed.).
McGraw-Hill.  
*Standard reference for the radar range equation, SNR, noise figure, and
Doppler frequency derivation. Primary source for equations 2, 8, 12.*

**[rohling2001]**  
Rohling, H., & Meinecke, M. M. (2001). Waveform design principles for
automotive radar systems. *Proceedings of the 2001 IEEE Radar Conference*,
1–7. https://doi.org/10.1109/NRC.2001.922968  
*Specific derivation of velocity resolution and maximum unambiguous velocity
for automotive FMCW systems. Primary source for equations 3, 4, 7, 9.*

**[meta2007]**  
Meta, A., Hoogeboom, P., & Ligthart, L. P. (2007). Signal processing for
FMCW SAR. *IEEE Transactions on Geoscience and Remote Sensing*, 45(11),
3519–3532. https://doi.org/10.1109/TGRS.2007.903703  
*Rigorous treatment of the analytic IF signal model and range bin mapping.
Primary source for equations 4 and 10.*

---

### B — Micro-Doppler theory

**[chen2011]**  
Chen, V. C. (2011). *The Micro-Doppler Effect in Radar*. Artech House.  
*Definitive monograph on micro-Doppler signatures. Primary source for the
vibration model in equation 5, limb-motion cadences, and STFT spectrogram
representation.*

**[chen2006]**  
Chen, V. C., Li, F., Ho, S.-S., & Wechsler, H. (2006). Micro-Doppler
effect in radar: phenomenon, model, and simulation study. *IEEE Transactions
on Aerospace and Electronic Systems*, 42(1), 2–21.
https://doi.org/10.1109/TAES.2006.1603402  
*Primary peer-reviewed paper establishing the sinusoidal micro-Doppler
modulation model. Primary source for equation 5.*

---

### C — THz-specific propagation and atmospheric effects

**[federici2005]**  
Federici, J. F., Schulkin, B., Huang, F., Gary, D., Barat, R., Oliveira,
F., & Zimdars, D. (2005). THz imaging and sensing for security applications
— explosives, weapons and drugs. *Semiconductor Science and Technology*,
20(7), S266–S280. https://doi.org/10.1088/0268-1242/20/7/018  
*Atmospheric window positions, water vapour absorption bands, and material
fingerprinting via THz spectroscopy. Primary source for equation 11 context.*

**[kleine2011]**  
Kleine-Ostmann, T., & Nagatsuma, T. (2011). A review of terahertz
communications research. *Journal of Infrared, Millimeter, and Terahertz
Waves*, 32(2), 143–171. https://doi.org/10.1007/s10943-010-9440-2  
*Atmospheric absorption windows (183 GHz, 325 GHz, 557 GHz peaks) and
propagation loss at THz frequencies. Primary source for equation 11 and
the frequency band classifications in the simulator GUI.*

---

### D — Radar simulation frameworks (referenced in ReadTheDocs)

**[schasler2021]**  
Schasler, C., Hoffmann, M., Braunig, J., Ullmann, I., Ebelt, R., &
Vossiek, M. (2021). A Realistic Radar Ray Tracing Simulator for Large
MIMO-Arrays in Automotive Environments. *IEEE Journal of Microwaves*, 1(4),
962–974. https://doi.org/10.1109/JMW.2021.3104722  
*Justification for IF-domain simulation (avoiding carrier-frequency
sampling). Basis for the "simulate the beat signal, not the carrier" design
decision in `fmcw_generator.cpp`.*

**[liu2022]**  
Liu, G., Yang, W., Li, P., Qin, G., Cai, J., Wang, Y., Wang, S., Yue, N.,
& Huang, D. (2022). MIMO Radar Parallel Simulation System Based on CPU/GPU
Architecture. *Sensors*, 22(1), 396.
https://doi.org/10.3390/s22010396

**[martin2022]**  
Martin, M. Y., Winberg, S. L., Gaffar, M. Y. A., & Macleod, D. (2022).
The Design and Implementation of a Ray-tracing Algorithm for Signal-level
Pulsed Radar Simulation Using the NVIDIA® OptiX™ Engine. *Journal of
Communications*, 17(9), 761–768.
https://doi.org/10.12720/jcm.17.9.761-768

**[mercuri2022]**  
Mercuri, M., Russo, P., Glassee, M., Castro, I. D., De Greef, E.,
Rykunov, M., Bauduin, M., Bourdoux, A., Ocket, I., Crupi, F., & Torfs, T.
(2022). Automatic radar-based 2-D localization exploiting vital signs
signatures. *Scientific Reports*, 12, Article 11671.
https://doi.org/10.1038/s41598-022-11671-1

**[srivastava2025]**  
Srivastava, S., Li, J., Mishra, P., Bansal, K., & Bharadia, D. (2025).
A Realistic Radar Simulator for End-to-End Autonomous Driving in CARLA
*(C-Shenron)*. *IEEE VTC 2025*.
https://doi.org/10.1109/VTC2025-Fall65116.2025.11310463

---

### E — Embedded signal processing (referenced in ReadTheDocs)

**[fftw2005]**  
Frigo, M., & Johnson, S. G. (2005). The Design and Implementation of
FFTW3. *Proceedings of the IEEE*, 93(2), 216–231.
https://doi.org/10.1109/JPROC.2003.823119  
*Primary reference for Phase 2 range-Doppler FFT pipeline.*

**[marnach2026]**  
Marnach, A. (2026). Comprehensive signal processing approaches for
non-contact heartbeat detection using 24 GHz FMCW radar. *ARS — Volumes*,
23, 127.  
*Moving-average and moving-RMS embedded filter reference for Phase 2 CA-CFAR
and Phase 3 real-time scheduling.*

---

### F — THz technology — published research (referenced in ReadTheDocs and Teradar white papers)

**[han2026]**  
Han, S. K. (2026). A Time-Synchronized Multi-Sensor drone dataset acquired
from multiple radars and RF receiver. *PMC*.  
*Measured drone micro-Doppler datasets. Cited in the fmcw_theory page for
micro-Doppler target models.*

**[mit_subthz]**  
MIT Microsystems Technology Lab (2019). Sub-terahertz imaging method.
*IEEE Journal of Solid State Circuits*.  
*Chip achieving 32 pixels on 1.2 mm², 4300× sensitivity over standard
pixels. Cited in ADAS white paper for resolution benchmarking.*

**[mustang_acti]**  
Mustang Technology / US Army Research Labs (2017). Terahertz Radar DVE
Imaging — ACTI (Active Covert Terahertz Imager), 300–330 GHz.  
*Demonstrated robust performance in dust storms, fog, and blinding
precipitation. Cited in ADAS and defense white papers.*

**[autosens2025]**  
AutoSens Europe (2025). Terahertz Imaging — A New Category of Automotive
Sensing Advancing ADAS and Autonomous Driving.

**[spie2021]**  
SPIE Photonics Focus (2021). Autonomous Cars Drive Terahertz Research.

**[nrc2022]**  
National Research Council Canada (2022). Sensors for Active Safety and
Driving Automation Systems.

---

## Cross-reference: equations to bibliography

| Equation | Description | Primary source(s) |
|---|---|---|
| 1 — Chirp phase $\phi(t)$ | Linear FM instantaneous phase | `[stove1992]`, `[richards2010]` |
| 2 — Round-trip delay $\tau(t)$ | Moving target delay | `[skolnik2001]`, `[stove1992]` |
| 3 — IF beat frequency $f_\text{beat}$ | Range + Doppler terms | `[stove1992]`, `[rohling2001]`, `[richards2010]` |
| 4 — Analytic IF signal $s_\text{IF}[i]$ | Complex exponential form | `[meta2007]`, `[rohling2001]` |
| 5 — Micro-Doppler vibration $R(t)$ | Sinusoidal phase modulation | `[chen2011]`, `[chen2006]`, `[han2026]` |
| 6 — Range resolution $\Delta r$ | $c/2B$ | `[richards2010]`, `[stove1992]` |
| 7 — Velocity resolution $\Delta v$ | $\lambda / 2N_c T_c$ | `[rohling2001]`, `[richards2010]` |
| 8 — Max unambiguous range $R_\text{max}$ | Nyquist range limit | `[skolnik2001]` |
| 9 — Max unambiguous velocity $v_\text{max}$ | PRF velocity limit | `[rohling2001]` |
| 10 — Range bin index $k_r$ | FFT bin mapping | `[meta2007]`, `[richards2010]` |
| 11 — Atmospheric loss $L_\text{atm}$ | ITU-R P.676 model | `[federici2005]`, `[kleine2011]` |
| 12 — SNR estimate | Simplified range equation | `[skolnik2001]` |
| IF-domain design decision | Why not simulate the carrier | `[schasler2021]` |
| Phase 2 FFT pipeline | FFTW3 range-Doppler | `[fftw2005]` |
| Phase 2 CFAR filter | Embedded CA-CFAR | `[marnach2026]` |

---

## Cross-reference: ReadTheDocs bibliography to foundational theory

The table below maps each entry in the ReadTheDocs bibliography to either a
foundational theory source (Section A–C above) or an application/framework
source (Section D–F), making the knowledge chain fully traceable.

| RTD key | Title (abbreviated) | Category | Maps to |
|---|---|---|---|
| `[liu2022]` | MIMO Radar Parallel Simulation (CPU/GPU) | Framework | Section D |
| `[schasler2021]` | Realistic Radar Ray Tracing Simulator | Framework + IF design | Section D; justifies eq. 4 via `[meta2007]` |
| `[martin2022]` | Ray-tracing with NVIDIA OptiX | Framework | Section D |
| `[mercuri2022]` | Radar-based 2-D localisation | Application | Section D |
| `[srivastava2025]` | C-Shenron CARLA radar simulator | Framework | Section D |
| `[han2026]` | Multi-sensor drone dataset | Measured data | Section F; validates eq. 5 via `[chen2006]` |
| `[marnach2026]` | Heartbeat detection 24 GHz FMCW | Embedded DSP | Section E |
| `[fftw2005]` | FFTW3 design and implementation | Library | Section E |
| `[mit_subthz]` | MIT sub-THz imaging chip | THz hardware | Section F |
| `[mustang_acti]` | Army Research Labs ACTI DVE | THz hardware | Section F |
| `[autosens2025]` | AutoSens THz automotive sensing | Industry | Section F |
| `[spie2021]` | Autonomous cars drive THz research | Industry | Section F |
| `[nrc2022]` | NRC Canada sensors for ADAS | Standards | Section F |

**Foundational theory sources not yet in ReadTheDocs bibliography** (recommended additions):

| Key | Source | Needed for |
|---|---|---|
| `[stove1992]` | Stove, IEE Proc-F 1992 | Equations 1–3 (chirp, delay, beat freq) |
| `[richards2010]` | Richards et al., Principles of Modern Radar | Equations 1, 3, 6, 7, 10 |
| `[skolnik2001]` | Skolnik, Introduction to Radar Systems | Equations 2, 8, 12 |
| `[rohling2001]` | Rohling & Meinecke, IEEE Radar Conf. 2001 | Equations 3, 4, 7, 9 |
| `[meta2007]` | Meta et al., IEEE TGRS 2007 | Equations 4, 10 |
| `[chen2011]` | Chen, The Micro-Doppler Effect in Radar | Equation 5 |
| `[chen2006]` | Chen et al., IEEE TAES 2006 | Equation 5 |
| `[federici2005]` | Federici et al., Semicond. Sci. Tech. 2005 | Equation 11 |
| `[kleine2011]` | Kleine-Ostmann & Nagatsuma, JIMT 2011 | Equation 11, THz band map |

---

*Document generated May 2026. To be maintained as `docs/reference/bibliography.rst` in the repository.*
