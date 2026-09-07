module;

#include <Core/GLM.hpp>

#include <memory>
#include <optional>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

export module Nyaanwork.Core.Types;
export import :Numbers;
export import :Numbers_operators;
export import :GLM_methods;

export namespace std
{
	using std::operator==;
	using std::get;
}

// STL
export namespace Nyaanwork::Types
{
	template<typename A, typename B>
	using Pair = std::pair<A, B>;
	template<typename Type>
	using PairOf = Pair<Type, Type>;
	using std::make_pair;

	template<typename Type>
	using Ptr = std::shared_ptr<Type>;
	template<typename Type>
	using EnableSharedFromThis = std::enable_shared_from_this<Type>;
	using std::make_shared;

	template<typename Type>
	using UPtr = std::unique_ptr<Type>;
	using std::make_unique;

	template<typename Type>
	using Opt = std::optional<Type>;
	using NullOpt = std::nullopt_t;
	using std::nullopt;

	using NoThrow = std::nothrow_t;
	using std::nothrow;

	using NullPtr = std::nullptr_t;

	using Str = std::string;
	using StrV = std::string_view;
}

// GLM
export namespace Nyaanwork::Types
{
	template<usize length, is_arithmetic Type>
	requires (0 < length && length <= 4)
	using vec = glm::vec<length, Type>;

	template<is_arithmetic Type>
	using vec2 = vec<2, Type>;

	template<is_arithmetic Type>
	using vec3 = vec<3, Type>;

	template<is_arithmetic Type>
	using vec4 = vec<4, Type>;

	template<usize cols, usize rows, is_arithmetic Type>
	requires ((1 < cols && cols <= 4) && (1 < rows && rows <= 4))
	using Matrix = glm::mat<cols, rows, Type>;

	template<is_arithmetic Type>
	using Matrix2 = Matrix<2, 2, Type>;

	template<is_arithmetic Type>
	using Matrix3 = Matrix<3, 3, Type>;

	template<is_arithmetic Type>
	using Matrix4 = Matrix<4, 4, Type>;

	template<is_arithmetic Type>
	using Matrix2x2 = Matrix<2, 2, Type>;
	template<is_arithmetic Type>
	using Matrix2x3 = Matrix<2, 3, Type>;
	template<is_arithmetic Type>
	using Matrix2x4 = Matrix<2, 4, Type>;

	template<is_arithmetic Type>
	using Matrix3x2 = Matrix<3, 2, Type>;
	template<is_arithmetic Type>
	using Matrix3x3 = Matrix<3, 3, Type>;
	template<is_arithmetic Type>
	using Matrix3x4 = Matrix<3, 4, Type>;

	template<is_arithmetic Type>
	using Matrix4x2 = Matrix<4, 2, Type>;
	template<is_arithmetic Type>
	using Matrix4x3 = Matrix<4, 3, Type>;
	template<is_arithmetic Type>
	using Matrix4x4 = Matrix<4, 4, Type>;

	template<is_floating Type>
	using Quaternion = glm::qua<Type>;
}

export namespace Nyaanwork
{
	using namespace Types;
}
