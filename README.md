<h1 align="center">
  <b>lbm-imgui</b>
  <br>
</h1>

<h4 align="center">An OpenGL lattice Boltzmann fluid simulation tool using <a href="https://github.com/ocornut/imgui">Dear ImGui</a>.</h4>
<br>

[demo.webm](https://github.com/rafaelanderka/lbm-imgui/assets/44682224/dbe86e76-990f-415e-857f-9b58945ab389)

## Overview

This project provides an OpenGL implementation of the advection-diffusion [Lattice Boltzmann Method](https://en.wikipedia.org/wiki/Lattice_Boltzmann_methods).
The code is written in C++/GLSL and utilizes [Dear ImGui](https://github.com/ocornut/imgui) to provide the UI.

The GLSL shaders were ported from my [WebGL2 LBM fluid simulator](https://github.com/rafaelanderka/lattice-boltzmann-simulator), which I developed for the [BioFM research group](https://www.biofm-research.com/).

## Usage

This project requires OpenGL 3.3+ and can be built with CMake.

1. Clone the repository and submodules:
```sh
git clone --recurse-submodules https://github.com/rafaelanderka/lbm-imgui.git
```
If you have already cloned the repository without the submodules, you can fetch the submodules separately by running:
```sh
git submodule update --init --recursive
```
2. Build the project using CMake in a dedicated `build` directory:
```sh
# Starting in the project root
mkdir build
cd build
cmake ..
make
```

The executable will be placed in `build/bin`.

*Note: The LBM GPU shaders are compiled at runtime for your specific hardware. Thus, additional `shaders` and `resources` folders are created in the `bin` directory to store GLSL shaders and GUI assets needed by the executable.*

## License
This project is licensed under the MIT License.
