module;

#include <windows.h>

#include <thread>

export module Nyaanwork.WindowSystem.Backends.Windows:Window.methods.init;
import :Exception;
import :Instance.decl;
import :SAFETY;
import :Window.decl;
import :Window.methods.drop;

import Nyaanwork.Core.Math;
import Nyaanwork.Core.Types;

export namespace Nyaanwork::WindowSystem::Backends::Windows
{
	Window::Window(Ptr<Instance> instance, const Info& info)
		: m_instance(instance), m_semaphore(0)
	{
		m_thread = std::thread([this](const Info* ptr_info)
		{
			if (!SUCCEEDED(OleInitialize(nullptr)))
				throw Error::Exception(Error::Code::fail_to_init_Ole);

			auto& i = *m_instance;
			auto& info = *ptr_info;

			{
				auto [min, max, _] = info.bounds.resolution;

				static constexpr vec2<u16> supermin(1);
				min = Math::max(supermin, min);
				max = Math::max(min, max);

				p_resolution.min = min;
				p_resolution.max = max;
			}

			const auto& pos = p_position = info.bounds.position;
			p_resolution.value =
				clamp(info.bounds.resolution.value, p_resolution.min, p_resolution.max);
			const auto& res = p_resolution.value = adjust_resolution(p_resolution.value);

			m_cursor =
				static_cast<InstanceWindowInterface&>(i).load_cursor(Cursor::default_);

			m_handle.reset(CreateWindowExA(0, i.instance_class_name().c_str(),
										   "", WS_OVERLAPPEDWINDOW,
										   p_position.x, p_position.y, res.x, res.y,
										   nullptr, nullptr, i.handle(),
										   nullptr));
			if (!m_handle)
				throw Error::Exception(Error::Code::fail_to_craete_window);

			auto drop = make_unique<DropTarget>(this, handle());
			RegisterDragDrop(handle(), drop.get());

			SetWindowLongPtr(handle(), GWLP_USERDATA, reinterpret_cast<usize>(this));

			auto [raw_mutex, raw_buffer] = SAFETY::get().raw_input_buffer();
			m_raw.mutex = raw_mutex;
			m_raw.buffer = raw_buffer;

			m_style = GetWindowLong(handle(), GWL_STYLE);

			title(info.title);
			resizable(info.resizable);
			borderless(info.borderless);
			fullscreen(info.fullscreen);

			ShowWindow(handle(), SW_SHOW);
			UpdateWindow(handle());

			show_mouse(info.mouse.show);
			grab_mouse(info.mouse.grab);
			centralize_mouse(info.mouse.centralize);

			m_semaphore.release(); // ptr_info now UB
			m_work = true;

			MSG msg = {};
			while (m_work)
			{
				GetMessage(&msg, handle(), 0, 0);
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			m_semaphore.acquire();

			RevokeDragDrop(handle());
			OleUninitialize();
		}, &info);

		m_semaphore.acquire();
	}

	Window::~Window()
	{
		resolution_impl({});
		m_semaphore.release();

		SetWindowLongPtrA(handle(), GWLP_USERDATA, 0);

		m_work = false;
		m_thread.join();
	}
}
