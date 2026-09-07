module;

#include <Core/Defines.hpp>
#include <Nyaanwork/WindowSystem.hpp>

#include <vector>

export module Nyaanwork.WindowSystem.Backends.Base:Instance;
import :Window.decl;
import :decl;

import Nyaanwork.Core.System;
import Nyaanwork.Core.Types;
import Nyaanwork.Core.Utils.Signals;

#ifdef NYAANWORK_WINDOW_SYSTEM_OPENGL
import Nyaanwork.WindowSystem.OpenGL;
#endif

namespace Nyaanwork::WindowSystem::Backends::Base
{
	enum class InstanceSlots
	{
		tablet_connected, tablet_disconnected,
		displays_changed,
		keyboard_layout_changed,
	};
}

export namespace Nyaanwork::WindowSystem::Backends::Base
{
	class Instance
		: public SyncedSignalsSender<InstanceSlots,
									 SingleSlotInfo<false, Instance&>>
	{
	public:
		using SyncedSignalsSender::Slots;
		using SyncedSignalsSender::SignalPosition;

		struct Displays
		{
			struct Info
			{
				Str name;
				Window::Position position;
				Window::Resolution resolution;
				usize refresh_rate;

				struct {
					vec2<u16> mm;
					f32 inch;
				} size;
			};

			std::vector<Info> displays;
			usize primary;
		};

		Instance(const Instance&) = delete;
		Instance& operator=(const Instance&) = delete;

		Instance(Instance&&) = delete;
		Instance& operator=(Instance&&) = delete;

		virtual ~Instance() = default;

		virtual Str full_name() NYAAN_CNOEX = 0;
		virtual Str short_name() NYAAN_CNOEX = 0;

		Str window_class_name() NYAAN_CNOEX
			{ return m_window_class_name; }

		virtual Ptr<Window> create_window(const Window::Info& info) = 0;

		virtual Ptr<Window> focused_window() = 0;
		virtual Ptr<const Window> focused_window() const = 0;

		virtual Ptr<Window> mouse_focused_window() = 0;
		virtual Ptr<const Window> mouse_focused_window() const = 0;

		virtual Str keyboard_layout() const = 0;

		virtual Displays displays() const = 0;

		virtual bool multiwindow_support() NYAAN_CNOEX = 0;
		virtual bool have_tablet() const = 0;

		#ifdef NYAANWORK_WINDOW_SYSTEM_OPENGL
		virtual Ptr<OpenGL::Instance>
			create_opengl_instance(OpenGL::Instance::API api) = 0;
		#endif

		#ifdef NYAANWORK_WINDOW_SYSTEM_VULKAN
		virtual std::vector<const char*> vulkan_instance_extensions() const = 0;
		#endif

	protected:
		Instance(Opt<Str> class_name)
		{
			if (!class_name)
			{
				auto path =
					System::executable_path().filename().replace_extension("");
				m_window_class_name = path.string();
			}
			else
			{
				m_window_class_name = *class_name;
			}
		}

	private:
		Str m_window_class_name;
	};
}
