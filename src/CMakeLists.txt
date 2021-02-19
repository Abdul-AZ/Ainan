
set(SOURCES_LIST
    "main.cpp"
    "pch.h"  "pch.cpp"
    "Log.h"  "Log.cpp"

    "editor/AppStatusWindow.h"         "editor/AppStatusWindow.cpp"
    "editor/Camera.h"                  "editor/Camera.cpp"
    "editor/CurveEditor.h"             "editor/CurveEditor.cpp"
    "editor/Editor.h"                  "editor/Editor.cpp"
    "editor/EditorPreferences.h"       "editor/EditorPreferences.cpp"
    "editor/EditorStyles.h"            "editor/EditorStyles.cpp"
    "editor/Exporter.h"                "editor/Exporter.cpp"
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

    "renderer/Renderer.h"            "renderer/Renderer.cpp"
    "renderer/RendererAPI.h"
    "renderer/RendererContext.h"
    "renderer/RenderCommand.h"
    "renderer/RenderCommandQueue.h"  "renderer/RenderCommandQueue.cpp"
    "renderer/VertexBuffer.h"        "renderer/VertexBuffer.cpp"
    "renderer/IndexBuffer.h"
    "renderer/UniformBuffer.h"       "renderer/UniformBuffer.cpp"
    "renderer/ShaderProgram.h"       "renderer/ShaderProgram.cpp"
    "renderer/Texture.h"             "renderer/Texture.cpp"
    "renderer/Rectangle.h"
    "renderer/Framebuffer.h"         "renderer/Framebuffer.cpp"
    "renderer/Image.h"               "renderer/Image.cpp"
    "renderer/RenderSurface.h"       "renderer/RenderSurface.cpp"

    "renderer/opengl/OpenGLRendererAPI.h"      "renderer/opengl/OpenGLRendererAPI.cpp"
    "renderer/opengl/OpenGLRendererContext.h"  "renderer/opengl/OpenGLRendererContext.cpp"

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

    "../submodules/ImGuizmo/ImGuizmo.cpp"
    )

list(APPEND DEFINITIONS_LIST
    IMGUI_DEFINE_MATH_OPERATORS=1
    )

set(SHADER_FILES)
set(GENERATED_SHADER_FILES)

if(WIN32)

    list(APPEND DEFINITIONS_LIST
        PLATFORM_WINDOWS=1
        )

    list(APPEND SOURCES_LIST
        "windows/WinMain.cpp"
        "renderer/d3d11/D3D11RendererAPI.h"      "renderer/d3d11/D3D11RendererAPI.cpp"
        "renderer/d3d11/D3D11RendererContext.h"  "renderer/d3d11/D3D11RendererContext.cpp"
        )

    
    list(APPEND STATIC_LIBRARIES
        "d3d11.lib"
        "d3dcompiler.lib"
        "DXGI.lib"
        )

    ##add commands to cross-compile shaders to hlsl
    while(GLSL_SHADERS)

        list(SUBLIST GLSL_SHADERS 0 6 CURRENT_SHADERS)
        list(REMOVE_AT GLSL_SHADERS 0 1 2 3 4 5)

        list(GET CURRENT_SHADERS 1 CURRENT_VERTEX_SHADER_PATH)
        list(GET CURRENT_SHADERS 3 CURRENT_FRAGMENT_SHADER_PATH)
        list(GET CURRENT_SHADERS 5 CURRENT_SHADER_OUTPUT_PATH)

        #set output extension correctly as specified in glslcc
        string(REGEX REPLACE "[.]cso$" "_vs.cso" CURRENT_VERTEX_SHADER_OUTPUT ${CURRENT_SHADER_OUTPUT_PATH})
        string(REGEX REPLACE "[.]cso$" "_fs.cso" CURRENT_FRAGMENT_SHADER_OUTPUT ${CURRENT_SHADER_OUTPUT_PATH})

        list(APPEND SHADER_FILES ${CURRENT_VERTEX_SHADER_PATH} ${CURRENT_FRAGMENT_SHADER_PATH})
        list(APPEND GENERATED_SHADER_FILES ${CURRENT_VERTEX_SHADER_OUTPUT} ${CURRENT_FRAGMENT_SHADER_OUTPUT})

        set(GLSLCC_FLAGS
            --include-dirs=${GLSL_SHADERS_DIR}
            --bin #this means compile to d3d11 hlsl bytecode
            --lang=hlsl
            )

        add_custom_command(OUTPUT ${CURRENT_VERTEX_SHADER_OUTPUT} ${CURRENT_FRAGMENT_SHADER_OUTPUT}
                           DEPENDS ${CURRENT_VERTEX_SHADER_PATH} ${CURRENT_FRAGMENT_SHADER_PATH}
                           COMMAND ${GLSLCC_BIN_PATH} ${CURRENT_SHADERS} ${GLSLCC_FLAGS}
                           )
    endwhile()

    #generate visual studio filters
    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES_LIST})
    source_group("shaders" FILES ${SHADER_FILES})
    source_group("compiled shaders" FILES ${GENERATED_SHADER_FILES})
endif()


set(INCLUDE_LIST
    "."
    "../submodules/imgui"
    "../submodules/imgui/examples"
    "../submodules/imgui/misc/cpp" 
    "../submodules/glm" 
    "../submodules/spdlog/include"
    "../submodules/ImGuizmo"
    "vendor"
    )


add_executable(Core WIN32 ${SOURCES_LIST} ${IMGUI_SOURCE_FILES} ${SHADER_FILES} ${GENERATED_SHADER_FILES})
set_source_files_properties(${GENERATED_SHADER_FILES} PROPERTIES GENERATED TRUE)
target_precompile_headers(Core PRIVATE "pch.h")
target_include_directories(Core PRIVATE ${INCLUDE_LIST})
target_compile_definitions(Core PRIVATE ${DEFINITIONS_LIST})
target_link_libraries(Core ${STATIC_LIBRARIES})
set_target_properties(Core PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
