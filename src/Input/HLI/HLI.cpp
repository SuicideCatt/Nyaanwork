module;

#include <Core/Defines.hpp>
#include <Nyaanwork/Input.hpp>

#include <array>
#include <source_location>
#include <unordered_map>
#include <variant>

export module Nyaanwork.Input.HLI:decl;

import Nyaanwork.Core.Math;
import Nyaanwork.Core.Types;
import Nyaanwork.Core.Utils.ClearType;
import Nyaanwork.Core.Utils.PtrContainer;
import Nyaanwork.WindowSystem.Codes;
import Nyaanwork.WindowSystem;

export namespace Nyaanwork::Input::Codes
{
	using WindowSystem::Codes::Key;
	using WindowSystem::Codes::Button;

	template<typename C>
	concept is_code = std::same_as<Key, C> || std::same_as<Button, C>;
}

export namespace Nyaanwork::Input
{
	constexpr usize max_combination_size = NYAANWORK_INPUT_COMBINATION_SIZE;
	constexpr usize max_combinations_count = NYAANWORK_INPUT_COMBINATIONS_COUNT;
}

namespace Nyaanwork::Input
{
	template<typename T>
	using Combination = std::array<T, max_combination_size>;

	template<typename T>
	using Combinations = std::array<T, max_combinations_count>;

	template<typename AxisType, typename... AxisTypes>
	concept is_axis_type = (std::same_as<AxisType, AxisTypes> || ...);
}

export namespace Nyaanwork::Input
{
	struct Action
	{
		constexpr Action() = default;

		template<Codes::is_code... Codes>
		requires (sizeof...(Codes) > 0 || sizeof...(Codes) <= max_combination_size)
		constexpr Action(Codes... codes)
			: combination({codes...}), size(sizeof...(Codes)) {}

		constexpr Action(const Action&) = default;
		constexpr Action& operator=(const Action&) = default;

		Combination<std::variant<Codes::Key, Codes::Button>> combination;
		u8 size = 0;
	};

	template<typename _Type>
	struct AxisParam
	{
		using Type = _Type;

		constexpr AxisParam() = default;

		template<Codes::is_code Codes>
		constexpr AxisParam(Codes code, _Type value)
			: value(value), code(code) {}

		constexpr AxisParam(const AxisParam&) = default;
		constexpr AxisParam& operator=(const AxisParam&) = default;

		_Type value;
		std::variant<Codes::Key, Codes::Button> code;
	};

	template<typename _Type>
	struct Axis
	{
		using Param = Input::AxisParam<_Type>;
		using Type = _Type;

		constexpr Axis() = default;

		template<std::same_as<Param>... Params>
		requires (sizeof...(Params) > 0 || sizeof...(Params) <= max_combination_size)
		constexpr Axis(Params... values)
			: combination({values...}), size(sizeof...(Params))
		{
			max = combination[0].value;
			min = combination[0].value;
			for (usize i = 1; i < size; ++i)
			{
				max = Nyaanwork::max(max, combination[i].value);
				min = Nyaanwork::min(min, combination[i].value);
			}
		}

		constexpr Axis(const Axis&) = default;
		constexpr Axis& operator=(const Axis&) = default;

		Combination<Param> combination;
		_Type min = _Type();
		_Type max = _Type();
		u8 size = 0;
	};

	using Axis1D = Axis<f32>;
	using Axis2D = Axis<vec2<f32>>;
	// using Axis3D = Axis<vec3<f32>>;

	class HLI final : public EnableSharedFromThis<HLI>
	{
	public:
		struct Action
		{
			bool press = false, hold = false, release = false;
		};

		template<typename Type>
		using Axis = Input::Axis<Type>::Type;

		using Axis1D = Input::Axis1D::Type;
		using Axis2D = Input::Axis2D::Type;
		// using Axis3D = Input::Axis3D::Type;

		using RecordedString = WindowSystem::InputState::RecordedString;

		HLI() = default;

		bool binary_state(Codes::Key key) const
			{ return m_input_state.window.key(key); }
		bool binary_state(Codes::Button button) const
			{ return m_input_state.window.button(button); }

