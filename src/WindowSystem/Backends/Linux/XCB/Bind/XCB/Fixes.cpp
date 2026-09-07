module;

#include <WindowSystem/Backends/Linux/XCB/Defines.hpp>

#include <xcb/xfixes.h>

export module Nyaanwork.WindowSystem.Backends.Linux.XCB:Bind.XCB.Fixes;
import :Bind.XCB.Base;
import :Bind.XCB.Helper;

export namespace XCB::Fixes
{
	void show(Connection* c, Window::ID window) noexcept
		{ xcb_xfixes_show_cursor(c, window); }
	void hide(Connection* c, Window::ID window) noexcept
		{ xcb_xfixes_hide_cursor(c, window); }
}

export namespace XCB::Fixes::Request
{
	XCB_REQUEST_TYPE(QueryVersion, xcb_xfixes_query_version);
}
