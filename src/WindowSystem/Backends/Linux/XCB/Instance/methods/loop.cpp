module;

#include <Core/Defines.hpp>

#include <chrono>
#include <ranges>
#include <span>

export module Nyaanwork.WindowSystem.Backends.Linux.XCB:Instance.methods.loop;
import :Bind.XCB;
import :Exception;
import :Instance.decl;
import :Window.decl;

import Nyaanwork.Core.Math;
import Nyaanwork.Core.System;
import Nyaanwork.WindowSystem.Backends.Base;
import Nyaanwork.WindowSystem.Codes;

namespace Nyaanwork::WindowSystem::Backends::Linux::XCB
{
	template<typename To>
	To& cast(::XCB::Events::Generic::element_type& ev)
		{ return reinterpret_cast<To&>(ev); }

	template<typename To>
	To& cast(::XCB::Events::GE_Event& ev)
		{ return reinterpret_cast<To&>(ev); }

	// window handler
	#define WH static_cast<HandleEventsI&>(*window)

	// maybe this function work on Wayland
	Opt<Codes::Key> get_key(u8 key)
	{
		using K = Codes::Key;
		switch (key)
		{
		case 38: return K::a;
		case 56: return K::b;
		case 54: return K::c;
		case 40: return K::d;
		case 26: return K::e;
		case 41: return K::f;
		case 42: return K::g;
		case 43: return K::h;
		case 31: return K::i;
		case 44: return K::j;
		case 45: return K::k;
		case 46: return K::l;
		case 58: return K::m;
		case 57: return K::n;
		case 32: return K::o;
		case 33: return K::p;
		case 24: return K::q;
		case 27: return K::r;
		case 39: return K::s;
		case 28: return K::t;
		case 30: return K::u;
		case 55: return K::v;
		case 25: return K::w;
		case 53: return K::x;
		case 29: return K::y;
		case 52: return K::z;

		case 19: return K::num_0;
		case 10: // K::num_1
		case 11: // K::num_2
		case 12: // K::num_3
		case 13: // K::num_4
		case 14: // K::num_5
		case 15: // K::num_6
		case 16: // K::num_7
		case 17: // K::num_8
		case 18: // K::num_9
			return static_cast<K>(key - (10 - static_cast<u8>(K::num_1)));

		case 111: return K::up;
		case 113: return K::left;
		case 116: return K::down;
		case 114: return K::right;

		case 50: return K::shift_left;
		case 62: return K::shift_right;
		case 37: return K::ctrl_left;
		case 105: return K::ctrl_right;
		case 64: return K::alt_left;
		case 108: return K::alt_right;

		case 36: return K::return_;
		case 9: return K::escape;
		case 22: return K::backspace;
		case 65: return K::space;
		case 23: return K::tab;

		case 20: return K::minus;
		case 21: return K::equals;
		case 34: return K::bracket_left;
		case 35: return K::bracket_right;
		case 61: return K::slash;
		case 51: return K::backslash;
		case 47: return K::semicolon;
		case 48: return K::apostrophe;
		case 49: return K::grave;
		case 59: return K::comma;
		case 60: return K::period;

		case 67: // K::f1
		case 68: // K::f2
		case 69: // K::f3
		case 70: // K::f4
		case 71: // K::f5
		case 72: // K::f6
		case 73: // K::f7
		case 74: // K::f8
		case 75: // K::f9
		case 76: // K::f10
			return static_cast<K>(key - (67 - static_cast<u8>(K::f1)));
		case 95: // K::f11
		case 96: // K::f12
			return static_cast<K>(key - (95 - static_cast<u8>(K::f11)));

		case 118: return K::insert;
		case 119: return K::delete_;
		case 110: return K::home;
		case 115: return K::end;
		case 112: return K::pageup;
		case 117: return K::pagedown;

		case 90: return K::np_0;
		case 87: return K::np_1;
		case 88: return K::np_2;
		case 89: return K::np_3;
		case 83: return K::np_4;
		case 84: return K::np_5;
		case 85: return K::np_6;
		case 79: return K::np_7;
		case 80: return K::np_8;
		case 81: return K::np_9;
		case 106: return K::np_divide;
		case 63: return K::np_multiply;
		case 82: return K::np_minus;
		case 86: return K::np_plus;
		case 104: return K::np_enter;
		case 91: return K::np_period;

		default:
			return nullopt;
		}
	}

