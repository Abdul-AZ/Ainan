![CircleCI](https://img.shields.io/circleci/build/gh/AbdullrahmanAlzeidi/Ainan/master?label=All%20Platform%20Build&logo=circleci)
# Ainan
Ainan is a visual effects and motion graphics making application mainly targeting game developers.

# Supported Platforms

The project current only works on Windows. Experimental support for archlinux is under development and is integrated in CI.

There are plans to support more linux distributions and possiby even mac at some point. However this is not the focus currently.

# Building the Project
The project is built using [CMake](https://cmake.org/) with the usual CMake workflow.

Example building steps on Windows with CMake installed using Visual Studio 2019 and the Command Line on the repository directory:
1. `git clone https://github.com/AbdullrahmanAlzeidi/Ainan.git --recursive`
2. `cd Ainan`
3. `mkdir build`
4. `cd build`
5. `cmake "Visual Studio 16 2019" ..`
6. open Ainan.sln and build

# Contribute
There are no strict rules for contributing, feel free to open an issue for anything! I will be sure to respond to issues and pull requests quickly.

Also, feel free to contact me through Email.

# Examples
![](github/appGIF.gif)
![](github/appScreenshot.PNG)

# Libraries used
* [GLFW](https://www.glfw.org/)
* [Dear ImGui](https://github.com/ocornut/imgui)
* [ImGuizmo](https://github.com/CedricGuillemet/ImGuizmo)
* [glad](https://github.com/Dav1dde/glad)
* [stb](https://github.com/nothings/stb)
* [GLM](https://github.com/g-truc/glm)
* [nlohmann/json](https://github.com/nlohmann/json)
* [FastNoise](https://github.com/Auburn/FastNoise)
* [glslcc](https://github.com/septag/glslcc)
* [spdlog](https://github.com/gabime/spdlog)
* [libav](https://libav.org)
* [assimp](https://www.assimp.org/)
