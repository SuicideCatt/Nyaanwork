module;

#include <windows.h>
#include <mfobjects.h> // QWORD

#include <mutex>
#include <string>
#include <utility>

export module Nyaanwork.WindowSystem.Backends.Windows:Window.methods.events;
import :Exception;
import :Instance.decl;
import :Window.decl;

import Nyaanwork.Core.Math;
import Nyaanwork.Core.StringConvertor;
import Nyaanwork.Core.Types;
import Nyaanwork.Core.Utils.Flags;
import Nyaanwork.WindowSystem.Codes;

// instance interface
#define II static_cast<InstanceWindowInterface&>(i)

namespace Nyaanwork::WindowSystem::Backends::Windows
{
	LRESULT windows_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
using Slots = Window::Slots;

		auto user_data = GetWindowLongPtr(hwnd, GWLP_USERDATA);

		if (user_data == 0)
			return DefWindowProc(hwnd, msg, wparam, lparam);

		return reinterpret_cast<Window*>(user_data)->proc(msg, wparam, lparam);
	}

	Opt<Codes::Key> get_key(u16 key)
	{
		using K = Codes::Key;
		switch (key)
		{
		case 0x001e: return K::a;
		case 0x0030: return K::b;
		case 0x002e: return K::c;
		case 0x0020: return K::d;
		case 0x0012: return K::e;
		case 0x0021: return K::f;
		case 0x0022: return K::g;
		case 0x0023: return K::h;
		case 0x0017: return K::i;
		case 0x0024: return K::j;
		case 0x0025: return K::k;
		case 0x0026: return K::l;
		case 0x0032: return K::m;
		case 0x0031: return K::n;
		case 0x0018: return K::o;
		case 0x0019: return K::p;
		case 0x0010: return K::q;
		case 0x0013: return K::r;
		case 0x001f: return K::s;
		case 0x0014: return K::t;
		case 0x0016: return K::u;
		case 0x002f: return K::v;
		case 0x0011: return K::w;
		case 0x002d: return K::x;
		case 0x0015: return K::y;
		case 0x002c: return K::z;

		case 0x000b: return K::num_0;
		case 0x0002: // K::num_1
		case 0x0003: // K::num_2
		case 0x0004: // K::num_3
		case 0x0005: // K::num_4
		case 0x0006: // K::num_5
		case 0x0007: // K::num_6
		case 0x0008: // K::num_7
		case 0x0009: // K::num_8
		case 0x000a: // K::num_9
			return static_cast<K>(key + (static_cast<u8>(K::num_1)) - 2);

		case 0xe048: return K::up;
		case 0xe04b: return K::left;
		case 0xe050: return K::down;
		case 0xe04d: return K::right;

		case 0x002a: return K::shift_left;
		case 0x0036: return K::shift_right;
		case 0x001d: return K::ctrl_left;
		case 0xe01d: return K::ctrl_right;
		case 0x0038: return K::alt_left;
		case 0xe038: return K::alt_right;

		case 0x001c: return K::return_;
		case 0x0001: return K::escape;
		case 0x000e: return K::backspace;
		case 0x0039: return K::space;
		case 0x000f: return K::tab;

		case 0x000c: return K::minus;
		case 0x000d: return K::equals;
		case 0x001a: return K::bracket_left;
		case 0x001b: return K::bracket_right;
		case 0x0035: return K::slash;
		case 0x002b: return K::backslash;
		case 0x0027: return K::semicolon;
		case 0x0028: return K::apostrophe;
		case 0x0029: return K::grave;
		case 0x0033: return K::comma;
		case 0x0034: return K::period;

		case 0x003b: // K::f1
		case 0x003c: // K::f2
		case 0x003d: // K::f3
		case 0x003e: // K::f4
		case 0x003f: // K::f5
		case 0x0040: // K::f6
		case 0x0041: // K::f7
		case 0x0042: // K::f8
		case 0x0043: // K::f9
		case 0x0044: // K::f10
			return static_cast<K>(key + (static_cast<u8>(K::f1) - 59));
		case 0x0057: // K::f11
		case 0x0058: // K::f12
			return static_cast<K>(key + (static_cast<u8>(K::f11) - 87));

		case 0xe052: return K::insert;
		case 0xe053: return K::delete_;
		case 0xe047: return K::home;
		case 0xe04f: return K::end;
		case 0xe049: return K::pageup;
		case 0xe051: return K::pagedown;

		case 0x0052: return K::np_0;
		case 0x004f: return K::np_1;
		case 0x0050: return K::np_2;
		case 0x0051: return K::np_3;
		case 0x004b: return K::np_4;
		case 0x004c: return K::np_5;
		case 0x004d: return K::np_6;
		case 0x0047: return K::np_7;
		case 0x0048: return K::np_8;
		case 0x0049: return K::np_9;
		case 0xe035: return K::np_divide; // FIXME: untested
		case 0x0037: return K::np_multiply; // FIXME: untested
		case 0x004a: return K::np_minus; // FIXME: untested
		case 0x004e: return K::np_plus; // FIXME: untested
		case 0xe01c: return K::np_enter; // FIXME: untested
		case 0x0053: return K::np_period; // FIXME: untested

		default:
			return nullopt;
		}
	}
}

