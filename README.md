<!--
  Authors: Iñaki Zabala.
  -->

# MoorDyn v1 and v2 testbench and packages for U++
**MoorDyn is an advanced mooring model, designed to analyse the behaviour of advanced stationkeeping systems for offshore floating platforms and ships.**

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
<img src="https://github.com/izabala123/BEMRosetta/blob/master/other/md%20resources/platforms-windows_linux-blue.svg" alt="Platforms">
<img src="https://github.com/izabala123/BEMRosetta/blob/master/other/md%20resources/build-passed-success.svg" alt="Build status">
<img src="https://img.shields.io/github/last-commit/izabala123/bemrosetta.svg" alt="Last commit">

This repository includes the source code for [MoorDyn](https://moordyn.readthedocs.io/en/latest/) v1 and v2, packaged for direct use in U++ programs.

It also includes test programs to compare the results obtained with this wrapper with those obtained with [the binaries distributed by NREL](https://github.com/FloatingArrayDesign/MoorDyn/releases).

The versions of MoorDyn included are:
- v1: [1.01.02](https://github.com/FloatingArrayDesign/MoorDyn/releases/tag/v1.01.02)
- v2: [2.3.5](https://github.com/FloatingArrayDesign/MoorDyn/releases/tag/v2.3.5)

This wrapper will be maintained periodically to keep up to date with changes made to MoorDyn.

On the [Releases](https://github.com/BEMRosetta/MoorDyn/releases) page can be found 64-bit and 32-bit versions of DLLs equivalent to the official MoorDyn DLLs, along with the binary test programs.

These test programs take as inputs a configuration of moorings (lines.txt) and a temporary record of platform positions (6 degrees of freedom). With both, they perform a simulation with MoorDyn v1 (MoorDyn_v1_test.exe) or MoorDyn v2 (MoorDyn_v2_test.exe). Remember that each of the executables requires a lines file in the corresponding version. Before launching the binaries, please give them permission for Windows to allow it.
