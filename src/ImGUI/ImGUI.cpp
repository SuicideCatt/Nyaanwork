module;

#include <Core/Defines.hpp>

#include <imgui.h>
#include <imgui_internal.h>

#include <utility>

export module Nyaanwork.ImGUI;
export import :Exception;

import Nyaanwork.Core.Types;
import Nyaanwork.Core.Utils.Flags;
import Nyaanwork.Core.Utils.PtrContainer;
import Nyaanwork.WindowSystem;

namespace Nyaanwork
{
	static ImGuiKey key_to_imgui(WindowSystem::Codes::Key key)
	{
		switch (key)
		{
		#define CASE(key, ImGui) \
			case WindowSystem::Codes::Key::key: return ImGuiKey_##ImGui
		#define CASE_ARROW(key, ImGui) CASE(key, ImGui##Arrow)
		// LRB - Left/Right Button
		#define CASE_LRB(key, ImGui) \
			CASE(key##_left, Left##ImGui); CASE(key##_right, Right##ImGui)
		#define CASE_FUNC(id) CASE(f##id, F##id)
		#define CASE_NP(key, ImGui) CASE(np_##key, Keypad##ImGui)
		// NPN - NumPad Number
		#define CASE_NPN(id) CASE_NP(id, id)

		CASE(a, A); CASE(b, B); CASE(c, C); CASE(d, D); CASE(e, E); CASE(f, F);
		CASE(g, G); CASE(h, H); CASE(i, I); CASE(j, J); CASE(k, K); CASE(l, L);
		CASE(m, M); CASE(n, N); CASE(o, O); CASE(p, P); CASE(q, Q); CASE(r, R);
		CASE(s, S); CASE(t, T); CASE(u, U); CASE(v, V);
		CASE(w, W); CASE(x, X); CASE(y, Y); CASE(z, Z);

		CASE(num_0, 0); CASE(num_1, 1); CASE(num_2, 2); CASE(num_3, 3); CASE(num_4, 4);
		CASE(num_5, 5); CASE(num_6, 6); CASE(num_7, 7); CASE(num_8, 8); CASE(num_9, 9);

		CASE_ARROW(up, Up); CASE_ARROW(down, Down);
		CASE_ARROW(left, Left); CASE_ARROW(right, Right);

		CASE_LRB(shift, Shift); CASE_LRB(ctrl, Ctrl); CASE_LRB(alt, Alt);

		CASE(return_, Enter); CASE(escape, Escape);
		CASE(backspace, Backspace); CASE(space, Space);
		CASE(tab, Tab);

		CASE(minus, Minus); CASE(equals, Equal);
		CASE_LRB(bracket, Bracket);
		CASE(slash, Slash); CASE(backslash, Backslash);
		CASE(semicolon, Semicolon); CASE(apostrophe, Apostrophe);
		CASE(grave, GraveAccent); CASE(comma, Comma); CASE(period, Period);

		CASE_FUNC(1); CASE_FUNC(2); CASE_FUNC(3); CASE_FUNC(4); CASE_FUNC(5);
		CASE_FUNC(6); CASE_FUNC(7); CASE_FUNC(8); CASE_FUNC(9);
		CASE_FUNC(10); CASE_FUNC(11); CASE_FUNC(12);

		CASE(insert, Insert); CASE(delete_, Delete);
		CASE(home, Home); CASE(end, End);
		CASE(pageup, PageUp); CASE(pagedown, PageDown);

		CASE_NPN(0); CASE_NPN(1); CASE_NPN(2); CASE_NPN(3); CASE_NPN(4);
		CASE_NPN(5); CASE_NPN(6); CASE_NPN(7); CASE_NPN(8); CASE_NPN(9);

		CASE_NP(divide, Divide); CASE_NP(multiply, Multiply);
		CASE_NP(minus, Subtract); CASE_NP(plus, Add);
		CASE_NP(enter, Enter); CASE_NP(period, Decimal);

		#undef CASE
		#undef CASE_ARROW
		#undef CASE_LRB
		#undef CASE_FUNC
		#undef CASE_NP
		#undef CASE_NPN
		}
	}

	static int button_to_imgui(WindowSystem::Codes::Button key)
	{
		switch (key)
		{
		case WindowSystem::Codes::Button::left:
			return 0;
		case WindowSystem::Codes::Button::middle:
			return 2;
		case WindowSystem::Codes::Button::right:
			return 1;
		case WindowSystem::Codes::Button::x1:
			return 3;
		case WindowSystem::Codes::Button::x2:
			return 4;
		}
	}
}

export namespace Nyaanwork
{
	class ImGUI
	{
	private:
		using W = WindowSystem::Window;

