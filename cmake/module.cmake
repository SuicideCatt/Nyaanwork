include_guard(GLOBAL)

function(nyaanwork_add_module name)
	if (ARGC GREATER 1)
		set(NO "")
		set(DEPS "")
		set(OPT_DEPS "")
		set(BIN_DEPS "")

		set(prepend "")
		set(li "NO")
		foreach (arg ${ARGV})
			if (arg STREQUAL "DEPENDS")
				set(li "DEPS")
				set(prepend "${PROJECT_NAME}-")
				continue()
			elseif (arg STREQUAL "OPTIONAL")
				set(li "OPT_DEPS")
				set(prepend "${PROJECT_NAME}-")
				continue()
			elseif (arg STREQUAL "BINARY")
				set(li "BIN_DEPS")
				set(prepend "")
				continue()
			endif()

			list(APPEND ${li} "${prepend}${arg}")
		endforeach()

		foreach (dep ${OPT_DEPS})
			if (NOT TARGET "${dep}")
				list(REMOVE_ITEM OPT_DEPS "${dep}")
			endif()
		endforeach()
	endif()

	set(STANDELONE_MODULES FALSE)

	#include configure file
	add_subdirectory("src/${name}")

	add_library("${PROJECT_NAME}-${name}")
	target_compile_features("${PROJECT_NAME}-${name}" PUBLIC cxx_std_23)

	if (NOT STANDELONE_MODULES)
		list_directory("${CMAKE_CURRENT_SOURCE_DIR}/src/${name}" cpp MODULES)
	endif()

	string(TOLOWER "${PROJECT_NAME}_${name}" FILE_SET_NAME)
	target_sources("${PROJECT_NAME}-${name}"
		PUBLIC FILE_SET "${FILE_SET_NAME}" TYPE CXX_MODULES FILES ${MODULES})
	target_include_directories("${PROJECT_NAME}-${name}"
		PUBLIC "${CMAKE_BINARY_DIR}/Nyaanwork/include"
			   "${CMAKE_CURRENT_SOURCE_DIR}/third_party/include"
		PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/include"
	)
	if (CONFIG_NYAANWORK_CROSS_COMPILE_FROM_LINUX)
		target_include_directories("${PROJECT_NAME}-${name}"
			PUBLIC "${CMAKE_BINARY_DIR}/CrossCompile/include"
		)
	endif()
	target_link_libraries("${PROJECT_NAME}-${name}"
						  PUBLIC ${DEPS} ${OPT_DEPS} ${BIN_DEPS})

	target_link_libraries(${PROJECT_NAME} INTERFACE "${PROJECT_NAME}-${name}")
endfunction()

function(nyaanwork_add_module_with_config name kconfig_var)
	if (ARGC GREATER 2)
		set(ADDITIONAL ${ARGV})
		list(POP_FRONT ADDITIONAL out_var out_var)
	endif()

	if(CONFIG_${kconfig_var})
		nyaanwork_add_module(${name} ${ADDITIONAL})
	else()
		#include configure file
		add_subdirectory("src/${name}")
	endif()

	configure_file(
		"${CMAKE_CURRENT_SOURCE_DIR}/src/${name}/${name}.hpp.in"
		"${CMAKE_BINARY_DIR}/Nyaanwork/include/Nyaanwork/${name}.hpp"
	)
endfunction()
