# 3yee Equation Viewer
### "Three-Dee? Three-Yee!"

A 3D parametric equation viewer

## Usage

3yee can render equations parametrized by the spacial variables `u` and `v`, and the time variable `t`.

Edit the parametric equations for `x`, `y`, and `z` in the built-in editor. The viewer will automatically refresh for a valid set of equations, or print an error to the console if compilation fails.

The set of valid functions runnable by the equation editor are provided by GLSL 330. A list of these functions can be found in the [GLSL 330 Specification](https://www.khronos.org/registry/OpenGL/specs/gl/GLSLangSpec.3.30.pdf), in Chapter 8 (begins at page 80).

## Compiling

3yee depends on [SDL2](https://www.libsdl.org/download-2.0.php), OpenGL 3.3, and [GLM](https://glm.g-truc.net/0.9.9/index.html).

After installing these dependencies, create a new build directory, run CMake, and then run Make.

```
mkdir build && cd build
cmake ..
make
```