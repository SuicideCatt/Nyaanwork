module;

#include <mutex>
#include <shared_mutex>

export module Nyaanwork.WindowSystem.Backends.Base:Window.methods.lock;
import :Window.decl;

import Nyaanwork.Core.Types;

export namespace Nyaanwork::WindowSystem::Backends::Base
{
	std::shared_lock<std::shared_mutex> Window::shared_lock() const
		{ return std::shared_lock<std::shared_mutex>{m_mutex}; }

	std::unique_lock<std::shared_mutex> Window::unique_lock()
		{ return std::unique_lock<std::shared_mutex>{m_mutex}; }

	Opt<std::shared_lock<std::shared_mutex>> Window::try_shared_lock() const
	{
		std::shared_lock lock(m_mutex, std::try_to_lock);
		if (lock)
			return std::move(lock);
		else
			return nullopt;
	}

	Opt<std::unique_lock<std::shared_mutex>> Window::try_unique_lock()
	{
		std::unique_lock lock(m_mutex, std::try_to_lock);
		if (lock)
			return std::move(lock);
		else
			return nullopt;
	}
}