		Action state(Codes::Key key)
		{
			auto& [a, c] = m_state.keys[key];
			c = 3;
			return a;
		}
		Action state(Codes::Button button)
		{
			auto& [a, c] = m_state.buttons[button];
			c = 3;
			return a;
		}

		vec2<f32> position() const
			{ return m_input_state.window.position(); }
		vec2<f32> motion() const
			{ return m_input_state.window.motion(); }
		vec2<f32> wheel() const
			{ return m_input_state.window.wheel(); }

		bool is_pen_on_surface() const
			{ return m_input_state.window.is_pen_on_surface(); }
		f32 pen_pressure() const
			{ return m_input_state.window.pen_pressure(); }
		vec2<f32> pen_tilt_degrees() const
			{ return m_input_state.window.pen_tilt_degrees(); }
		vec2<f32> pen_tilt_radians() const
			{ return m_input_state.window.pen_tilt_radians(); }

		RecordedString string() const
			{ return m_input_state.window.string(); }

		template<std::same_as<Input::Action>... Binds>
		requires (sizeof...(Binds) <= max_combinations_count)
		void action_add(const Str& name, Binds... binds);
		void action_remove(const Str& name) noexcept;

		Input::Action action_slot(const Str& name, usize slot);
		void action_slot(const Str& name, usize slot, Input::Action bind);

		Action action(const Str& name);

		template<is_axis_type<Axis1D, Axis2D> AxisType,
				 std::same_as<Input::Axis<AxisType>>... Binds>
		requires (sizeof...(Binds) <= max_combinations_count)
		void axis_add(const Str& name, Binds... binds);
		template<is_axis_type<Axis1D, Axis2D> AxisType>
		void axis_remove(const Str& name);

		template<is_axis_type<Axis1D, Axis2D> AxisType>
		Input::Axis<AxisType> axis_slot(const Str& name, usize slot);
		template<is_axis_type<Axis1D, Axis2D> AxisType>
		void axis_slot(const Str& name, usize slot, Input::Axis<AxisType> bind);

		template<is_axis_type<Axis1D, Axis2D> AxisType>
		Axis<AxisType> axis(const Str& name);

		void update(Ptr<WindowSystem::Window> window,
					PtrContainer<WindowSystem::InputState> state = nullptr);

	private:
		struct ActionCont
		{
			Combinations<Input::Action> combinations = {};
			Action data = {};
		};

		template<typename AxisType>
		struct AxisCont
		{
			Combinations<Input::Axis<AxisType>> combinations = {};
			AxisType min = {};
			AxisType max = {};
			AxisType data = {};
		};

		auto action_axis_add(auto& cont, const Str& name, std::source_location src,
							 const auto&... binds) -> ClearType<decltype(cont[""])>&;

		auto action_axis_get(auto& cont, const Str& name, std::source_location src)
			-> ClearType<decltype(cont[""])>&;

		auto action_axis_slot(auto& cont, const Str& name, usize slot,
					std::source_location src = std::source_location::current())
			-> ClearType<decltype(cont[""].combinations[0])>;
		auto action_axis_slot(auto& cont, const Str& name, usize slot,
					const auto& bind,
					std::source_location src = std::source_location::current())
			-> ClearType<decltype(cont[""])>&;

		auto action_axis(auto& cont, const Str& name,
						 std::source_location src = std::source_location::current())
			-> decltype(cont[""].data);

		void update_window_input(Ptr<WindowSystem::Window> window,
								 PtrContainer<WindowSystem::InputState> state);
		void update_action_axis();

		struct
		{
			WindowSystem::InputState window;
			bool window_keyboard_focused = false;
			bool window_mouse_focused = false;
		} m_input_state;

		struct
		{
			struct State
			{
				Action action;
				u8 count = 0;
			};

			std::unordered_map<Codes::Key, State> keys;
			std::unordered_map<Codes::Button, State> buttons;
		} m_state;

		std::unordered_map<Str, ActionCont> m_actions;
		struct {
			std::unordered_map<Str, AxisCont<Axis1D>> d1;
			std::unordered_map<Str, AxisCont<Axis2D>> d2;
			// std::unordered_map<Str, AxisCont<Axis3D>> d3;
		} m_axis;
	};
}
