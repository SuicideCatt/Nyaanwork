module;

#include <Core/Defines.hpp>

#include <shellscalingapi.h>
#include <windows.h>

export module Nyaanwork.WindowSystem.Backends.Windows:Instance.methods.methods;
import :Instance.decl;
import :Window.decl;

import Nyaanwork.Core.Math;
import Nyaanwork.Core.StringConvertor;
import Nyaanwork.Core.Types;
import Nyaanwork.WindowSystem.Backends.Base;

namespace Nyaanwork::WindowSystem::Backends::Windows
{
	static BOOL CALLBACK
		add_displays_callback(HMONITOR hmonitor, HDC hdc,
							  LPRECT rect, LPARAM user_data)
	{
		auto& [displays, primary_i] =
			*reinterpret_cast<Instance::Displays*>(user_data);

		MONITORINFOEXW info = {};
		info.cbSize = sizeof(info);

		if (GetMonitorInfoW(hmonitor, &info))
		{
			if (info.dwFlags & MONITORINFOF_PRIMARY)
				primary_i = displays.size();

			auto& display = displays.emplace_back();

			// FIXME: find better way
			display.name = StringConvertor_WChar().convert(info.szDevice);

			display.position = {info.rcMonitor.left, info.rcMonitor.top};
			display.resolution = {
				info.rcMonitor.right - info.rcMonitor.left,
				info.rcMonitor.bottom - info.rcMonitor.top
			};

			{
				DEVMODEW dev_mode = {};
				auto res = EnumDisplaySettingsW(info.szDevice, 1, &dev_mode);
				display.refresh_rate = res? dev_mode.dmDisplayFrequency : 60;
			}

			vec2<UINT> udpi = {};
			GetDpiForMonitor(hmonitor, MDT_RAW_DPI, &udpi.x, &udpi.y);
			vec2<f128> dpi = udpi;

			auto inch = vec2<f128>(display.resolution) / dpi;

			display.size.mm = inch * 25.4_f128;
			display.size.inch = length(inch);
		}

		return true;
	}
}

export namespace Nyaanwork::WindowSystem::Backends::Windows
{
	Ptr<Base::Window> Instance::create_window(const Base::Window::Info& info)
		{ return create_native_window(info); }

	Ptr<Window> Instance::create_native_window(const Base::Window::Info& info)
		{ return std::make_shared<Window>(shared_from_this(), info); }

	Ptr<Base::Window> Instance::focused_window()
		{ return focused_native_window(); }

	Ptr<const Base::Window> Instance::focused_window() const
		{ return focused_native_window(); }

	Ptr<Window> Instance::focused_native_window()
	{
		auto l = lock();
		if (m_windows.keyboard)
			return m_windows.keyboard->shared_from_this();
		return nullptr;
	}

	Ptr<const Window> Instance::focused_native_window() const
	{
		auto l = lock();
		if (m_windows.keyboard)
			return m_windows.keyboard->shared_from_this();
		return nullptr;
	}

	Ptr<Base::Window> Instance::mouse_focused_window()
		{ return mouse_focused_native_window(); }

	Ptr<const Base::Window> Instance::mouse_focused_window() const
		{ return mouse_focused_native_window(); }

	Ptr<Window> Instance::mouse_focused_native_window()
	{
		auto l = lock();
		if (m_windows.mouse)
			return m_windows.mouse->shared_from_this();
		return nullptr;
	}

	Ptr<const Window> Instance::mouse_focused_native_window() const
	{
		auto l = lock();
		if (m_windows.mouse)
			return m_windows.mouse->shared_from_this();
		return nullptr;
	}

	auto Instance::displays() const -> Displays
	{
		auto l = lock();

		if (m_displays_changed)
		{
			m_displays.displays.clear();
			EnumDisplayMonitors(nullptr, nullptr, &add_displays_callback,
								reinterpret_cast<LPARAM>(&m_displays));
		}

		return m_displays;
	}

	HCURSOR Instance::load_cursor(Base::Window::Cursor cursor)
	{
		auto l = lock();

		auto itr = m_cursors.find(cursor);
		if (itr != m_cursors.end())
			return itr->second.get();

		using enum Base::Window::Cursor;
		decltype(IDC_ARROW) windows_cursor;
		switch (cursor)
		{
		case progress:
			windows_cursor = IDC_APPSTARTING;
			break;

		case wait:
			windows_cursor = IDC_WAIT;
			break;

		case text:
		case vertical_text:
			windows_cursor = IDC_IBEAM;
			break;

		case crosshair:
		case cell:
			windows_cursor = IDC_CROSS;
			break;

		case copy:
		case alias:
		case grab:
		case grabbing:
		case context_menu:
		case pointer:
		case zoom_in:
		case zoom_out:
			windows_cursor = IDC_HAND;
			break;

		case no_drop:
		case not_allowed:
			windows_cursor = IDC_NO;
			break;

		case help:
			windows_cursor = IDC_HELP;
			break;

		case move:
		case all_scroll:
			windows_cursor = IDC_SIZEALL;
			break;

		case trdl_resize:
		case tr_resize:
		case dl_resize:
			windows_cursor = IDC_SIZENESW;
			break;

		case tldr_resize:
		case tl_resize:
		case dr_resize:
			windows_cursor = IDC_SIZENWSE;
			break;

		case lr_resize:
		case l_resize:
		case r_resize:
		case col_resize:
			windows_cursor = IDC_SIZEWE;
			break;

		case td_resize:
		case t_resize:
		case d_resize:
		case row_resize:
			windows_cursor = IDC_SIZENS;
			break;

		default:
			windows_cursor = IDC_ARROW;
		};

		auto& ptr = m_cursors[cursor];
		ptr.reset(LoadCursor(nullptr, windows_cursor));
		return ptr.get();
	}
}
