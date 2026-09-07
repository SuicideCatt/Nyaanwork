module;

#include <WindowSystem/Backends/Linux/XCB/Defines.hpp>

#include <xcb/xcb.h>

#include <concepts>
#include <span>

export module Nyaanwork.WindowSystem.Backends.Linux.XCB:Bind.XCB.Base.Window;
import :Bind.XCB.Base.Core;
import :Bind.XCB.Base.Events;
import :Bind.XCB.Helper;
import :Exception;

import Nyaanwork.Core.Types;
import Nyaanwork.Core.Utils.Flags;

export namespace XCB
{
	using VisualID = xcb_visualid_t;
}

export namespace XCB::Window
{
	using ID = xcb_window_t;
	static_assert(std::same_as<ID, XCB::ID>);

	enum class Class : Nyaanwork::u16
	{
		#define XCB_FLAG(name, X) XCB_FLAG_HELPER(name, XCB_WINDOW_CLASS, X)

		XCB_FLAG(copy_from_parent, COPY_FROM_PARENT),
		XCB_FLAG(input_output, INPUT_OUTPUT),
		XCB_FLAG(input_only, INPUT_ONLY),

		#undef XCB_FLAG
	};

	enum class CW : Nyaanwork::u32
	{
		#define XCB_FLAG(name, X) XCB_FLAG_HELPER(name, XCB_CW, X)

		XCB_FLAG(back_pixmap, BACK_PIXMAP),
		XCB_FLAG(back_pixel, BACK_PIXEL),
		XCB_FLAG(border_pixmap, BORDER_PIXMAP),
		XCB_FLAG(border_pixel, BORDER_PIXEL),
		XCB_FLAG(bit_gravity, BIT_GRAVITY),
		XCB_FLAG(win_gravity, WIN_GRAVITY),
		XCB_FLAG(backing_store, BACKING_STORE),
		XCB_FLAG(backing_planes, BACKING_PLANES),
		XCB_FLAG(backing_pixel, BACKING_PIXEL),
		XCB_FLAG(override_redirect, OVERRIDE_REDIRECT),
		XCB_FLAG(save_under, SAVE_UNDER),
		XCB_FLAG(event_mask, EVENT_MASK),
		XCB_FLAG(dont_propagate, DONT_PROPAGATE),
		XCB_FLAG(colormap, COLORMAP),
		XCB_FLAG(cursor, CURSOR),

		#undef XCB_FLAG
	};

	enum class Config : Nyaanwork::u16
	{
		#define XCB_FLAG(name, X) XCB_FLAG_HELPER(name, XCB_CONFIG_WINDOW, X)

		XCB_FLAG(x, X),
		XCB_FLAG(y, Y),
		XCB_FLAG(width, WIDTH),
		XCB_FLAG(height, HEIGHT),
		XCB_FLAG(border_width, BORDER_WIDTH),
		XCB_FLAG(sibling, SIBLING),
		XCB_FLAG(stack_mode, STACK_MODE),

		#undef XCB_FLAG
	};
}

export template<>
struct Nyaanwork::FlagTraits<XCB::Window::CW>
	: Nyaanwork::FlagInfo<XCB::Window::CW>
{
	static constexpr bool is_bit_type = true;

	static constexpr Flags all =
		Type::back_pixmap | Type::back_pixel | Type::border_pixmap
		| Type::border_pixel | Type::bit_gravity | Type::win_gravity
		| Type::backing_store | Type::backing_planes | Type::backing_pixel
		| Type::override_redirect | Type::save_under | Type::event_mask
		| Type::dont_propagate | Type::colormap | Type::cursor;
};

export template<>
struct Nyaanwork::FlagTraits<XCB::Window::Config>
	: Nyaanwork::FlagInfo<XCB::Window::Config>
{
	static constexpr bool is_bit_type = true;

	static constexpr Flags position = Type::x | Type::y;
	static constexpr Flags resolution = Type::width | Type::height;

	static constexpr Flags all = position | resolution
		| Type::border_width | Type::sibling | Type::stack_mode ;
};

export namespace XCB::Window
{
	using CreateValueList = xcb_create_window_value_list_t;
	void create(Connection* c, Nyaanwork::u8 depth, ID id, ID parent,
				Nyaanwork::vec2<Nyaanwork::i16> position,
				Nyaanwork::vec2<Nyaanwork::u16> resolution,
				Nyaanwork::u16 border_width, Class class_, VisualID visual,
				Nyaanwork::Flags<CW> use_from_list,
				const CreateValueList& list) noexcept
	{
		xcb_create_window_aux(
			c, depth, id, parent,
			position.x, position.y, resolution.x, resolution.y, border_width,
			static_cast<Nyaanwork::u16>(class_), visual, use_from_list, &list
		);
	}

	void map(Connection* c, ID id) noexcept
		{ xcb_map_window(c, id); }

	using ConfigValueList = xcb_configure_window_value_list_t;
	void configure(Connection* c, ID id,
				   Nyaanwork::Flags<Config> use_from_list,
				   const ConfigValueList& list) noexcept
		{ xcb_configure_window_aux(c, id, use_from_list, &list); }

	using ChangeAttributesList = xcb_change_window_attributes_value_list_t;
	void change_attributes(Connection* c, ID id,
						   Nyaanwork::Flags<CW> use_from_list,
						   const ChangeAttributesList& list) noexcept
		{ xcb_change_window_attributes_aux(c, id, use_from_list, &list); }

	template<Nyaanwork::is_integral T, is_atom AtomProp, is_atom AtomType>
	requires (sizeof(T) <= 4)
	void change_property(Connection* c, ID id,
						 PropertyChangeMode mode,
						 AtomProp property, AtomType type,
						 std::span<T const> data) noexcept
	{
		xcb_change_property(c, static_cast<Nyaanwork::u8>(mode), id,
							static_cast<Atom>(property), static_cast<Atom>(type),
							sizeof(T)*8, data.size(), data.data());
	}

	XCB_REQUEST_TYPE(GetProperty, xcb_get_property);
	template<Nyaanwork::is_integral Type,
			 is_atom AtomProp, is_atom AtomType = Atoms>
	requires (sizeof(Type) <= 4)
	Nyaanwork::Pair<GetProperty::Reply, std::span<Type const>>
		get_property(Connection* c, ID id,
					 AtomProp property, AtomType type = Atoms::any,
					 bool delete_ = false)
	{
		using GP = GetProperty;
		Nyaanwork::Pair<GetProperty::Reply, std::span<Type const>> res;
		auto& [r, s] = res;

		r = GP::request_reply(c, false, id,
							  static_cast<Atom>(property), static_cast<Atom>(type),
							  0, 0);
		r = GP::request_reply(c, delete_, id,
							  static_cast<Atom>(property), static_cast<Atom>(type),
							  0, r->bytes_after);

		using namespace Nyaanwork::WindowSystem::Backends::Linux::XCB::Error;
		if (r->format != sizeof(Type)*8)
			throw Exception(Code::wrong_format);

		auto raw = xcb_get_property_value(r.get());
		s = {reinterpret_cast<const Type*>(raw), r->value_len};

		return res;
	}

	template<is_atom AtomSelect, is_atom AtomTarget, is_atom AtomProp>
	void convert_selection(Connection* c, ID requestor, Timestamp time,
					AtomSelect selection, AtomTarget target, AtomProp property)
	{
		xcb_convert_selection(c, requestor,
							  static_cast<Atom>(selection),
							  static_cast<Atom>(target),
							  static_cast<Atom>(property),
							  time);
	}

	void destroy(Connection* c, ID id) noexcept
		{ xcb_destroy_window(c, id); }
}
