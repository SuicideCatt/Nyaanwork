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
		#define CASE_RESIZE(name, text) CASE_TEXT(name##_resize, text "-resize")
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
		CASE_RESIZE(trdl, "nesw");
		CASE_RESIZE(tldr, "nwse");
		CASE_RESIZE(t, "n");
		CASE_RESIZE(d, "s");
		CASE_RESIZE(td, "ns");
		CASE_RESIZE(row, "row");
		CASE_RESIZE(l, "w");
		CASE_RESIZE(r, "e");
		CASE_RESIZE(lr, "ew");
		CASE_RESIZE(col, "col");
		CASE_RESIZE(tl, "nw");
		CASE_RESIZE(tr, "ne");
		CASE_RESIZE(dl, "sw");
		CASE_RESIZE(dr, "se");
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
