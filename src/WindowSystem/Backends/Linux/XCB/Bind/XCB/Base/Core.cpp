module;

#include <WindowSystem/Backends/Linux/XCB/Defines.hpp>

#include <xcb/xcb.h>

#include <concepts>

export module Nyaanwork.WindowSystem.Backends.Linux.XCB:Bind.XCB.Base.Core;
import :Bind.XCB.Helper;
import :Exception;

import Nyaanwork.Core.Types;

export namespace XCB
{
	constexpr auto none = XCB_NONE;
	constexpr auto copy_from_parent = XCB_COPY_FROM_PARENT;
	constexpr auto current_time = XCB_CURRENT_TIME;

	using Atom = xcb_atom_t;
	enum class Atoms : Atom
	{
		#define XCB_FLAG(name, X) XCB_FLAG_HELPER(name, XCB_ATOM, X)

		XCB_FLAG(none, NONE),
		XCB_FLAG(any, ANY),
		XCB_FLAG(primary, PRIMARY),
		XCB_FLAG(secondary, SECONDARY),
		XCB_FLAG(arc, ARC),
		XCB_FLAG(atom, ATOM),
		XCB_FLAG(bitmap, BITMAP),
		XCB_FLAG(cardinal, CARDINAL),
		XCB_FLAG(colormap, COLORMAP),
		XCB_FLAG(cursor, CURSOR),
		XCB_FLAG(cut_buffer0, CUT_BUFFER0),
		XCB_FLAG(cut_buffer1, CUT_BUFFER1),
		XCB_FLAG(cut_buffer2, CUT_BUFFER2),
		XCB_FLAG(cut_buffer3, CUT_BUFFER3),
		XCB_FLAG(cut_buffer4, CUT_BUFFER4),
		XCB_FLAG(cut_buffer5, CUT_BUFFER5),
		XCB_FLAG(cut_buffer6, CUT_BUFFER6),
		XCB_FLAG(cut_buffer7, CUT_BUFFER7),
		XCB_FLAG(drawable, DRAWABLE),
		XCB_FLAG(font, FONT),
		XCB_FLAG(integer, INTEGER),
		XCB_FLAG(pixmap, PIXMAP),
		XCB_FLAG(point, POINT),
		XCB_FLAG(rectangle, RECTANGLE),
		XCB_FLAG(resource_manager, RESOURCE_MANAGER),
		XCB_FLAG(rgb_color_map, RGB_COLOR_MAP),
		XCB_FLAG(rgb_best_map, RGB_BEST_MAP),
		XCB_FLAG(rgb_blue_map, RGB_BLUE_MAP),
		XCB_FLAG(rgb_default_map, RGB_DEFAULT_MAP),
		XCB_FLAG(rgb_gray_map, RGB_GRAY_MAP),
		XCB_FLAG(rgb_green_map, RGB_GREEN_MAP),
		XCB_FLAG(rgb_red_map, RGB_RED_MAP),
		XCB_FLAG(string, STRING),
		XCB_FLAG(visualid, VISUALID),
		XCB_FLAG(window, WINDOW),
		XCB_FLAG(wm_command, WM_COMMAND),
		XCB_FLAG(wm_hints, WM_HINTS),
		XCB_FLAG(wm_client_machine, WM_CLIENT_MACHINE),
		XCB_FLAG(wm_icon_name, WM_ICON_NAME),
		XCB_FLAG(wm_icon_size, WM_ICON_SIZE),
		XCB_FLAG(wm_name, WM_NAME),
		XCB_FLAG(wm_normal_hints, WM_NORMAL_HINTS),
		XCB_FLAG(wm_size_hints, WM_SIZE_HINTS),
		XCB_FLAG(wm_zoom_hints, WM_ZOOM_HINTS),
		XCB_FLAG(min_space, MIN_SPACE),
		XCB_FLAG(norm_space, NORM_SPACE),
		XCB_FLAG(max_space, MAX_SPACE),
		XCB_FLAG(end_space, END_SPACE),
		XCB_FLAG(superscript_x, SUPERSCRIPT_X),
		XCB_FLAG(superscript_y, SUPERSCRIPT_Y),
		XCB_FLAG(subscript_x, SUBSCRIPT_X),
		XCB_FLAG(subscript_y, SUBSCRIPT_Y),
		XCB_FLAG(underline_position, UNDERLINE_POSITION),
		XCB_FLAG(underline_thickness, UNDERLINE_THICKNESS),
		XCB_FLAG(strikeout_ascent, STRIKEOUT_ASCENT),
		XCB_FLAG(strikeout_descent, STRIKEOUT_DESCENT),
		XCB_FLAG(italic_angle, ITALIC_ANGLE),
		XCB_FLAG(x_height, X_HEIGHT),
		XCB_FLAG(quad_width, QUAD_WIDTH),
		XCB_FLAG(weight, WEIGHT),
		XCB_FLAG(point_size, POINT_SIZE),
		XCB_FLAG(resolution, RESOLUTION),
		XCB_FLAG(copyright, COPYRIGHT),
		XCB_FLAG(notice, NOTICE),
		XCB_FLAG(font_name, FONT_NAME),
		XCB_FLAG(family_name, FAMILY_NAME),
		XCB_FLAG(full_name, FULL_NAME),
		XCB_FLAG(cap_height, CAP_HEIGHT),
		XCB_FLAG(wm_class, WM_CLASS),
		XCB_FLAG(wm_transient_for, WM_TRANSIENT_FOR),

		#undef XCB_FLAG
	};
	template<typename _Atom>
	concept is_atom = (std::same_as<_Atom, Atom> || std::same_as<_Atom, Atoms>);

	using Connection = xcb_connection_t;
	using Screen = xcb_screen_t;
	using Data = Nyaanwork::Pair<Connection*, Screen*>;
	using CData = Nyaanwork::Pair<Connection*, const Screen*>;

	using Timestamp = xcb_timestamp_t;

	using ID = Nyaanwork::u32;
	ID generate_id(Connection* c)
	{
		ID id = xcb_generate_id(c);

		using namespace Nyaanwork::WindowSystem::Backends::Linux::XCB::Error;
		if (id == static_cast<Nyaanwork::u32>(-1)) NYAAN_UNLIK
			throw Exception(Code::fail_to_create_id);

		return id;
	}

	Screen* root_screen(Connection* c) noexcept
		{ return xcb_setup_roots_iterator(xcb_get_setup(c)).data; }

	int flush(Connection* c) noexcept
		{ return xcb_flush(c); }

	enum class PropertyChangeMode : Nyaanwork::u8
	{
		#define XCB_FLAG(name, X_name) \
			name = XCB_PROP_MODE_##X_name

		XCB_FLAG(replace, REPLACE),
		XCB_FLAG(prepend, PREPEND),
		XCB_FLAG(append, APPEND),

		#undef XCB_FLAG
	};
}

export namespace XCB::Request
{
	XCB_REQUEST_TYPE(QueryExtension, xcb_query_extension);
	XCB_REQUEST_TYPE(InternAtom, xcb_intern_atom);
}
