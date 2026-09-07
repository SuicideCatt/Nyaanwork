set(STANDELONE_MODULES TRUE PARENT_SCOPE)

set(modules "${CMAKE_CURRENT_SOURCE_DIR}/Codes.cpp")
list_directory("${CMAKE_CURRENT_SOURCE_DIR}/Backends/Base" cpp MODULES_BACKENDS_BASE)
list_directory("${CMAKE_CURRENT_SOURCE_DIR}/main" cpp MODULES_MAIN)
list(APPEND modules ${MODULES_BACKENDS_BASE} ${MODULES_MAIN})

set(bin_deps ${BIN_DEPS})

set(NYAANWORK_WINDOW_SYSTEM FALSE CACHE BOOL "Enable WindowSystem module" FORCE)

set(NYAANWORK_WINDOW_SYSTEM_OPENGL FALSE CACHE BOOL "Enable OpenGL support" FORCE)
set(NYAANWORK_WINDOW_SYSTEM_VULKAN FALSE CACHE BOOL "Enable Vulkan support" FORCE)

set(NYAANWORK_WINDOW_SYSTEM_BACKENDS_LINUX FALSE
	CACHE BOOL "Enable Linux backends support" FORCE)
set(NYAANWORK_WINDOW_SYSTEM_BACKENDS_LINUX_XCB FALSE
	CACHE BOOL "Enable Linux XCB(X11) backend support" FORCE)

set(NYAANWORK_WINDOW_SYSTEM_BACKENDS_WINDOWS FALSE
	CACHE BOOL "Enable ms windows backends support" FORCE)

if (CONFIG_NYAANWORK_WINDOW_SYSTEM)
	set(NYAANWORK_WINDOW_SYSTEM TRUE CACHE BOOL "Enable WindowSystem module" FORCE)

	if (CONFIG_NYAANWORK_WINDOW_SYSTEM_OPENGL)
		set(NYAANWORK_WINDOW_SYSTEM_OPENGL TRUE
			CACHE BOOL "Enable OpenGL support" FORCE)

		list(APPEND modules
			"${CMAKE_CURRENT_SOURCE_DIR}/OpenGL/Exception.cpp"
			"${CMAKE_CURRENT_SOURCE_DIR}/OpenGL/API.cpp"
			"${CMAKE_CURRENT_SOURCE_DIR}/OpenGL/OpenGL.cpp"
		)

		if (WIN32 OR (LINUX AND CONFIG_NYAANWORK_CROSS_COMPILE_FROM_LINUX))
			list(APPEND modules "${CMAKE_CURRENT_SOURCE_DIR}/OpenGL/WGL.cpp")
			list(APPEND bin_deps opengl32 gdi32)
		else()
			find_package(OpenGL COMPONENTS EGL OpenGL REQUIRED)
			list(APPEND modules "${CMAKE_CURRENT_SOURCE_DIR}/OpenGL/EGL.cpp")
			list(APPEND bin_deps ${OPENGL_egl_LIBRARY} ${OPENGL_opengl_LIBRARY})
		endif()
	endif()

	if (CONFIG_NYAANWORK_WINDOW_SYSTEM_VULKAN)
		set(NYAANWORK_WINDOW_SYSTEM_VULKAN TRUE
			CACHE BOOL "Enable Vulkan support" FORCE)

		find_package(Vulkan REQUIRED)
		find_package(VulkanHeaders REQUIRED)

		if (LINUX AND CONFIG_NYAANWORK_CROSS_COMPILE_FROM_LINUX)
			cross_compile_include("/usr/include" "vulkan")
			cross_compile_include("/usr/include" "vk_video")
			list(APPEND bin_deps vulkan-1)
		else()
			list(APPEND bin_deps ${Vulkan_LIBRARIES})
		endif()
	endif()

	if (CONFIG_NYAANWORK_WINDOW_SYSTEM_BACKENDS_LINUX AND LINUX
		AND NOT CONFIG_NYAANWORK_CROSS_COMPILE_FROM_LINUX)
		set(NYAANWORK_WINDOW_SYSTEM_BACKENDS_LINUX TRUE
			CACHE BOOL "Enable Linux backends support" FORCE)

		list(APPEND modules
			 "${CMAKE_CURRENT_SOURCE_DIR}/Backends/Linux/CursorName.cpp")

		if (CONFIG_NYAANWORK_WINDOW_SYSTEM_BACKENDS_LINUX_XCB)
			set(NYAANWORK_WINDOW_SYSTEM_BACKENDS_LINUX_XCB TRUE
				CACHE BOOL "Enable Linux XCB(X11) backend support" FORCE)

			find_package(X11 COMPONENTS
						 X11_xcb xcb
						 xcb_randr xcb_xfixes xcb_cursor
						 xcb_xinput xkbcommon xkbcommon_X11 xcb_xkb
						 REQUIRED
			)

			list_directory("${CMAKE_CURRENT_SOURCE_DIR}/Backends/Linux/XCB" cpp
					 MODULES_LINUX_XCB)
			list(APPEND modules ${MODULES_LINUX_XCB})

			list(APPEND bin_deps
				 ${X11_X11_LIB} ${X11_X11_xcb_LIB} ${X11_xcb_LIB}
				 ${X11_xcb_randr_LIB} ${X11_xcb_xfixes_LIB} ${X11_xcb_cursor_LIB}
				 ${X11_xcb_xinput_LIB}
				 ${X11_xkbcommon_LIB} ${X11_xkbcommon_X11_LIB} ${X11_xcb_xkb_LIB}
			)
		endif()
	endif()

	if (CONFIG_NYAANWORK_WINDOW_SYSTEM_BACKENDS_WINDOWS
		AND (WIN32 OR CONFIG_NYAANWORK_CROSS_COMPILE_FROM_LINUX))
		set(NYAANWORK_WINDOW_SYSTEM_BACKENDS_WINDOWS TRUE
			CACHE BOOL "Enable ms windows backends support" FORCE)

		list_directory("${CMAKE_CURRENT_SOURCE_DIR}/Backends/Windows" cpp
				MODULES_WINDOWS)
		list(APPEND modules ${MODULES_WINDOWS})

		list(APPEND bin_deps ole32 uuid shcore)
	endif()
endif()

set(MODULES ${modules} PARENT_SCOPE)
set(BIN_DEPS ${bin_deps} PARENT_SCOPE)
