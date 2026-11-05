module;

#include <Core/Defines.hpp>
#include <Nyaanwork/HighOpenGL.hpp>

#include <format>
#include <source_location>

export module Nyaanwork.HighOpenGL.Core330:Checks;
import :Context;

namespace Nyaanwork::HighOpenGL::Core330
{
	void check_texture_slot(Context& context, u8 slot,
							std::source_location src =
								std::source_location::current())
			noexcept(!Context::dynamic_limits_mode && !is_debug)
	{
		using src_loc = Error::Exception::src_loc;
		auto check = [&slot, &src](auto limit)
		{
			if (!(slot < limit)) NYAAN_UNLIK
			{
				auto msg = std::format("texture_slots: !({} < {})",
									   slot, limit);
				throw Error::Exception(msg, Error::Code::limit_exceeded, src);
			}
		};

		if constexpr (Context::dynamic_limits_mode)
			check(context.limits().texture_slots);
		#ifndef NYAANWORK_HIGH_OPENGL_CORE330_DYNAMIC_LIMITS
		else if constexpr (is_debug)
			check(Context::limits().texture_slots);
		#endif
	}

	void check_unifrom_slot(Context& context, u8 slot,
							std::source_location src =
								std::source_location::current())
			noexcept(!Context::dynamic_limits_mode && !is_debug)
	{
		using src_loc = Error::Exception::src_loc;
		auto check = [&slot](auto limit, src_loc src = src_loc::current())
		{
			if (!(slot < limit)) NYAAN_UNLIK
			{
				auto msg = std::format("uniform_buffer_bindings: !({} < {})",
									   slot, limit);
				throw Error::Exception(msg, Error::Code::limit_exceeded, src);
			}
		};

		if constexpr (Context::dynamic_limits_mode)
			check(context.limits().uniform_buffer_bindings);
		#ifndef NYAANWORK_HIGH_OPENGL_CORE330_DYNAMIC_LIMITS
		else if constexpr (is_debug)
			check(Context::limits().uniform_buffer_bindings);
		#endif
	}
}
