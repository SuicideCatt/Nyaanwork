module;

#include <Core/Defines.hpp>

#include <windows.h>

#include <format>

export module Nyaanwork.WindowSystem.Backends.Windows:Instance.methods.init;
import :Instance.decl;
import :Exception;
import :Window.decl;

import Nyaanwork.Core.Types;
import Nyaanwork.WindowSystem.Backends.Base;

export namespace Nyaanwork::WindowSystem::Backends::Windows
{
	Instance::Instance(Opt<Str> class_name)
		: Base::Instance(class_name),
		  m_window_class(std::format("{}{{{}}}", window_class_name(),
									 SAFETY::get().add_instance(*this))),
		  m_handle(get_executable_module()),
		  m_displays_changed(true)
	{
		WNDCLASSEXA wnd_class = {
			.cbSize = sizeof(WNDCLASSEXA),
			.style = CS_BYTEALIGNCLIENT | CS_OWNDC,
			.lpfnWndProc = &windows_proc,
			.hInstance = handle(),
			.lpszClassName = m_window_class.c_str(),
		};

		if (!RegisterClassExA(&wnd_class)) NYAAN_UNLIK
		{
			throw Error::Exception(m_window_class,
								   Error::Code::fail_to_register_window_class);
		}
	}

	Instance::~Instance()
	{
		UnregisterClassA(m_window_class.c_str(), handle());

		SAFETY::get().remove_instance(*this);
	}
}
