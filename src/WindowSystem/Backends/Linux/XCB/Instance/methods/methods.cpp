module;

#include <Core/Defines.hpp>
#include <Nyaanwork/WindowSystem.hpp>

#include <unordered_map>
#include <vector>

export module Nyaanwork.WindowSystem.Backends.Linux.XCB:Instance.methods.methods;
import :Bind.XCB;
import :Exception;
import :Instance.decl;
import :Window.decl;

import Nyaanwork.Core.Math;
import Nyaanwork.Core.Types;
import Nyaanwork.WindowSystem.Backends.Base;
import Nyaanwork.WindowSystem.Backends.Linux.CursorName;

#ifdef NYAANWORK_WINDOW_SYSTEM_OPENGL
import Nyaanwork.WindowSystem.OpenGL;
#endif

namespace Nyaanwork::WindowSystem::Backends::Linux::XCB::InstanceBlocks
{
	bool Input::add_if_tablet(::XCB::Connection* c, const Atoms& atoms,
							  ::XCB::Input::DeviceID device_id) noexcept
	{
		using namespace ::XCB::Input;
		using namespace ::XCB::Input::Request;

		if (tablets.contains(device_id)) NYAAN_UNLIK
			return false;

		auto device = QueryDevice::request_reply(c, device_id);

		bool is_tablet = false;
		Tablet tablet;

		auto info = *device.infos().begin();
		::XCB::Input::DeviceClassData data;
		for (auto dclass : info.classes())
		{
			using enum ::XCB::Input::ValuatorMode;
			using enum ::XCB::Input::DeviceClassType;

			if (dclass->type == DeviceClassType::valuator)
			{
				dclass.unpack(data);

				auto& [num, label, min, max, value, res, mode, _] = data.valuator;

				static constexpr auto val_mode = ValuatorMode::absolute;
				if (mode != val_mode || !(num < 8)) NYAAN_UNLIK
					continue;

				using Bind = Tablet::Bind;
				const auto& labels = atoms.input.stylus_labels;
				Bind bind = Bind::none;

				if (label == labels.position.x)
				{
					bind = Bind::position_x;
				}
				else if (label == labels.position.y)
				{
					bind = Bind::position_y;
				}
				else if (label == labels.pressure)
				{
					is_tablet = true;
					tablet.fp3232.pressure_max = max;
					tablet.pressure_max = f64_from_fp3232(max);
					bind = Bind::pressure;
				}
				else if (label == labels.tilt.x)
				{
					tablet.fp3232.tilt.max.x = max;
					tablet.fp3232.tilt.min.x = min;
					tablet.tilt.max.x = f64_from_fp3232(max);
					tablet.tilt.min.x = f64_from_fp3232(min);
					bind = Bind::tilt_x;
				}
				else if (label == labels.tilt.y)
				{
					tablet.fp3232.tilt.max.y = max;
					tablet.fp3232.tilt.min.y = min;
					tablet.tilt.max.y = f64_from_fp3232(max);
					tablet.tilt.min.y = f64_from_fp3232(min);
					bind = Bind::tilt_y;
				}

				tablet.binds[num] = bind;
			}
		}

		if (is_tablet)
			tablets.emplace(device_id, tablet);

		return is_tablet;
	}

	bool Input::is_any_tablet_pen_on_surface() NYAAN_CNOEX
	{
		bool res = false;
		for (const auto& [_, tablet] : tablets)
			res = tablet.on_surface || res;
		return res;
	}

	void XKB::create_keymap_state(::XCB::Connection* c)
	{
		keymap = ::XCB::XKB::create_keymap(context, c, core_keyboard);
		state = ::XCB::XKB::create_state(keymap, c, core_keyboard);
		layout = state.current_layout(keymap);
	}

	::XCB::Cursor::Cursor Cursor::load_or_get(Base::Window::Cursor cursor) noexcept
	{
		auto it = cursors.find(cursor);
		if (it != cursors.end())
			return it->second;

		return cursors[cursor] = context.load(css_xcursor_name(cursor));
	}

	void Cursor::clear(::XCB::Connection* c) noexcept
	{
		for (auto& [_, cursor] : cursors)
			context.destory(c, cursor);
	}
}

export namespace Nyaanwork::WindowSystem::Backends::Linux::XCB
{
	Ptr<Base::Window> Instance::create_window(const Base::Window::Info& info)
		{ return create_native_window(info); }

