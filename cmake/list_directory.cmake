include_guard(GLOBAL)

function(list_directory path extension result)
	file(GLOB_RECURSE files
		 LIST_DIRECTORIES false
		 RELATIVE "${path}/" "${path}/*.${extension}")

	set(res "")
	foreach(file ${files})
		list(APPEND res "${path}/${file}")
	endforeach()

	set(${result} ${res} PARENT_SCOPE)
endfunction()
