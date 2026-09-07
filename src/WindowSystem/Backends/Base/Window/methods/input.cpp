module;

#include <Core/Defines.hpp>

#include <utility>

export module Nyaanwork.WindowSystem.Backends.Base:Window.methods.input;
import :Window.decl;

import Nyaanwork.Core.Types;

export namespace Nyaanwork::WindowSystem::Backends::Base
{
	InputState Window::input_state()
	{
		auto l = unique_lock();
		return std::move(p_input);
	}

	bool Window::key_state(Codes::Key key) const
	{
		auto l = shared_lock();
		return p_input.keyboard.state[key];
	}

	vec2<f32> Window::mouse_position() const
	{
		auto l = shared_lock();
		return p_input.mouse.position;
	}

	vec2<f32> Window::mouse_motion()
	{
		vec2<f32> motion;

		auto l = unique_lock();
		return std::exchange(p_input.mouse.motion, vec2<f32>(0));
	}

	vec2<f32> Window::mouse_wheel()
	{
		vec2<f32> wheel;

		{
			auto l = shared_lock();
			wheel = p_input.mouse.wheel;
		}

		if (auto l = try_unique_lock())
			p_input.mouse.wheel = {};

		return wheel;
	}

	bool Window::button_state(Codes::Button button) const
	{
		auto l = shared_lock();
		return p_input.mouse.state[button];
	}

	bool Window::tablet_on_surface() const
	{
		auto l = shared_lock();
		return p_input.tablet.on_surface;
	}

	f32 Window::tablet_pressure() const
	{
		auto l = shared_lock();
		return p_input.tablet.pressure;
	}

	vec2<f32> Window::tablet_tilt_degrees() const
	{
		auto l = shared_lock();
		return p_input.tablet.tilt.degrees;
	}

	vec2<f32> Window::tablet_tilt_radians() const
	{
		auto l = shared_lock();
		return p_input.tablet.tilt.radians;
	}

	bool Window::string_recording() const
	{
		auto l = shared_lock();
		return string_recording_no_lock();
	}

	void Window::string_recording(bool mode)
	{
		if (mode == string_recording()) [[unlikely]]
			return;

		auto l = unique_lock();

		if (mode)
			p_state |= State::string_recording;
		else
			p_state &= ~State::string_recording;

		auto& [str, sizes, over] = p_input.keyboard.recorded;
		str.clear();
		sizes.clear();
		over = 0;
	}

	auto Window::recorded_string() -> RecordedString
	{
		auto l = unique_lock();
		return std::move(p_input.keyboard.recorded);
	}

	bool Window::string_recording_no_lock() NYAAN_CNOEX
		{ return p_state & State::string_recording; }
}