export namespace Nyaanwork::WindowSystem::Backends::Windows
{
	LRESULT Window::proc(UINT msg, WPARAM wparam, LPARAM lparam)
	{
		auto handle_focus = [](auto& wstate, bool mode, auto&& state)
		{
			if (mode)
				wstate |= state;
			else
				wstate &= ~state;
		};

		switch (msg)
		{
		case WM_MOVE:
			{
				Window::Position pos = {LOWORD(lparam), HIWORD(lparam)};
				if (position() != pos)
				{
					{
						auto l = unique_lock();
						p_position = pos;
					}
					signal_send<Slots::moved>(*this, pos);
				}

				if (grab_mouse())
					update_rect();
			}
			return 0;
		case WM_SIZE:
			{
				switch (wparam)
				{
				case SIZE_MINIMIZED:
					{
						auto l = unique_lock();
						p_state |= Window::State::minimized;
					}
					signal_send<Slots::minimized>(*this);
					break;

				case SIZE_RESTORED:
					if (minimized())
					{
						{
							auto l = unique_lock();
							p_state &= ~Window::State::minimized;
						}
						signal_send<Slots::unminimized>(*this);
					}
					break;

				default:
					break;
				}

				RECT rect;
				GetClientRect(handle(), &rect);
				Window::Resolution res = {
					rect.right - rect.left,
					rect.bottom - rect.top,
				};
				if (resolution() != res)
				{
					{
						auto l = unique_lock();
						p_resolution.value = res;
					}
					signal_send<Slots::resized>(*this, res);
				}

				if (grab_mouse())
					update_rect();
			}
			return 0;

		case WM_GETMINMAXINFO:
			{
				auto mmi = reinterpret_cast<MINMAXINFO*>(lparam);

				Window::Resolution min, max;

				if (fullscreen())
				{
					auto& i = *native_instance();
					min = max =
						i.displays().displays[display()].resolution;
				}
				else
				{
					auto l = shared_lock();
					min = adjust_resolution(p_resolution.min,
											m_style);
					max = adjust_resolution(p_resolution.max,
											m_style);
				}

				mmi->ptMinTrackSize = {min.x, min.y};
				mmi->ptMaxTrackSize = {max.x, max.y};
			}
			return 0;

		case WM_SETFOCUS:
		case WM_KILLFOCUS:
			{
				bool focus = msg == WM_SETFOCUS;
				{
					{
						auto l = unique_lock();
						handle_focus(p_state, focus,
									 Window::State::focus_keyboard);
					}

					if (grab_mouse())
					{
						if (mouse_focused() && focus)
							update_rect();
						else
							ClipCursor(nullptr);
					}

					auto& i = *native_instance();
					if (focus)
					{
						II.handle_keyboard_focus(this);
						signal_send<Slots::focus_gained>(*this);
					}
					else
					{
						II.handle_keyboard_unfocus(this);
						signal_send<Slots::focus_lost>(*this);
					}
				}
			}
			return 0;

		case WM_MOUSEMOVE:
			{
				auto l = unique_lock();

				auto old = p_input.mouse.position;
				p_input.mouse.position = {LOWORD(lparam), HIWORD(lparam)};
			}
		case WM_MOUSELEAVE:
			{
				bool focus = msg == WM_MOUSEMOVE;
				if (mouse_focused() != focus)
				{
					if (focus)
					{
						TRACKMOUSEEVENT ev;
						ev.cbSize = sizeof(TRACKMOUSEEVENT);
						ev.dwFlags = TME_LEAVE;
						ev.hwndTrack = handle();
						ev.dwHoverTime = HOVER_DEFAULT;
						if (!TrackMouseEvent(&ev))
						{
							using namespace Error;
							throw Exception(Code::fail_to_enable_mouse_tracking);
						}
					}

					{
						auto l = unique_lock();
						handle_focus(p_state, focus,
									 Window::State::focus_mouse);
					}

					if (grab_mouse())
					{
						if (keyboard_focused() && focus)
							update_rect();
						else
							ClipCursor(nullptr);
					}

					if (!show_mouse())
						show_mouse_impl(!focus);

					auto& i = *native_instance();
					if (focus)
					{
						II.handle_mouse_focus(this);
						signal_send<Slots::mouse_enter>(*this);
					}
					else
					{
						II.handle_mouse_unfocus(this);
						signal_send<Slots::mouse_leave>(*this);
					}
				}
			}
			return 0;

		case WM_SETCURSOR:
			{
				auto l = shared_lock();
				SetCursor(m_cursor);
			}
			return TRUE;

		case WM_CLOSE:
			close();
			return 0;

		case WM_INPUT:
			handle_raw_input(reinterpret_cast<HRAWINPUT>(lparam));
			return 0;

		case WM_DISPLAYCHANGE:
			{
				auto& i = *native_instance();
				static_cast<InstanceWindowInterface&>(i).handle_displays_change();
			}
			return 0;

		case WM::title:
			{
				auto title = StringConvertor_WChar().convert(this->title());
				SetWindowTextW(handle(), title.c_str());
			}
			return 0;

		case WM::posres:
			{
				UINT flags = 0;

				if (wparam)
					flags |= SWP_NOSIZE;

				if (lparam)
					flags |= SWP_NOREPOSITION | SWP_NOMOVE;

				Position position;
				Resolution resolution;
				{
					auto l = shared_lock();
					position = p_position;
					resolution = adjust_resolution(p_resolution.value, m_style);
				}

				auto& [x, y] = position;
				auto& [w, h] = resolution;
				SetWindowPos(handle(), HWND_NOTOPMOST, x, y, w, h,
							 flags | SWP_NOACTIVATE);
			}
			return 0;

		case WM::fullscreen:
			{
				auto& i = *native_instance();
				auto [x, y] = i.displays().displays[display()].position;
				auto [w, h] = resolution();
				SetWindowPos(handle(), HWND_NOTOPMOST, x, y, w, h,
							 SWP_NOACTIVATE);
			}
			return 0;

		case WM::unfullscreen:
			{
				Position position;
				Resolution resolution;
				{
					auto offset = GetSystemMetrics(SM_CYCAPTION);

					auto l = shared_lock();
					position = m_unfullscreen.position;
					position.y -= offset;
					resolution = adjust_resolution(m_unfullscreen.resolution, m_style);
				}

				auto& [x, y] = position;
				auto& [w, h] = resolution;
				SetWindowPos(handle(), HWND_NOTOPMOST, x, y, w, h,
							 SWP_NOACTIVATE);
			}
			return 0;

		default:
			return DefWindowProc(handle(), msg, wparam, lparam);
		}
	}

