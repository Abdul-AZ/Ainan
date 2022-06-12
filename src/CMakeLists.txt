add_executable(Core WIN32     
    "main.cpp"
    "pch.h"  "pch.cpp"
    "Log.h"  "Log.cpp"
)

add_subdirectory("vendor")
add_subdirectory("editor")
add_subdirectory("file")
add_subdirectory("renderer")
add_subdirectory("environment")

list(APPEND DEFINITIONS_LIST
    IMGUI_DEFINE_MATH_OPERATORS=1
    GLFW_INCLUDE_NONE=1
	_CRT_SECURE_NO_WARNINGS=1
    )

set(SHADER_FILES)
set(GENERATED_SHADER_FILES)

if(WIN32)

    list(APPEND DEFINITIONS_LIST
        PLATFORM_WINDOWS=1
		STARTING_BROWSER_DIRECTORY="C:\\\\"
        )

    target_sources(Core PUBLIC
        "windows/WinMain.cpp"
        "windows/WindowsFileEvaluator.h"         "windows/WindowsFileEvaluator.cpp"
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
    get_target_property(MY_PROJECT_SOURCES Core SOURCES)
    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${MY_PROJECT_SOURCES})
    source_group("shaders" FILES ${SHADER_FILES})
    source_group("compiled shaders" FILES ${GENERATED_SHADER_FILES})
else()
	list(APPEND DEFINITIONS_LIST
		STARTING_BROWSER_DIRECTORY="/"
        )
endif()

#Add imgui files after making the visual studio file filters because they are outside the src directory
target_sources(Core PUBLIC
                    "../submodules/imgui/imgui.cpp"
                    "../submodules/imgui/imgui_demo.cpp"
                    "../submodules/imgui/imgui_draw.cpp"
                    "../submodules/imgui/imgui_widgets.cpp"
                    "../submodules/imgui/imgui_tables.cpp"
                    "../submodules/imgui/misc/cpp/imgui_stdlib.cpp"
                    "../submodules/imgui/imgui_internal.h"

                    "../submodules/ImGuizmo/ImGuizmo.cpp"
)

target_include_directories(Core PRIVATE 
    "."
    "../submodules/imgui"
    "../submodules/imgui/examples"
    "../submodules/imgui/misc/cpp" 
    "../submodules/glm" 
    "../submodules/spdlog/include"
    "../submodules/ImGuizmo"
    "../submodules/assimp/include/assimp"
    "vendor"
)

target_sources(Core PUBLIC ${SHADER_FILES} ${GENERATED_SHADER_FILES})
set_source_files_properties(${GENERATED_SHADER_FILES} PROPERTIES GENERATED TRUE)
target_precompile_headers(Core PRIVATE "pch.h")
target_compile_definitions(Core PRIVATE ${DEFINITIONS_LIST})
target_compile_options(Core PRIVATE -fpermissive)
target_link_libraries(Core ${STATIC_LIBRARIES})
set_target_properties(Core PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
