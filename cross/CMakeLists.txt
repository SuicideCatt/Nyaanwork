if (LINUX)
	kconfig_add_menu("Cross compile" Kconfig.mingw)

	if (CONFIG_NYAANWORK_CROSS_COMPILE_FROM_LINUX)
		set(CMAKE_SYSTEM_NAME Windows CACHE STRING "" FORCE)
		set(CMAKE_C_COMPILER_TARGET x86_64-w64-mingw32 CACHE STRING "" FORCE)
		set(CMAKE_CXX_COMPILER_TARGET x86_64-w64-mingw32 CACHE STRING "" FORCE)
		file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/CrossCompile/include")

		target_link_options(Nyaanwork INTERFACE "-pthread")
		target_link_libraries(Nyaanwork INTERFACE stdc++exp pthread)

		function(cross_compile_parse_path path out_path)
			string(SUBSTRING "${path}" 0 6 begin_build)
			string(SUBSTRING "${path}" 0 7 begin_source)

			if ("${begin_build}" STREQUAL "$build")
				string(SUBSTRING "${path}" 6 -1 all_path)
				set("${out_path}" "${CMAKE_BINARY_DIR}${all_path}" PARENT_SCOPE)
			elseif ("${begin_source}" STREQUAL "$source")
				string(SUBSTRING "${path}" 7 -1 all_path)
				set("${out_path}" "${CMAKE_SOURCE_DIR}${all_path}" PARENT_SCOPE)
			else()
				set("${out_path}" "${path}" PARENT_SCOPE)
			endif()
		endfunction()

		cross_compile_parse_path(
			"${CONFIG_NYAANWORK_CROSS_COMPILE_FROM_LINUX_LIBRARIES}"
			libs
		)
		target_link_directories(Nyaanwork INTERFACE "${libs}")

		function(cross_compile_include path name)
			file(CREATE_LINK
				"${path}/${name}" "${CMAKE_BINARY_DIR}/CrossCompile/include/${name}"
				SYMBOLIC
			)
		endfunction()
	else()
		set(CMAKE_SYSTEM_NAME Linux CACHE STRING "" FORCE)
		unset(CMAKE_C_COMPILER_TARGET CACHE)
		unset(CMAKE_CXX_COMPILER_TARGET CACHE)
	endif()
endif()

