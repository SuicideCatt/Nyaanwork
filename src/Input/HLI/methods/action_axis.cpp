module;

#include <Core/Defines.hpp>

#include <concepts>
#include <format>
#include <source_location>

export module Nyaanwork.Input.HLI:methods.action_axis;
import :Exception;
import :decl;

import Nyaanwork.Core.Utils.ClearType;

export namespace Nyaanwork::Input
{
	template<std::same_as<Input::Action>... Binds>
	requires (sizeof...(Binds) <= max_combinations_count)
	void HLI::action_add(const Str& name, Binds... binds)
	{
		action_axis_add(m_actions, name, std::source_location::current(),
						binds...);
	}

	void HLI::action_remove(const Str& name) noexcept
		{ m_actions.erase(name); }

	Input::Action HLI::action_slot(const Str& name, usize slot)
		{ return action_axis_slot(m_actions, name, slot); }

	void HLI::action_slot(const Str& name, usize slot, Input::Action bind)
		{ action_axis_slot(m_actions, name, slot, bind); }

	auto HLI::action(const Str& name) -> Action
		{ return action_axis(m_actions, name); }

	template<is_axis_type<Axis1D::Type, Axis2D::Type> AxisType,
			 std::same_as<Input::Axis<AxisType>>... Binds>
	requires (sizeof...(Binds) <= max_combinations_count)
	void HLI::axis_add(const Str& name, Binds... binds)
	{
		auto add = [this, &name](auto& cont, auto src, const auto&... binds)
		{
			auto& data = action_axis_add(cont, name, src, binds...);

			if constexpr (sizeof...(Binds) > 0)
			{
				data.min = AxisType(limits<Axis1D>::infinity());
				data.max = AxisType(-limits<Axis1D>::infinity());

				for (const auto& [combination, min, max, size] : data.combinations)
				{
					if (size > 0)
					{
						data.min = Math::min(cont.min, min);
						data.max = Math::max(cont.max, max);
					}
				}
			}
		};

		using src_loc = std::source_location;
		if constexpr (std::same_as<AxisType, Axis1D>)
			add(m_axis.d1, src_loc::current(), binds...);
		else if constexpr (std::same_as<AxisType, Axis2D>)
			add(m_axis.d2, src_loc::current(), binds...);
	}

	template<is_axis_type<Axis1D::Type, Axis2D::Type> AxisType>
	void HLI::axis_remove(const Str& name)
	{
		if constexpr (std::same_as<AxisType, Axis1D>)
			m_axis.d1.erase(name);
		else if constexpr (std::same_as<AxisType, Axis2D>)
			m_axis.d2.erase(name);
	}

	template<is_axis_type<Axis1D::Type, Axis2D::Type> AxisType>
	Input::Axis<AxisType> HLI::axis_slot(const Str& name, usize slot)
	{
		if constexpr (std::same_as<AxisType, Axis1D>)
			return action_axis_slot(m_axis.d1, name, slot);
		else if constexpr (std::same_as<AxisType, Axis2D>)
			return action_axis_slot(m_axis.d2, name, slot);
	}

	template<is_axis_type<Axis1D::Type, Axis2D::Type> AxisType>
	void HLI::axis_slot(const Str& name, usize slot,
						  Input::Axis<AxisType> bind)
	{
		using src_loc = std::source_location;
		auto add = [this, &name, &slot, &bind](auto& cont,
											   src_loc src = src_loc::current())
		{
			auto& data = action_axis_slot(cont, name, slot, bind, src);

			data.min = AxisType(limits<Axis1D>::infinity());
			data.max = AxisType(-limits<Axis1D>::infinity());

			for (const auto& [combination, min, max, size] : data.combinations)
			{
				if (size > 0)
				{
					data.min = Math::min(data.min, min);
					data.max = Math::max(data.max, max);
				}
			}
		};

		if constexpr (std::same_as<AxisType, Axis1D>)
			add(m_axis.d1);
		else if constexpr (std::same_as<AxisType, Axis2D>)
			add(m_axis.d2);
	}

	template<is_axis_type<Axis1D::Type, Axis2D::Type> AxisType>
	auto HLI::axis(const Str& name) -> Axis<AxisType>
	{
		if constexpr (std::same_as<AxisType, Axis1D>)
			return action_axis(m_axis.d1, name);
		else if constexpr (std::same_as<AxisType, Axis2D>)
			return action_axis(m_axis.d2, name);
	}

	auto HLI::action_axis_add(auto& cont, const Str& name,
							  std::source_location src, const auto&... binds)
		-> ClearType<decltype(cont[""])>&
	{
		using namespace HLI_Error;
		static constexpr auto err_code =
			std::same_as<ClearType<decltype(cont[""])>, ActionCont>?
				Code::action_already_added : Code::axis_already_added;

		auto [itr, done] = cont.emplace(name, ClearType<decltype(cont[""])>{});
		if (!done) NYAAN_UNLIK
			throw Exception(name, err_code, src);

		auto& data_cont = itr->second;
		data_cont.combinations = {binds...};
		return data_cont;
	}

	auto HLI::action_axis_get(auto& cont, const Str& name, std::source_location src)
		-> ClearType<decltype(cont[""])>&
	{
		using namespace HLI_Error;
		static constexpr auto err_code =
			std::same_as<ClearType<decltype(cont[""])>, ActionCont>?
				Code::action_not_found : Code::axis_not_found;

		auto itr = cont.find(name);
		if (itr == cont.end()) NYAAN_UNLIK
			throw Exception(name, err_code, src);

		return itr->second;
	}

	auto HLI::action_axis_slot(auto& cont, const Str& name, usize slot,
							   std::source_location src)
		-> ClearType<decltype(cont[""].combinations[0])>
	{
		if (!(slot < max_combinations_count)) NYAAN_UNLIK
		{
			throw HLI_Error::Exception(
				std::format("{}[{}]", name, slot),
				HLI_Error::Code::slot_index_not_less_than_max_combinations_count,
				src
			);
		}

		return action_axis_get(cont, name, src).combinations[slot];
	}

	auto HLI::action_axis_slot(auto& cont, const Str& name, usize slot,
							   const auto& bind, std::source_location src)
		-> ClearType<decltype(cont[""])>&
	{
		if (!(slot < max_combinations_count)) NYAAN_UNLIK
		{
			throw HLI_Error::Exception(
				std::format("{}[{}]", name, slot),
				HLI_Error::Code::slot_index_not_less_than_max_combinations_count,
				src
			);
		}

		auto& data_cont = action_axis_get(cont, name, src);
		data_cont.combinations[slot] = bind;
		return data_cont;
	}

	auto HLI::action_axis(auto& cont, const Str& name,
						  std::source_location src) -> decltype(cont[""].data)
		{ return action_axis_get(cont, name, src).data; }
}
