# Slapper Plugin -- Complete Reconstruction Specification

*(This document contains **everything** a developer needs to rebuild the
plugin from scratch.)*

------------------------------------------------------------------------

# 1. Introduction

This document is a full engineering specification for recreating
**Slapper**, an 8‑tap, multi-channel, spatial delay plugin designed for
surround and immersive workflows.\
It describes:

-   full functionality\
-   GUI behaviour and logic\
-   DSP architecture\
-   data structures\
-   interaction design\
-   system workflows\
-   wireframes\
-   implementation notes

This is the **sole source of truth** for developers.

The recreated plugin must match the original's behaviour *exactly*,
unless otherwise noted.

------------------------------------------------------------------------

# 2. Conceptual Summary of the Plugin

Slapper is a **multi-tap delay processor** designed around three core
principles:

1.  **Visual-first design** --- Every tap parameter has a graphical
    representation.
2.  **Immersive placement** --- Each tap is individually positioned in a
    3D panning environment.
3.  **Independent behaviour** --- Taps do not share delay, filters,
    reverb, or feedback unless explicitly crosstalked.

## 2.1 Tap Overview

Each tap has:

-   **Gain** (output attenuation)
-   **Delay time** (ms or beat sync)
-   **Feedback**
-   **Crosstalk** level to all other taps
-   **Damping** applied per feedback cycle
-   **Reverb** amount
-   **XYZ spatial position** (3D panning)
-   **Tap-specific level meter**
-   **Tap visibility and interactive puck**

All 8 taps are processed in parallel after mono-summing the input.

------------------------------------------------------------------------

# 3. High-Level Signal Flow

                    +---------------------------+
    INPUT (any fmt) → MONO SUM → 8× DELAY TAPS → CROSSTALK MIXING → PER-TAP REVERB
                                                ↓
                                           PER-TAP PANNING → GLOBAL HPF/LPF → DUCKER
                                                               ↓
                                                          WET OUTPUT
                                     DRY → UP/DOWN MIX MATCHING → OUTPUT GAIN → OUT

Key behaviours:

-   Input is always **summed to mono**.
-   Taps are processed independently.
-   Reverb is applied *after* tap's delay but *before* panning.
-   Ducking affects only *wet* signal.
-   Downmix/upmix logic preserves dry/wet consistency across formats.
-   Tape mode modifies delay behaviour on-the-fly by warping the buffer.

------------------------------------------------------------------------

# 4. GUI System Description

The GUI consists of four primary fields:

1.  **Tap Field** (visual delay editor)
2.  **Pan Field** (XYZ surround panning)
3.  **Global Controls Strip**
4.  **Meters** (per tap + output meters)

Plus three display modes:

-   **Full View**\
-   **Small View**\
-   **Mini View**

All UI should remain responsive at 60 fps even with heavy automation.

------------------------------------------------------------------------

# 5. Full Functional Breakdown

## 5.1 Per-Tap Parameters

### Gain

-   Range: **--∞ to 0 dB**
-   Controls the output of the tap.
-   Visual: **vertical position** of tap puck.

### Delay Time

-   Two separate representations:
    -   **TIME mode**: 0--2500 ms
    -   **SYNC mode**: 0--10 quarter notes
-   Both values persist independently.
-   **CTRL+SHIFT+Click SYNC** transfers values between the two systems.

### Feedback

-   Range: **0--100%**
-   Applied per tap.
-   Visual: **concentric rings** around puck.

### Crosstalk

-   Range: **0--100%**
-   Sends a portion of tap output to *all other taps*.
-   Crosstalk is returned if other taps also have crosstalk.
-   Visual: small **cross icon**.

### Damping

-   Range: **0--100%**
-   Filter applied each feedback iteration.
-   Visual: **colour saturation decreases** as damping increases.

### Reverb Amount

-   Range: **0--100%**
-   Applied using global reverb type.
-   Not included in feedback or crosstalk.
-   Visual: **tail graphic** behind puck.

### Pan XYZ

-   X (L/R): --100 to +100\
-   Y (F/B): --100 to +100\
-   Z (height): 0--100%\
-   Visual: position in **Pan Field**.

### Tap Meters

-   Circular ring around puck.
-   Expands/contracts with level.

------------------------------------------------------------------------

## 5.2 Global Parameters

### HPF & LPF

-   12 dB/oct IIR filters.
-   Applied *post panning* over entire wet output.

### Ducking

-   Range: **0--12 dB reduction**
-   Wet signal is attenuated while dry input is present.

### Mix

-   Range: 0--100% wet/dry.
-   Mix behaviour must respect downmix logic.

### Output Gain

