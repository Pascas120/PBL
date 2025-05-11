# CPM - package manager
include(CPM)

# OpenGL
find_package(OpenGL REQUIRED)

# glad
set(GLAD_DIR ${CMAKE_CURRENT_LIST_DIR}/glad)
set(glad_SOURCE_DIR ${GLAD_DIR}/include CACHE INTERNAL "")
add_library(glad STATIC ${GLAD_DIR}/src/glad.c)
target_include_directories(glad PRIVATE ${GLAD_DIR}/include)

# stb_image
set(STB_IMAGE_DIR ${CMAKE_CURRENT_LIST_DIR}/stb_image)
set(stb_image_SOURCE_DIR ${STB_IMAGE_DIR} CACHE INTERNAL "")
add_library(stb_image STATIC ${STB_IMAGE_DIR}/stb_image.cpp)
target_include_directories(stb_image PRIVATE ${STB_IMAGE_DIR})

# TODO: remove (editor)
# ImGuizmo
set(ImGuizmo_DIR ${CMAKE_CURRENT_LIST_DIR}/ImGuizmo)
set(ImGuizmo_SOURCE_DIR ${ImGuizmo_DIR} CACHE INTERNAL "")
file (GLOB ImGuizmo_SOURCE_FILES ${ImGuizmo_SOURCE_DIR}/*.cpp)
file (GLOB ImGuizmo_HEADER_FILES ${ImGuizmo_SOURCE_DIR}/*.h)
add_library(ImGuizmo STATIC ${ImGuizmo_SOURCE_FILES} ${ImGuizmo_HEADER_FILES})
target_include_directories(ImGuizmo PUBLIC ${ImGuizmo_SOURCE_DIR})

# other
CPMAddPackage("gh:assimp/assimp@5.4.3")
CPMAddPackage("gh:glfw/glfw#3.3.8")
CPMAddPackage("gh:g-truc/glm#1.0.1")
CPMAddPackage("gh:ocornut/imgui@1.91.9b-docking") # TODO: remove (editor)
CPMAddPackage("gh:gabime/spdlog@1.10.0")
CPMAddPackage("gh:nlohmann/json@3.12.0")
CPMAddPackage("gh:btzy/nativefiledialog-extended@1.2.1") # TODO: remove (editor)
CPMAddPackage(
		NAME freetype
		GITHUB_REPOSITORY freetype/freetype
		GIT_TAG VER-2-13-2
		OPTIONS "FT_DISABLE_HARFBUZZ ON" # jeżeli chcesz uniknąć dodatkowej zależności
)


set(imgui_SOURCE_DIR ${imgui_SOURCE_DIR} CACHE INTERNAL "") # TODO: remove (editor)
add_library(imgui STATIC ${imgui_SOURCE_DIR}/imgui.cpp
					     ${imgui_SOURCE_DIR}/imgui_demo.cpp
					     ${imgui_SOURCE_DIR}/imgui_draw.cpp
						 ${imgui_SOURCE_DIR}/imgui_tables.cpp
					     ${imgui_SOURCE_DIR}/imgui_widgets.cpp
)
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR})

target_link_libraries(ImGuizmo PUBLIC imgui)


set_target_properties(glad
                      stb_image
                      assimp
                      glfw
                      glm
                      imgui
					  ImGuizmo
                      spdlog
					  nlohmann_json
					  nfd
					  PROPERTIES FOLDER "thirdparty")

if (TARGET zlibstatic)
    set_target_properties(zlibstatic PROPERTIES FOLDER "thirdparty")
endif()
if (TARGET freetype)
	set_target_properties(freetype PROPERTIES FOLDER "thirdparty")
endif()
