link_libraries(Nyaanwork)

add_custom_target(Nyaanwork-Tests)

function(nyaanwork_add_texec module name)
	set(target "Nyaanwork-Tests-${module}-${name}")
	string(REPLACE "/" "-" target "${target}")
	add_executable(${target} "${module}/${name}.cpp")
	set_target_properties(${target} PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Nyaanwork/Tests/${module}"
		OUTPUT_NAME "${name}")

	add_dependencies(Nyaanwork-Tests ${target})
	Nyaanwork_run_target(${target} ${target})
endfunction()

if (CONFIG_NYAANWORK_TESTS_CORE)
	nyaanwork_add_texec(Core signals)
	nyaanwork_add_texec(Core glm_print)
	nyaanwork_add_texec(Core system)
	nyaanwork_add_texec(Core string_convertor)
endif()

if (CONFIG_NYAANWORK_TESTS_ASSET)
	if (CONFIG_NYAANWORK_ASSET_IMAGE)
		nyaanwork_add_texec(Asset pixel_print)
		nyaanwork_add_texec(Asset image)
	endif()

	if (CONFIG_NYAANWORK_ASSET_TILE)
		nyaanwork_add_texec(Asset tile)
	endif()
endif()

if (CONFIG_NYAANWORK_TESTS_WINDOW_SYSTEM)
	nyaanwork_add_texec(WindowSystem main)
	nyaanwork_add_texec(WindowSystem multi_instance_launch)
	nyaanwork_add_texec(WindowSystem multi_instance)

	if (CONFIG_NYAANWORK_WINDOW_SYSTEM_OPENGL)
		nyaanwork_add_texec(WindowSystem gl)
	endif()

	if (CONFIG_NYAANWORK_WINDOW_SYSTEM_VULKAN)
		nyaanwork_add_texec(WindowSystem vk)
	endif()

	if (CONFIG_NYAANWORK_WINDOW_SYSTEM_OPENGL AND CONFIG_NYAANWORK_WINDOW_SYSTEM_VULKAN)
		nyaanwork_add_texec(WindowSystem gl_vk)
	endif()
endif()

if (CONFIG_NYAANWORK_TESTS_INPUT)
	nyaanwork_add_texec(Input hli)
endif()

if (CONFIG_NYAANWORK_TESTS_IMGUI AND CONFIG_NYAANWORK_WINDOW_SYSTEM_OPENGL)
	nyaanwork_add_texec(ImGUI main)
endif()

if (CONFIG_NYAANWORK_TESTS_HIGH_OPENGL AND CONFIG_NYAANWORK_HIGH_OPENGL_CORE330)
	nyaanwork_add_texec(HighOpenGL/Core330 init)
	nyaanwork_add_texec(HighOpenGL/Core330 triangle)
	nyaanwork_add_texec(HighOpenGL/Core330 multi_mesh)
	nyaanwork_add_texec(HighOpenGL/Core330 texture)
	nyaanwork_add_texec(HighOpenGL/Core330 framebuffer)
	nyaanwork_add_texec(HighOpenGL/Core330 uniform_buffer_multi_mesh)
	nyaanwork_add_texec(HighOpenGL/Core330 nyaanwork_logo)
	nyaanwork_add_texec(HighOpenGL/Core330 cube)
	nyaanwork_add_texec(HighOpenGL/Core330 cube_blend)
	nyaanwork_add_texec(HighOpenGL/Core330 cube_stencil)
endif()
