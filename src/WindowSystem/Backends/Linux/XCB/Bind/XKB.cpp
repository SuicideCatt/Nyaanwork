// this file can be moved to Linux, it's can work in Wayland

module;

#include <WindowSystem/Backends/Linux/XCB/Defines.hpp>

#include <xkbcommon/xkbcommon.h>

export module Nyaanwork.WindowSystem.Backends.Linux.XCB:Bind.XKB;
import :Exception;

import Nyaanwork.Core.Types;
import Nyaanwork.Core.Utils.Flags;
import Nyaanwork.Core.Utils.UPtr;

namespace XKB
{
	enum class ContextFlags : Nyaanwork::EnumTraits<xkb_context_flags>::MaskType
	{
		#define XKB_FLAG(name, X) XCB_FLAG_HELPER(name, XKB_CONTEXT, X)

		XKB_FLAG(no_default_includes, NO_DEFAULT_INCLUDES),
		XKB_FLAG(no_environment_names, NO_ENVIRONMENT_NAMES),
		XKB_FLAG(no_secure_getenv, NO_SECURE_GETENV),

		#undef XKB_FLAG
	};
}

export template<>
struct Nyaanwork::FlagTraits<XKB::ContextFlags>
	: Nyaanwork::FlagInfo<XKB::ContextFlags>
{
	static constexpr bool is_bit_type = true;

	static constexpr Flags all = Type::no_default_includes
						| Type::no_environment_names | Type::no_secure_getenv;
};

export namespace XKB
{
	using Keycode = xkb_keycode_t;
	using ModMask = xkb_mod_mask_t;
	using LayoutIndex = xkb_layout_index_t;

	class Context final
		: private Nyaanwork::GetUPtr<xkb_context, &xkb_context_unref>
	{
	public:
		using unique_ptr::element_type;
		using unique_ptr::pointer;

		using Flags = ContextFlags;
		using FlagsT = Nyaanwork::Flags<ContextFlags>;

		Context() = default;

		Context(FlagsT flags)
			: unique_ptr(xkb_context_new(
				static_cast<xkb_context_flags>(static_cast<FlagsT::MaskType>(flags))))
		{
			using namespace Nyaanwork::WindowSystem::Backends::Linux::XCB::Error;
			if (!*this) NYAAN_UNLIK
				throw Exception(Code::fail_to_create_xkb_context);
		}

		Context(const Context&) = delete;
		Context& operator=(const Context&) = delete;

		Context(Context&&) = default;
		Context& operator=(Context&&) = default;

		~Context() = default;

		using unique_ptr::operator bool;

		using unique_ptr::get;

		using unique_ptr::release;
		using unique_ptr::reset;

		using unique_ptr::operator*;
		using unique_ptr::operator->;
	};

	class Keymap final
		: private Nyaanwork::GetUPtr<xkb_keymap, &xkb_keymap_unref>
	{
	public:
		using unique_ptr::element_type;
		using unique_ptr::pointer;

		Keymap() = default;

		Keymap(pointer keymap) : unique_ptr(keymap)
		{
			using namespace Nyaanwork::WindowSystem::Backends::Linux::XCB::Error;
			if (!keymap) NYAAN_UNLIK
				throw Exception(Code::fail_to_create_xkb_keymap);
		}

		Keymap(const Keymap&) = delete;
		Keymap& operator=(const Keymap&) = delete;

		Keymap(Keymap&&) = default;
		Keymap& operator=(Keymap&&) = default;

		~Keymap() = default;

		using unique_ptr::operator bool;

		using unique_ptr::get;

		using unique_ptr::release;
		using unique_ptr::reset;

		using unique_ptr::operator*;
		using unique_ptr::operator->;

		Nyaanwork::usize layouts_count() NYAAN_CNOEX
			{ return xkb_keymap_num_layouts(get()); }
		Nyaanwork::Str layout_name(Nyaanwork::usize i) NYAAN_CNOEX
			{ return xkb_keymap_layout_get_name(get(), i); }
	};

	class State final
		: private Nyaanwork::GetUPtr<xkb_state, &xkb_state_unref>
	{
	public:
		using unique_ptr::element_type;
		using unique_ptr::pointer;

		State() = default;

		State(pointer state) : unique_ptr(state)
		{
			using namespace Nyaanwork::WindowSystem::Backends::Linux::XCB::Error;
			if (!state) NYAAN_UNLIK
				throw Exception(Code::fail_to_create_xkb_state);
		}

		State(const State&) = delete;
		State& operator=(const State&) = delete;

		State(State&&) = default;
		State& operator=(State&&) = default;

		~State() = default;

		using unique_ptr::operator bool;

		using unique_ptr::get;

		using unique_ptr::release;
		using unique_ptr::reset;

		using unique_ptr::operator*;
		using unique_ptr::operator->;

		Nyaanwork::usize current_layout(const Keymap& km) noexcept
		{
			static constexpr auto type = XKB_STATE_LAYOUT_LATCHED;

			auto layouts = km.layouts_count();
			for (Nyaanwork::usize i = 0; i < layouts; ++i)
				if (xkb_state_layout_index_is_active(get(), i, type)) NYAAN_UNLIK
					return i;

			return 0;
		}

		Nyaanwork::Str key_utf8(Keycode key) NYAAN_CNOEX
		{
			auto size = xkb_state_key_get_utf8(get(), key, nullptr, 0)+1;
			Nyaanwork::Str name(size, '\0');
			size = xkb_state_key_get_utf8(get(), key, name.data(), size);
			name.resize(size);
			return name;
		}

		void update_mask(ModMask depressed_mods, ModMask latched_mods,
						 ModMask locked_mods,
						 LayoutIndex depressed_layout, LayoutIndex latched_layout,
						 LayoutIndex locked_layout)
		{
			xkb_state_update_mask(get(),
								depressed_mods, latched_mods, locked_mods,
								depressed_layout, latched_layout, locked_layout);
		}
	};
}