	void Window::handle_raw_input(HRAWINPUT hraw)
	{
		constexpr auto header = sizeof(RAWINPUTHEADER);
		auto& vec = *m_raw.buffer;

		auto filter = [this](RAWINPUT* input)
		{
			switch (input->header.dwType)
			{
			case RIM_TYPEKEYBOARD:
				handle_raw_input_keyboard(input->data.keyboard);
				break;
			case RIM_TYPEMOUSE:
				handle_raw_input_mouse(input->data.mouse);
				break;

			default:
				break;
			}
		};

		{
			std::scoped_lock l(*m_raw.mutex);
			auto ptr = reinterpret_cast<RAWINPUT*>(vec.data());
			UINT size = vec.size();
			GetRawInputData(hraw, RID_INPUT, vec.data(), &size, header);
			filter(ptr);

			while (true)
			{
				ptr = reinterpret_cast<RAWINPUT*>(vec.data());
				size = vec.size();
				auto count = GetRawInputBuffer(ptr, &size, header);

				if (count == 0)
				{
					break;
				}
				else if (count == static_cast<UINT>(-1))
				{
					vec.resize(max<UINT>(size, vec.size()*2));
					continue;
				}

				for (usize i = 0; i < count; ++i, ptr = NEXTRAWINPUTBLOCK(ptr))
					filter(ptr);

				break;
			}
		}

		auto l = unique_lock();
		p_input.keyboard.state = m_raw.keyboard_state;
		p_input.mouse.motion += std::exchange(m_raw.mouse.motion, {});
		p_input.mouse.wheel += std::exchange(m_raw.mouse.wheel, {});
		p_input.mouse.state = m_raw.mouse.state;
	}