	public:
		ImGUI()
			: m_context(ImGui::CreateContext())
		{
			IMGUI_CHECKVERSION();

			auto& io = m_context->IO;

			if (io.BackendPlatformUserData != nullptr) NYAAN_UNLIK
				throw ImGUI_Error::Exception(ImGUI_Error::Code::already_use_other_backend);

			io.BackendPlatformName = "Nyaanwork";
			io.BackendPlatformUserData = this;
			io.BackendFlags =
				ImGuiBackendFlags_HasMouseCursors | ImGuiBackendFlags_HasSetMousePos;
		}

		ImGUI(const ImGUI&) = delete;
		ImGUI& operator=(const ImGUI&) = delete;

		ImGUI(ImGUI&& oth) noexcept
			: m_context(std::move(oth.m_context)),
			  m_string_recording_owner(std::exchange(oth.m_string_recording_owner,
													 false)),
			  m_cursor_owner(std::exchange(oth.m_cursor_owner, false)),
			  m_keyboard_focus(std::exchange(oth.m_keyboard_focus, false))
			{ m_context->IO.BackendPlatformUserData = this; }

		ImGUI& operator=(ImGUI&& oth) noexcept
		{
			if (this != &oth)
			{
				m_context = std::move(oth.m_context);
				m_string_recording_owner = std::exchange(oth.m_string_recording_owner,
														 false);
				m_cursor_owner = std::exchange(oth.m_cursor_owner, false);
				m_keyboard_focus = std::exchange(oth.m_keyboard_focus, false);

				m_context->IO.BackendPlatformUserData = this;
			}

			return *this;
		}

		~ImGUI()
		{
			if (!m_context) NYAAN_UNLIK
				return;

			auto& io = m_context->IO;
			if (io.BackendPlatformUserData == this)
			{
				io.BackendPlatformName = nullptr;
				io.BackendPlatformUserData = nullptr;
				io.BackendFlags = 0;
			}

			ImGui::DestroyContext(m_context.ptr());
		}

		ImGuiContext* context() NYAAN_CNOEX
			{ return m_context.get(); }
		bool is_current() NYAAN_CNOEX
			{ return m_context && ImGui::GetCurrentContext() == m_context.ptr(); }
		void make_current() noexcept
		{
			if (m_context)
				return ImGui::SetCurrentContext(m_context.ptr());
		}

		void update(Ptr<W> window,
					PtrContainer<WindowSystem::InputState> state = nullptr)
		{
			if (!m_context || !window) NYAAN_UNLIK
				return;

			auto& io = m_context->IO;

			auto keyboard_focus = window->keyboard_focused();
			if (m_keyboard_focus != keyboard_focus)
				io.AddFocusEvent(m_keyboard_focus = keyboard_focus);
			else if (!keyboard_focus)
				return;

			update_cursor(*window, io);

			auto input = [&state, &window]
			{
				if (state)
					return *state;
				else
					return window->input_state();
			}();

			update_keyboard(*window, io, input);
			update_mouse(*window, io, input);
		}

