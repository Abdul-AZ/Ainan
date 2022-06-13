target_sources(Core PUBLIC

    "AppStatusWindow.h"         "AppStatusWindow.cpp"
    "CurveEditor.h"             "CurveEditor.cpp"
    "Editor.h"                  "Editor.cpp"
    "EditorPreferences.h"       "EditorPreferences.cpp"
    "EditorStyles.h"            "EditorStyles.cpp"
    "Exporter.h"                "Exporter.cpp"
    "Grid.h"                    "Grid.cpp"
    "ImGuiWrapper.h"            "ImGuiWrapper.cpp"
    "InputManager.h"            "InputManager.cpp"
    "InterpolationSelector.h"   "InterpolationSelector.cpp"
    "ParticleCustomizer.h"      "ParticleCustomizer.cpp"
    "Profiler.h"                "Profiler.cpp"
    "ViewportWindow.h"          "ViewportWindow.cpp"
    "Window.h"                  "Window.cpp"
    "EditorCamera.h"            "EditorCamera.cpp"
    "ThreadPool.h"              "ThreadPool.cpp"


    #No CMakeLists.txt in the customizers folder because it is going to be refactored(and probably deleted) soon
    "customizers/ColorCustomizer.h"      "customizers/ColorCustomizer.cpp"
    "customizers/ForceCustomizer.h"      "customizers/ForceCustomizer.cpp"
    "customizers/LifetimeCustomizer.h"   "customizers/LifetimeCustomizer.cpp"
    "customizers/NoiseCustomizer.h"      "customizers/NoiseCustomizer.cpp"
    "customizers/ScaleCustomizer.h"      "customizers/ScaleCustomizer.cpp"
    "customizers/TextureCustomizer.h"    "customizers/TextureCustomizer.cpp"
    "customizers/VelocityCustomizer.h"   "customizers/VelocityCustomizer.cpp"
)