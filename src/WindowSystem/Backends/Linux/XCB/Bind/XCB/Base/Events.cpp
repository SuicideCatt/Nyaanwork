module;

#include <WindowSystem/Backends/Linux/XCB/Defines.hpp>

#include <xcb/xcb.h>

#include <chrono>
#include <thread>

export module Nyaanwork.WindowSystem.Backends.Linux.XCB:Bind.XCB.Base.Events;
import :Bind.XCB.Base.Core;
import :Bind.XCB.Helper;
import :Exception;

import Nyaanwork.Core.Types;
import Nyaanwork.Core.Utils.Flags;

export namespace XCB::Events
{
	enum class Flags : Nyaanwork::u32
	{
		#define XCB_FLAG(name, X) XCB_FLAG_HELPER(name, XCB_EVENT_MASK, X)

		XCB_FLAG(key_press, KEY_PRESS),
		XCB_FLAG(key_release, KEY_RELEASE),
		XCB_FLAG(button_press, BUTTON_PRESS),
		XCB_FLAG(button_release, BUTTON_RELEASE),
		XCB_FLAG(enter_window, ENTER_WINDOW),
		XCB_FLAG(leave_window, LEAVE_WINDOW),
		XCB_FLAG(pointer_motion, POINTER_MOTION),
		XCB_FLAG(pointer_motion_hint, POINTER_MOTION_HINT),
		XCB_FLAG(button_1_motion, BUTTON_1_MOTION),
		XCB_FLAG(button_2_motion, BUTTON_2_MOTION),
		XCB_FLAG(button_3_motion, BUTTON_3_MOTION),
		XCB_FLAG(button_4_motion, BUTTON_4_MOTION),
		XCB_FLAG(button_5_motion, BUTTON_5_MOTION),
		XCB_FLAG(button_motion, BUTTON_MOTION),
		XCB_FLAG(keymap_state, KEYMAP_STATE),
		XCB_FLAG(exposure, EXPOSURE),
		XCB_FLAG(visibility_change, VISIBILITY_CHANGE),
		XCB_FLAG(structure_notify, STRUCTURE_NOTIFY),
		XCB_FLAG(resize_redirect, RESIZE_REDIRECT),
		XCB_FLAG(substructure_notify, SUBSTRUCTURE_NOTIFY),
		XCB_FLAG(substructure_redirect, SUBSTRUCTURE_REDIRECT),
		XCB_FLAG(focus_change, FOCUS_CHANGE),
		XCB_FLAG(property_change, PROPERTY_CHANGE),
		XCB_FLAG(color_map_change, COLOR_MAP_CHANGE),
		XCB_FLAG(owner_grab_button, OWNER_GRAB_BUTTON),

		#undef XCB_FLAG
	};
}

export template<>
struct Nyaanwork::FlagTraits<XCB::Events::Flags>
	: Nyaanwork::FlagInfo<XCB::Events::Flags>
{
	static constexpr bool is_bit_type = true;

	static constexpr Flags key = Type::key_press | Type::key_release;
	static constexpr Flags button = Type::button_press | Type::button_release;

	static constexpr Flags keyboard_focus = Type::focus_change;
	static constexpr Flags mouse_focus = Type::enter_window | Type::leave_window;
	static constexpr Flags focus = keyboard_focus | mouse_focus;

	static constexpr Flags pointer = Type::pointer_motion | Type::pointer_motion_hint;
	static constexpr Flags buttons_motion =
		Type::button_1_motion | Type::button_2_motion
		| Type::button_3_motion | Type::button_4_motion
		| Type::button_5_motion | Type::button_motion;

	static constexpr Flags substructure =
		Type::substructure_notify | Type::substructure_redirect;

	static constexpr Flags all =
		key | button | focus | pointer | buttons_motion | substructure
		| Type::keymap_state | Type::exposure | Type::visibility_change
		| Type::structure_notify | Type::resize_redirect | Type::property_change
		| Type::color_map_change | Type::owner_grab_button;
};

export namespace XCB::Events
{
	template<typename Ev>
	void send(Connection* c, ID window, Nyaanwork::Flags<Flags> selected,
			  const Ev& event, bool propagate = false) noexcept
	{
		xcb_send_event(c, propagate, window, selected,
					   reinterpret_cast<const char*>(&event));
	}

	using Generic = Helper::GetUPtr<xcb_generic_event_t>;
	template<typename R, typename P>
	Generic wait(Connection* c, bool pool, std::chrono::duration<R, P> wait_time)
	{
		xcb_generic_event_t* ev = nullptr;

		if (pool) NYAAN_UNLIK
		{
			std::this_thread::sleep_for(wait_time);
			ev = xcb_poll_for_event(c);
		}
		else
		{
			ev = xcb_wait_for_event(c);
		}

		return Generic(ev);
	}

	using Focus = xcb_focus_in_event_t;
	using MouseFocus = xcb_enter_notify_event_t;
	using ConfigureNotify = xcb_configure_notify_event_t;
	using PropertyNotify = xcb_property_notify_event_t;
	using ClientMessage = xcb_client_message_event_t;
	using SelectionNotify = xcb_selection_notify_event_t;
	using GE_Event = xcb_ge_generic_event_t;

	namespace Response
	{
		#define XCB_RESPONSE(name, X) constexpr Nyaanwork::u8 name = XCB_##X

		XCB_RESPONSE(focus_in, FOCUS_IN);
		XCB_RESPONSE(focus_out, FOCUS_OUT);
		XCB_RESPONSE(enter_notify, ENTER_NOTIFY);
		XCB_RESPONSE(leave_notify, LEAVE_NOTIFY);
		XCB_RESPONSE(configure_notify, CONFIGURE_NOTIFY);
		XCB_RESPONSE(property_notify, PROPERTY_NOTIFY);
		XCB_RESPONSE(client_message, CLIENT_MESSAGE);
		XCB_RESPONSE(selection_notify, SELECTION_NOTIFY);
		XCB_RESPONSE(ge_generic, GE_GENERIC);

		#undef XCB_RESPONSE
	};
}