		void new_frame(Ptr<W> window)
		{
			if (!m_context || !window) NYAAN_UNLIK
				return;

			auto& io = m_context->IO;
			auto [w, h] = vec2<decltype(ImVec2::x)>(window->resolution());
			io.DisplaySize = {w, h};

			ImGui::NewFrame();
		}

	private:
		void update_keyboard(W& window, ImGuiIO& io, WindowSystem::InputState& input)
		{
			bool rec;
			if ((rec = window.string_recording()))
				io.AddInputCharactersUTF8(input.string().string.c_str());

			if (io.WantTextInput && !m_string_recording_owner && !rec)
			{
				m_string_recording_owner = true;
				window.string_recording(true);
			}
			if (!io.WantTextInput && m_string_recording_owner)
			{
				m_string_recording_owner = false;
				window.string_recording(false);
			}

			using K = WindowSystem::Codes::Key;
			using KT = WindowSystem::Codes::Traits<K>;
			using KET = EnumTraits<K>;
			for (usize i = 0; i < KT::count; ++i)
			{
				auto key = KET::cast(i);
				io.AddKeyEvent(key_to_imgui(key), input.key(key));
			}
		}

		void update_cursor(W& window, ImGuiIO& io)
		{
			bool show_mouse = window.show_mouse();
			W::Cursor cursor = window.cursor();

			bool allow_owning = false;
			if ((show_mouse && cursor == W::Cursor::default_) || m_cursor_owner)
				allow_owning = true;

			auto imgui_cursor = m_context->MouseCursor;
			if (imgui_cursor && allow_owning)
			{
				show_mouse = true;
				cursor = W::Cursor::default_;

				switch (imgui_cursor)
				{
				#define CASE(cur, ImGui) \
					case ImGuiMouseCursor_##ImGui: \
						cursor = W::Cursor::cur; \
						break \

				CASE(text, TextInput);

				CASE(all_scroll, ResizeAll);
				CASE(td_resize, ResizeNS); CASE(lr_resize, ResizeEW);
				CASE(tldr_resize, ResizeNWSE); CASE(trdl_resize, ResizeNESW);

				CASE(grab, Hand);
				CASE(wait, Wait); CASE(progress, Progress);
				CASE(not_allowed, NotAllowed);

				case ImGuiMouseCursor_None:
					show_mouse = false;
					break;

				default:
					break;

				#undef CASE
				}

				m_cursor_owner = true;
				window.show_mouse(show_mouse);
				window.cursor(cursor);
			}
			else if (m_cursor_owner)
			{
				m_cursor_owner = false;
				window.show_mouse(true);
				window.cursor(W::Cursor::default_);
			}
		}

		void update_mouse(W& window, ImGuiIO& io, WindowSystem::InputState& input)
		{
			if (io.WantSetMousePos)
				window.warp_mouse({io.MousePos.x, io.MousePos.y});

			auto source = ImGuiMouseSource_Mouse;
			if (input.is_pen_on_surface())
				source = ImGuiMouseSource_Pen;

			auto [mpx, mpy] = input.position();
			io.AddMouseSourceEvent(source);
			io.AddMousePosEvent(mpx, mpy);

			auto [mwx, mwy] = input.wheel();
			io.AddMouseSourceEvent(source);
			io.AddMouseWheelEvent(mwx, mwy);

			using B = WindowSystem::Codes::Button;
			using BT = WindowSystem::Codes::Traits<B>;
			using BET = EnumTraits<B>;
			for (usize i = 0; i < BT::count; ++i)
			{
				auto button = BET::cast(i);
				io.AddMouseSourceEvent(source);
				io.AddMouseButtonEvent(button_to_imgui(button), input.button(button));
			}

			io.PenPressure = input.pen_pressure();
		}

		PtrContainer<ImGuiContext> m_context;

		bool m_string_recording_owner = false;
		bool m_cursor_owner = false;
		bool m_keyboard_focus = false;
	};
}
