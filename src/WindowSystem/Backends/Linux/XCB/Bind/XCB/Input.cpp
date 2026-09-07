module;

#include <WindowSystem/Backends/Linux/XCB/Defines.hpp>

#include <xcb/xinput.h>

#include <span>

export module Nyaanwork.WindowSystem.Backends.Linux.XCB:Bind.XCB.Input;
import :Bind.XCB.Base;
import :Bind.XCB.Helper;

import Nyaanwork.Core.Utils.Flags;
import Nyaanwork.Core.Utils.PtrContainer;

namespace XCB::Input
{
	enum class EventFlags
		: Nyaanwork::EnumTraits<xcb_input_xi_event_mask_t>::MaskType
	{
		#define XCB_FLAG(name, X) XCB_FLAG_HELPER(name, XCB_INPUT_XI_EVENT_MASK, X)

		XCB_FLAG(device_changed, DEVICE_CHANGED),
		XCB_FLAG(key_press, KEY_PRESS),
		XCB_FLAG(key_release, KEY_RELEASE),
		XCB_FLAG(button_press, BUTTON_PRESS),
		XCB_FLAG(button_release, BUTTON_RELEASE),
		XCB_FLAG(motion, MOTION),
		XCB_FLAG(enter, ENTER),
		XCB_FLAG(leave, LEAVE),
		XCB_FLAG(focus_in, FOCUS_IN),
		XCB_FLAG(focus_out, FOCUS_OUT),
		XCB_FLAG(hierarchy, HIERARCHY),
		XCB_FLAG(property, PROPERTY),
		XCB_FLAG(raw_key_press, RAW_KEY_PRESS),
		XCB_FLAG(raw_key_release, RAW_KEY_RELEASE),
		XCB_FLAG(raw_button_press, RAW_BUTTON_PRESS),
		XCB_FLAG(raw_button_release, RAW_BUTTON_RELEASE),
		XCB_FLAG(raw_motion, RAW_MOTION),
		XCB_FLAG(touch_begin, TOUCH_BEGIN),
		XCB_FLAG(touch_update, TOUCH_UPDATE),
		XCB_FLAG(touch_end, TOUCH_END),
		XCB_FLAG(touch_ownership, TOUCH_OWNERSHIP),
		XCB_FLAG(raw_touch_begin, RAW_TOUCH_BEGIN),
		XCB_FLAG(raw_touch_update, RAW_TOUCH_UPDATE),
		XCB_FLAG(raw_touch_end, RAW_TOUCH_END),
		XCB_FLAG(barrier_hit, BARRIER_HIT),
		XCB_FLAG(barrier_leave, BARRIER_LEAVE),

		#undef XCB_FLAG
	};
}

export template<>
struct Nyaanwork::FlagTraits<XCB::Input::EventFlags>
	: Nyaanwork::FlagInfo<XCB::Input::EventFlags>
{
	static constexpr bool is_bit_type = true;

	static constexpr Flags device =
		Type::device_changed | Type::hierarchy | Type::property;

	static constexpr Flags key = Type::key_press | Type::key_release;
	static constexpr Flags button = Type::button_press | Type::button_release;
	static constexpr Flags kb = key | button;

	static constexpr Flags raw_key = Type::raw_key_press | Type::raw_key_release;
	static constexpr Flags raw_button =
		Type::raw_button_press | Type::raw_button_release;
	static constexpr Flags raw_kb = raw_key | raw_button;

	static constexpr Flags all_kb = kb | raw_kb;

	static constexpr Flags mouse_focus = Type::enter | Type::leave;
	static constexpr Flags keyboard_focus = Type::focus_in | Type::focus_out;
	static constexpr Flags focus = mouse_focus | keyboard_focus;

	static constexpr Flags all_motion = Type::motion | Type::raw_motion;

	static constexpr Flags touch = Type::touch_begin | Type::touch_update
									| Type::touch_end | Type::touch_ownership;
	static constexpr Flags raw_touch = Type::raw_touch_begin | Type::raw_touch_update
										| Type::raw_touch_end;
	static constexpr Flags all_touch = touch | raw_touch;

	static constexpr Flags barrier = Type::barrier_hit | Type::barrier_leave;

	static constexpr Flags all =
		all_kb | focus | all_motion | all_touch | barrier | device;
};

