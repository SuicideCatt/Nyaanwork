module;

#include <Core/Defines.hpp>

#include <thread>
#include <unordered_map>

export module Nyaanwork.WindowSystem.Backends.Linux.XCB:Instance.methods.init;
import :Bind.XCB;
import :Exception;
import :Instance.decl;

import Nyaanwork.WindowSystem.Backends.Base;
import Nyaanwork.WindowSystem.Backends.Linux.CursorName;

namespace Nyaanwork::WindowSystem::Backends::Linux::XCB::InstanceBlocks
{
	Main::Main()
		: xlib(nullptr), xcb{xlib.get_xcb_connection(), nullptr}
	{
		xcb.second = ::XCB::root_screen(xcb.first);
	}

	Extensions::Extensions(::XCB::Connection* connection)
	{
		static constexpr std::array extensions = {
			StrV("XFIXES"),
			StrV("XInputExtension"),
			StrV("XKEYBOARD"),
			StrV("RANDR"),
			StrV("XWAYLAND"),
		};

		using R = ::XCB::Request::QueryExtension;
		std::unordered_map<StrV, R::Package> crs;

		for (auto& ex : extensions)
			crs[ex] = R::Package(connection, ex.size(), ex.data());

		if (!crs["XFIXES"].get()->major_opcode) NYAAN_UNLIK
			throw Error::Exception("XFixes", Error::Code::extension_not_supported);

		if (!(xinput_opcode = crs["XInputExtension"].get()->major_opcode)) NYAAN_UNLIK
			throw Error::Exception("XInput", Error::Code::extension_not_supported);

		if (!crs["XKEYBOARD"].get()->major_opcode) NYAAN_UNLIK
			throw Error::Exception("XKB", Error::Code::extension_not_supported);
		xkb_event = crs["XKEYBOARD"].get()->first_event;

		if (!crs["RANDR"].get()->major_opcode) NYAAN_UNLIK
			throw Error::Exception("XRandR", Error::Code::extension_not_supported);
		xrandr_event = crs["RANDR"].get()->first_event;

		xwayland = crs["XWAYLAND"].get()->major_opcode;
	}

	Atoms::Atoms(::XCB::Connection* connection) noexcept
	{
		struct LoadAtom
		{
			StrV name = "";
			bool only_if_exists = true;
		};

		static constexpr std::array atoms = {
			LoadAtom{"WM_PROTOCOLS"},
			LoadAtom{"WM_DELETE_WINDOW"},
			LoadAtom{"_NET_WM_STATE"},
			LoadAtom{"_NET_WM_STATE_FULLSCREEN"},
			LoadAtom{"_NET_WM_ICON"},
			LoadAtom{"_NET_WM_NAME"},
			LoadAtom{"_NET_WM_PING"},
			LoadAtom{"_NET_WM_STATE_HIDDEN"},
			LoadAtom{"_MOTIF_WM_HINTS"},

			// LoadAtom{"CLIPBOARD"},

			LoadAtom{"UTF8_STRING"},
			LoadAtom{"text/plain;charset=utf-8", false},
			LoadAtom{"text/plain", false},
			LoadAtom{"TEXT"},
			LoadAtom{"text/uri-list", false},

			LoadAtom{"xwayland-pointer"},
			LoadAtom{"Wacom Serial IDs"},

			LoadAtom{"Abs X"},
			LoadAtom{"Abs Y"},
			LoadAtom{"Abs Tilt X"},
			LoadAtom{"Abs Tilt Y"},
			LoadAtom{"Abs Pressure"},

			LoadAtom{"XdndAware"},
			LoadAtom{"XdndEnter"},
			LoadAtom{"XdndLeave"},
			LoadAtom{"XdndPosition"},
			LoadAtom{"XdndStatus"},
			LoadAtom{"XdndTypeList"},
			LoadAtom{"XdndActionCopy"},
			LoadAtom{"XdndDrop"},
			LoadAtom{"XdndFinished"},
			LoadAtom{"XdndSelection"},
		};

		using R = ::XCB::Request::InternAtom;
		std::unordered_map<StrV, R::Package> crs;

		for (auto& [name, only_if_exists] : atoms)
		{
			crs[name] = R::Package(connection, only_if_exists,
								   name.size(), name.data());
		}

		protocols = crs["WM_PROTOCOLS"].get()->atom;
		delete_window = crs["WM_DELETE_WINDOW"].get()->atom;
		net_wm.state = crs["_NET_WM_STATE"].get()->atom;
		net_wm.fullscreen = crs["_NET_WM_STATE_FULLSCREEN"].get()->atom;
		net_wm.icon = crs["_NET_WM_ICON"].get()->atom;
		net_wm.name = crs["_NET_WM_NAME"].get()->atom;
		net_wm.ping = crs["_NET_WM_PING"].get()->atom;
		net_wm.hidden = crs["_NET_WM_STATE_HIDDEN"].get()->atom;
		motif_wm_hints = crs["_MOTIF_WM_HINTS"].get()->atom;

		// clipboard = crs["CLIPBOARD"].get()->atom;

		types.utf8_string = crs["UTF8_STRING"].get()->atom;
		types.utf8_plain = crs["text/plain;charset=utf-8"].get()->atom;
		types.plain = crs["text/plain"].get()->atom;
		types.text = crs["TEXT"].get()->atom;
		types.uri_list = crs["text/uri-list"].get()->atom;

		input.wacom.serial_ids = crs["Wacom Serial IDs"].get()->atom;

		input.stylus_labels.position.x = crs["Abs X"].get()->atom;
		input.stylus_labels.position.y = crs["Abs Y"].get()->atom;
		input.stylus_labels.tilt.x = crs["Abs Tilt X"].get()->atom;
		input.stylus_labels.tilt.y = crs["Abs Tilt Y"].get()->atom;
		input.stylus_labels.pressure = crs["Abs Pressure"].get()->atom;

		dnd.aware = crs["XdndAware"].get()->atom;
		dnd.enter = crs["XdndEnter"].get()->atom;
		dnd.leave = crs["XdndLeave"].get()->atom;
		dnd.position = crs["XdndPosition"].get()->atom;
		dnd.status = crs["XdndStatus"].get()->atom;
		dnd.type_list = crs["XdndTypeList"].get()->atom;
		dnd.action_copy = crs["XdndActionCopy"].get()->atom;
		dnd.drop = crs["XdndDrop"].get()->atom;
		dnd.finished = crs["XdndFinished"].get()->atom;
		dnd.selection = crs["XdndSelection"].get()->atom;
	}

