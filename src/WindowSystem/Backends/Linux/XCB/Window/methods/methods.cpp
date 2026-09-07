module;

#include <Nyaanwork/Asset.hpp>
#include <Nyaanwork/WindowSystem.hpp>

#ifdef NYAANWORK_WINDOW_SYSTEM_VULKAN
#define VK_USE_PLATFORM_XCB_KHR
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan.hpp>
#endif

#include <vector>

export module Nyaanwork.WindowSystem.Backends.Linux.XCB:Window.methods.methods;
import :Bind.XCB;
import :Instance.decl;
import :Window.decl;

import Nyaanwork.Core.Math;
import Nyaanwork.Core.Types;
import Nyaanwork.WindowSystem.Backends.Base;

export namespace Nyaanwork::WindowSystem::Backends::Linux::XCB
{
	Ptr<Base::Instance> Window::instance() const
		{ return m_instance; }

	#ifdef NYAANWORK_WINDOW_SYSTEM_VULKAN
	vk::SurfaceKHR Window::create_vulkan_surface(vk::Instance instance,
						vk::Optional<vk::AllocationCallbacks const> allocator)
	{
		vk::XcbSurfaceCreateInfoKHR info = {
			.connection = m_instance->xconnection(),
			.window = xwindow(),
		};

		return instance.createXcbSurfaceKHR(info, allocator);
	}
	#endif

	void Window::title_impl(const Str& title)
	{
		auto& i = *m_instance;
		auto c = i.xconnection();
		auto& atoms = i.atoms();

		using ::XCB::Window::change_property;
		using enum ::XCB::PropertyChangeMode;
		change_property<char>(c, xwindow(), replace,
							  ::XCB::Atoms::wm_name, ::XCB::Atoms::string,
							  title);
		change_property<char>(c, xwindow(), replace,
							  atoms.net_wm.name, atoms.types.utf8_string,
							  title);

		::XCB::flush(c);
	}

	#ifdef NYAANWORK_ASSET_IMAGE
	void Window::icon_impl(const Asset::Image<Asset::Pixels::RGBA>& icon)
	{
		std::vector<u32> buffer(icon.size()+2);

		{
			auto [x, y] = icon.resolution();
			buffer[0] = x;
			buffer[1] = y;
		}

		auto target = buffer.data()+2;
		for (auto& p : icon)
			*target++ = (p.b) | (p.g << 8) | (p.r << 16) | (p.a << 24);

		auto& i = *m_instance;
		auto c = i.xconnection();
		const auto& icon_atom = m_instance->atoms().net_wm.icon;

		using ::XCB::Window::change_property;
		using enum ::XCB::PropertyChangeMode;
		change_property<u32>(c, xwindow(), replace,
							 icon_atom, ::XCB::Atoms::cardinal,
							 buffer);

		::XCB::flush(c);
	}
	#endif

	void Window::position_impl(Position position)
	{
		namespace W = ::XCB::Window;
		auto c = m_instance->xconnection();

		W::configure(c, xwindow(),
					 FlagTraits<W::Config>::position,
					 {
						.x = position.x,
						.y = position.y,
					 });

		::XCB::flush(c);
	}

	void Window::resolution_impl(vec2<u16> resolution)
	{
		namespace W = ::XCB::Window;
		auto c = m_instance->xconnection();

		W::configure(c, xwindow(),
					 FlagTraits<W::Config>::resolution,
					 {
						.width = resolution.x,
						.height = resolution.y,
					 });

		if (!resizable_no_lock())
			min_max_resolution_impl(resolution, resolution);

		::XCB::flush(c);
	}

	void Window::min_resolution_impl(vec2<u16> resolution)
	{
		auto c = m_instance->xconnection();

		::XCB::SizeHints{
			.flags = ::XCB::SizeHints::Hints::p_min_size,
			.min_resolution = resolution,
		}.change(c, xwindow());

		::XCB::flush(c);
	}

	void Window::max_resolution_impl(vec2<u16> resolution)
	{
		auto c = m_instance->xconnection();

		::XCB::SizeHints{
			.flags = ::XCB::SizeHints::Hints::p_max_size,
			.max_resolution = resolution,
		}.change(c, xwindow());

		::XCB::flush(c);
	}

