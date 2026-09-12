module;

#include <Core/Defines.hpp>
#include <Nyaanwork/WindowSystem.hpp>

#ifdef NYAANWORK_WINDOW_SYSTEM_VULKAN
#define VK_USE_PLATFORM_XCB_KHR
#include <vulkan/vulkan.hpp>
#endif

#include <atomic>
#include <mutex>
#include <thread>
#include <unordered_map>

export module Nyaanwork.WindowSystem.Backends.Linux.XCB:Instance.decl;
import :Bind.XCB;
import :Bind.XKB;
import :Bind.Xlib;
import :decl;

import Nyaanwork.Core.Types;
import Nyaanwork.Core.Utils.UPtr;
import Nyaanwork.WindowSystem.Backends.Base;

#ifdef NYAANWORK_WINDOW_SYSTEM_OPENGL
import Nyaanwork.WindowSystem.OpenGL;
#endif

namespace Nyaanwork::WindowSystem::Backends::Linux::XCB::InstanceBlocks
{
	struct Main
	{
		Main();

		Xlib::Display xlib;
		::XCB::Data xcb;
	};

	struct Extensions
	{
		Extensions(::XCB::Connection* c);

		u8 xinput_opcode;
		u8 xkb_event;
		u8 xrandr_event;
		bool xwayland;
	};

	struct Atoms
	{
		using Atom = ::XCB::Atom;

		Atoms(::XCB::Connection* c) noexcept;

		Atom protocols, delete_window, motif_wm_hints;
		// Atom clipboard;

		struct
		{
			Atom utf8_string, utf8_plain, plain, text;
			Atom uri_list;
		} types;

		struct
		{
			Atom state, fullscreen, icon, name, ping, hidden;
		} net_wm;

		struct
		{
			struct
			{
				struct
				{
					Atom x, y;
				} position, tilt;
				Atom pressure;
			} stylus_labels;
			struct
			{
				Atom serial_ids;
			} wacom;
		} input;

		struct
		{
			Atom aware, enter, leave, finished, status;
			Atom position, type_list;
			Atom action_copy, drop, selection;
		} dnd;
	};

	struct Input
	{
		struct Tablet
		{
			enum class Bind : u8
			{
				none,
				position_x, position_y,
				pressure,
				tilt_x, tilt_y,
			};

			struct vec2
			{
				::XCB::Input::fp3232 x, y;
			};

			std::array<Bind, 8> binds = {};

			struct
			{
				::XCB::Input::fp3232 pressure_max = {};
				struct
				{
					vec2 max, min;
				} tilt = {};
			} fp3232;

			f64 pressure_max = {};
			struct
			{
				Nyaanwork::vec2<f64> max, min;
			} tilt = {};

			vec2 position_old = {};

			bool on_surface = false;
		};

		Input(::XCB::Data data, const Atoms& atoms) noexcept;

		bool add_if_tablet(::XCB::Connection* c, const Atoms& atoms,
						   ::XCB::Input::DeviceID device_id) noexcept;

		bool is_any_tablet_pen_on_surface() NYAAN_CNOEX;

		std::unordered_map<::XCB::Input::DeviceID, Tablet> tablets;
	};

	struct XKB
	{
		XKB(::XCB::Connection* c);

		void create_keymap_state(::XCB::Connection* c);

		const ::XKB::Context context;
		const ::XCB::XKB::ID core_keyboard;

		::XKB::Keymap keymap;
		::XKB::State state;

		usize layout;
	};

	struct Cursor
	{
		Cursor(::XCB::Data xdata);

		::XCB::Cursor::Cursor load_or_get(Base::Window::Cursor cursor) noexcept;
		void clear(::XCB::Connection* c) noexcept;

		::XCB::Cursor::Context context;
		std::unordered_map<Base::Window::Cursor, ::XCB::Cursor::Cursor> cursors;
	};
}

namespace Nyaanwork::WindowSystem::Backends::Linux::XCB
{
	class InstanceWindowInterface
	{
	public:
		virtual ~InstanceWindowInterface() = default;

