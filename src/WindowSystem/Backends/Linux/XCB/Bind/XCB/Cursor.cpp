module;

#include <WindowSystem/Backends/Linux/XCB/Defines.hpp>

#include <xcb/xcb_cursor.h>

export module Nyaanwork.WindowSystem.Backends.Linux.XCB:Bind.XCB.Cursor;
import :Bind.XCB.Base;
import :Bind.XCB.Helper;
import :Exception;

import Nyaanwork.Core.Utils.Flags;
import Nyaanwork.Core.Utils.PtrContainer;

export namespace XCB::Cursor
{
	using Cursor = xcb_cursor_t;

	class Context final
		: private Nyaanwork::GetUPtr<xcb_cursor_context_t,
									 &xcb_cursor_context_free>
	{
	public:
		using unique_ptr::element_type;
		using unique_ptr::pointer;

		Context() = default;

		Context(Connection* c, Screen* s)
		{
			pointer cnt;

			using namespace Nyaanwork::WindowSystem::Backends::Linux::XCB::Error;
			if (xcb_cursor_context_new(c, s, &cnt))
				throw Exception(Code::fail_to_create_xkb_context);

			reset(cnt);
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

		Cursor load(const Nyaanwork::Str& name) noexcept
			{ return xcb_cursor_load_cursor(get(), name.c_str()); }

		static void destory(Connection* c, Cursor cursor) noexcept
			{ xcb_free_cursor(c, cursor); }
	};
}
