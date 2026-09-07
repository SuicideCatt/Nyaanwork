module;

#include <WindowSystem/Backends/Linux/XCB/Defines.hpp>

export module Nyaanwork.WindowSystem.Backends.Linux.XCB:Bind.XCB.Hints;
import :Bind.XCB.Base;
import :Bind.XCB.Helper;

import Nyaanwork.Core.Types;
import Nyaanwork.Core.Utils.Flags;

namespace XCB
{
	enum class SizeHintsFlags : Nyaanwork::u32
	{
		us_position = 1 << 0,
		us_size = 1 << 1,
		p_position = 1 << 2,
		p_size = 1 << 3,
		p_min_size = 1 << 4,
		p_max_size = 1 << 5,
		p_resize_inc = 1 << 6,
		p_aspect = 1 << 7,
		base_size = 1 << 8,
		p_win_gravity = 1 << 9,
	};

	enum class MotifHintsFlags : Nyaanwork::u32
	{
		functions = 1 << 0,
		decorations = 1 << 1,
		input_mode = 1 << 2,
		status = 1 << 3,
	};
}

export template<>
struct Nyaanwork::FlagTraits<XCB::SizeHintsFlags>
	: Nyaanwork::FlagInfo<XCB::SizeHintsFlags>
{
	static constexpr bool is_bit_type = true;
	static constexpr Flags all =
		Type::us_position | Type::us_size
		| Type::p_position | Type::p_size
		| Type::p_min_size | Type::p_max_size
		| Type::p_resize_inc | Type::p_aspect
		| Type::base_size | Type::p_win_gravity;
};

export template<>
struct Nyaanwork::FlagTraits<XCB::MotifHintsFlags>
	: Nyaanwork::FlagInfo<XCB::MotifHintsFlags>
{
	static constexpr bool is_bit_type = true;
	static constexpr Flags all =
		Type::functions | Type::decorations | Type::input_mode | Type::status;
};

export namespace XCB
{
	struct SizeHints
	{
		using i32 = Nyaanwork::i32;
		using u32 = Nyaanwork::u32;
		using vec2 = Nyaanwork::vec2<i32>;

		using Hints = SizeHintsFlags;

		void change(Connection* c, Window::ID window) NYAAN_CNOEX
		{
			using enum PropertyChangeMode;

			auto ptr = reinterpret_cast<const u32*>(this);
			static constexpr auto size = sizeof(SizeHints) / sizeof(u32);

			namespace W = Window;
			W::change_property<u32>(c, window, replace,
									Atoms::wm_normal_hints, Atoms::wm_size_hints,
									{ptr, size});
		}

		Nyaanwork::Flags<Hints> flags;
		vec2 position = {};
		vec2 resolution = {};
		vec2 min_resolution = {};
		vec2 max_resolution = {};
		vec2 resolution_inc = {};
		struct {
			i32  num = 0, den = 0;
		} min_aspect, max_aspect;
		vec2 base_resolution = {};
		u32 win_gravity = 0;
	};

	struct MotifHints
	{
		using i32 = Nyaanwork::i32;
		using u32 = Nyaanwork::u32;
		using vec2 = Nyaanwork::vec2<i32>;

		using Hints = MotifHintsFlags;

		void change(Connection* c, Window::ID window,
					Atom motif_wm_hints) NYAAN_CNOEX
		{
			using enum PropertyChangeMode;

			auto ptr = reinterpret_cast<const u32*>(this);
			static constexpr auto size = sizeof(MotifHints) / sizeof(u32);

			namespace W = Window;
			W::change_property<u32>(c, window, replace,
									motif_wm_hints, motif_wm_hints,
									{ptr, size});
		}

		Nyaanwork::Flags<Hints> flags;
		u32 functions = 0;
		u32 decorations = 0;
		i32 input_mode = 0;
		u32 status = 0;
	};
}
