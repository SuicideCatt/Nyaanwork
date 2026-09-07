module;

#include <Core/Defines.hpp>
#include <Nyaanwork/Asset.hpp>

#include <utility>

export module Nyaanwork.WindowSystem.Backends.Base:Window.methods.methods;
import :Instance;
import :Window.decl;

import Nyaanwork.Core.Math;
import Nyaanwork.Core.Types;

#ifdef NYAANWORK_ASSET_IMAGE
import Nyaanwork.Asset.Image;
#endif

export namespace Nyaanwork::WindowSystem::Backends::Base
{
	Str Window::title() const
	{
		auto l = shared_lock();
		return p_title;
	}

	void Window::title(Str title)
	{
		auto l = unique_lock();
		title_impl(p_title = std::move(title));
	}

	#ifdef NYAANWORK_ASSET_IMAGE
	Asset::Image<Asset::Pixels::RGBA> Window::icon() const
	{
		auto l = shared_lock();
		return p_icon;
	}

	void Window::icon(Asset::Image<Asset::Pixels::RGBA> icon)
	{
		auto l = unique_lock();
		icon_impl(p_icon = std::move(icon));
	}
	#endif

	auto Window::position() const -> Position
	{
		auto l = shared_lock();
		return p_position;
	}

	void Window::position(Position position)
	{
		{
			auto l = unique_lock();
			position_impl(p_position = position);
		}

		signal_send<Slots::moved>(*this, position);
	}

	auto Window::resolution() const -> Resolution
	{
		auto l = shared_lock();
		return p_resolution.value;
	}

	void Window::resolution(Resolution resolution)
	{
		if (this->resolution() == resolution) NYAAN_UNLIK
			return;

		{
			auto l = unique_lock();

			resolution = clamp(resolution, p_resolution.min, p_resolution.max);
			resolution_impl(p_resolution.value = resolution);
		}

		signal_send<Slots::resized>(*this, resolution);
	}

	auto Window::min_resolution() const -> Resolution
	{
		auto l = shared_lock();
		return p_resolution.min;
	}

	void Window::min_resolution(Resolution resolution)
	{
		if (this->min_resolution() == resolution) NYAAN_UNLIK
			return;

		static constexpr Resolution supermin(1);
		resolution = max(supermin, resolution);

		bool change_curr = false;
		{
			auto l = unique_lock();

			resolution = min(p_resolution.max, resolution);
			p_resolution.min = resolution;

			auto curr_resolution = clamp(p_resolution.value,
									 p_resolution.min, p_resolution.max);
			if ((change_curr = curr_resolution != p_resolution.value)) NYAAN_UNLIK
				resolution_impl(curr_resolution);

			min_resolution_impl(p_resolution.min);
		}

		if (change_curr) NYAAN_UNLIK
			signal_send<Slots::resized>(*this, resolution);
	}

	auto Window::max_resolution() const -> Resolution
	{
		auto l = shared_lock();
		return p_resolution.max;
	}

	void Window::max_resolution(Resolution resolution)
	{
		if (this->max_resolution() == resolution) NYAAN_UNLIK
			return;

		bool change_curr = false;
		{
			auto l = unique_lock();

			resolution = max(p_resolution.min, resolution);
			p_resolution.max = resolution;

			auto curr_resolution = clamp(p_resolution.value,
										 p_resolution.min, p_resolution.max);
			if ((change_curr = curr_resolution != p_resolution.value)) NYAAN_UNLIK
				resolution_impl(curr_resolution);

			max_resolution_impl(p_resolution.max);
		}

		if (change_curr) NYAAN_UNLIK
			signal_send<Slots::resized>(*this, resolution);
	}

	bool Window::resizable() const
	{
		auto l = shared_lock();
		return resizable_no_lock();
	}

	void Window::resizable(bool mode)
	{
		if (mode == resizable())
			return;

		auto l = unique_lock();

		if (mode)
			p_state |= State::resizable;
		else
			p_state &= ~State::resizable;

		resizable_impl(mode);
	}

	bool Window::fullscreen() const
	{
		auto l = shared_lock();
		return fullscreen_no_lock();
	}

	void Window::fullscreen(bool mode)
	{
		if (mode == fullscreen())
			return;

		auto l = unique_lock();

		if (mode)
			p_state |= State::fullscreen;
		else
			p_state &= ~State::fullscreen;

		fullscreen_impl(mode);
	}

	bool Window::borderless() const
	{
		auto l = shared_lock();
		return borderless_no_lock();
	}

	void Window::borderless(bool mode)
	{
		if (mode == borderless())
			return;

		auto l = unique_lock();

		if (mode)
			p_state |= State::borderless;
		else
			p_state &= ~State::borderless;

		borderless_impl(mode);
	}

	auto Window::cursor() const -> Window::Cursor
	{
		auto l = shared_lock();
		return p_cursor;
	}

	void Window::cursor(Cursor cur)
	{
		if (cursor() == cur)
			return;

		auto l = unique_lock();
		cursor_impl(p_cursor = cur);
	}

	bool Window::show_mouse() const
	{
		auto l = shared_lock();
		return show_mouse_no_lock();
	}

