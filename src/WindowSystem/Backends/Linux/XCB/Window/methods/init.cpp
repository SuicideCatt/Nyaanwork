module;

#include <array>

export module Nyaanwork.WindowSystem.Backends.Linux.XCB:Window.methods.init;
import :Bind.XCB;
import :Instance.decl;
import :Window.decl;

import Nyaanwork.Core.Math;
import Nyaanwork.Core.Types;
import Nyaanwork.Core.Utils.Flags;

export namespace Nyaanwork::WindowSystem::Backends::Linux::XCB
{
	Window::Window(Ptr<Instance> instance, const Info& info)
		: m_instance(instance),
		  m_xwindow([this]
			{
				return static_cast<InstanceWindowInterface&>(*m_instance)
					.register_window_begin();
			}())
	{
		auto& i = *m_instance;
		auto [c, s] = i.xdata();

		{
			auto [min, max, _] = info.bounds.resolution;

			static constexpr vec2<u16> supermin(1);
			min = Math::max(supermin, min);
			max = Math::max(min, max);

			p_resolution.min = min;
			p_resolution.max = max;
		}

		const auto& pos = p_position = info.bounds.position;
		const auto& res = p_resolution.value =
			clamp(info.bounds.resolution.value, p_resolution.min, p_resolution.max);

		namespace W = ::XCB::Window;
		using CW = W::CW;
		using E = ::XCB::Events::Flags;
		W::create(c, ::XCB::copy_from_parent, m_xwindow, s->root,
				  pos, res, 0, W::Class::copy_from_parent,
				  s->root_visual, CW::back_pixel | CW::event_mask,
				  {
				     .background_pixel = s->black_pixel,
				     .event_mask = E::structure_notify | FlagTraits<E>::focus
									| E::property_change
				  });

		{
			auto [min, max, _] = info.bounds.resolution;

			static constexpr vec2<u16> supermin(1);
			min = Math::max(supermin, min);
			max = Math::max(min, max);

			min_max_resolution_impl(p_resolution.min, p_resolution.max);
		}

		using PCM = ::XCB::PropertyChangeMode;
		auto& atoms = i.atoms();
		using Atoms = ::XCB::Atoms;

		{
			std::array protocols = {atoms.delete_window, atoms.net_wm.ping};
			W::change_property<::XCB::Atom>(c, m_xwindow, PCM::replace,
											atoms.protocols, Atoms::atom,
											protocols);
		}

		{
			auto inst = info.widnow_instance_name;

			auto cls = i.window_class_name();

			if (!inst)
				inst = cls;

			auto change_prop = [&c, w=xwindow()](const Str& str)
			{
				W::change_property<char>(c, w, PCM::append,
										 Atoms::wm_class, Atoms::string,
										 {str.data(), str.size()+1});
			};

			change_prop(*inst);
			change_prop(cls);
		}

		{
			static constexpr u32 version = 5;
			W::change_property<u32>(c, m_xwindow, PCM::replace,
									atoms.dnd.aware, Atoms::atom,
									{&version, 1});
		}

		title(info.title);

		resizable(info.resizable);

		W::map(c, m_xwindow);

		borderless(info.borderless);
		fullscreen(info.fullscreen);

		show_mouse(info.mouse.show);
		grab_mouse(info.mouse.grab);
		centralize_mouse(info.mouse.centralize);

		static_cast<InstanceWindowInterface&>(i).register_window_end(*this);
	}

	Window::~Window()
	{
		auto& i = *m_instance;
		auto c = i.xconnection();

		static_cast<InstanceWindowInterface&>(i).unregister_window(*this);

		::XCB::Window::destroy(c, m_xwindow);
		::XCB::flush(c);
	}
}