	void Window::handle_raw_input_keyboard(const RAWKEYBOARD& keyboard)
	{
		u16 code = keyboard.MakeCode;
		bool down = !(keyboard.Flags & RI_KEY_BREAK);

		if (static_cast<bool>(keyboard.Flags & RI_KEY_E0))
			code |= 0xe000;
		else if (static_cast<bool>(keyboard.Flags & RI_KEY_E1))
			code |= 0xe100;

		auto key = get_key(code);
		if (!key)
			return;

		m_raw.keyboard_state[*key] = down;

		if (string_recording() && down)
		{
			if (Codes::Traits<Codes::Key>::is_printable(*key))
			{
				UPtr<BYTE[]> kstate(new BYTE[256]);
				GetKeyboardState(kstate.get());

				std::wstring wstr((4*sizeof(wchar_t))+1, '\0');
				auto len = ToUnicode(keyboard.VKey, keyboard.MakeCode,
									 kstate.get(), wstr.data(), wstr.size(), 0);
				wstr.resize(len);

				auto str = StringConvertor_WChar().convert(wstr);

				auto l = unique_lock();
				p_input.keyboard.recorded.string += str;
				p_input.keyboard.recorded.char_sizes.push_back(str.size());
			}
			else if (*key == Codes::Key::return_)
			{
				auto l = unique_lock();
				p_input.keyboard.recorded.string += '\n';
				p_input.keyboard.recorded.char_sizes.push_back(1);
			}
			else if (*key == Codes::Key::backspace)
			{
				auto l = unique_lock();
				auto& [str, sizes, over] = p_input.keyboard.recorded;

				if (sizes.empty()) [[unlikely]]
				{
					++over;
					return;
				}

				for (usize i = 0; i < sizes.back(); ++i)
					str.pop_back();

				sizes.pop_back();
			}
		}
	}

	void Window::handle_raw_input_mouse(const RAWMOUSE& mouse)
	{
		bool focus = false;
		{
			auto l = shared_lock();
			focus = keyboard_focused_no_lock() && mouse_focused_no_lock();
		}

		if (focus)
		{
			auto set_button =
				[&flags=mouse.usButtonFlags,
				 &state=m_raw.mouse.state](Codes::Button button,
										   USHORT up, USHORT down)
				{
					if (flags & up)
						state[button] = false;
					else if (flags & down)
						state[button] = true;
				};

			#define handle(name, num) \
				set_button(Codes::Button::name, \
						   RI_MOUSE_BUTTON_##num##_UP, \
						   RI_MOUSE_BUTTON_##num##_DOWN)

			handle(left, 1);
			handle(middle, 3);
			handle(right, 2);
			handle(x1, 4);
			handle(x2, 5);

			auto data = static_cast<SHORT>(mouse.usButtonData);
			if (mouse.usButtonFlags & RI_MOUSE_WHEEL)
				 m_raw.mouse.wheel.y = static_cast<f32>(data) / 120.f;
			else if (mouse.usButtonFlags & RI_MOUSE_HWHEEL)
				 m_raw.mouse.wheel.x = static_cast<f32>(data) / 120.f;

			m_raw.mouse.motion.x += mouse.lLastX;
			m_raw.mouse.motion.y += mouse.lLastY;

			if (centralize_mouse())
				warp_mouse_impl(resolution()/2_u16);
		}
	}
}
