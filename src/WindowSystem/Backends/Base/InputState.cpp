export module Nyaanwork.WindowSystem.Backends.Base:InputState;
import :Window.InputState;
import :decl;

import Nyaanwork.Core.Types;
import Nyaanwork.WindowSystem.Codes;

export namespace Nyaanwork::WindowSystem::Backends::Base
{
	class InputState final
	{
	public:
		using RecordedString = WindowInputState::Keyboard::RecordedString;

		InputState() = default;

		InputState(WindowInputState state)
			: m_state(state) {}

		InputState(const InputState&) = default;
		InputState& operator=(const InputState&) = default;

		InputState(InputState&&) = default;
		InputState& operator=(InputState&&) = default;

		bool key(Codes::Key key) const
			{ return m_state.keyboard.state[key]; }

		bool button(Codes::Button button) const
			{ return m_state.mouse.state[button]; }

		vec2<f32> position() const
			{ return m_state.mouse.position; }

		vec2<f32> motion() const
			{ return m_state.mouse.motion; }

		vec2<f32> wheel() const
			{ return m_state.mouse.wheel; }

		bool is_pen_on_surface() const
			{ return m_state.tablet.on_surface; }

		f32 pen_pressure() const
			{ return m_state.tablet.pressure; }

		vec2<f32> pen_tilt_degrees() const
			{ return m_state.tablet.tilt.degrees; }

		vec2<f32> pen_tilt_radians() const
			{ return m_state.tablet.tilt.radians; }

		RecordedString string() const
			{ return m_state.keyboard.recorded; }

	private:
		WindowInputState m_state;
	};
}
