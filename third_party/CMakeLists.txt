if (CONFIG_NYAANWORK_IMGUI)
	add_library(imgui STATIC)
	target_include_directories(imgui PUBLIC include)

	set(IMGUI module/imgui)
	set(IMGUI_BACKENDS "${IMGUI}/backends")

	target_sources(imgui PRIVATE
		"${IMGUI}/imgui.cpp" "${IMGUI}/imgui_draw.cpp"
		"${IMGUI}/imgui_tables.cpp" "${IMGUI}/imgui_widgets.cpp"
		"${IMGUI}/imgui_demo.cpp"
	)

	if (CONFIG_NYAANWORK_WINDOW_SYSTEM_OPENGL)
		target_sources(imgui PRIVATE "${IMGUI_BACKENDS}/imgui_impl_opengl3.cpp")
	endif()

	if (CONFIG_NYAANWORK_WINDOW_SYSTEM_VULKAN)
		if (LINUX AND CONFIG_NYAANWORK_CROSS_COMPILE_FROM_LINUX)
			target_include_directories(imgui PUBLIC
									   "${CMAKE_BINARY_DIR}/CrossCompile/include")
		endif()
		target_sources(imgui PRIVATE "${IMGUI_BACKENDS}/imgui_impl_vulkan.cpp")
	endif()
endif()
