module;

#include <Core/Defines.hpp>
#include <Nyaanwork/Asset.hpp>
#include <Nyaanwork/WindowSystem.hpp>

#include <windows.h>

#ifdef NYAANWORK_WINDOW_SYSTEM_VULKAN
#include <vulkan/vulkan.hpp>
#endif

#include <atomic>
#include <mutex>
#include <semaphore>
#include <thread>

export module Nyaanwork.WindowSystem.Backends.Windows:Window.decl;
import :decl;

import Nyaanwork.Core.Math;
import Nyaanwork.Core.Types;
import Nyaanwork.Core.Utils.ClearType;
import Nyaanwork.Core.Utils.UPtr;
import Nyaanwork.WindowSystem.Backends.Base;

#ifdef NYAANWORK_ASSET_IMAGE
import Nyaanwork.Asset.Image;
#endif

#ifdef NYAANWORK_WINDOW_SYSTEM_OPENGL
import Nyaanwork.WindowSystem.OpenGL;
#endif

namespace Nyaanwork::WindowSystem::Backends::Windows::WM
{
	using Msg = decltype(MSG::message);
	constexpr Msg base =		 WM_USER;
	constexpr Msg title =		 base + 0;
	constexpr Msg posres =		 base + 1;
	constexpr Msg fullscreen =	 base + 2;
	constexpr Msg unfullscreen = base + 3;
}

namespace Nyaanwork::WindowSystem::Backends::Windows
{
	class DropTarget;
	LRESULT CALLBACK windows_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
}

export namespace Nyaanwork::WindowSystem::Backends::Windows
{
	class Window final
		: public Base::Window, public EnableSharedFromThis<Window>
	{
	public:
		Window(Ptr<Instance> instance, const Info& info);
		~Window();

		Ptr<Base::Instance> instance() const override;
		Ptr<Instance> native_instance() const
			{ return m_instance; }

		HWND handle() NYAAN_CNOEX
			{ return m_handle.get(); }

		#ifdef NYAANWORK_WINDOW_SYSTEM_OPENGL
		Ptr<OpenGL::Surface>
			create_opengl_surface(Ptr<OpenGL::Instance> instance) override
			{ return make_shared<OpenGL::Surface>(instance, handle()); }

		Ptr<OpenGL::SurfaceContext>
			create_opengl_surface_context(Ptr<OpenGL::Instance> instance,
										  OpenGL::Context::Profile profile,
										  vec2<u8> version) override
		{
			return make_shared<OpenGL::SurfaceContext>(instance, profile,
													   version, handle());
		}
		#endif

		#ifdef NYAANWORK_WINDOW_SYSTEM_VULKAN
		NYAAN_ND vk::SurfaceKHR
			create_vulkan_surface(
				vk::Instance instance,
				vk::Optional<vk::AllocationCallbacks const> allocator = nullptr) override;
		#endif

	private:
		friend class DropTarget;
		friend LRESULT windows_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

		static Resolution adjust_resolution(Resolution resolution,
											LONG style = WS_OVERLAPPEDWINDOW) noexcept
		{
			RECT rect = {0, 0, resolution.x, resolution.y};
			AdjustWindowRect(&rect, style, false);
			vec2<i32> res(rect.right - rect.left, rect.bottom - rect.top);
			return clamp(res, {1, 1}, vec2<i32>(limits<u16>::max()));
		}

		void change_style(UINT flags, bool add)
		{
			m_style = GetWindowLong(handle(), GWL_STYLE);

			if (add)
				m_style |= flags;
			else
				m_style &= ~flags;

			SetWindowLong(handle(), GWL_STYLE, m_style);
		}

		RECT client_rect() noexcept
		{
			RECT rect;
			GetWindowRect(handle(), &rect);

			auto caption = GetSystemMetrics(SM_CYCAPTION);
			rect.top += caption;

			auto x_border = GetSystemMetrics(SM_CXSIZEFRAME);
			rect.left += x_border;
			rect.right -= x_border;

			auto y_border = GetSystemMetrics(SM_CYSIZEFRAME);
			rect.top += y_border;
			rect.bottom -= y_border;

			return rect;
		}

		void update_rect() noexcept
		{
			RECT rect = client_rect();
			ClipCursor(&rect);
		};

		LRESULT proc(UINT msg, WPARAM wparam, LPARAM lparam);

		void title_impl(const Str& title) override
			{ PostMessage(handle(), WM::title, 0, 0); }

		#ifdef NYAANWORK_ASSET_IMAGE
		void icon_impl(const Asset::Image<Asset::Pixels::RGBA>& icon) override;
		#endif

		void position_impl(Position position) override
			{ PostMessage(handle(), WM::posres, true, false); }

		void resolution_impl(Resolution resolution) override
			{ PostMessage(handle(), WM::posres, false, true); }

		void min_resolution_impl(Resolution resolution) override {};
		void max_resolution_impl(Resolution resolution) override {};

		void resizable_impl(bool mode) override
		{
			change_style(WS_SIZEBOX | WS_MAXIMIZEBOX, mode);
			if (m_work)
				resolution_impl({});
		}

		void fullscreen_impl(bool mode) override;
		void borderless_impl(bool mode) override
		{
			change_style(WS_BORDER | WS_CAPTION, !mode);
			if (m_work)
				resolution_impl({});
		}

		void cursor_impl(Cursor cursor) override;

		void show_mouse_impl(bool mode) override
		{
			if (mode)
			{
				cursor_impl(p_cursor);
			}
			else
			{
				m_cursor = nullptr;
				if (mouse_focused_no_lock())
					PostMessage(handle(), WM_SETCURSOR, 0, 0);
			}
		}

		void grab_mouse_impl(bool mode) override
		{
			if (keyboard_focused_no_lock() && mouse_focused_no_lock())
			{
				if (mode)
					update_rect();
				else
					ClipCursor(nullptr);
			}
		}

		void warp_mouse_impl(Position position) override
		{
			RECT rect = client_rect();
			position += Position(rect.left, rect.top);
			SetCursorPos(position.x, position.y);
		}

		void handle_raw_input(HRAWINPUT hraw);
		void handle_raw_input_keyboard(const RAWKEYBOARD& input);
		void handle_raw_input_mouse(const RAWMOUSE& input);

		Ptr<Instance> m_instance;
		GetUPtr<ClearType<HWND>, &DestroyWindow> m_handle;

		LONG m_style;
		struct {
			LONG style;
			Position position;
			Resolution resolution;
		} m_unfullscreen;

		struct
		{
			decltype(p_input.keyboard.state) keyboard_state;
			struct
			{
				Position motion;
				vec2<f32> wheel;
				decltype(p_input.mouse.state) state;
			} mouse;
			std::mutex* mutex;
			std::vector<u8>* buffer;
		} m_raw;

		#ifdef NYAANWORK_ASSET_IMAGE
		GetUPtr<ClearType<HICON>, &DestroyIcon> m_icon;
		#endif

		HCURSOR m_cursor;

		std::atomic_bool m_work;
		std::binary_semaphore m_semaphore;
		std::thread m_thread;
	};
}