	Ptr<Window> Instance::create_native_window(const Base::Window::Info& info)
		{ return make_shared<Window>(shared_from_this(), info); }

	Ptr<Base::Window> Instance::focused_window()
		{ return focused_native_window(); }

	Ptr<const Base::Window> Instance::focused_window() const
		{ return focused_native_window(); }

	Ptr<Window> Instance::focused_native_window()
	{
		auto l = lock();
		if (m_windows.focused.keyboard)
			return m_windows.focused.keyboard->shared_from_this();
		return nullptr;
	}

	Ptr<const Window> Instance::focused_native_window() const
	{
		auto l = lock();
		if (m_windows.focused.keyboard)
			return m_windows.focused.keyboard->shared_from_this();
		return nullptr;
	}

	Ptr<Base::Window> Instance::mouse_focused_window()
		{ return mouse_focused_native_window(); }

	Ptr<const Base::Window> Instance::mouse_focused_window() const
		{ return mouse_focused_native_window(); }

	Ptr<Window> Instance::mouse_focused_native_window()
	{
		auto l = lock();
		if (m_windows.focused.mouse)
			return m_windows.focused.mouse->shared_from_this();
		return nullptr;
	}

	Ptr<const Window> Instance::mouse_focused_native_window() const
	{
		auto l = lock();
		if (m_windows.focused.mouse)
			return m_windows.focused.mouse->shared_from_this();
		return nullptr;
	}

	Str Instance::keyboard_layout() const
	{
		auto l = lock();
		return m_xkb.keymap.layout_name(m_xkb.layout);
	}

	auto Instance::displays() const -> Displays
	{
		auto l = lock();

		if (m_thread.displays_changed)
		{
			auto [c, s] = xdata();
			auto& root = s->root;

			auto& [displays, primary_i] = m_displays;
			displays.clear();

			using namespace ::XCB::RandR;
			using namespace Request;

			auto resources = GetScreenResources::Package(c, root);
			auto primary_package = GetOutputPrimary::Package(c, root);

			auto& time = resources.get()->timestamp;
			auto modes = resources.get().modes();
			auto outputs = resources.get().outputs();

			usize primary = primary_package.get()->output;

			for (usize i = 0; i < outputs.size(); ++i)
			{
				auto output = GetOutputInfo::request_reply(c, outputs[i], time);

				if (!output.valid())
					continue;

				auto crtc_package = GetCRTCInfo::Package(c, output->crtc, time);

				auto& display = displays.emplace_back();

				if (xwayland()? primary == i : primary == outputs[i])
					primary_i = displays.size()-1;

				display.name = output.name();

				auto& crtc = crtc_package.get();

				display.size.mm = {output->mm_width, output->mm_height};
				display.size.inch =
					length(vec2<f128>(display.size.mm)) * 0.03937007874;

				display.position = {crtc->x, crtc->y};
				display.resolution = {crtc->width, crtc->height};

				const auto& mode = modes[
					std::ranges::find(modes, crtc->mode, &ModeInfo::id) - modes.begin()
				];

				f64 dot_clock = mode.dot_clock;
				u64 total = mode.htotal * mode.vtotal;
				f64 total_f64 = total;

				display.refresh_rate = total? round(dot_clock / total_f64) : 0.0;
			}
		}

		return m_displays;
	}

	::XCB::Window::ID Instance::register_window_begin()
	{
		m_thread.mutex.lock();
		return generate_id();
	}

	void Instance::register_window_end(Window& window)
	{
		m_windows.all[window.xwindow()] = &window;
		m_thread.no_windows = m_windows.all.empty();
		m_thread.mutex.unlock();
	}

	void Instance::unregister_window(Window& window)
	{
		auto l = lock();
		m_windows.all.erase(window.xwindow());
		m_thread.no_windows = m_windows.all.empty();

		auto clean_focused = [window=&window](auto& focused)
		{
			if (focused == window)
				focused = nullptr;
		};

		clean_focused(m_windows.focused.keyboard);
		clean_focused(m_windows.focused.mouse);
	}

	::XCB::Cursor::Cursor Instance::load_cursor(Base::Window::Cursor cursor)
	{
		auto l = lock();
		return m_cursor.load_or_get(cursor);
	}
}
