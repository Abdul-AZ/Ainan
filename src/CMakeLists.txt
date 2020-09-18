
set(SOURCES_LIST
    "main.cpp"
    "pch.cpp"

    "editor/AppStatusWindow.h"         "editor/AppStatusWindow.cpp"
    "editor/Camera.h"                  "editor/Camera.cpp"
    "editor/CurveEditor.h"             "editor/CurveEditor.cpp"
    "editor/Editor.h"                  "editor/Editor.cpp"
    "editor/EditorPreferences.h"       "editor/EditorPreferences.cpp"
    "editor/EditorStyles.h"            "editor/EditorStyles.cpp"
    "editor/ExportCamera.h"            "editor/ExportCamera.cpp"
    "editor/Gizmo.h"                   "editor/Gizmo.cpp"
    "editor/Grid.h"                    "editor/Grid.cpp"
    "editor/ImGuiWrapper.h"            "editor/ImGuiWrapper.cpp"
    "editor/InputManager.h"            "editor/InputManager.cpp"
    "editor/InterpolationSelector.h"   "editor/InterpolationSelector.cpp"
    "editor/ParticleCustomizer.h"      "editor/ParticleCustomizer.cpp"
    "editor/ViewportWindow.h"          "editor/ViewportWindow.cpp"
    "editor/Window.h"                  "editor/Window.cpp"

    "editor/customizers/ColorCustomizer.h"      "editor/customizers/ColorCustomizer.cpp"
    "editor/customizers/ForceCustomizer.h"      "editor/customizers/ForceCustomizer.cpp"
    "editor/customizers/LifetimeCustomizer.h"   "editor/customizers/LifetimeCustomizer.cpp"
    "editor/customizers/NoiseCustomizer.h"      "editor/customizers/NoiseCustomizer.cpp"
    "editor/customizers/ScaleCustomizer.h"      "editor/customizers/ScaleCustomizer.cpp"
    "editor/customizers/TextureCustomizer.h"    "editor/customizers/TextureCustomizer.cpp"
    "editor/customizers/VelocityCustomizer.h"   "editor/customizers/VelocityCustomizer.cpp"

    "environment/Environment.h"
    "environment/EnvironmentObjectInterface.h" "environment/EnvironmentObjectInterface.cpp"
    "environment/EnvLoad.cpp"
    "environment/EnvSave.cpp"
    "environment/LitSprite.h"                  "environment/LitSprite.cpp"
    "environment/ParticleSystem.h"             "environment/ParticleSystem.cpp"
    "environment/RadialLight.h"                "environment/RadialLight.cpp"
    "environment/SpotLight.h"                  "environment/SpotLight.cpp"
    "environment/Sprite.h"                     "environment/Sprite.cpp"

    "file/AssetManager.h"     "file/AssetManager.cpp"
    "file/FileBrowser.h"      "file/FileBrowser.cpp"
    "file/FolderBrowser.h"    "file/FolderBrowser.cpp"
    "file/SaveItemBrowser.h"  "file/SaveItemBrowser.cpp"

    "renderer/Renderer.h"         "renderer/Renderer.cpp"
    "renderer/RendererAPI.h"
    "renderer/RendererContext.h"
    "renderer/VertexBuffer.h"
    "renderer/IndexBuffer.h"
    "renderer/UniformBuffer.h"
    "renderer/ShaderProgram.h"
    "renderer/Texture.h"
    "renderer/Rectangle.h"
    "renderer/FrameBuffer.h"
    "renderer/Image.h"            "renderer/Image.cpp"
    "renderer/RenderSurface.h"    "renderer/RenderSurface.cpp"

    "renderer/opengl/OpenGLFrameBuffer.h"      "renderer/opengl/OpenGLFrameBuffer.cpp"
    "renderer/opengl/OpenGLIndexBuffer.h"      "renderer/opengl/OpenGLIndexBuffer.cpp"
    "renderer/opengl/OpenGLRendererAPI.h"      "renderer/opengl/OpenGLRendererAPI.cpp"
    "renderer/opengl/OpenGLRendererContext.h"  "renderer/opengl/OpenGLRendererContext.cpp"
    "renderer/opengl/OpenGLShaderProgram.h"    "renderer/opengl/OpenGLShaderProgram.cpp"
    "renderer/opengl/OpenGLTexture.h"          "renderer/opengl/OpenGLTexture.cpp"
    "renderer/opengl/OpenGLUniformBuffer.h"    "renderer/opengl/OpenGLUniformBuffer.cpp"
    "renderer/opengl/OpenGLVertexBuffer.h"     "renderer/opengl/OpenGLVertexBuffer.cpp"

    "vendor/glad/glad.h"            "vendor/glad/glad.cpp"
    "vendor/stb/stb_image.h"        "vendor/stb/stb_image.cpp"
    "vendor/stb/stb_image_write.h"  "vendor/stb/stb_image_write.cpp"
    )

