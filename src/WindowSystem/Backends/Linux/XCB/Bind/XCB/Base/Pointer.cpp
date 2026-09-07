module;

#include <WindowSystem/Backends/Linux/XCB/Defines.hpp>

#include <xcb/xcb.h>

export module Nyaanwork.WindowSystem.Backends.Linux.XCB:Bind.XCB.Base.Pointer;
import :Bind.XCB.Base.Core;
import :Bind.XCB.Base.Events;
import :Bind.XCB.Base.Window;
import :Bind.XCB.Helper;
import :Exception;

import Nyaanwork.Core.Types;

export namespace XCB::Pointer
{
	enum Mode : Nyaanwork::u8
	{
		sync = XCB_GRAB_MODE_SYNC,
		async = XCB_GRAB_MODE_ASYNC,
	};

	XCB_REQUEST_TYPE(Grab, xcb_grab_pointer);

	void ungrab(Connection* c, Timestamp time = current_time) noexcept
		{ xcb_ungrab_pointer(c, time); }

	void warp(Connection* c, Window::ID window, Nyaanwork::vec2<Nyaanwork::i16> pos)
		{ xcb_warp_pointer(c, none, window, 0, 0, 0, 0, pos.x, pos.y); }
}
