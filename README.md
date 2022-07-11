# About

This tool allows the user to load a protein *(.pdb)* and a corresponding trajectory *(.xtc)*, 
visualize them with different methods and compute their Residue Surface Proximity. 

![preview](https://github.com/UniTuebingen-BDVA/AminoAcidVis/blob/master/doc/images/preview/preview-alpha3.jpg?raw=true "Tool preview")

# Requirements
OpenGL 4.5 compatible hardware and driver.

# Setup
* Clone the repository, including all submodules: `clone --recurse-submodules  http://github.com/UniTuebingen-BDVA/AminoAcidVis.git`.
* Install Qt.
  * The tested version is Qt 5.15.2 with MinGW 8.1.0. Install the approprriate Qt version and the compiler that comes with it.
* Add the `QT_PLUGIN_PATH` to your environment variables (system or run-configuration), e.g.,`QT_PLUGIN_PATH=C:\Qt\5.15.2\mingw81_64\plugins`.
* Append the compiler binary directory to the `PATH`, e.g., `PATH=%PATH%;C:\Qt\5.15.2\mingw81_64\bin`.
* Configure the project:
  * Run CMake, it will only configure the XTC build.
  * Build the XTC library.
  * Run CMake again to finish the configuration.

#### Example Toolchain MinGW in CLion
![preview](https://github.com/UniTuebingen-BDVA/AminoAcidVis/blob/master/doc/images/setup/mingw_toolcahin_clion.png?raw=true "Tool preview")

# Used external libraries
* [glew-cmake](https://github.com/Perlmint/glew-cmake) - The OpenGL Extension Wrangler Library 
* [glm](https://github.com/g-truc/glm) - OpenGL Mathematics
* [Parts of mdtraj](https://github.com/mdtraj/mdtraj) - A modern, open library for the analysis of molecular dynamics trajectories http://www.mdtraj.org
* [Cartesian wireframe](https://github.com/rreusser/glsl-solid-wireframe) - draw wireframes on a triangular mesh using a fragment shader