set(IMGUI_SOURCE_FILES
    "../submodules/imgui/imgui.cpp"
    "../submodules/imgui/imgui_demo.cpp"
    "../submodules/imgui/imgui_draw.cpp"
    "../submodules/imgui/imgui_widgets.cpp"
    "../submodules/imgui/misc/cpp/imgui_stdlib.cpp"
    "../submodules/imgui/imgui_internal.h"
    )

list(APPEND DEFINITIONS_LIST
    IMGUI_DEFINE_MATH_OPERATORS=1
    )

if(WIN32)

    list(APPEND DEFINITIONS_LIST
        PLATFORM_WINDOWS=1
        )

    list(APPEND SOURCES_LIST
        "windows/WinMain.cpp"
        "renderer/d3d11/D3D11FrameBuffer.h"      "renderer/d3d11/D3D11FrameBuffer.cpp"
        "renderer/d3d11/D3D11IndexBuffer.h"      "renderer/d3d11/D3D11IndexBuffer.cpp"
        "renderer/d3d11/D3D11RendererAPI.h"      "renderer/d3d11/D3D11RendererAPI.cpp"
        "renderer/d3d11/D3D11RendererContext.h"  "renderer/d3d11/D3D11RendererContext.cpp"
        "renderer/d3d11/D3D11ShaderProgram.h"    "renderer/d3d11/D3D11ShaderProgram.cpp"
        "renderer/d3d11/D3D11Texture.h"          "renderer/d3d11/D3D11Texture.cpp"
        "renderer/d3d11/D3D11UniformBuffer.h"    "renderer/d3d11/D3D11UniformBuffer.cpp"
        "renderer/d3d11/D3D11VertexBuffer.h"     "renderer/d3d11/D3D11VertexBuffer.cpp"
        )

    
    list(APPEND STATIC_LIBRARIES
        "d3d11.lib"
        "d3dcompiler.lib"
        )

endif()

set(INCLUDE_LIST
    "."
    "../submodules/imgui"
    "../submodules/imgui/examples"
    "../submodules/imgui/misc/cpp" 
    "../submodules/glm" 
    "vendor"
    )


add_executable(Core WIN32 ${SOURCES_LIST} ${IMGUI_SOURCE_FILES})
target_precompile_headers(Core PRIVATE "pch.h")
target_include_directories(Core PRIVATE ${INCLUDE_LIST})
target_compile_definitions(Core PRIVATE ${DEFINITIONS_LIST})
target_link_libraries(Core ${STATIC_LIBRARIES})
set_target_properties(Core PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

##cross-compile shaders to hlsl on windows
if(WIN32)
    while(GLSL_SHADERS)

        list(SUBLIST GLSL_SHADERS 0 6 CURRENT_SHADERS)
        list(REMOVE_AT GLSL_SHADERS 0 1 2 3 4 5)

        set(GLSLCC_FLAGS
            -I ${GLSL_SHADERS_DIR}
            -b
            -l hlsl
            )

        add_custom_command(TARGET Core PRE_BUILD
                           COMMAND ${GLSLCC_BIN_PATH} ${CURRENT_SHADERS} ${GLSLCC_FLAGS}
                           )
    endwhile()
endif()