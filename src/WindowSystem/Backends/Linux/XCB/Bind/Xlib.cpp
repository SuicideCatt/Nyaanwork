// EGL not work with XCB::Connection, but with Xlib::Display work

module;

#include <Core/Defines.hpp>

#include <X11/Xlib-xcb.h>

export module Nyaanwork.WindowSystem.Backends.Linux.XCB:Bind.Xlib;
import :Exception;

import Nyaanwork.Core.Utils.UPtr;

export namespace Xlib
{
	using EventQueueOwner = XEventQueueOwner;
	using enum EventQueueOwner;

	class Display final : private Nyaanwork::GetUPtr<::Display, &XCloseDisplay>
	{
	public:
		using unique_ptr::element_type;
		using unique_ptr::pointer;

		Display() = default;

		Display(const char* display)
			: unique_ptr(XOpenDisplay(display))
		{
			using namespace Nyaanwork::WindowSystem::Backends::Linux::XCB::Error;
			if (!operator bool()) NYAAN_UNLIK
				throw Exception(Code::fail_to_open_xlib_display);
		}

		Display(const Display&) = delete;
		Display& operator=(const Display&) = delete;

		Display(Display&&) = default;
		Display& operator=(Display&&) = default;

		~Display() = default;

		using unique_ptr::operator bool;

		using unique_ptr::get;

		using unique_ptr::release;
		using unique_ptr::reset;

		using unique_ptr::operator*;
		using unique_ptr::operator->;

		xcb_connection_t* get_xcb_connection() const
		{
			XSetEventQueueOwner(get(), XCBOwnsEventQueue);

			using namespace Nyaanwork::WindowSystem::Backends::Linux::XCB::Error;
			auto xcb = XGetXCBConnection(get());
			if (!xcb) NYAAN_UNLIK
				throw Exception(Code::cannot_get_xcb_connection);
			return xcb;
		}
	};
}
