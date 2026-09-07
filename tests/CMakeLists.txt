link_libraries(Nyaanwork)

function(nyaanwork_add_texec module name)
	add_executable("Nyaanwork-Tests-${module}-${name}" "${module}/${name}.cpp")
	set_target_properties("Nyaanwork-Tests-${module}-${name}" PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Nyaanwork/Tests/${module}"
		OUTPUT_NAME "${name}")
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
