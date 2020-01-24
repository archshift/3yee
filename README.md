# 3yee Equation Viewer
### "Three-Dee? Three-Yee!"

A 3D parametric equation viewer

## Usage

3yee can render equations parametrized by the spacial variables `u` and `v`, and the time variable `t`.

Edit the parametric equations for `x`, `y`, and `z` in the built-in editor. The viewer will automatically refresh for a valid set of equations, or print an error to the console if compilation fails.

The set of valid functions runnable by the equation editor are provided by GLSL 300 ES. A list of these functions can be found in the [GLSL 300 ES Specification](https://www.khronos.org/registry/OpenGL/specs/es/3.0/GLSL_ES_Specification_3.00.pdf), in Chapter 8 (begins at page 86).

## Compiling

3yee depends on [SDL2](https://www.libsdl.org/download-2.0.php) and OpenGL 3 ES.

After installing these dependencies, create a new build directory, run CMake, and then run Make.

```
mkdir build && cd build
cmake ..
make
```

If you would like to build for running in a web browser, download the emscripten toolchain and run the `./build-emscripten.sh` script.