export namespace XCB::Input
{
	using fp3232 = xcb_input_fp3232_t;
	Nyaanwork::f64 f64_from_fp3232(fp3232 fp) noexcept
	{
		using R = Nyaanwork::f64;
		using Frac = decltype(fp.frac);
		using L = Nyaanwork::limits<Frac>;

		auto frac = static_cast<R>(fp.frac) / static_cast<R>(L::max());
		return static_cast<R>(fp.integral) + frac;
	}

	using DeviceID = xcb_input_device_id_t;
	constexpr DeviceID all_devices = XCB_INPUT_DEVICE_ALL;
	constexpr DeviceID all_master_devices = XCB_INPUT_DEVICE_ALL_MASTER;

	using DeviceInfo = xcb_input_device_info_t;

	using XI_DeviceInfo = xcb_input_xi_device_info_t;
	using XI_DeviceInfos = Helper::Range<xcb_input_xi_query_device_infos_iterator,
										 xcb_input_xi_device_info_next>;

	using DeviceClass = xcb_input_device_class_t;
	using DeviceClassData = xcb_input_device_class_data_t;
	using DeviceClasses = Helper::Range<xcb_input_xi_device_info_classes_iterator,
										xcb_input_device_class_next>;

	enum class DeviceClassType : Nyaanwork::u16
	{
		#define XCB_FLAG(name, X) \
			XCB_FLAG_HELPER(name, XCB_INPUT_DEVICE_CLASS_TYPE, X)

		XCB_FLAG(key, KEY),
		XCB_FLAG(button, BUTTON),
		XCB_FLAG(valuator, VALUATOR),
		XCB_FLAG(scroll, SCROLL),
		XCB_FLAG(touch, TOUCH),
		XCB_FLAG(gesture, GESTURE),

		#undef XCB_FLAG
	};

	enum class ValuatorMode : Nyaanwork::u8
	{
		#define XCB_FLAG(name, X) XCB_FLAG_HELPER(name, XCB_INPUT_VALUATOR_MODE, X)

		XCB_FLAG(relative, RELATIVE),
		XCB_FLAG(absolute, ABSOLUTE),

		#undef XCB_FLAG
	};

	using HierarchyInfo = xcb_input_hierarchy_info_t;

	enum class HierarchyFlags
		: Nyaanwork::EnumTraits<xcb_input_hierarchy_mask_t>::MaskType
	{
		#define XCB_FLAG(name, X) XCB_FLAG_HELPER(name, XCB_INPUT_HIERARCHY_MASK, X)

		XCB_FLAG(master_added, MASTER_ADDED),
		XCB_FLAG(master_removed, MASTER_REMOVED),
		XCB_FLAG(slave_added, SLAVE_ADDED),
		XCB_FLAG(slave_removed, SLAVE_REMOVED),
		XCB_FLAG(slave_attached, SLAVE_ATTACHED),
		XCB_FLAG(slave_detached, SLAVE_DETACHED),
		XCB_FLAG(device_enabled, DEVICE_ENABLED),
		XCB_FLAG(device_disabled, DEVICE_DISABLED),

		#undef XCB_FLAG
	};
}

export constexpr bool operator==(Nyaanwork::u16 a,
								 XCB::Input::DeviceClassType b) noexcept
	{ return a == Nyaanwork::EnumTraits<XCB::Input::DeviceClassType>::cast(b); }
export constexpr bool operator==(XCB::Input::DeviceClassType a,
								 Nyaanwork::u16 b) noexcept
	{ return b == a; }

export constexpr bool operator==(xcb_input_device_class_type_t a,
								 XCB::Input::DeviceClassType b) noexcept
	{ return a == Nyaanwork::EnumTraits<XCB::Input::DeviceClassType>::cast(b); }
export constexpr bool operator==(XCB::Input::DeviceClassType a,
								 xcb_input_device_class_type_t b) noexcept
	{ return b == a; }

