MfxVCG
======

*An [OpenMfx](http://openmesheffect.org/) plug-in for [VCGlib](http://vcg.isti.cnr.it/vcglib)*

### About

To summarize, this project enables one to use the geometry filters implemented in MeshLab in other softwares such as Blender. It is an [OpenMfx](http://openmesheffect.org/) plug-in that exposes mesh filters provided by the [VCGlib](http://vcg.isti.cnr.it/vcglib) to any host. VCGlib is the library at the core of [MeshLab](http://www.meshlab.net/), an open source software awarded for its proximity with scientific research on geometry processing. An Open Mesh Effect host is a Digital Content Creation tool able to load OpenMfx plug-ins, for instance [this branch of Blender](https://github.com/eliemichel/OpenMeshEffectForBlender).

### Downloading

This repository contains *submodules* to include the VCGlib repository, so **do not use** the *download zip* button provided by GitHub. To clone the repository using git, run:

```
git clone --recurse-submodules https://github.com/eliemichel/MfxVcg.git
```

If you forgot to use the `--recurse-submodules`, you can get submodules with:

```
git submodule update --init --recursive
```

### Building

This is a standard [CMake](https://cmake.org/) project. Building it consits in running:

```
mkdir build
cd build
cmake ..
```

Alternatively, you can run one of the `build-something` scripts.

Once CMake has run, you can build the project using your favorite IDE or with the following command line in the `build` directory:

```
cmake --build . --config Debug
```

### Running

The output of the build is not an executable. It is an OpenFX plug-in called `MfxVCG.ofx`. It is created within the `build` directory, in `src` or `src/Debug` or `src/Release` or something similar depending on your compiler.

You can open this plug-in in any Open Mesh Effect host, for instance the [Open Mesh Effect for Blender branch](https://github.com/eliemichel/OpenMeshEffectForBlender) using an *OpenMfx modifier*.

### Current status

This plug-in currently is a proof of concept. It provides two example filters: convex hull generation and laplacian filter. To add more filters, look for all the instances of `MAX_PLUGINS`, they come with comments about what to do when increasing it.

### License

This software as a whole is released under the terms of the GPLv3 license. Some of its part uses the Apache 2 license. See [LICENSE.txt](LICENSE.txt) and copyright notices in individual files for details.

```
MfxVCG is an OpenMfx plug-in providing VCGlib-based filters.
Copyright (C) 2019-2021  Élie Michel

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
```