	Opt<Codes::Button> get_button(u8 button)
	{
		switch (button)
		{
		case 1: return Codes::Button::left;
		case 2: return Codes::Button::middle;
		case 3: return Codes::Button::right;
		case 8: return Codes::Button::x1;
		case 9: return Codes::Button::x2;

		default:
			return nullopt;
		}
	}
}

export namespace Nyaanwork::WindowSystem::Backends::Linux::XCB
{
	void Instance::event_loop()
	{
		::XCB::Timestamp ignore_enter_leave_time = 0;
		bool wait_selection = false;

		while (m_thread.work)
		{
			using namespace ::XCB::Events;

			auto xevent_ptr = wait(xconnection(),
								   wait_selection || m_thread.no_windows,
								   std::chrono::milliseconds(1));

			if (!xevent_ptr) NYAAN_UNLIK
				continue;

			auto& xevent = *xevent_ptr;
			auto xevent_type = xevent.response_type & 0x7f;

			auto focus_handle = [this](auto& ev, auto& focused_window,
									   auto&& handle, bool mode)
			{
				Window* window = nullptr;

				{
					auto l = lock();

					auto itr = m_windows.all.find(ev.event);
					if (itr != m_windows.all.end())
					{
						window = itr->second;

						if (mode)
						{
							if (window == focused_window)
								return;
							focused_window = window;
						}
						else
						{
							focused_window = nullptr;
						}
					}
					else
					{
						return;
					}
				}

				(WH.*handle)(mode);
			};

			namespace R = Response;
			switch (xevent_type)
			{
			case R::focus_in:
			case R::focus_out:
				{
					auto& ev = cast<Focus>(xevent);
					focus_handle(ev, m_windows.focused.keyboard,
								 &HandleEventsI::handle_keyboard_focus,
								 xevent_type == R::focus_in);
				}
				break;

			case R::enter_notify:
			case R::leave_notify:
				{
					auto& ev = cast<MouseFocus>(xevent);

					auto mode = xevent_type == R::enter_notify;
					if (!mode && ev.time == ignore_enter_leave_time)
						break;

					focus_handle(ev, m_windows.focused.mouse,
								 &HandleEventsI::handle_mouse_focus,
								 mode);

					if (auto window = get_window_focused_mouse())
					{
						bool on_surface = m_input.is_any_tablet_pen_on_surface();
						WH.handle_tablet_on_surface(on_surface);
					}
					else if (xwayland())
					{
						for (auto& [id, tablet] : m_input.tablets)
							tablet.on_surface = false;
					}
				}
				break;

			case R::configure_notify:
				{
					auto& ev = cast<ConfigureNotify>(xevent);

					Window::Position pos(ev.x, ev.y);
					Window::Resolution res(ev.width, ev.height);

					if (auto window = get_window(ev.window))
						WH.handle_configure(pos, res);
				}
				break;
			case R::property_notify:
				{
					auto& ev = cast<PropertyNotify>(xevent);

					if (ev.atom == atoms().net_wm.state)
						if (auto window = get_window(ev.window))
							WH.handle_change_property();
				}
				break;

			case R::client_message:
				handle_client_message(wait_selection, cast<ClientMessage>(xevent));
				break;
			case R::selection_notify:
				wait_selection = false;
				handle_selection_notify(cast<SelectionNotify>(xevent));
				break;

			case R::ge_generic:
				handle_ge_event(ignore_enter_leave_time, cast<GE_Event>(xevent));
				break;

			default:
				namespace RandR = ::XCB::RandR::Events;
				if (xevent_type == m_extensions.xkb_event)
				{
					handle_xkb_event(xevent);
				}
				else if (xevent_type ==
							(m_extensions.xrandr_event+RandR::screen_change_notify))
				{
					m_thread.displays_changed = true;
					signal_send<Slots::displays_changed>(*this);
				}
				break;
			}
		}
	}