export constexpr bool operator==(Nyaanwork::u8 a,
								 XCB::Input::ValuatorMode b) noexcept
	{ return a == Nyaanwork::EnumTraits<XCB::Input::ValuatorMode>::cast(b); }
export constexpr bool operator==(XCB::Input::ValuatorMode a,
								 Nyaanwork::u8 b) noexcept
	{ return b == a; }

export constexpr bool operator==(xcb_input_valuator_mode_t a,
								 XCB::Input::ValuatorMode b) noexcept
	{ return a == Nyaanwork::EnumTraits<XCB::Input::ValuatorMode>::cast(b); }
export constexpr bool operator==(XCB::Input::ValuatorMode a,
								 xcb_input_valuator_mode_t b) noexcept
	{ return b == a; }

export template<>
struct Nyaanwork::FlagTraits<XCB::Input::HierarchyFlags>
	: Nyaanwork::FlagInfo<XCB::Input::HierarchyFlags>
{
	static constexpr bool is_bit_type = true;

	static constexpr Flags master = Type::master_added | Type::master_removed;
	static constexpr Flags slave = Type::slave_added | Type::slave_removed
									| Type::slave_attached | Type::slave_detached;
	static constexpr Flags device = Type::device_enabled | Type::device_disabled;

	static constexpr Flags add = Type::slave_added | Type::device_enabled;
	static constexpr Flags remove = Type::slave_removed | Type::device_disabled;

	static constexpr Flags all = master | slave | device | add | remove;
};

namespace XCB::Helper
{
	template<>
	struct View<xcb_input_raw_motion_event_t>
		: private xcb_input_raw_motion_event_t
	{
		using Type = xcb_input_raw_motion_event_t;

		View() = delete;

		Type* get() noexcept
			{ return static_cast<Type*>(this); }
		const Type* get() NYAAN_CNOEX
			{ return static_cast<const Type*>(this); }

		Type& operator*() noexcept
			{ return *get(); }
		const Type& operator*() NYAAN_CNOEX
			{ return *get(); }

		Type* operator->() noexcept
			{ return get(); }
		const Type* operator->() NYAAN_CNOEX
			{ return get(); }

		std::span<Input::fp3232 const> axisvalues() NYAAN_CNOEX
		{
			return std::span(
				xcb_input_raw_button_press_axisvalues_raw(get()),
				xcb_input_raw_button_press_axisvalues_length(get())
			);
		}

		std::span<Nyaanwork::u32 const> valuator_mask() NYAAN_CNOEX
		{
			return std::span(
				xcb_input_raw_button_press_valuator_mask(get()),
				xcb_input_raw_button_press_valuator_mask_length(get())
			);
		}

		using Type::response_type;
		using Type::extension;
		using Type::sequence;
		using Type::length;
		using Type::event_type;
		using Type::deviceid;
		using Type::time;
		using Type::detail;
		using Type::sourceid;
		using Type::valuators_len;
		using Type::flags;
		using Type::full_sequence;
	};

	template<>
	struct View<xcb_input_hierarchy_event_t>
		: private xcb_input_hierarchy_event_t
	{
		using Type = xcb_input_hierarchy_event_t;

		View() = delete;

		Type* get() noexcept
			{ return static_cast<Type*>(this); }
		const Type* get() NYAAN_CNOEX
			{ return static_cast<const Type*>(this); }

		Type& operator*() noexcept
			{ return *get(); }
		const Type& operator*() NYAAN_CNOEX
			{ return *get(); }

		Type* operator->() noexcept
			{ return get(); }
		const Type* operator->() NYAAN_CNOEX
			{ return get(); }

		std::span<Input::HierarchyInfo const> infos() NYAAN_CNOEX
		{
			return std::span(
				xcb_input_hierarchy_infos(get()),
				xcb_input_hierarchy_infos_length(get())
			);
		};

		using Type::response_type;
		using Type::extension;
		using Type::sequence;
		using Type::length;
		using Type::event_type;
		using Type::deviceid;
		using Type::time;
		using Type::flags;
		using Type::num_infos;
		using Type::full_sequence;
	};
}

export namespace XCB::Input
{
	struct Events
	{
		using Flags = EventFlags;

