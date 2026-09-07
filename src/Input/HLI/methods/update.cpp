module;

#include <Core/Defines.hpp>

#include <variant>

export module Nyaanwork.Input.HLI:methods.update;
import :decl;

import Nyaanwork.Core.Math;
import Nyaanwork.Core.Types;

namespace Nyaanwork::Input
{
	static void update_action(bool state, HLI::Action& action)
	{
		auto& [p, h, r] = action;

		p = state == true && state != h;
		r = state == false && state != h;
		h = state;
	}

	template<typename>
	struct GetAxisLim;

	template<is_arithmetic _Type>
	struct GetAxisLim<_Type>
	{
		using Lim = limits<_Type>;
	};

	template<usize i, is_arithmetic _Type>
	struct GetAxisLim<vec<i, _Type>>
	{
		using Lim = GetAxisLim<_Type>::Lim;
	};
}

export namespace Nyaanwork::Input
{
	void HLI::update(Ptr<WindowSystem::Window> window)
	{
		update_window_input(window);
		update_action_axis();
	}

	void HLI::update_window_input(Ptr<WindowSystem::Window> window)
	{
		if (!window) NYAAN_UNLIK
			return;

		bool keyboard_focused = window->keyboard_focused();
		bool mouse_focused = window->mouse_focused();
		m_input_state.window = window->input_state();

		using WSIS = WindowSystem::InputState;
		auto upd = [](auto& states, auto& input, auto&& proj)
		{
			for (auto itr = states.begin(), last = states.end(); itr != last;)
			{
				auto& [kb, state] = *itr;

				--state.count;
				if (state.count == 0) NYAAN_UNLIK
				{
					itr = states.erase(itr);
					continue;
				}
				else
				{
					++itr;
				}

				update_action((input.*proj)(kb), state.action);
			}
		};

		if (m_input_state.window_keyboard_focused || keyboard_focused)
			upd(m_state.keys, m_input_state.window, &WSIS::key);
		if (m_input_state.window_mouse_focused || mouse_focused)
			upd(m_state.buttons, m_input_state.window, &WSIS::button);

		m_input_state.window_keyboard_focused = keyboard_focused;
		m_input_state.window_mouse_focused = mouse_focused;
	}

	void HLI::update_action_axis()
	{
		for (auto& [name, action] : m_actions)
		{
			bool state = false;
			for (usize i = 0; i < action.combinations.size() && !state; ++i)
			{
				auto& [combination, size] = action.combinations[i];

				bool combination_state = size != 0;
				for (usize i = 0; i < size; ++i)
				{
					std::visit([this, &combination_state](const auto& code)
					{
						combination_state = combination_state && binary_state(code);
					}, combination[i]);
				}

				state = state || combination_state;
			}

			update_action(state, action.data);
		}

		auto upd_axis = [this](auto& cont)
		{

			for (auto& [name, axis] : cont)
			{
				using Data = decltype(cont[""].data);
				using Lim = GetAxisLim<Data>::Lim;

				Data value = Data();
				for (auto& [combination, min, max, size] : axis.combinations)
				{
					Data combination_value = Data();

					for (usize i = 0; i < size; ++i)
					{
						auto& [value, code] = combination[i];

						std::visit([this, &combination_value, value](const auto& code)
						{
							if (binary_state(code))
								combination_value += value;
						}, code);
					}

					value += clamp(combination_value, min, max);
				}

				axis.data = clamp(value, axis.min, axis.max);
			}
		};

		upd_axis(m_axis.d1);
		upd_axis(m_axis.d2);
	}
}