	void Window::show_mouse(bool mode)
	{
		if (mode == show_mouse())
			return;

		auto l = unique_lock();

		if (mode)
			p_state |= State::mouse_show;
		else
			p_state &= ~State::mouse_show;

		show_mouse_impl(mode);
	}

	bool Window::grab_mouse() const
	{
		auto l = shared_lock();
		return grab_mouse_no_lock();
	}

	void Window::grab_mouse(bool mode)
	{
		if (mode == grab_mouse())
			return;

		auto l = unique_lock();

		if (mode)
			p_state |= State::mouse_grab;
		else
			p_state &= ~State::mouse_grab;

		grab_mouse_impl(mode);
	}

	bool Window::centralize_mouse() const
	{
		auto l = shared_lock();
		return centralize_mouse_no_lock();
	}

	void Window::centralize_mouse(bool mode)
	{
		if (mode == centralize_mouse())
			return;

		auto l = unique_lock();

		if (mode)
		{
			p_state |= State::mouse_centralize;

			if (mouse_focused_no_lock())
				warp_mouse_impl(p_resolution.value/2_u16);
		}
		else
			p_state &= ~State::mouse_centralize;

		// centralize_mouse_impl(mode);
	}

	bool Window::capture_mouse() const
	{
		auto l = shared_lock();
		return capture_mouse_no_lock();
	}

	void Window::capture_mouse(bool mode)
	{
		if (mode == capture_mouse())
			return;

		auto l = unique_lock();

		bool old_show = show_mouse_no_lock();
		bool old_grab = grab_mouse_no_lock();

		if (mode)
		{
			p_state |= State::mouse_grab | State::mouse_centralize;
			p_state &= ~State::mouse_show;
		}
		else
		{
			p_state &= ~(State::mouse_grab | State::mouse_centralize);
			p_state |= State::mouse_show;
		}

		if (mode != old_grab)
			grab_mouse_impl(mode);

		if (mode == old_show)
			show_mouse_impl(!mode);

		if (mouse_focused_no_lock() && mode)
			warp_mouse_impl(p_resolution.value/2_u16);
		// if (mode != show_mouse_no_lock())
		// 	centralize_mouse_impl(mode);
	}

	void Window::warp_mouse(Position position)
	{
		position = clamp(vec2<i32>(position), {0, 0}, vec2<i32>(resolution()));

		auto l = unique_lock();
		p_input.mouse.position = position;
		warp_mouse_impl(position);
	}

	bool Window::keyboard_focused() const
	{
		auto l = shared_lock();
		return keyboard_focused_no_lock();
	}

	bool Window::mouse_focused() const
	{
		auto l = shared_lock();
		return mouse_focused_no_lock();
	}

	bool Window::minimized() const
	{
		auto l = shared_lock();
		return minimized_no_lock();
	}

	bool Window::should_close() const
	{
		auto l = shared_lock();
		return should_close_no_lock();
	}

	void Window::cancel_close()
	{
		auto l = unique_lock();
		p_state &= ~State::should_close;
	}

	void Window::close()
	{
		{
			auto l = unique_lock();
			p_state |= State::should_close;
		}

		signal_send<Slots::should_close>(*this);
	}

	usize Window::display() const
	{
		auto l = shared_lock();
		return display_impl();
	}

	bool Window::should_close_no_lock() NYAAN_CNOEX
		{ return p_state & State::should_close; }

	bool Window::resizable_no_lock() NYAAN_CNOEX
		{ return p_state & State::resizable; }

	bool Window::fullscreen_no_lock() NYAAN_CNOEX
		{ return p_state & State::fullscreen; }

	bool Window::borderless_no_lock() NYAAN_CNOEX
		{ return p_state & State::borderless; }

	bool Window::keyboard_focused_no_lock() NYAAN_CNOEX
		{ return p_state & State::focus_keyboard; }

	bool Window::mouse_focused_no_lock() NYAAN_CNOEX
		{ return p_state & State::focus_mouse; }

	bool Window::minimized_no_lock() NYAAN_CNOEX
		{ return p_state & State::minimized; }

	bool Window::show_mouse_no_lock() NYAAN_CNOEX
		{ return p_state & State::mouse_show; }

	bool Window::grab_mouse_no_lock() NYAAN_CNOEX
		{ return p_state & State::mouse_grab; }

	bool Window::centralize_mouse_no_lock() NYAAN_CNOEX
		{ return p_state & State::mouse_centralize; }

	bool Window::capture_mouse_no_lock() NYAAN_CNOEX
	{
		return !show_mouse_no_lock() && grab_mouse_no_lock()
				&& centralize_mouse_no_lock();
	}

	usize Window::display_impl() const
	{
		const auto displays = instance()->displays().displays;

		for (usize i = 0; i < displays.size(); ++i)
		{
			auto& display = displays[i];

			vec2<i32> s = display.position;
			auto e = vec2<i32>(display.position) + vec2<i32>(display.resolution);

			auto pos = vec2<i32>(p_position) + vec2<i32>(p_resolution.value/2_u16);

			auto in_range = [](auto x, auto min, auto max)
				{ return min <= x && x <= max; };
			auto in_range_vec = [&in_range](auto x, auto min, auto max)
			{
				return in_range(x.x, min.x, max.x) && in_range(x.y, min.y, max.y);
			};

			if (in_range_vec(pos, s, e))
				return i;
		}

		return 0;
	}
}
