# Ainan
Ainan is a visual effects and motion graphics making application mainly targeting game developers.

# Supported Platforms

At the moment it only runs on windows.
There are plans for linux and mac support.

# Building the Project
The project is built using Premake5 (see [Premake](https://github.com/premake/premake-core)). Tested on Visual Studio 2019 but should work on other IDE's.

Steps on building in Visual Studio:

1. `premake5 vs2019` in your project directory.
2. Open visual studio solution file and build.

Similar steps can be made to build for systems other than windows (other than that you have to get a version of premake that is specific for your platform, basically grab the latest release of [Premake](https://github.com/premake/premake-core) for your OS). 

# Contribute
There are no strict rules for contributing, feel free to open an issue for anything! I will be sure to respond to issues and pull requests quickly.

Also, feel free to contact me through Email.

# Examples
![](github/appGIF.gif)
![](github/appScreenshot.PNG)

# Libraries used
* GLFW
* Dear ImGui
* glad
* stb image
* GLM
* Premake5
* nlohmann/json
* FastNoise
