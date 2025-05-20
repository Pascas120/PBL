# Add source files
file(GLOB_RECURSE SOURCE_FILES 
	 *.c
	 *.cpp)
	
# Add header files
file(GLOB_RECURSE HEADER_FILES 
	 *.h
	 *.hpp)
	 
# Search for the assets files
file(GLOB_RECURSE ASSETS_FILES 
	 ${CMAKE_SOURCE_DIR}/res/*.*)

set_source_files_properties(${ASSETS_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES})						   
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${HEADER_FILES})
source_group(TREE ${CMAKE_SOURCE_DIR}         FILES ${ASSETS_FILES})

# Define the executable
add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES} ${ASSETS_FILES}
		TextRenderer.cpp
		TextRenderer.h
		ECS/Components.h
		ECS/EntityManager.cpp
		ECS/EntityManager.h
		ECS/ComponentStorage.cpp
		ECS/ComponentStorage.h
        ECS/TransformSystem.cpp
		ECS/TransformSystem.h
		ECS/RenderingSystem.cpp
		ECS/RenderingSystem.h
		ECS/BoundingVolumes.cpp
		ECS/BoundingVolumes.h
		ECS/EventSystem.cpp
		ECS/EventSystem.h
		Bone.cpp
		Bone.h
		Animation.cpp
		Animation.h
		AssimpGLMHelpers.h
		Animator.cpp
		Animator.h
)

target_compile_definitions(${PROJECT_NAME} PRIVATE GLFW_INCLUDE_NONE)
target_compile_definitions(${PROJECT_NAME} PRIVATE LIBRARY_SUFFIX="")

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
												  ${glad_SOURCE_DIR}
												  ${stb_image_SOURCE_DIR}
												  ${imgui_SOURCE_DIR})

target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES})
target_link_libraries(${PROJECT_NAME} glad)
target_link_libraries(${PROJECT_NAME} stb_image)
target_link_libraries(${PROJECT_NAME} assimp)
target_link_libraries(${PROJECT_NAME} glfw)
target_link_libraries(${PROJECT_NAME} spdlog)
target_link_libraries(${PROJECT_NAME} glm::glm)
target_link_libraries(${PROJECT_NAME} freetype)
target_link_libraries(${PROJECT_NAME} nlohmann_json::nlohmann_json)

# TODO: remove (editor)
target_link_libraries(${PROJECT_NAME} imgui)
target_link_libraries(${PROJECT_NAME} ImGuizmo)
target_link_libraries(${PROJECT_NAME} nfd)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD 
				   COMMAND ${CMAKE_COMMAND} -E create_symlink 
				   ${CMAKE_SOURCE_DIR}/res 
				   ${CMAKE_CURRENT_BINARY_DIR}/res)

if(MSVC)
    target_compile_definitions(${PROJECT_NAME} PUBLIC NOMINMAX)
endif()
