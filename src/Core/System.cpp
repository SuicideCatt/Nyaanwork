module;

#include <Core/Defines.hpp>

#ifdef __linux__
#include <limits.h>
#include <unistd.h>
#elif defined(_WIN64)
#include <windows.h>
#include <psapi.h>
#endif

#include <filesystem>
#include <utility>

export module Nyaanwork.Core.System;

import Nyaanwork.Core.Types;

export namespace Nyaanwork::System
{
	Str decode_uri(StrV uri)
	{
		Str res(uri.size(), '\0');

		char buf = '\0';
		size_t i = 0;
		for (auto c : uri)
		{
			if (c == '%' && i == 0)
			{
				++i;
				continue;
			}
			else if (i > 0 && i <= 2)
			{
				if (c >= 'A' && c <= 'F')
					c += 10 - 'A';
				else if (c >= 'a' && c <= 'f')
					c += 10 - 'a';
				else if (c >= '0' && c <= '9')
					c += 0 - '0';

				buf |= c << ((2 - i) * 4);

				++i;

				if (i == 3)
				{
					c = std::exchange(buf, '\0');
					i = 0;
				}
				else
				{
					continue;
				}
			}

			res += c;
		}

		return res;
	}

	std::filesystem::path executable_path() noexcept
	{
		try
		{
			#ifdef __linux__
			return std::filesystem::read_symlink("/proc/self/exe");
			#elif defined(_WIN64)
			HINSTANCE modules[1];
			DWORD size;
			if (!EnumProcessModules(GetCurrentProcess(), modules, sizeof(modules), &size))
				modules[0] = nullptr;

			std::wstring result(MAX_PATH, '\0');
			auto len = GetModuleFileNameW(modules[0], result.data(), result.size());
			if (len == 0 || len == MAX_PATH) NYAAN_UNLIK
				return "";
			result.resize(len);
			return std::move(result);
			#endif
		}
		catch(const std::exception&) {}

		return "";
	}
}
