export module Nyaanwork.WindowSystem.Backends.Linux.XCB:Instance.methods.get_window;
import :Bind.XCB;
import :Instance.decl;

export namespace Nyaanwork::WindowSystem::Backends::Linux::XCB
{
	Window* Instance::get_window(::XCB::Window::ID window) const
	{
		auto l = lock();

		auto itr = m_windows.all.find(window);
		if (itr == m_windows.all.end()) [[unlikely]]
			return nullptr;
		return itr->second;
	}

	Window* Instance::get_window_focused_keyboard() const
	{
		auto l = lock();
		return m_windows.focused.keyboard;
	}

	Window* Instance::get_window_focused_mouse() const
	{
		auto l = lock();
		return m_windows.focused.mouse;
	}
}
