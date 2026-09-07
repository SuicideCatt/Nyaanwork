module;

#include <Core/Defines.hpp>

#include <windows.h>
#include <psapi.h>

#include <algorithm>
#include <atomic>
#include <locale>
#include <mutex>
#include <vector>

export module Nyaanwork.WindowSystem.Backends.Windows:SAFETY;
import :Exception;
import :decl;

import Nyaanwork.Core.Types;

#define FUNC_NAME      Nyaanwork_WindowSystem_Backends_Windows_SAFETY
#define FUNC_NAME_STR "Nyaanwork_WindowSystem_Backends_Windows_SAFETY"

namespace Nyaanwork::WindowSystem::Backends::Windows
{
	export class SAFETY;
}

extern "C" __attribute__((dllexport))
	Nyaanwork::WindowSystem::Backends::Windows::SAFETY* FUNC_NAME();

export namespace Nyaanwork::WindowSystem::Backends::Windows
{
	HINSTANCE get_executable_module()
	{
		HINSTANCE modules[1];
		DWORD size = 1;

		auto res = EnumProcessModules(GetCurrentProcess(),
									  modules, sizeof(modules), &size);
		if (!res) NYAAN_UNLIK
			throw Error::Exception(Error::Code::cannot_enumarate_modules);

		return modules[0];
	}

	class SAFETY final
	{
	public:
		~SAFETY()
		{
			if (m_work)
				deinit();
		}

		static SAFETY& get()
		{
			using F = SAFETY* (*)();
			auto f = GetProcAddress(get_executable_module(), FUNC_NAME_STR);
			if (!f) NYAAN_UNLIK // FIXME: Nyaanwork always may linked to executable
				throw Error::Exception(Error::Code::no_safety_getter);

			return *reinterpret_cast<F>(f)();
		}

		u32 add_instance(Instance& instance)
		{
			u16 id;
			{
				std::scoped_lock l(m_mutex);

				auto itr = std::ranges::find(m_instances, nullptr);
				if (itr == m_instances.end())
				{
					auto size = m_instances.size();
					if (size > limits<u32>::max()) NYAAN_UNLIK
					{
						using namespace Error;
						throw Exception(Code::instance_count_greater_than_u16_max);
					}

					m_instances.emplace_back(&instance);
					id = size;
				}
				else
				{
					*itr = &instance;
					id = itr - m_instances.begin();
				}
			}

			if (!m_work) NYAAN_UNLIK
				init();

			return id;
		}

		void remove_instance(Instance& instance)
		{
			if (!m_work) NYAAN_UNLIK
				throw Error::Exception(Error::Code::safety_not_inited);

			usize instances = 0;
			{
				std::scoped_lock l(m_mutex);

				for (auto& data : m_instances)
				{
					if (data == &instance) NYAAN_UNLIK
						data = nullptr;

					if (data != nullptr)
						++instances;
				}

				if (instances == 0 && m_work)
					deinit();
			}
		}

		Pair<std::mutex*, std::vector<u8>*> raw_input_buffer()
		{
			if (!m_work) NYAAN_UNLIK
				throw Error::Exception(Error::Code::safety_not_inited);

			return {
				&m_mutex,
				&m_raw_input_buffer
			};
		}

	private:
		void init()
		{
			std::scoped_lock l(m_mutex);

			HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
			if (hr == S_FALSE) NYAAN_UNLIK
				throw Error::Exception(Error::Code::fail_to_init_COM);

			if (m_raw_input_buffer.empty())
			{
				// FIXME: In MinGW std::locale not work for en_US.utf8 and others
				std::setlocale(LC_ALL, "en_US.utf8");

				RAWINPUTDEVICE raw_dev[] = {
					{0x1, 0x2, 0, nullptr},
					{0x1, 0x6, 0, nullptr},
				};
				RegisterRawInputDevices(raw_dev, std::size(raw_dev),
										sizeof(RAWINPUTDEVICE));

				m_raw_input_buffer.resize(64 * sizeof(RAWINPUT));
			}

			m_work = true;
		}

		void deinit()
		{
			m_work = false;
			m_instances.clear();

			CoUninitialize();
		}

		std::vector<Instance*> m_instances;

		std::vector<u8> m_raw_input_buffer;

		std::atomic_bool m_work = false;
		std::mutex m_mutex;
	};
}

extern "C" __attribute__((dllexport))
	Nyaanwork::WindowSystem::Backends::Windows::SAFETY* FUNC_NAME()
{
	static Nyaanwork::WindowSystem::Backends::Windows::SAFETY safety;
	return &safety;
}
