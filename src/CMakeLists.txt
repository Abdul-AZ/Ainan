
set(SOURCES_LIST
    "main.cpp"
    "pch.cpp"

    "editor/AppStatusWindow.cpp"
    "editor/Camera.cpp"
    "editor/CurveEditor.cpp"
    "editor/Editor.cpp"
    "editor/EditorPreferences.cpp"
    "editor/EditorStyles.cpp"
    "editor/ExportCamera.cpp"
    "editor/Gizmo.cpp"
    "editor/Grid.cpp"
    "editor/ImGuiWrapper.cpp"
    "editor/InputManager.cpp"
    "editor/InterpolationSelector.cpp"
    "editor/ParticleCustomizer.cpp"
    "editor/ViewportWindow.cpp"
    "editor/Window.cpp"
    "editor/customizers/ColorCustomizer.cpp"
    "editor/customizers/ForceCustomizer.cpp"
    "editor/customizers/LifetimeCustomizer.cpp"
    "editor/customizers/NoiseCustomizer.cpp"
    "editor/customizers/ScaleCustomizer.cpp"
    "editor/customizers/TextureCustomizer.cpp"
    "editor/customizers/VelocityCustomizer.cpp"

    "environment/EnvironmentObjectInterface.cpp"
    "environment/EnvLoad.cpp"
    "environment/EnvSave.cpp"
    "environment/LitSprite.cpp"
    "environment/ParticleSystem.cpp"
    "environment/RadialLight.cpp"
    "environment/SpotLight.cpp"
    "environment/Sprite.cpp"

    "file/AssetManager.cpp"
    "file/FileBrowser.cpp"
    "file/FolderBrowser.cpp"
    "file/SaveItemBrowser.cpp"

    "renderer/Renderer.cpp"
    "renderer/Image.cpp"
    "renderer/RenderSurface.cpp"
    "renderer/opengl/OpenGLFrameBuffer.cpp"
    "renderer/opengl/OpenGLIndexBuffer.cpp"
    "renderer/opengl/OpenGLRendererAPI.cpp"
    "renderer/opengl/OpenGLRendererContext.cpp"
    "renderer/opengl/OpenGLShaderProgram.cpp"
    "renderer/opengl/OpenGLTexture.cpp"
    "renderer/opengl/OpenGLUniformBuffer.cpp"
    "renderer/opengl/OpenGLVertexBuffer.cpp"

    "vendor/glad/glad.cpp"
    "vendor/stb/stb_image.cpp"
    "vendor/stb/stb_image_write.cpp"
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
        "renderer/d3d11/D3D11FrameBuffer.cpp"
        "renderer/d3d11/D3D11IndexBuffer.cpp"
        "renderer/d3d11/D3D11RendererAPI.cpp"
        "renderer/d3d11/D3D11RendererContext.cpp"
        "renderer/d3d11/D3D11ShaderProgram.cpp"
        "renderer/d3d11/D3D11Texture.cpp"
        "renderer/d3d11/D3D11UniformBuffer.cpp"
        "renderer/d3d11/D3D11VertexBuffer.cpp"
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


add_executable(Core ${SOURCES_LIST} ${IMGUI_SOURCE_FILES})
target_precompile_headers(Core PRIVATE "pch.h")
target_include_directories(Core PRIVATE ${INCLUDE_LIST})
target_compile_definitions(Core PRIVATE ${DEFINITIONS_LIST})
target_link_libraries(Core ${STATIC_LIBRARIES})
set_target_properties(Core PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})