	protected:
		friend Window;

		virtual ::XCB::Window::ID register_window_begin() = 0;
		virtual void register_window_end(Window& window) = 0;
		virtual void unregister_window(Window& window) = 0;

		virtual ::XCB::Cursor::Cursor load_cursor(Base::Window::Cursor cursor) = 0;
	};
}

export namespace Nyaanwork::WindowSystem::Backends::Linux::XCB
{
	using Atoms = InstanceBlocks::Atoms;

	class Instance final
		: public Base::Instance, public InstanceWindowInterface,
		  public EnableSharedFromThis<Instance>
	{
	public:
		Instance(Opt<Str> window_class_name);
		~Instance();

		Str full_name() NYAAN_CNOEX override
			{ return "Nyaanwork::WindowSystem::Backends::Linux::XCB"; }
		Str short_name() NYAAN_CNOEX override
			{ return "XCB"; }

		::XCB::Connection* xconnection() NYAAN_CNOEX
			{ return m_main.xcb.first; }

		const ::XCB::Screen* xscreen() NYAAN_CNOEX
			{ return m_main.xcb.second; }

		::XCB::Data xdata() NYAAN_CNOEX
			{ return {m_main.xcb.first, m_main.xcb.second}; }

		bool xwayland() NYAAN_CNOEX
			 { return m_extensions.xwayland; }

		const Atoms& atoms() NYAAN_CNOEX
			{ return m_atoms; }

		u32 generate_id()
			{ return ::XCB::generate_id(xconnection()); }

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

		Str keyboard_layout() const override;

		Displays displays() const override;

		bool multiwindow_support() NYAAN_CNOEX override
			{ return true; }

		bool have_tablet() const override
			{ return m_thread.have_tablet; }

		#ifdef NYAANWORK_WINDOW_SYSTEM_OPENGL
		Ptr<OpenGL::Instance>
			create_opengl_instance(OpenGL::Instance::API api) override
			{ return make_shared<OpenGL::Instance>(api, m_main.xlib.get()); }
		#endif

		#ifdef NYAANWORK_WINDOW_SYSTEM_VULKAN
		std::vector<const char*> vulkan_instance_extensions() const override
		{
			return {
				vk::KHRSurfaceExtensionName, vk::KHRXcbSurfaceExtensionName,
			};
		}
		#endif

	private:
		std::scoped_lock<std::mutex> lock() const
			{ return std::scoped_lock(m_thread.mutex); }

		Window* get_window(::XCB::Window::ID window) const;
		Window* get_window_focused_keyboard() const;
		Window* get_window_focused_mouse() const;

		void event_loop();
		void handle_client_message(bool& enable_wait,
								   ::XCB::Events::ClientMessage& ev);
		void handle_selection_notify(::XCB::Events::SelectionNotify& ev);
		void handle_ge_event(::XCB::Timestamp& ignore_enter_leave,
							 ::XCB::Events::GE_Event& ev);
		void handle_xkb_event(::XCB::Events::Generic::element_type& ev);

		::XCB::Window::ID register_window_begin() override;
		void register_window_end(Window& window) override;
		void unregister_window(Window& window) override;

		::XCB::Cursor::Cursor load_cursor(Base::Window::Cursor cursor) override;

		struct
		{
			const InstanceBlocks::Main m_main;
			const InstanceBlocks::Extensions m_extensions;
			const Atoms m_atoms;
		}; //read-only

		InstanceBlocks::Input m_input;
		InstanceBlocks::XKB m_xkb;
		InstanceBlocks::Cursor m_cursor;

		struct
		{
			std::unordered_map<::XCB::Window::ID, Window*> all;
			struct
			{
				Window* keyboard = nullptr;
				Window* mouse = nullptr;
			} focused;
		} m_windows;

		mutable Displays m_displays;

		struct
		{
			std::atomic_bool displays_changed;
			std::atomic_bool have_tablet;
			std::atomic_bool work;
			std::atomic_bool no_windows;
			mutable std::mutex mutex;
			std::thread thread;
		} m_thread;
	};
}
