include_guard(GLOBAL)

function(nyaanwork_run_target name target)
	if (NOT TARGET "${target}")
		message(FATAL_ERROR "Target ${target} not exists")
	endif()

	get_target_property(type ${target} TYPE)
	if (NOT "${type}" STREQUAL "EXECUTABLE")
		message(FATAL_ERROR "Target ${target} not executable")
	endif()

	get_target_property(executable ${target} OUTPUT_NAME)
	get_target_property(path ${target} RUNTIME_OUTPUT_DIRECTORY)
	if ("${executable}" STREQUAL "executable-NOTFOUND")
		set(executable ${target})
	endif()
	if ("${path}" STREQUAL "path-NOTFOUND")
		set(path ${CMAKE_CURRENT_BINARY_DIR})
	endif()

	set(args ${ARGV})
	list(POP_FRONT args out_var out_var)

	if (CONFIG_NYAANWORK_CROSS_COMPILE_FROM_LINUX)
		string(LENGTH "${executable}" executable_len)
		math(EXPR executable_ext_begin "${executable_len}-4")
		string(SUBSTRING "${executable}" ${executable_ext_begin} 4 executable_ext)

		if (NOT "${executable_ext}" STREQUAL ".exe")
			set(executable "${executable}.exe")
		endif()

		cross_compile_parse_path(
			"${CONFIG_NYAANWORK_CROSS_COMPILE_FROM_LINUX_WINE_PREFIX}"
			prefix
		)
		if (NOT "${prefix}" STREQUAL "")
			set(prefix "WINEPREFIX=${prefix}")
		endif()

		cross_compile_parse_path(
			"${CONFIG_NYAANWORK_CROSS_COMPILE_FROM_LINUX_LIBRARIES}"
			libs
		)
		list(PREPEND libs "/usr/x86_64-w64-mingw32/bin")
		string(REPLACE ";" "\;" libs "${libs}")

		set(runner_env "${prefix}" "WINEPATH=\"${libs}\"")
		set(runner "${CONFIG_NYAANWORK_CROSS_COMPILE_FROM_LINUX_WINE}")
	else()
		set(runner_env "")
		set(runner "")
	endif()

	add_custom_target(
		"run-${name}"
		"${CMAKE_COMMAND}" -E env ${runner_env} --
		"${runner}"
		"${path}/${executable}" ${args}
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		COMMENT "Running ${executable}"
		USES_TERMINAL
	)
	add_dependencies("run-${name}" ${target})
endfunction()

function(nyaanwork_run_target_default name)
	if (TARGET run)
		message(FATAL_ERROR "Default run target exists")
		return()
	endif()

	add_custom_target(run)
	add_dependencies(run "run-${name}")
endfunction()
