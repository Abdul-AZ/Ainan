# Ainan
Ainan is an app developed in c++ that uses graphic API's like OpenGL (support for other API's is planned) to make cool graphics easily and export them.
This app is meant to be used by game artists and hobbyists.

At the moment it only runs on windows.

There are plans for linux and mac support (that is why I'm looking for contributers with linux/mac see contribute)

# Building the Project
The project is built using Premake5 (see [Premake](https://github.com/premake/premake-core)). I only tested it on windows (tested on MSVC2017) as of now.

Steps on building in windows:

1. Open commandline window on project directory.
2. Type "premake5 vsYOUR_VISUAL_STUDIO_VERSION", so for me it was "premake5 vs2017".
3. Open the newly generated sln file from the build folder.
4. Build and run the executable in VisualStudio.

Similar steps can be made to build for systems other than windows (other than that you have to get a version of premake that is specific for your platform, basically grab the latest release of [Premake](https://github.com/premake/premake-core) for your OS) but I do not guarantee they will work for now. 

# Documentation
Still to be made, I'm planning to slowly build it through time. It will be available through the wiki page of this repository.

# Examples
![](github/appGIF.gif)
![](github/appScreenshot.PNG)

# Contribute
Every contribution is accepted however we will have a discussion on contributions that add new features. But other contributions like bug fixes, documentation and code cleaning/refactoring are more than welcomed.

Also do not be afraid to open an issue on the slightest issue/question/bug or whatever you have.

# Libraries used
* GLFW
* Dear ImGui
* glad
* stb image
* GLM
* Premake5
* nlohmann/json
