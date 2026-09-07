#pragma once

#if __cplusplus < 202300L
	#error "no C++23?"
#endif

#define NYAAN_ND [[nodiscard]]
#define NYAAN_ND_ML [[nodiscard("Memory leak")]]
#define NYAAN_ND_MSG(t) [[nodiscard(t)]]

#define NYAAN_LIK [[likely]]
#define NYAAN_UNLIK [[unlikely]]

#define NYAAN_CNOEX const noexcept

namespace Nyaanwork
{
	#ifdef NDEBUG
	static constexpr bool is_debug = false;
	#else
	static constexpr bool is_debug = true;
	#endif
}
