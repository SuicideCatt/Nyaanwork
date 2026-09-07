// how to document is it, help...

module;

#include <Core/Defines.hpp>
#include <Core/GLM.hpp>

#include <glm/gtc/matrix_transform.hpp>

export module Nyaanwork.Core.Math;

import Nyaanwork.Core.Types;

export namespace Nyaanwork::Math
{
	using glm::fma;
	using glm::min; using glm::max;
	using glm::clamp;
	using glm::abs;
	using glm::sign;
	using glm::mod;
	using glm::modf;

	using glm::step; using glm::smoothstep;
	using glm::trunc;
	using glm::fract;

	using glm::mix; using std::lerp; using glm::lerp; using glm::slerp;

	using glm::ceil; using glm::floor; using glm::round;

	template<is_floating T>
	NYAAN_ND T round_even(T x) { return glm::roundEven(x); }

	template<glm::length_t l, is_floating T, glm::qualifier q>
	NYAAN_ND glm::vec<l, T, q> round_even(const glm::vec<l, T, q>& x)
		{ return glm::roundEven(x); }

	using glm::degrees; using glm::radians;
	using glm::sin; using glm::sinh; using glm::asin; using glm::asinh;
	using glm::cos; using glm::cosh; using glm::acos; using glm::acosh;
	using glm::tan; using glm::tanh; using glm::atan; using glm::atanh;

	using glm::length;
	using glm::normalize;
	using glm::distance;
	using glm::dot; using glm::cross;

	using glm::reflect;
	using glm::refract;

	using glm::faceforward;

	using glm::exp; using glm::exp2; using glm::ldexp; using glm::frexp;
	using glm::log; using std::log2; using glm::log2; using std::log10;

	template<glm::length_t l, is_floating T, glm::qualifier q>
	NYAAN_ND glm::vec<l, T, q> log10(const glm::vec<l, T, q>& vec) noexcept
		{ return glm::detail::functor1<glm::vec, l, T, T, q>::call(log10, vec); }

	using glm::pow;
	using glm::sqrt; using glm::inversesqrt;

	NYAAN_ND f32 q_rsqrt(f32 x) noexcept
	{
		f32 x2 = x * 0.5;
		static constexpr f32 threehalfs = 1.5;

		auto& i = reinterpret_cast<u32&>(x);
		i = 0x5f3759df - (i >> 1);

		return x * (threehalfs  - (x2 * x * x));
	}

	template<glm::length_t l, glm::qualifier q>
	NYAAN_ND glm::vec<l, f32, q> q_rsqrt(const glm::vec<l, f32, q>& vec) noexcept
		{ return glm::detail::functor1<glm::vec, l, f32, f32, q>::call(q_rsqrt, vec); }

	template<glm::length_t c, glm::length_t r, is_floating T, glm::qualifier q>
	NYAAN_ND glm::mat<c, r, T, q>
		matrix_compnent_multiply(const glm::mat<c, r, T, q>& x,
								 const glm::mat<c, r, T, q>& y)
		{ return glm::matrixCompMult(x, y); }
	template<glm::length_t cl, glm::length_t rl, is_floating T, glm::qualifier q>
	NYAAN_ND glm::mat<cl, rl, T, q>
		outer_product(const glm::vec<cl, T, q>& c,
					  const glm::vec<rl, T, q>& r)
		{ return glm::outerProduct(c, r); }

	using glm::determinant;
	using glm::inverse;
	using glm::transpose;

	using glm::mat3_cast;
	using glm::mat4_cast;

	using glm::ortho; using glm::perspective;
	using glm::translate; using glm::rotate; using glm::scale;

	template<is_floating T, glm::qualifier q>
	NYAAN_ND glm::mat<4, 4, T, q>
		look_at(const glm::vec<3, T, q>& eye,
				const glm::vec<3, T, q>& center,
				const glm::vec<3, T, q>& up)
		{ return glm::lookAt(eye, center, up); }

	using glm::roll; using glm::pitch; using glm::yaw;
	using glm::angle; using glm::axis;
	using glm::conjugate;

	using glm::quat_cast;

	template<is_floating T>
	NYAAN_ND glm::qua<T> angle_axis(T angle, T x, T y, T z)
		{ return glm::angleAxis(angle, x, y, z); }

	template<is_floating T, glm::qualifier q>
	NYAAN_ND glm::qua<T, q> angle_axis(T angle, const glm::vec<3, T, q>& axis)
		{ return glm::angleAxis(angle, axis); }

	template<is_floating T, glm::qualifier q>
	NYAAN_ND glm::vec<3, T, q> euler_angles(const glm::qua<T, q>& x)
		{ return glm::eulerAngles(x); }
}

export namespace Nyaanwork
{
	using namespace Math;
}
