module;

#include <Core/Defines.hpp>
#include <Nyaanwork/Asset.hpp>
#include <Nyaanwork/WindowSystem.hpp>

#ifdef NYAANWORK_WINDOW_SYSTEM_VULKAN
#include <vulkan/vulkan.hpp>
#endif

#include <vector>

export module Nyaanwork.WindowSystem.Backends.Linux.XCB:Window.decl;
import :Bind.XCB;
import :decl;

import Nyaanwork.Core.Types;
import Nyaanwork.WindowSystem.Backends.Base;
import Nyaanwork.WindowSystem.Codes;

#ifdef NYAANWORK_ASSET_IMAGE
import Nyaanwork.Asset.Image;
#endif

#ifdef NYAANWORK_WINDOW_SYSTEM_OPENGL
import Nyaanwork.WindowSystem.OpenGL;
#endif

namespace Nyaanwork::WindowSystem::Backends::Linux::XCB
{
	class HandleEventsI
	{
	public:
		virtual ~HandleEventsI() = default;

	protected:
		struct DropBuffer
		{
			i32 xdnd_version;
			::XCB::Window::ID source;
			::XCB::Atom atom;
		};

	private:
		friend Instance;

		virtual void handle_keyboard_focus(bool focused) = 0;
		virtual void handle_mouse_focus(bool focused) = 0;

		virtual void handle_change_property() = 0;

		virtual void handle_configure(Base::Window::Position position,
									  Base::Window::Resolution resolution) = 0;

		virtual void handle_key(Codes::Key key, bool mode) = 0;

		virtual void handle_button(Codes::Button key, bool mode) = 0;
		virtual void handle_mouse_position(Base::Window::Position position) = 0;
		virtual void handle_mouse_motion(vec2<f32> motion) = 0;
		virtual void handle_mouse_wheel(vec2<f32> wheel) = 0;

		virtual void handle_tablet_on_surface(bool on_surface) = 0;
		virtual void handle_tablet_data(f32 pressure, vec2<f32> tilt) = 0;

		virtual void handle_dnd_enter_uri(::XCB::Window::ID source,
										  i32 xdnd_version) = 0;
		virtual void handle_dnd_enter_text(::XCB::Atom atom, ::XCB::Window::ID source,
										   i32 xdnd_version) = 0;
		virtual void handle_dnd_cancel() = 0;
		virtual void handle_dnd_done_uri(const std::vector<Str>& uris) = 0;
		virtual void handle_dnd_done_text(const Str& text) = 0;
		virtual Opt<DropBuffer> drop_data() const = 0;

		virtual void handle_record_new_string(Str&& text) = 0;
		virtual void handle_record_newline() = 0;
		virtual void handle_record_pop() = 0;
	};
}

export namespace Nyaanwork::WindowSystem::Backends::Linux::XCB
{
	class Window final
		: public Base::Window, public HandleEventsI,
		  public EnableSharedFromThis<Window>
	{
	public:
		Window(Ptr<Instance> instance, const Info& info);
		~Window();

		Ptr<Instance> native_instance() const
			{ return m_instance; }
		Ptr<Base::Instance> instance() const override;

		::XCB::Window::ID xwindow() NYAAN_CNOEX
			{ return m_xwindow; }

		#ifdef NYAANWORK_WINDOW_SYSTEM_OPENGL
		virtual Ptr<OpenGL::Surface>
			create_opengl_surface(Ptr<OpenGL::Instance> instance) override
			{ return make_shared<OpenGL::Surface>(instance, xwindow()); }

		virtual Ptr<OpenGL::SurfaceContext>
			create_opengl_surface_context(Ptr<OpenGL::Instance> instance,
										  OpenGL::Context::Profile profile,
										  vec2<u8> version) override
		{
			return make_shared<OpenGL::SurfaceContext>(instance, profile,
													   version, xwindow());
		}
		#endif

		#ifdef NYAANWORK_WINDOW_SYSTEM_VULKAN
		NYAAN_ND virtual vk::SurfaceKHR
			create_vulkan_surface(
				vk::Instance instance,
				vk::Optional<vk::AllocationCallbacks const> allocator = nullptr) override;
		#endif

	private:
		void title_impl(const Str& title) override;

		#ifdef NYAANWORK_ASSET_IMAGE
		virtual void icon_impl(const Asset::Image<Asset::Pixels::RGBA>& icon) override;
		#endif

		void position_impl(Position position) override;
		void resolution_impl(Resolution resolution) override;
		void min_resolution_impl(Resolution resolution) override;
		void max_resolution_impl(Resolution resolution) override;
		void min_max_resolution_impl(Resolution min, Resolution max) noexcept;

		void resizable_impl(bool mode) override;
		void fullscreen_impl(bool mode) override;
		void borderless_impl(bool mode) override;

		void cursor_impl(Cursor cursor) override;
		void show_mouse_impl(bool mode) override;
		void grab_mouse_impl(bool mode) override;

		void warp_mouse_impl(Position position) override;

		void handle_keyboard_focus(bool focused) override;
		void handle_mouse_focus(bool focused) override;

		void handle_change_property() override;

		void handle_configure(Base::Window::Position position,
							  Base::Window::Resolution resolution) override;

		void handle_key(Codes::Key key, bool mode) override;

		void handle_button(Codes::Button key, bool mode) override;
		void handle_mouse_position(Base::Window::Position position) override;
		void handle_mouse_motion(vec2<f32> motion) override;
		void handle_mouse_wheel(vec2<f32> wheel) override;

		void handle_tablet_on_surface(bool on_surface) override;
		void handle_tablet_data(f32 pressure, vec2<f32> tilt) override;

		void handle_dnd_enter_uri(::XCB::Window::ID source,
								  i32 xdnd_version) override;
		void handle_dnd_enter_text(::XCB::Atom atom, ::XCB::Window::ID source,
								   i32 xdnd_version) override;
		void handle_dnd_cancel() override;
		void handle_dnd_done_uri(const std::vector<Str>& uris) override;
		void handle_dnd_done_text(const Str& text) override;
		Opt<DropBuffer> drop_data() const override;

		void handle_record_new_string(Str&& text) override;
		void handle_record_newline() override;
		void handle_record_pop() override;

		const Ptr<Instance> m_instance;
		const ::XCB::Window::ID m_xwindow;

		Opt<DropBuffer> m_drop_buffer;

		bool m_cursor_showed = true;
		bool m_cursor_grabbed = false;
	};
}
