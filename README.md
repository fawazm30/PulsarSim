# PulsarSim
![CI](https://github.com/fawazm30/PulsarSim/actions/workflows/ci.yml/badge.svg)

A real-time pulsar simulation written in C++17 with raw OpenGL 4.1. It models a rotating neutron star with a tilted magnetic dipole, two beams of particles, spin-down, a live light curve, and pulse audio.

> **Demo:** _add your GIF or video link here_
>
> ![PulsarSim demo](docs/demo.gif)

## Features

- Tilted magnetic dipole that rotates about a spin axis, with two beams (one per pole)
- Particle-based beams with an adjustable bend
- Spin-down: the period slowly grows according to the period derivative (Ṗ)
- Live HUD: period, frequency, Ṗ, surface magnetic field, characteristic age
- Light curve: brightness seen by a fixed observer ("Earth"), updated in real time
- Real pulsar presets: Crab, Vela, B1919+21, J0437-4715
- Playback speed slider (slow motion for fast pulsars)
- Pulse audio that ticks once per rotation (louder when a beam points at the camera)
- Bloom (HDR, bright-pass, Gaussian blur), starfield, orbit camera
- Unit tests for the physics code (Catch2)

## Physics

| Quantity | Formula | Used for |
|---|---|---|
| Frequency | f = 1 / P | HUD |
| Surface magnetic field | B ≈ 3.2×10¹⁹ √(P·Ṗ) G | HUD |
| Characteristic age | τ = P / (2Ṗ) | HUD |
| Spin-down | P(t+dt) = P + Ṗ·dt | Period slowly grows |
| Beam direction | spin axis tilted by α, rotated by phase φ | Beam rendering |
| Light curve | d = cos²α + sin²α·cos φ, pulse = exp(−(1−d)/w²) | Observer brightness |
| Beam width | ∝ 1/√P | Slower pulsars get wider beams |

See [References](#references) for sources.

## Modelling choices and limits

This is a visual model, not a research code. Be aware of these simplifications:

- **Spin-down speedup.** Real spin-down is far too slow to see. A time-scale factor (`5×10⁻⁵ · P / Ṗ`) is applied so the period visibly creeps. The HUD shows this factor.
- **Playback speed.** Real pulsars spin too fast to watch. Slow motion affects only the visible spin. Displayed physics values (P, Ṗ, B, age) stay real.
- **Beam width.** The 1/√P scaling is physically motivated, but its constant is tuned to look good at P = 0.2 s. It is not fitted to data.
- **Tilt angle.** Presets use the tilt slider, since real inclination angles are poorly constrained.
- **Precision.** Period and phase are `double`. With `float`, the tiny per-frame change in P was lost to rounding.
- **Magnetic field.** The B formula assumes a dipole in vacuum, so it is an estimate.

## Preset data

| Pulsar | P (s) | Ṗ (s/s) |
|---|---|---|
| Crab (B0531+21) | 0.0337 (approx.) | 4.2×10⁻¹³ (approx.) |
| Vela (B0833-45) | 0.089328 | 1.24×10⁻¹³ |
| B1919+21 | 1.3373 | 1.349×10⁻¹⁵ |
| J0437-4715 | 0.0057575 | 5.73×10⁻²⁰ |

Compare against the [ATNF Pulsar Catalogue](https://www.atnf.csiro.au/research/pulsar/psrcat/) for authoritative values.

## Controls

| Input | Action |
|---|---|
| Left-drag | Orbit camera |
| Scroll | Zoom |
| `R` | Reset camera |
| `Esc` | Quit |
| Controls panel | Preset, tilt, period (Custom only), playback speed, bend, beam spread, flow speed, pulse width, sound, bloom, stars |

## Build and run (macOS)

Requirements: CMake, a C++17 compiler, GLFW, GLM.

```bash
brew install cmake glfw glm

# Dear ImGui (not committed to this repo)
git clone https://github.com/ocornut/imgui.git external/imgui

# Configure and build
cmake -S . -B build
cmake --build build

# Run from the project root (shader paths are relative)
./build/PulsarSim
```

Notes:
- GLAD and miniaudio live in `external/`.
- After adding a new `.cpp` file, re-run `cmake -S . -B build`.

## Testing

Unit tests cover the `Pulsar` class (no OpenGL needed). Catch2 is downloaded automatically by CMake.

```bash
cmake -S . -B build
cmake --build build --target pulsar_tests
./build/pulsar_tests
```

| Test | What it checks |
|---|---|
| Phase advance | Phase grows by 2π/P · dt |
| Phase wrap | Phase stays in [0, 2π) |
| Spin-down | Period grows by Ṗ · dt · timeScale |
| Zero tilt | Beam points along the spin axis |
| Unit vector | Beam direction has length 1 |
| Tilt angle | Beam stays at the tilt angle from the spin axis at any phase |
| Playback speed | Slows the visible spin, not the physics |
| Accumulation | Tiny per-frame spin-down adds up correctly over 100,000 frames |

Rendering, shaders, and audio are checked manually.

## Project layout

```
src/
  main.cpp              window, main loop, HUD, controls
  Pulsar.h/.cpp         physics: phase, period, spin-down, beam direction
  Shader.h/.cpp         shader loading
  renderer/
    Camera              orbit camera
    Sphere              UV sphere (the star)
    BeamParticles       particle beams
    Starfield           background stars
    Bloom               HDR bloom post-processing
  audio/PulseAudio      pulse sound (miniaudio)
  shaders/              GLSL shaders
tests/
  test_pulsar.cpp       Catch2 unit tests
external/               glad, imgui, miniaudio
```

## Tech stack

C++17 · CMake · OpenGL 4.1 core · GLFW · GLAD · GLM · Dear ImGui · miniaudio · Catch2

## What I learned

- Structuring a C++ project with CMake and third-party libraries
- OpenGL rendering: shaders, additive blending, point sprites, framebuffers
- Post-processing: HDR render targets, bright-pass, ping-pong blur
- Real-time audio hand-off between threads using atomics
- Numerical precision: why `double` matters for slow drift over fast cycles
- Unit testing with Catch2 and CMake
- Turning physics formulas into something you can see and interact with

## Possible next steps

- Glitch events (sudden spin-ups, as seen in Vela)
- Fit preset beam widths to published pulse profiles
- Continuous integration to run the tests on every push

## References

**Pulsar physics**
1. Lorimer, D. R. & Kramer, M. _Handbook of Pulsar Astronomy_. Cambridge University Press, 2004. (Spin-down, characteristic age, and the B ≈ 3.2×10¹⁹ √(P·Ṗ) G estimate; pulse geometry.)
2. Goldreich, P. & Julian, W. H. "Pulsar Electrodynamics." _The Astrophysical Journal_ 157, 869 (1969).
3. Hewish, A., Bell, S. J., Pilkington, J. D. H., Scott, P. F. & Collins, R. A. "Observation of a Rapidly Pulsating Radio Source." _Nature_ 217, 709–713 (1968). (Discovery of the first pulsar, B1919+21.)

**Pulsar data**
4. Manchester, R. N., Hobbs, G. B., Teoh, A. & Hobbs, M. "The ATNF Pulsar Catalogue." _The Astronomical Journal_ 129, 1993 (2005). Online: <https://www.atnf.csiro.au/research/pulsar/psrcat/>
5. Pilia, M. et al. LOFAR pulse profiles of radio pulsars, _Astronomy & Astrophysics_ (2016). (B1919+21 period data.) _Check the exact citation before publishing._
6. Fermi-LAT Collaboration, Third Pulsar Catalog (3PC). (J0437-4715 spin parameters.)
7. arXiv:2506.16687. (Vela period derivative.)
8. Vela Pulsar, Wikipedia. (Vela period.)

**Graphics and libraries**
9. de Vries, J. _Learn OpenGL_: Bloom and HDR chapters. <https://learnopengl.com>
10. GLFW: <https://www.glfw.org> · GLAD: <https://glad.dav1d.de> · GLM: <https://github.com/g-truc/glm>
11. Dear ImGui: <https://github.com/ocornut/imgui>
12. miniaudio: <https://miniaud.io>
13. Catch2: <https://github.com/catchorg/Catch2>