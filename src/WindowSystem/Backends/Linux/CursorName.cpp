export module Nyaanwork.WindowSystem.Backends.Linux.CursorName;

import Nyaanwork.Core.Types;
import Nyaanwork.WindowSystem.Backends.Base;

export namespace Nyaanwork::WindowSystem::Backends::Linux
{
	constexpr Str css_xcursor_name(Base::Window::Cursor cursor) noexcept
	{
		using enum Base::Window::Cursor;

		#define CASE_TEXT(name, text) case name: return text
		#define CASE(name) CASE_TEXT(name, #name)
		#define CASE_RESIZE(name) CASE_TEXT(name##_resize, #name "-resize")
		#define CASE_ZOOM(name) CASE_TEXT(zoom_##name, "zoom-" #name)
		switch (cursor)
		{
		CASE(progress);
		CASE(wait);
		CASE(text);
		CASE_TEXT(vertical_text, "vertical-text");
		CASE(crosshair);
		CASE(copy);
		CASE(alias);
		CASE(grab);
		CASE(grabbing);
		CASE_TEXT(no_drop, "no-drop");
		CASE_TEXT(not_allowed, "not-allowed");
		CASE(pointer);
		CASE_TEXT(context_menu, "context-menu");
		CASE(help);
		CASE(cell);
		CASE_RESIZE(trdl);
		CASE_RESIZE(tldr);
		CASE_RESIZE(t);
		CASE_RESIZE(d);
		CASE_RESIZE(td);
		CASE_RESIZE(row);
		CASE_RESIZE(l);
		CASE_RESIZE(r);
		CASE_RESIZE(lr);
		CASE_RESIZE(col);
		CASE_RESIZE(tl);
		CASE_RESIZE(tr);
		CASE_RESIZE(dl);
		CASE_RESIZE(dr);
		CASE(move);
		CASE_TEXT(all_scroll, "all-scroll");
		CASE_ZOOM(in);
		CASE_ZOOM(out);

		default:
			return "default";
		};
		#undef CASE_ZOOM
		#undef CASE_RESIZE
		#undef CASE
		#undef CASE_TEXT
	}
}