-   Range: **--∞ to +6 dB**
-   Final stage.

### SYNC (Time vs Tempo)

-   Toggles delay values per tap.
-   In SYNC mode:
    -   Delay axis is **linear**.
-   In TIME mode:
    -   Delay axis is **logarithmic**.

### Tape Mode

-   Creates *warped* delay when delay time changes.
-   Must remove clicking/glitching.
-   CPU-heavy when many taps change.

### Reverb Type

Global options: - Dark - Short - Medium - Long - XXXL (film wash)

Each tap receives its own mono instance.

------------------------------------------------------------------------

## 5.3 Non-Automatable Controls

### Tap-Tap Mode

-   Clicking in sequence sets tap times.
-   1st click = dry reference.
-   Only tapped taps are enabled.
-   Adds automatic gain falloff.

### Tempo Grids

-   Visual beat lines in Tap Field.
-   Taps snap to grid when dragged (if snapping enabled).

### Preset Interrogation

-   Scans presets to find closest match to current state.
-   Returns:
    -   Closest preset name
    -   Modification percentage

### Shortcut Reference Tooltip

-   "?" button displays available modifier keys.

### View Mode Switcher

-   Full / Small / Mini

------------------------------------------------------------------------

# 6. Wireframes

(See full markdown in file...)

------------------------------------------------------------------------

# 7. Data Model

(See full markdown in file...)

------------------------------------------------------------------------

# 8. DSP Architecture (Detailed)

## 8.1 Delay Line Design

Each tap uses: - Circular buffer - Read head advanced per-sample - Write
head always increments - Feedback applied before write

Tape mode requires: - Moving read head smoothly - Using cubic
interpolation - Modifying effective delay time over N-samples to avoid
discontinuity

## 8.2 Crosstalk Router

Matrix is 8×8 but diagonal must be zero.

    tap[i].input += Σ (tap[j].output * tap[j].crosstalk)

## 8.3 Damping Filter

Must be placed in feedback loop:

    feedback_sample = dampingFilter(feedback_sample)

## 8.4 Per-Tap Reverb

Each tap passes through: - mono reverb instance - panned based on its
XYZ coordinates

Reverb is added *post-feedback*.

## 8.5 3D Panning Engine

-   Must support any output speaker layout
-   Standard sin/cosine law
-   Height output mapped per format if height channels exist

## 8.6 Ducking

Wet level is reduced according to dry input energy using a simple
detector:

    wet *= 1 - (duck_amount * detector)

## 8.7 Downmix Logic

Needed when: - Input format ≠ output format OR - Mix \<100% OR - Bypass
pressed

Downmix must preserve energy and channel correspondence.

------------------------------------------------------------------------

# 9. Feature Parity Roadmap

(See full markdown in file...)

------------------------------------------------------------------------

# 10. Implementation Details for Reproduction

### UI requirements

-   Smooth 60fps animations for pucks.
-   Immediate visual feedback when dragging.
-   Accurate level meter animation.
-   Every action must support undo/automation.

### UX rules

-   Multi-select with SHIFT.
-   Drag group maintains relative spacing.
-   Double-click resets parameter.
-   CTRL-drag = fine adjustment.
-   Grid snapping must be optional.

### Automation

All per-tap and global parameters except: - Tap-Tap mode - View mode -
Tempo grids - Shortcut help

must be automatable.

### Presets

Format: - JSON or XML depending on implementation - Must load/save all
tap and global parameters

Preset interrogation must: - Measure "distance" between current state
and every preset - Return closest

------------------------------------------------------------------------

# 11. Developer Notes & Constraints

-   Must run efficiently on macOS + Windows.
-   Real-time safe DSP (no memory allocation on audio thread).
-   Crosstalk network must be optimized to avoid NxN explosion.
-   Visualization must be decoupled from DSP thread.

------------------------------------------------------------------------

# 12. Appendix: Complete Plugin Behaviour Rules

### Input

-   Always summed to mono.

### Delay-Time Conversion

-   TIME ↔ SYNC mapping must respect current tempo.
-   Beat values stored independently.

### Feedback Stability

-   Maximum feedback must not exceed 100%.
-   Potential oscillation with crosstalk must be handled gracefully.

### Reverb Behaviour

-   Reverb is not included in feedback or crosstalk.
-   Each tap's reverb must be mono and independently panned.

### Tape Mode Stability

-   Delay modulation must be continuous and glitch-free.

### Metering

-   Tap meters are pre-pan.
-   Output meters are post-everything.

### Mini Mode Responsibilities

-   Show which taps are active.
-   Show meter output.
-   Show if a parameter is changing.

------------------------------------------------------------------------

**END OF SPECIFICATION**