	void Instance::handle_client_message(bool& enable_wait,
										 ::XCB::Events::ClientMessage& ev)
	{
		auto window = get_window(ev.window);
		if (!window) NYAAN_UNLIK
			return;

		auto c = xconnection();
		const auto& dnd = atoms().dnd;

		const auto& type = ev.type;
		if (type == dnd.enter)
		{
			const auto& data = ev.data.data32;
			bool use_list = data[1] & 1;
			auto& source = data[0];
			auto xdnd_version = data[1] >> 24;

			namespace W = ::XCB::Window;
			W::GetProperty::Reply r;
			std::span<::XCB::Atom const> atoms;
			if (use_list)
			{
				using ::XCB::Window::get_property;
				auto [gp_r, gp_atoms] =
					get_property<::XCB::Atom>(c, source, dnd.type_list);
				r = std::move(gp_r);
				atoms = gp_atoms;
			}
			else
			{
				atoms = {&data[2], 3};
			}

			auto types = this->atoms().types;

			bool is_text = false, is_uri = false;
			::XCB::Atom drop_atom = ::XCB::none;
			for (auto& atom : atoms)
			{
				if (atom == ::XCB::none) NYAAN_UNLIK
					continue;

				if (atom == types.utf8_plain
					|| atom == types.plain
					|| atom == types.utf8_string
					|| atom == types.text)
				{
					is_text = true;

					if (drop_atom == ::XCB::none)
						drop_atom = atom;
				}
				else if (atom == types.uri_list)
				{
						is_uri = true;
				}
			}
			r.reset();

			if (is_uri)
				WH.handle_dnd_enter_uri(source, xdnd_version);
			else if (is_text)
				WH.handle_dnd_enter_text(drop_atom, source, xdnd_version);
		}
		else if (type == dnd.position)
		{
			auto& data = ev.data.data32;

			Window::Position pos(data[2] >> 16, data[2] & 0xffff);
			pos -= window->position();
			pos = clamp(pos, {0, 0}, Window::Position(window->resolution()));

			WH.handle_mouse_position(pos);

			using namespace ::XCB::Events;
			send<ClientMessage>(c, data[0], {}, {
				.response_type = Response::client_message,
				.format = 32,
				.window = data[0],
				.type = dnd.status,
				.data = {
					.data32 = {
						ev.window, static_cast<bool>(WH.drop_data()),
						0, 0, dnd.action_copy,
					}
				}
			});

			::XCB::flush(c);
		}
		else if (type == dnd.drop)
		{
			auto& data = ev.data.data32;

			if (auto buf = WH.drop_data())
			{
				int time = buf->xdnd_version >= 1? data[2] : ::XCB::current_time;
				::XCB::Window::convert_selection(c, ev.window, time,
								dnd.selection, buf->atom, ::XCB::Atoms::primary);
				::XCB::flush(c);
				enable_wait = true;
			}
			else
			{
				using namespace ::XCB::Events;
				send<ClientMessage>(c, data[0], {}, {
					.response_type = Response::client_message,
					.format = 32,
					.window = data[0],
					.type = dnd.finished,
					.data = {
						.data32 = {ev.window, 0, ::XCB::none}
					}
				});

				::XCB::flush(c);
			}
		}
		else if (type == dnd.leave)
		{
			WH.handle_dnd_cancel();
		}
		else if (type == this->atoms().protocols && ev.format == 32)
		{
			const auto& protocol = ev.data.data32[0];
			const auto& atoms = this->atoms();

			if (protocol == atoms.delete_window)
			{
				window->close();
			}
			else
			if (protocol == atoms.net_wm.ping)
			{
				ev.window = xscreen()->root;

				using namespace ::XCB::Events;
				using ::XCB::Events::Flags;
				static constexpr auto flags =
					Flags::substructure_redirect | Flags::substructure_notify;
				send(c, ev.window, flags, ev);

				::XCB::flush(c);
			}
		}
	}

	void Instance::handle_selection_notify(::XCB::Events::SelectionNotify& ev)
	{
		auto window = get_window(ev.requestor);
		if (!window) NYAAN_UNLIK
			return;

		auto c = xconnection();

		auto buf = WH.drop_data();
		if (buf && buf->atom == ev.target)
		{
			namespace W = ::XCB::Window;
			using ::XCB::Atoms;
			auto [r, s] = W::get_property<char>(c, ev.requestor,
												Atoms::primary, Atoms::any);
			StrV text(s);

			if (buf->atom == atoms().types.uri_list)
			{
				static constexpr StrV delim = "\r\n";

				std::vector<Str> uris;
				for (const auto token : std::views::split(text, delim))
				{
					if (token.empty()) NYAAN_UNLIK
						continue;

					uris.push_back(System::decode_uri(StrV(token)));
				}
				WH.handle_dnd_done_uri(uris);
			}
			else
			{
				WH.handle_dnd_done_text(Str(text));
			}

			auto& dnd = atoms().dnd;

			using namespace ::XCB::Events;
			send<ClientMessage>(c, buf->source, {}, {
				.response_type = Response::client_message,
				.format = 32,
				.window = buf->source,
				.type = dnd.finished,
				.data = {
					.data32 = {ev.requestor, 1, dnd.action_copy, ::XCB::none}
				}
			});

			::XCB::flush(c);
		}
	}

