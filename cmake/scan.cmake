include_guard(GLOBAL)

cmake_path(GET CMAKE_CXX_COMPILER PARENT_PATH CXX_COMPILER_DIR)
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang"
	AND CMAKE_EXPORT_COMPILE_COMMANDS
	AND EXISTS "${CXX_COMPILER_DIR}/analyze-build")
	add_custom_target(
		scan
		"${CMAKE_COMMAND}" -E env --
		"${CXX_COMPILER_DIR}/analyze-build" "-v"
		"--cdb=${CMAKE_BINARY_DIR}/compile_commands.json"
		"--use-analyzer=${CMAKE_CXX_COMPILER}"
		"-enable-checker=core.CallAndMessage"
		"-enable-checker=core.DivideZero"
		"-enable-checker=core.FixedAddressDereference"
		"-enable-checker=core.NonNullParamChecker"
		"-enable-checker=core.NullDereference"
		"-enable-checker=core.NullPointerArithm"
		"-enable-checker=core.StackAddressEscape"
		"-enable-checker=core.UndefinedBinaryOperatorResult"
		"-enable-checker=core.VLASize"
		"-enable-checker=core.uninitialized.ArraySubscript"
		"-enable-checker=core.uninitialized.Assign"
		"-enable-checker=core.uninitialized.Branch"
		"-enable-checker=core.uninitialized.CapturedBlockVariable"
		"-enable-checker=core.uninitialized.NewArraySize"
		"-enable-checker=core.uninitialized.UndefReturn"
		"-enable-checker=cplusplus.ArrayDelete"
		"-enable-checker=cplusplus.InnerPointer"
		"-enable-checker=cplusplus.Move"
		"-enable-checker=cplusplus.NewDelete"
		"-enable-checker=cplusplus.NewDeleteLeaks"
		"-enable-checker=cplusplus.PlacementNew"
		"-enable-checker=cplusplus.PureVirtualCall"
		"-enable-checker=cplusplus.StringChecker"
		"-enable-checker=deadcode.DeadStores"
		"-enable-checker=nullability.NullPassedToNonnull"
		"-enable-checker=nullability.NullReturnedFromNonnull"
		"-enable-checker=nullability.NullableDereferenced"
		"-enable-checker=nullability.NullablePassedToNonnull"
		"-enable-checker=nullability.NullableReturnedFromNonnull"
		"-enable-checker=optin.core.EnumCastOutOfRange"
		"-enable-checker=optin.cplusplus.UninitializedObject"
		"-enable-checker=optin.cplusplus.VirtualCall"
		"-enable-checker=security.ArrayBound"
		"-enable-checker=security.FloatLoopCounter"
		"-enable-checker=security.insecureAPI.DeprecatedOrUnsafeBufferHandling"
		"-enable-checker=security.insecureAPI.UncheckedReturn"
		"--output=${CMAKE_BINARY_DIR}/scan/"
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		COMMENT "Running analyze-build"
	)
endif()
