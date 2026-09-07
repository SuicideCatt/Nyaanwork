module;

#include <Core/Defines.hpp>

#include <vector>

export module Nyaanwork.WindowSystem.Backends.Linux.XCB:Window.methods.handle;
import :Bind.XCB;
import :Exception;
import :Instance.decl;
import :Window.decl;

import Nyaanwork.Core.Math;

namespace Nyaanwork::WindowSystem::Backends::Linux::XCB
{
	void handle_focus(auto& wstate, bool mode, auto&& state)
	{
		if (mode)
			wstate |= state;
		else
			wstate &= ~state;
	}
}

export namespace Nyaanwork::WindowSystem::Backends::Linux::XCB
{
	void Window::handle_keyboard_focus(bool focused)
	{
		{
			auto l = unique_lock();
			handle_focus(p_state, focused, State::focus_keyboard);

			// if grab mode enabled and keyboard and mouse focused:
			//   enable grabbing
			// if keyboard unfocused: disable grabbing
			if (grab_mouse_no_lock())
			{
				if (mouse_focused_no_lock() && focused)
					grab_mouse_impl(true);
				else if (!focused)
					grab_mouse_impl(false);
			}

			if (!focused)
				p_input.keyboard.state.data = {};
		}

		if (focused)
			signal_send<Slots::focus_gained>(*this);
		else
			signal_send<Slots::focus_lost>(*this);
	}

	void Window::handle_mouse_focus(bool focused)
	{
		{
			auto l = unique_lock();
			handle_focus(p_state, focused, State::focus_mouse);

			// if grab mode enabled and keyboard and mouse focused:
			//   enable grabbing
			// if mouse unfocused: disable grabbing (is imposible!!!)
			if (grab_mouse_no_lock())
			{
				if (keyboard_focused_no_lock() && focused)
					grab_mouse_impl(true);
				else if (!focused)
					grab_mouse_impl(false);
			}

			// if mouse is hidden: show mouse if unfocused and hide if focused
			if (!show_mouse_no_lock())
				show_mouse_impl(!focused);

			if (!focused)
			{
				p_input.mouse.state.data = {};
				p_input.mouse.wheel = {};
				p_input.mouse.motion = {};

				p_input.tablet.on_surface = false;
				p_input.tablet.pressure = 0.f;
				p_input.tablet.tilt = {};
			}
		}

		if (focused)
			signal_send<Slots::mouse_enter>(*this);
		else
			signal_send<Slots::mouse_leave>(*this);
	}

	void Window::handle_change_property()
	{
		namespace W = ::XCB::Window;

		auto& i = *m_instance;
		auto& net_wm = i.atoms().net_wm;

		auto [r, atoms] =
			W::get_property<::XCB::Atom>(i.xconnection(),
										 xwindow(), net_wm.state);

		bool minimized = false;
		for (auto& atom : atoms)
			if (atom == net_wm.hidden)
				minimized = true;

		if (minimized != this->minimized())
		{
			{
				auto l = unique_lock();
				if (minimized)
					p_state |= State::minimized;
				else
					p_state &= ~State::minimized;
			}

			if (minimized)
				signal_send<Slots::minimized>(*this);
			else
				signal_send<Slots::unminimized>(*this);
		}
	}

	void Window::handle_configure(Base::Window::Position position,
								  Base::Window::Resolution resolution)
	{
		if (this->position() != position)
		{
			{
				auto l = unique_lock();
				p_position = position;
			}

			signal_send<Slots::moved>(*this, position);
		}

		if (this->resolution() != resolution)
		{
			{
				auto l = unique_lock();
				p_resolution.value = resolution;
			}

			signal_send<Slots::resized>(*this, resolution);
		}
	}

	void Window::handle_key(Codes::Key key, bool mode)
	{
		auto l = unique_lock();
		p_input.keyboard.state[key] = mode;
	}

	void Window::handle_button(Codes::Button button, bool mode)
	{
		auto l = unique_lock();
		p_input.mouse.state[button] = mode;
	}

	void Window::handle_mouse_position(Position position)
	{
		auto l = unique_lock();
		p_input.mouse.position = position;
	}

	void Window::handle_mouse_motion(vec2<f32> motion)
	{
		auto l = unique_lock();
		p_input.mouse.motion += motion;
	}

	void Window::handle_mouse_wheel(vec2<f32> wheel)
	{
		auto l = unique_lock();
		p_input.mouse.wheel = wheel;
	}

	void Window::handle_tablet_on_surface(bool on_surface)
	{
		auto l = unique_lock();
		p_input.tablet.on_surface = on_surface;
		if (!on_surface)
		{
			p_input.tablet.pressure = 0.f;
			p_input.tablet.tilt = {};
		}
	}

	void Window::handle_tablet_data(f32 pressure, vec2<f32> tilt)
	{
		auto l = unique_lock();
		if (p_input.tablet.on_surface)
		{
			p_input.tablet.pressure = pressure;
			p_input.tablet.tilt.degrees = tilt;
			p_input.tablet.tilt.radians = radians(tilt);
		}
	}

	void Window::handle_dnd_enter_uri(::XCB::Window::ID source, i32 xdnd_version)
	{
		if (m_drop_buffer) NYAAN_UNLIK
			throw Error::Exception(Error::Code::drag_and_drop_alredy_begined);

		auto& buf = m_drop_buffer.emplace();
		buf.atom = m_instance->atoms().types.uri_list;
		buf.source = source;
		buf.xdnd_version = xdnd_version;

		signal_send<Slots::drop_begin>(*this, DropDataType::uri);
	}

	void Window::handle_dnd_enter_text(::XCB::Atom atom, ::XCB::Window::ID source,
									   i32 xdnd_version)
	{
		if (m_drop_buffer) NYAAN_UNLIK
			throw Error::Exception(Error::Code::drag_and_drop_alredy_begined);

		auto& buf = m_drop_buffer.emplace();
		buf.atom = atom;
		buf.source = source;
		buf.xdnd_version = xdnd_version;

		signal_send<Slots::drop_begin>(*this, DropDataType::text);
	}

	void Window::handle_dnd_cancel()
	{
		m_drop_buffer = nullopt;
		signal_send<Slots::drop_cancel>(*this);
	}

	void Window::handle_dnd_done_uri(const std::vector<Str>& uris)
	{
		m_drop_buffer = nullopt;
		signal_send<Slots::drop_uri>(*this, uris);
		signal_send<Slots::drop_done>(*this);
	}

	void Window::handle_dnd_done_text(const Str& text)
	{
		m_drop_buffer = nullopt;
		signal_send<Slots::drop_text>(*this, text);
		signal_send<Slots::drop_done>(*this);
	}

	auto Window::drop_data() const -> Opt<DropBuffer>
		{ return m_drop_buffer; }

	void Window::handle_record_new_string(Str&& text)
	{
		auto l = unique_lock();
		auto& [str, sizes, over] = p_input.keyboard.recorded;

		str += text;
		sizes.push_back(text.size());
	}

	void Window::handle_record_newline()
	{
		auto l = unique_lock();
		auto& [str, sizes, over] = p_input.keyboard.recorded;

		str += '\n';
		sizes.push_back(1);
	}

	void Window::handle_record_pop()
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
