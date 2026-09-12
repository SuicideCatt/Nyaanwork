cmake_minimum_required(VERSION 3.30.9)

project(Nyaanwork
	VERSION 0.0.1
	DESCRIPTION "Framework for make games or game engines, nyaa~~~~"
	LANGUAGES CXX
)

add_subdirectory(third_party/module/cmake-kconfig)

include(cmake/list_directory.cmake)
include(cmake/module.cmake)

add_library(${PROJECT_NAME} INTERFACE)

kconfig_add_menu(${PROJECT_NAME} Kconfig)
add_subdirectory(cross)

target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_23)
target_include_directories(${PROJECT_NAME}
	INTERFACE "${CMAKE_BINARY_DIR}/Nyaanwork/include"
)

add_subdirectory(third_party)

nyaanwork_add_module(Core)
nyaanwork_add_module_with_config(Asset NYAANWORK_ASSET DEPENDS Core)
nyaanwork_add_module_with_config(WindowSystem NYAANWORK_WINDOW_SYSTEM
								 DEPENDS Core OPTIONAL Asset)
nyaanwork_add_module_with_config(Input NYAANWORK_INPUT
								 DEPENDS Core WindowSystem)
nyaanwork_add_module_with_config(ImGUI NYAANWORK_IMGUI
								 DEPENDS Core WindowSystem
								 BINARY imgui)

include(cmake/scan.cmake)
include(cmake/run.cmake)

if(CONFIG_NYAANWORK_TESTS)
	add_subdirectory(tests)
endif()