	void Instance::handle_ge_event(::XCB::Timestamp& ignore_enter_leave,
								   ::XCB::Events::GE_Event& ev)
	{
		if (ev.extension != m_extensions.xinput_opcode)
			return;

		auto handle_kb = [this](auto& ev, auto& focused_window,
								auto&& handle, auto&& get_from_code, bool mode)
		{
			Window* window = nullptr;
			{
				auto l = lock();
				if (!(window = focused_window))
					return ;
			}

			auto kb = get_from_code(ev.detail);
			if (!kb)
				return;

			(WH.*handle)(*kb, mode);
		};

		namespace I = ::XCB::Input;
		using E = I::Events;
		using R = E::Response;
		switch (ev.event_type)
		{
		case R::key_press:
			{
				auto& xiev = cast<E::Key>(ev);

				auto window = get_window_focused_keyboard();

				if (window && window->string_recording())
				{
					auto key = get_key(xiev.detail);
					if (!key) NYAAN_UNLIK
						break;

					if (Codes::Traits<Codes::Key>::is_printable(*key))
						WH.handle_record_new_string(m_xkb.state.key_utf8(xiev.detail));
					else if (*key == Codes::Key::return_)
						WH.handle_record_newline();
					else if (*key == Codes::Key::backspace)
						WH.handle_record_pop();
				}
			}
			break;

		case R::raw_key_press:
		case R::raw_key_release:
			{
				auto& xiev = cast<E::RawKey>(ev);
				handle_kb(xiev, m_windows.focused.keyboard,
						  &HandleEventsI::handle_key, &get_key,
						  xiev.event_type == R::raw_key_press);
			}
			break;

		case R::raw_button_press:
		case R::raw_button_release:
			{
				auto& xiev = cast<E::RawButton>(ev);
				ignore_enter_leave = xiev.time;
				handle_kb(xiev, m_windows.focused.mouse,
						  &HandleEventsI::handle_button, &get_button,
						  xiev.event_type == R::raw_button_press);
			}
			break;

		case R::motion:
			{
				auto& xiev = cast<E::Motion>(ev);

				auto window = get_window_focused_mouse();
				if (!window) NYAAN_UNLIK
					break;

				Window::Position pos(xiev.root_x >> 16, xiev.root_y >> 16);
				pos -= window->position();
				pos = clamp(pos, {0, 0}, Window::Position(window->resolution()));

				// tablet: raw_motion set only pressure and tilt, motion set
				//   here
				bool tablet = m_input.tablets.contains(xiev.sourceid);
				if (tablet)
					WH.handle_mouse_motion(vec2<f32>(pos) - window->mouse_position());

				WH.handle_mouse_position(pos);
			}
			break;
		case R::raw_motion:
			{
				auto& xiev = cast<E::RawMotion>(ev);

				auto window = get_window_focused_mouse();
				if (!window) NYAAN_UNLIK
					break;

				auto raw = xiev.axisvalues();
				auto mask = xiev.valuator_mask();

				auto itr = m_input.tablets.find(xiev.sourceid);

				auto s = []<typename T>(Opt<vec2<T>>& opt, usize i) -> T&
				{
					if (opt)
						return (*opt)[i];
					else
						return opt.emplace()[i];
				};

				if (itr != m_input.tablets.end()) // is_tablet
				{
					using Tablet = InstanceBlocks::Input::Tablet;
					auto& tablet = itr->second;

					Tablet::vec2 position;
					Opt<f32> pressure;
					Opt<vec2<f32>> tilt;

					for (usize i = 0; i < raw.size(); ++i)
					{
						using B = Tablet::Bind;
						auto& val = raw[i];

						switch (tablet.binds[i])
						{
						case B::position_x:
							position.x = val;
							break;
						case B::position_y:
							position.y = val;
							break;

						case B::pressure:
							if (val.frac || val.integral)
								pressure = I::f64_from_fp3232(val) / tablet.pressure_max;
							break;

						case B::tilt_x:
							if (val.frac || val.integral)
								s(tilt, 0) = I::f64_from_fp3232(val);
							break;
						case B::tilt_y:
							if (val.frac || val.integral)
								s(tilt, 1) = I::f64_from_fp3232(val);
							break;

						default:
							break;
						}
					}

					// on xwayland property not changing
					//   but if tablet not on surface raw data is
					//   [old_pos_x, old_pos_y, 0, ...] (libinput: proximity out)
					if (xwayland())
					{
						auto eq = [](I::fp3232 a, I::fp3232 b)
							{ return a.frac == b.frac && a.integral == b.integral; };
						auto eqv = [&eq](Tablet::vec2 a, Tablet::vec2 b)
							{ return eq(a.x, b.x) && eq(a.y, b.y); };

						auto& old = tablet.position_old;

						bool moved = !eqv(position, old);
						tablet.on_surface = moved || pressure || tilt;
						old = position;

						auto on_surface =
							tablet.on_surface || m_input.is_any_tablet_pen_on_surface();
						WH.handle_tablet_on_surface(on_surface);
					}

					WH.handle_tablet_data(pressure.value_or(0.f),
										  tilt.value_or(vec2<f32>(0.f)));
				}
				else
				{
					usize i = 0;

					Opt<vec2<f32>> motion;
					Opt<vec2<f64>> wheel;

					if (mask[0] & 0b0001) // motion_x
						s(motion, 0) = I::f64_from_fp3232(raw[i++]);
					if (mask[0] & 0b0010) // motion_y
						s(motion, 1) = I::f64_from_fp3232(raw[i++]);

					if (motion)
						WH.handle_mouse_motion(*motion);

					if (mask[0] & 0b0100) // wheel_x
						s(wheel, 0) = I::f64_from_fp3232(raw[i++]);
					if (mask[0] & 0b1000) // wheel_y
						s(wheel, 1) = I::f64_from_fp3232(raw[i++]);

					if (!xwayland() && wheel)
						*wheel /= 120_f64;

					if (wheel)
						WH.handle_mouse_wheel(*wheel);

					if (window->centralize_mouse() && window->keyboard_focused())
						window->warp_mouse(window->resolution()/2_u16);
				}
			}
			break;

		case R::hierarchy:
			{
				auto& xiev = cast<E::Hierarchy>(ev);

				for (auto& info : xiev.infos())
				{
					using F = Flags<I::HierarchyFlags>;
					using T = F::Traits;

					F flags(info.flags);
					if (flags & T::add)
						m_input.add_if_tablet(xconnection(), atoms(), info.deviceid);
					else if (flags & T::remove)
						m_input.tablets.erase(info.deviceid);

					bool old_have_tablet = m_thread.have_tablet;
					bool new_have_tablet = m_thread.have_tablet = !m_input.tablets.empty();

					if (old_have_tablet != new_have_tablet)
					{
						if (new_have_tablet)
							signal_send<Slots::tablet_connected>(*this);
						else
							signal_send<Slots::tablet_disconnected>(*this);
					}
				}
			}
			break;

		case R::property:
			{
				// not work on xwayland, see raw_motion
				if (xwayland())
					break;

				namespace P = I::Property;

				auto& xiev = cast<E::Property>(ev);

				auto& wacom_serial = atoms().input.wacom.serial_ids;

				auto itr = m_input.tablets.find(xiev.deviceid);

				bool is_tablet = itr != m_input.tablets.end();
				bool mod = xiev.what == P::Flag::modified;
				bool atom = xiev.property == wacom_serial;

				if (is_tablet && mod && atom)
				{
					auto c = xconnection();
					auto [r, s] = P::get_property<u32>(c, xiev.deviceid, wacom_serial);

					auto& curr = itr->second.on_surface = s[4] != 0 || s[3] != 0;

					if (auto window = get_window_focused_mouse())
					{
						bool on_surface =
							curr || m_input.is_any_tablet_pen_on_surface();
						WH.handle_tablet_on_surface(on_surface);
					}
				}
			}
			break;

		default:
			break;
		}
	}

	void Instance::handle_xkb_event(::XCB::Events::Generic::element_type& xevent)
	{
		namespace XKB = ::XCB::XKB;
		namespace E = XKB::Events;
		namespace R = E::Response;

		switch (xevent.pad0)
		{
		case R::new_keyboard:
			{
				auto xkbev = cast<E::StateNotify>(xevent);
				if (xkbev.deviceID != m_xkb.core_keyboard)
					break;

				auto l = lock();
				m_xkb.create_keymap_state(xconnection());
			}
			break;

		case R::state:
			{
				auto xkbev = cast<E::StateNotify>(xevent);

				usize old_layout, new_layout;
				{
					auto l = lock();
					old_layout = m_xkb.layout;
					m_xkb.state.update_mask(
							xkbev.baseMods & 1, xkbev.latchedMods, xkbev.lockedMods,
							xkbev.baseGroup, xkbev.latchedGroup,
							new_layout = m_xkb.layout = xkbev.lockedGroup
					);
				}

				if (old_layout != new_layout)
					signal_send<Slots::keyboard_layout_changed>(*this);
			}
			break;

		default:
			break;
		}
	}
}