		using Key = xcb_input_key_press_event_t;
		using RawKey = xcb_input_raw_key_press_event_t;
		using RawButton = xcb_input_raw_button_press_event_t;

		using Motion = xcb_input_motion_event_t;
		using RawMotion = Helper::View<xcb_input_raw_motion_event_t>;

		using Hierarchy = Helper::View<xcb_input_hierarchy_event_t>;
		using Property = xcb_input_property_event_t;

		struct Response
		{
			#define XCB_INPUT_RESPONSE(name, X) \
				static constexpr Nyaanwork::u16 name = XCB_INPUT_##X;

			XCB_INPUT_RESPONSE(key_press, KEY_PRESS)
			XCB_INPUT_RESPONSE(raw_key_press, RAW_KEY_PRESS)
			XCB_INPUT_RESPONSE(raw_key_release, RAW_KEY_RELEASE)
			XCB_INPUT_RESPONSE(raw_button_press, RAW_BUTTON_PRESS)
			XCB_INPUT_RESPONSE(raw_button_release, RAW_BUTTON_RELEASE)
			XCB_INPUT_RESPONSE(motion, MOTION)
			XCB_INPUT_RESPONSE(raw_motion, RAW_MOTION)
			XCB_INPUT_RESPONSE(hierarchy, HIERARCHY)
			XCB_INPUT_RESPONSE(property, PROPERTY)

			#undef XCB_INPUT_RESPONSE
		};

		Events(DeviceID device, Nyaanwork::Flags<Flags> events) noexcept
			: head(device, sizeof(Flags) / sizeof(Nyaanwork::u32)),
			  events(events) {}

		void select(Connection *c, Window::ID window) noexcept
			{ xcb_input_xi_select_events(c, window, 1, &head); }

		xcb_input_event_mask_t head = {};
		Nyaanwork::Flags<Flags> events = {};
	};
}

export namespace XCB::Input::Property
{
	enum class Flag : Nyaanwork::u8
	{
		#define XCB_FLAG(name, X) XCB_FLAG_HELPER(name, XCB_INPUT_PROPERTY_FLAG, X)

		XCB_FLAG(deleted, DELETED),
		XCB_FLAG(created, CREATED),
		XCB_FLAG(modified, MODIFIED),

		#undef XCB_FLAG
	};

	XCB_REQUEST_TYPE(GetProperty, xcb_input_xi_get_property);
	template<Nyaanwork::is_integral Type,
			 is_atom AtomProp, is_atom AtomType = Atoms>
	requires (sizeof(Type) <= 4)
	Nyaanwork::Pair<GetProperty::Reply, std::span<Type const>>
		get_property(Connection* c, DeviceID id,
					 AtomProp property, AtomType type = Atoms::any,
					 bool delete_ = false)
	{
		using GP = GetProperty;
		Nyaanwork::Pair<GetProperty::Reply, std::span<Type const>> res;
		auto& [r, s] = res;

		r = GP::request_reply(c, id, false,
							  static_cast<Atom>(property), static_cast<Atom>(type),
							  0, 0);
		r = GP::request_reply(c, id, delete_,
							  static_cast<Atom>(property), static_cast<Atom>(type),
							  0, r->bytes_after);

		using namespace Nyaanwork::WindowSystem::Backends::Linux::XCB::Error;
		if (r->format != sizeof(Type)*8)
			throw Exception(Code::wrong_format);

		auto raw = xcb_input_xi_get_property_items(r.get());
		s = {reinterpret_cast<const Type*>(raw), r->num_items};

		return res;
	}
}

export constexpr bool operator==(Nyaanwork::u8 a,
								 XCB::Input::Property::Flag b) noexcept
	{ return a == Nyaanwork::EnumTraits<XCB::Input::Property::Flag>::cast(b); }
export constexpr bool operator==(XCB::Input::Property::Flag a,
								 Nyaanwork::u8 b) noexcept
	{ return b == a; }

export constexpr bool operator==(xcb_input_property_flag_t a,
								 XCB::Input::Property::Flag b) noexcept
	{ return a == Nyaanwork::EnumTraits<XCB::Input::Property::Flag>::cast(b); }