	void Window::min_max_resolution_impl(vec2<u16> min, vec2<u16> max) noexcept
	{
		auto c = m_instance->xconnection();

		using Hints = ::XCB::SizeHints::Hints;
		::XCB::SizeHints{
			.flags = Hints::p_min_size | Hints::p_max_size,
			.min_resolution = min,
			.max_resolution = max,
		}.change(c, xwindow());

		::XCB::flush(c);
	}

	void Window::resizable_impl(bool mode)
	{
		if (mode)
			min_max_resolution_impl(p_resolution.min, p_resolution.max);
		else
			min_max_resolution_impl(p_resolution.value, p_resolution.value);
	}

	void Window::fullscreen_impl(bool mode)
	{
		using namespace ::XCB::Events;
		using ::XCB::Events::Flags;
		auto& i = *m_instance;
		auto c = i.xconnection();
		auto& atoms = i.atoms();

		static constexpr auto flags =
			Flags::substructure_redirect | Flags::substructure_notify;
		send<ClientMessage>(c, xwindow(), flags, {
			.response_type = Response::client_message,
			.format = 32,
			.window = xwindow(),
			.type = atoms.net_wm.state,
			.data = {
				.data32 = {
					mode, atoms.net_wm.fullscreen,
					static_cast<::XCB::Atom>(::XCB::Atoms::none),
				}
			}
		}, true);

		::XCB::flush(c);
	}

	void Window::borderless_impl(bool mode)
	{
		auto& i = *m_instance;
		auto c = i.xconnection();
		auto& atoms = i.atoms().motif_wm_hints;

		::XCB::MotifHints{
			.flags = ::XCB::MotifHints::Hints::decorations,
			.decorations = !(mode),
		}.change(c, xwindow(), atoms);

		::XCB::flush(c);
	}

	void Window::cursor_impl(Cursor cursor)
	{
		namespace W = ::XCB::Window;
		auto& i = *m_instance;
		auto c = i.xconnection();

		W::change_attributes(c, xwindow(), W::CW::cursor, {
			.cursor = static_cast<InstanceWindowInterface&>(i).load_cursor(cursor),
		});

		::XCB::flush(c);
	}

	void Window::show_mouse_impl(bool mode)
	{
		// if cursor showed and mouse unfocused - skip
		//   see mouse focus handle
		//   on X11 hide work for root, hide cursor for all windows is not ok
		// or if cursor already showed/hiden - skip
		//   twice call can broke show and hide calls
		if ((m_cursor_showed && !mouse_focused_no_lock()) || mode == m_cursor_showed)
			return;

		auto c = m_instance->xconnection();

		using namespace ::XCB::Fixes;
		if ((m_cursor_showed = mode))
			show(c, xwindow());
		else
			hide(c, xwindow());

		::XCB::flush(c);
	}

	void Window::grab_mouse_impl(bool mode)
	{
		// if cursor not grabbed and unfocused a mouse or keyboard: skip,
		//   see mouse focus handle
		//   on X11 grab work allways, stealing cursor it's not ok
		//   on Xwayland grab if mouse is unfocused is can broke grab requests
		// or if cursor already [un]grabbed - skip, why? i forgot...
		//   writing comment after testing is bad idea, sorry
		if ((!m_cursor_grabbed
				&& (!keyboard_focused_no_lock() || !mouse_focused_no_lock()))
			|| mode == m_cursor_grabbed)
		{
			return;
		}

		namespace P = ::XCB::Pointer;
		auto c = m_instance->xconnection();

		if ((m_cursor_grabbed = mode))
		{
			using ::XCB::Events::Flags;
			static constexpr auto events = Flags::enter_window | Flags::leave_window;
			using I = decltype(events)::MaskType;
			P::Grab::request_reply(c, false, xwindow(),
								   static_cast<u16>(static_cast<I>(events)),
								   P::Mode::async, P::Mode::async,
								   xwindow(), ::XCB::none, ::XCB::current_time);
		}
		else
		{
			P::ungrab(c, ::XCB::current_time);
		}

		::XCB::flush(c);
	}

	void Window::warp_mouse_impl(Position pos)
	{
		namespace P = ::XCB::Pointer;
		auto& i = *m_instance;
		auto c = i.xconnection();
		bool fix = i.xwayland() && show_mouse_no_lock();

		if (fix) // SDL hack, on Xwayland need hide cursor before wrap
			show_mouse_impl(false);

		P::warp(c, xwindow(), pos);

		if (fix)
			show_mouse_impl(true); // show_mouse_impl call XCB::flush
		else
			::XCB::flush(c);
	}
}
