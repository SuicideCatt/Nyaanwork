module;

#include <WindowSystem/Backends/Linux/XCB/Defines.hpp>

#include <xkbcommon/xkbcommon-x11.h>

#define explicit explicit_
#include <xcb/xkb.h>
#undef explicit

export module Nyaanwork.WindowSystem.Backends.Linux.XCB:Bind.XCB.XKB;
import :Bind.XCB.Base;
import :Bind.XCB.Helper;
import :Bind.XKB;
import :Exception;

import Nyaanwork.Core.Types;
import Nyaanwork.Core.Utils.Flags;

export namespace XCB::XKB
{
	using namespace ::XKB;

	using ID = Nyaanwork::u32;

	using Version = Nyaanwork::vec2<Nyaanwork::u16>;
	constexpr Version min_version = {
		XKB_X11_MIN_MAJOR_XKB_VERSION, XKB_X11_MIN_MINOR_XKB_VERSION
	};

	Context init_extension(Connection* c,
						   Nyaanwork::vec2<Nyaanwork::u16> version = min_version,
						   Nyaanwork::vec2<Nyaanwork::u16>* version_out = nullptr,
						   Nyaanwork::u8* base_event_out = nullptr,
						   Nyaanwork::u8* base_error_out = nullptr)
	{
		Nyaanwork::u16 *x_out = nullptr, *y_out = nullptr;
		if (version_out)
		{
			auto& [x, y] = *version_out;
			x_out = &x;
			y_out = &y;
		}

		bool success =
			xkb_x11_setup_xkb_extension(c, version.x, version.y,
										XKB_X11_SETUP_XKB_EXTENSION_NO_FLAGS,
										x_out, y_out,
										base_event_out, base_error_out);

		using namespace Nyaanwork::WindowSystem::Backends::Linux::XCB::Error;
		if (!success) NYAAN_UNLIK
			throw Exception(Code::fail_to_init_xkb);

		return Context(Context::FlagsT(0));
	}

	ID core_keyboard(Connection* c)
	{
		auto kb = xkb_x11_get_core_keyboard_device_id(c);

		using namespace Nyaanwork::WindowSystem::Backends::Linux::XCB::Error;
		if (kb == -1) NYAAN_UNLIK
			throw Exception(Code::fail_to_get_core_keyboard);

		return static_cast<Nyaanwork::u32>(kb);
	}

	Keymap create_keymap(const Context& cnt, Connection* c, ID device_id)
	{
		return Keymap(xkb_x11_keymap_new_from_device(cnt.get(), c, device_id,
													 XKB_KEYMAP_COMPILE_NO_FLAGS));
	}

	State create_state(const Keymap& km, Connection* c, ID device_id)
		{ return State(xkb_x11_state_new_from_device(km.get(), c, device_id)); }
}

export namespace XCB::XKB::Events
{
	enum class Flags : Nyaanwork::u16
	{
		#define XKB_FLAG(name, X) XCB_FLAG_HELPER(name, XCB_XKB_EVENT_TYPE, X)

		XKB_FLAG(new_keyboard_notify, NEW_KEYBOARD_NOTIFY),
		XKB_FLAG(map_notify, MAP_NOTIFY),
		XKB_FLAG(state_notify, STATE_NOTIFY),
		XKB_FLAG(controls_notify, CONTROLS_NOTIFY),
		XKB_FLAG(indicator_state_notify, INDICATOR_STATE_NOTIFY),
		XKB_FLAG(indicator_map_notify, INDICATOR_MAP_NOTIFY),
		XKB_FLAG(names_notify, NAMES_NOTIFY),
		XKB_FLAG(compat_map_notify, COMPAT_MAP_NOTIFY),
		XKB_FLAG(bell_notify, BELL_NOTIFY),
		XKB_FLAG(action_message, ACTION_MESSAGE),
		XKB_FLAG(access_x_notify, ACCESS_X_NOTIFY),
		XKB_FLAG(extension_device_notify, EXTENSION_DEVICE_NOTIFY),

		#undef XKB_FLAG
	};

	enum class DeviceID : Nyaanwork::u16
	{
		#define XKB_FLAG(name, X) XCB_FLAG_HELPER(name, XCB_XKB_ID, X)

		XKB_FLAG(use_core_kbd, USE_CORE_KBD),
		XKB_FLAG(use_core_ptr, USE_CORE_PTR),
		XKB_FLAG(dflt_xi_class, DFLT_XI_CLASS),
		XKB_FLAG(dflt_xi_id, DFLT_XI_ID),
		XKB_FLAG(all_xi_class, ALL_XI_CLASS),
		XKB_FLAG(all_xi_id, ALL_XI_ID),
		XKB_FLAG(xi_none, XI_NONE),

		#undef XKB_FLAG
	};

	namespace Response
	{
		constexpr Nyaanwork::u8 new_keyboard = XCB_XKB_NEW_KEYBOARD_NOTIFY;
		constexpr Nyaanwork::u8 state = XCB_XKB_STATE_NOTIFY;
	}

	using StateNotify = xcb_xkb_state_notify_event_t;
}

export template<>
struct Nyaanwork::FlagTraits<XCB::XKB::Events::Flags>
	: Nyaanwork::FlagInfo<XCB::XKB::Events::Flags>
{
	static constexpr bool is_bit_type = true;

	static constexpr Flags all =
		Type::new_keyboard_notify | Type::map_notify | Type::state_notify
			| Type::controls_notify | Type::indicator_state_notify
			| Type::indicator_map_notify | Type::names_notify
			| Type::compat_map_notify | Type::bell_notify | Type::action_message
			| Type::access_x_notify | Type::extension_device_notify;
};

export namespace XCB::XKB::Events
{
	using FlagsT = Nyaanwork::Flags<Flags>;

	void select(Connection *c, DeviceID device_spec,
				FlagsT affect_which, FlagsT clear, FlagsT select_all,
				FlagsT affect_map, FlagsT map,
				const void* details = nullptr) noexcept
	{
		xcb_xkb_select_events(c, static_cast<Nyaanwork::u16>(device_spec),
							  affect_which, clear,
							  select_all, affect_map, map, details);
	}
}
