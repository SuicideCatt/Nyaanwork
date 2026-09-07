module;

#include <Core/Defines.hpp>
#include <Nyaanwork/Asset.hpp>
#include <Nyaanwork/WindowSystem.hpp>

#include <windows.h>

#ifdef NYAANWORK_WINDOW_SYSTEM_VULKAN
#define VK_USE_PLATFORM_WIN32_KHR
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan.hpp>
#endif

#include <algorithm>
#include <bit>
#include <span>
#include <vector>

export module Nyaanwork.WindowSystem.Backends.Windows:Window.methods.methods;
import :Exception;
import :Instance.decl;
import :Window.decl;

#ifdef NYAANWORK_ASSET_IMAGE
import Nyaanwork.Asset.Image;
#endif

export namespace Nyaanwork::WindowSystem::Backends::Windows
{
	Ptr<Base::Instance> Window::instance() const
		{ return native_instance(); }

	#ifdef NYAANWORK_WINDOW_SYSTEM_VULKAN
	vk::SurfaceKHR Window::create_vulkan_surface(vk::Instance instance,
						vk::Optional<vk::AllocationCallbacks const> allocator)
	{
		vk::Win32SurfaceCreateInfoKHR info = {
			.hinstance = m_instance->handle(),
			.hwnd = handle(),
		};

		return instance.createWin32SurfaceKHR(info, allocator);
	}
	#endif

	#ifdef NYAANWORK_ASSET_IMAGE
	void Window::icon_impl(const Asset::Image<Asset::Pixels::RGBA>& icon)
	{
		auto [w, h] = icon.resolution();
		auto mask_len = h * (w+7) / 8;

		std::vector<BYTE> bmp_data(sizeof(BITMAPINFOHEADER) + (w*h*sizeof(u32)) + mask_len);

		auto& header = *reinterpret_cast<BITMAPINFOHEADER*>(bmp_data.data());
		std::span<u32> pixels(
			reinterpret_cast<u32*>(bmp_data.data() + sizeof(BITMAPINFOHEADER)),
			w*h
		);
		std::span<BYTE> mask(
			bmp_data.data() + sizeof(BITMAPINFOHEADER) + pixels.size_bytes(),
			mask_len
		);

		auto swap_le = [](auto val) -> decltype(val)
		{
			if constexpr (std::endian::native != std::endian::little)
				return std::byteswap<decltype(val)>(val);
			return val;
		};
		auto swap_u32_le = [&swap_le](u32 val) { return swap_le(val); };
		auto swap_u16_le = [&swap_le](u16 val) { return swap_le(val); };

		header.biSize = swap_u32_le(sizeof(BITMAPINFOHEADER));
		header.biWidth = swap_u32_le(w);
		header.biHeight = swap_u32_le(h*2);
		header.biPlanes = swap_u16_le(1);
		header.biBitCount = swap_u16_le(32);
		header.biCompression = swap_u32_le(BI_RGB);
		header.biSizeImage = swap_u32_le(pixels.size_bytes());

		auto target = pixels.data();
		for (u16 y = h; y != 0; --y)
		{
			for (u16 x = 0; x < w; ++x)
			{
				auto& p = icon[x, y-1];
				*target++ = (p.b) | (p.g << 8) | (p.r << 16) | (p.a << 24);
			}
		}

		std::ranges::fill(mask, 0xFF);

		m_icon.reset(CreateIconFromResource(bmp_data.data(), bmp_data.size(),
											TRUE, 0x00030000));
		if (!m_icon) NYAAN_UNLIK
			throw Error::Exception(Error::Code::fail_to_create_icon);

		auto lparam = reinterpret_cast<LPARAM>(m_icon.get());
		PostMessage(handle(), WM_SETICON, ICON_SMALL, lparam);
		PostMessage(handle(), WM_SETICON, ICON_BIG, lparam);
	}
	#endif

	void Window::fullscreen_impl(bool mode)
	{
		WM::Msg post = 0;
		auto& [style, pos, res] = m_unfullscreen;
		if (mode)
		{
			style = m_style = GetWindowLong(handle(), GWL_STYLE);
			pos = p_position;
			res = p_resolution.value;

			m_style &= ~WS_OVERLAPPEDWINDOW;
			post = WM::fullscreen;
		}
		else
		{
			m_style = style;
			post = WM::unfullscreen;
		}

		SetWindowLong(handle(), GWL_STYLE, m_style);
		PostMessage(handle(), post, false, false);
	}

	void Window::cursor_impl(Cursor cursor)
	{
		if (show_mouse_no_lock())
		{
			auto& i = *m_instance;
			m_cursor =
				static_cast<InstanceWindowInterface&>(i).load_cursor(cursor);
			if (mouse_focused_no_lock())
				PostMessage(handle(), WM_SETCURSOR, 0, 0);
		}
	}
}
