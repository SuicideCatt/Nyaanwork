module;

#include <Core/Defines.hpp>
#include <Nyaanwork/WindowSystem.hpp>

#include <windows.h>

#ifdef NYAANWORK_WINDOW_SYSTEM_VULKAN
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>
#endif

#include <atomic>
#include <mutex>
#include <unordered_map>
#include <vector>

export module Nyaanwork.WindowSystem.Backends.Windows:Instance.decl;
import :SAFETY;
import :decl;

import Nyaanwork.Core.Utils.ClearType;
import Nyaanwork.Core.Utils.UPtr;
import Nyaanwork.WindowSystem.Backends.Base;

#ifdef NYAANWORK_WINDOW_SYSTEM_OPENGL
import Nyaanwork.WindowSystem.OpenGL;
#endif

namespace Nyaanwork::WindowSystem::Backends::Windows
{
	class InstanceWindowInterface
	{
	public:
		virtual ~InstanceWindowInterface() = default;

	protected:
		friend Window;

		virtual void handle_keyboard_focus(Window* window) = 0;
		virtual void handle_keyboard_unfocus(Window* window) = 0;

		virtual void handle_mouse_focus(Window* window) = 0;
		virtual void handle_mouse_unfocus(Window* window) = 0;

		virtual void handle_displays_change() = 0;

		virtual HCURSOR load_cursor(Base::Window::Cursor cursor) = 0;
	};
}

export namespace Nyaanwork::WindowSystem::Backends::Windows
{
	class Instance final
		: public Base::Instance, public InstanceWindowInterface,
		  public EnableSharedFromThis<Instance>
	{
	public:
		Instance(Opt<Str> class_name);
		~Instance();

		Str full_name() NYAAN_CNOEX override
			{ return "Nyaanwork::WindowSystem::Backends::Windows"; }
		Str short_name() NYAAN_CNOEX override
			{ return "MS Windows"; }

		Str instance_class_name() NYAAN_CNOEX
			{ return  m_window_class; }

		HINSTANCE handle() NYAAN_CNOEX
			{ return m_handle; }

		Ptr<Base::Window> create_window(const Base::Window::Info& info) override;
		Ptr<Window> create_native_window(const Base::Window::Info& info);

		Ptr<Base::Window> focused_window() override;
		Ptr<const Base::Window> focused_window() const override;

		Ptr<Window> focused_native_window();
		Ptr<const Window> focused_native_window() const;

		Ptr<Base::Window> mouse_focused_window() override;
		Ptr<const Base::Window> mouse_focused_window() const override;

		Ptr<Window> mouse_focused_native_window();
		Ptr<const Window> mouse_focused_native_window() const;

		Str keyboard_layout() const override
			{ return ""; }

		Displays displays() const override;

		bool multiwindow_support() NYAAN_CNOEX override
			{ return true; }

		bool have_tablet() const override
			{ return false; }

		#ifdef NYAANWORK_WINDOW_SYSTEM_OPENGL
		Ptr<OpenGL::Instance>
			create_opengl_instance(OpenGL::Instance::API api) override
			{ return make_shared<OpenGL::Instance>(api, nullptr); }
		#endif

		#ifdef NYAANWORK_WINDOW_SYSTEM_VULKAN
		std::vector<const char*> vulkan_instance_extensions() const override
		{
			return {
				vk::KHRSurfaceExtensionName, vk::KHRWin32SurfaceExtensionName,
			};
		}
		#endif

	private:
		std::scoped_lock<std::mutex> lock() const
			{ return std::scoped_lock(m_mutex); }

		void handle_displays_change() override
		{
			m_displays_changed = true;
			signal_send<Slots::displays_changed>(*this);
		}

		void handle_keyboard_focus(Window* window) override
		{
			auto l = lock();
			m_windows.keyboard = window;
		}

		void handle_keyboard_unfocus(Window* window) override
		{
			auto l = lock();
			if (m_windows.keyboard == window)
				m_windows.keyboard = nullptr;
		}

		void handle_mouse_focus(Window* window) override
		{
			auto l = lock();
			m_windows.mouse = window;
		}

		void handle_mouse_unfocus(Window* window) override
		{
			auto l = lock();
			if (m_windows.mouse == window)
				m_windows.mouse = nullptr;
		}

		HCURSOR load_cursor(Base::Window::Cursor cursor) override;

		Str m_window_class;
		HINSTANCE m_handle;

		std::unordered_map<Base::Window::Cursor,
						   GetUPtr<ClearType<HCURSOR>, &DestroyCursor>> m_cursors;

		struct {
			usize count;
			Window* keyboard;
			Window* mouse;
		} m_windows;

		std::atomic_bool m_displays_changed;
		mutable Displays m_displays;

		mutable std::mutex m_mutex;
	};
}