export constexpr bool operator==(XCB::Input::Property::Flag a,
								 xcb_input_property_flag_t b) noexcept
	{ return b == a; }

export namespace XCB::Input::Request
{
	XCB_REQUEST_TYPE(ListDevices, xcb_input_list_input_devices);
	XCB_REQUEST_TYPE(QueryDevice, xcb_input_xi_query_device);
	XCB_REQUEST_TYPE(QueryVersion, xcb_input_xi_query_version);
}

export namespace XCB::Helper
{
	template<>
	struct View<GetUPtr<xcb_input_list_input_devices_reply_t>> final
		: private GetUPtr<xcb_input_list_input_devices_reply_t>
	{
		using unique_ptr::element_type;
		using unique_ptr::pointer;

		using Type = element_type;

		View(unique_ptr reply = nullptr)
			: unique_ptr(std::move(reply)) {}

		View(const View&) = delete;
		View& operator=(const View&) = delete;

		View(View&&) = default;
		View& operator=(View&&) = default;

		~View() = default;

		using unique_ptr::operator bool;

		using unique_ptr::get;

		using unique_ptr::release;
		using unique_ptr::reset;

		using unique_ptr::operator*;
		using unique_ptr::operator->;

		std::span<Input::DeviceInfo const> devices() NYAAN_CNOEX
		{
			if (!*this) NYAAN_UNLIK
				return {};

			return std::span(
				xcb_input_list_input_devices_devices(get()),
				xcb_input_list_input_devices_devices_length(get())
			);
		}
	};

	template<>
	class View<Input::DeviceClass> final
	{
	public:
		using Type = Input::DeviceClass;

		using PtrContainer = Nyaanwork::PtrContainer<Type>;

		View() = default;

		View(Type& class_)
			: m_class(class_) {}

		Type* get() NYAAN_CNOEX
			{ return m_class.get(); }

		const Type& operator*() NYAAN_CNOEX(!Nyaanwork::is_debug)
			{ return *m_class; }

		const Type* operator->() NYAAN_CNOEX(!Nyaanwork::is_debug)
			{ return m_class.operator->(); }

		void unpack(Input::DeviceClassData& data) NYAAN_CNOEX
		{
			if (!m_class) NYAAN_UNLIK
				return;

			auto raw = xcb_input_device_class_data(get());
			xcb_input_device_class_data_unpack(raw, m_class->type, &data);
		}

	private:
		PtrContainer m_class;
	};

	template<>
	class View<Input::XI_DeviceInfo> final
	{
	public:
		using Type = Input::XI_DeviceInfo;

		using PtrContainer = Nyaanwork::PtrContainer<Type>;

		View() = default;

		View(Type& info)
			: m_info(info) {}

		Type* get() NYAAN_CNOEX
			{ return m_info.get(); }

		const Type& operator*() NYAAN_CNOEX(!Nyaanwork::is_debug)
			{ return *m_info; }

		const Type* operator->() NYAAN_CNOEX(!Nyaanwork::is_debug)
			{ return m_info.operator->(); }

		Input::DeviceClasses classes() NYAAN_CNOEX
		{
			if (!m_info) NYAAN_UNLIK
				return {};

			return {get()};
		}

	private:
		PtrContainer m_info;
	};

	template<>
	struct View<GetUPtr<xcb_input_xi_query_device_reply_t>> final
		: GetUPtr<xcb_input_xi_query_device_reply_t>
	{
		using unique_ptr::element_type;
		using unique_ptr::pointer;

		using Type = element_type;

		View(unique_ptr reply = nullptr)
			: unique_ptr(std::move(reply)) {}

		View(const View&) = delete;
		View& operator=(const View&) = delete;

		View(View&&) = default;
		View& operator=(View&&) = default;

		~View() = default;

		using unique_ptr::operator bool;

		using unique_ptr::get;

		using unique_ptr::release;
		using unique_ptr::reset;

		using unique_ptr::operator*;
		using unique_ptr::operator->;

		Input::XI_DeviceInfos infos() NYAAN_CNOEX
		{
			if (!*this) NYAAN_UNLIK
				return {};

			return {get()};
		}
	};
}