	Input::Input(::XCB::Data data, const Atoms& atoms) noexcept
	{
		using namespace ::XCB::Input;
		using namespace Request;

		using Flags = Events::Flags;
		using Traits = FlagTraits<Flags>;
		using enum Flags;

		auto& [c, s] = data;
		auto& root = s->root;

		QueryVersion::request_reply(c, 2, 4);

		Events{
			all_master_devices,
			Traits::raw_kb | raw_motion
		}.select(c, root);

		Events{
			all_devices,
			key_press | motion | hierarchy | property
		}.select(c, root);

		for (const auto& device : ListDevices::request_reply(c).devices())
			add_if_tablet(c, atoms, device.device_id);
	}

	XKB::XKB(::XCB::Connection* c)
		: context(::XCB::XKB::init_extension(c)),
		  core_keyboard(::XCB::XKB::core_keyboard(c))
	{
		create_keymap_state(c);

		using namespace ::XCB::XKB;
		using namespace ::XCB::XKB::Events;
		using enum Events::Flags;

		static constexpr Nyaanwork::Flags<Events::Flags> zero(0);
		static constexpr Flags map = new_keyboard_notify | state_notify;
		Events::select(c, DeviceID::use_core_kbd, map, zero, map, zero, zero);
	}

	Cursor::Cursor(::XCB::Data xdata)
		: context(xdata.first, xdata.second)
	{
		static constexpr auto cursor = Base::Window::Cursor::default_;
		static constexpr auto name = css_xcursor_name(cursor);
		cursors[cursor] = context.load(name);
	}
}

export namespace Nyaanwork::WindowSystem::Backends::Linux::XCB
{
	Instance::Instance(Opt<Str> window_class_name)
		: Base::Instance(std::move(window_class_name)),
		  m_main(), m_extensions(xconnection()), m_atoms(xconnection()),
		  m_input(xdata(), m_atoms), m_xkb(xconnection()),
		  m_cursor(m_main.xcb)
	{
		auto [c, s] = xdata();
		auto& root = s->root;

		::XCB::Fixes::Request::QueryVersion::request_reply(c, 4, 0);

		using namespace ::XCB::RandR::Events;
		::XCB::RandR::Request::QueryVersion::request_reply(c, 1, 3);
		select(c, root, Event::output_change);

		m_thread.displays_changed = true;
		m_thread.have_tablet = !m_input.tablets.empty();
		m_thread.no_windows = true;
		m_thread.work = true;
		m_thread.thread = std::thread(&Instance::event_loop, this);
	}

	Instance::~Instance()
	{
		m_thread.work = false;
		m_thread.thread.join();

		m_cursor.clear(xconnection());
	}
}
