cmake_minimum_required(VERSION 3.13)
project(ssphh)

if (APPLE)
   include_directories(/usr/local/include)
   link_directories(/usr/local/lib)
endif()

if (!APPLE)
	# target_link_libraries(simplegame stdc++fs)
endif()

include_directories(${PROJECT_SOURCE_DIR}/include)

add_executable(${PROJECT_NAME}
	src/AuroraVisualization.cpp
	src/camera_animation.cpp
	src/Damselfish.cpp
	src/DragDrop.cpp
	src/GLFW_template.cpp
	src/GLUT_template.cpp
	src/help_window.cpp
	src/jellyfish.cpp
	src/main.cpp
	src/PbskyViewController.cpp
	# src/pch.cpp
	src/renderer_config_window.cpp
	src/renderer_window.cpp
	src/scene_editor_window.cpp
	src/scene_graph_window.cpp
	src/sphhapp_ssphh.cpp
	src/SphlJob.cpp
	src/ssphhapp.cpp
	src/ssphhapp_pathanim.cpp
	src/ssphhapp_pbsky.cpp
	src/ssphhapp_renderconfigs.cpp
	src/ssphhapp_render.cpp
	src/ssphhapp_render_gl11.cpp
	src/ssphhapp_render_gles30.cpp
	src/ssphhapp_render_shadows.cpp
	src/ssphhapp_scenegraph.cpp
	src/ssphhapp_unicornfish.cpp
	src/ssphhapp_update.cpp
	src/ssphhapp_usercontrols.cpp
	src/ssphh_corona.cpp
	src/ssphh_imgui.cpp
	src/ssphh_imgui_tests.cpp
	src/ssphh_menu.cpp
	src/ssphh_ppmcompare.cpp
	src/ssphh_shaders.cpp
	src/ssphh_tests.cpp
	src/ssphh_uf_broker.cpp
	src/ssphh_uf_client.cpp
	src/ssphh_uf.cpp
	src/ssphh_uf_imgui.cpp
	src/ssphh_uf_worker.cpp
	src/ssphh_window.cpp
	src/tests.cpp
	src/tool_window.cpp
	src/unicornfish_window.cpp
    )

target_link_libraries(
	${PROJECT_NAME}
	PRIVATE
	hatchetfish
	damselfish
	fluxions-gte
	fluxions-base
	fluxions-ssg
	fluxions-renderer
	fluxions
	starfish
	viperfish
	unicornfish)

# find_library(SDL2_LIB NAMES SDL2)
# find_library(SDL2_IMAGE_LIB NAMES SDL2_image)
# find_library(SDL2_MIXER_LIB NAMES SDL2_mixer)
# find_library(SDL2_TTF_LIB NAMES SDL2_ttf)
# find_library(CZMQ_LIB NAMES czmq)
# find_library(GLFW_LIB NAMES glfw)
# find_library(CURL_LIB NAMES curl)
# find_library(PTHREAD_LIB NAMES pthread)
# find_library(ILMIMF_LIB NAMES IlmImf)
# find_library(ILMHALF_LIB NAMES Half)


if (APPLE)
	find_package(OpenGL REQUIRED)
endif()

if (!APPLE)
	find_library(OPENGL_LIB NAMES GL)
	target_link_libraries(${PROJECT_NAME} PRIVATE GL)
endif()

find_package(SDL2 CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2main SDL2::SDL2-static)

find_package(sdl2-image CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2_image)

# find_library(GLEW REQUIRED)
# target_link_libraries(${PROJECT_NAME} PRIVATE GLEW)

find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw)

find_package(imgui CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE imgui::imgui)

find_package(IlmBase CONFIG REQUIRED)
# Note: 2 target(s) were omitted.
target_link_libraries(${PROJECT_NAME} PRIVATE IlmBase::Iex IlmBase::Half IlmBase::Imath IlmBase::IexMath)

find_package(OpenEXR CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE OpenEXR::IlmImf OpenEXR::IlmImfUtil OpenEXR::IlmImfConfig)

find_package(czmq CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE czmq czmq-static)

find_package(unofficial-sodium CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE unofficial-sodium::sodium)

find_package(curl CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE curl)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)

