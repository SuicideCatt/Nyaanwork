module;

#include <Core/Defines.hpp>

#include <windows.h>

#include <algorithm>
#include <ranges>
#include <vector>

export module Nyaanwork.WindowSystem.Backends.Windows:Window.methods.drop;
import :Instance.decl;
import :Window.decl;

import Nyaanwork.Core.StringConvertor;
import Nyaanwork.Core.System;
import Nyaanwork.Core.Types;

namespace Nyaanwork::WindowSystem::Backends::Windows
{
	class DropTarget final : public IDropTarget
	{
	public:
		DropTarget(Window* window, HWND hwnd)
			: m_refcount(0), m_window(window), m_hwnd(hwnd),
			  m_format_text(RegisterClipboardFormatA("text/plain;charset=utf-8")),
			  m_format_file(RegisterClipboardFormatA("text/uri-list")) {}

		HRESULT QueryInterface(REFIID riid, void** ppv) override
		{
			if (ppv == nullptr)
				return E_INVALIDARG;

			*ppv = nullptr;

			if (riid == IID_IUnknown || riid == IID_IDropTarget)
				*ppv = this;

			if (*ppv)
			{
				AddRef();
				return S_OK;
			}

			return E_NOINTERFACE;
		}

		ULONG AddRef() override
			{ return ++m_refcount; }

		ULONG Release() override
		{
			if (m_refcount > 0)
				--m_refcount;

			if (m_refcount == 0)
				return 0;

			return m_refcount;
		}

		HRESULT DragEnter(IDataObject* data, DWORD, POINTL pt, DWORD*) override
		{
			using S = Window::Slots;
			using T = Window::DropDataType;

			if (check_format(m_format_file, data) || check_format(CF_HDROP, data))
			{
				m_window->signal_send<S::drop_begin>(*m_window, T::uri);
			}
			else if (check_format(m_format_text, data)
					 || check_format(CF_UNICODETEXT, data)
					 || check_format(CF_TEXT, data))
			{
				m_window->signal_send<S::drop_begin>(*m_window, T::text);
			}

			{
				auto l = m_window->unique_lock();
				m_window->p_input.mouse.position = {pt.x, pt.y};
			}

			return S_OK;
		}

		HRESULT DragOver(DWORD, POINTL pt, DWORD*) override
		{
			{
				auto l = m_window->unique_lock();
				m_window->p_input.mouse.position = {pt.x, pt.y};
			}

			return S_OK;
		}

		HRESULT DragLeave() override
		{
			using S = Window::Slots;
			m_window->signal_send<S::drop_cancel>(*m_window);
			return S_OK;
		}

		HRESULT Drop(IDataObject* data, DWORD, POINTL pt, DWORD*) override
		{
			using S = Window::Slots;

			Opt<FORMATETC> f;

			auto work = [data, &f, &window = *m_window](UINT format, auto&& call)
			{
				if (!(f = check_format(format, data)))
					return false;

				STGMEDIUM med;
				if (!SUCCEEDED(data->GetData(&*f, &med)))
					return false;

				auto size = GlobalSize(med.hGlobal);
				void* buf = GlobalLock(med.hGlobal);

				if (!buf)
					return false;

				call(buf, size);

				GlobalUnlock(med.hGlobal);
				ReleaseStgMedium(&med);

				return true;
			};

			work(m_format_file, [this](const void* buf, usize size)
			{
				static constexpr StrV delim = "\r\n";
				StrV text(reinterpret_cast<const char*>(buf), size);

				std::vector<Str> uris;
				for (const auto token : std::views::split(text, delim))
				{
					if (token.empty()) NYAAN_UNLIK
						continue;

					uris.push_back(System::decode_uri(StrV(token)));
				}

				m_window->signal_send<S::drop_uri>(*m_window, uris);
			})
			|| work(CF_HDROP, [this](void* buf, usize)
			{
				Str path;
				std::vector<Str> uris;

				auto drop = reinterpret_cast<HDROP>(buf);
				usize count = DragQueryFileA(drop, 0xFFFFFFFF, nullptr, 0);
				for (usize i = 0; i < count; ++i)
				{
					usize size = DragQueryFileA(drop, i, nullptr, 0)+1;
					path.resize(size, '\0');
					size = DragQueryFileA(drop, i, path.data(), path.size());
					path.resize(size);
					std::ranges::replace(path, '\\', '/');
					uris.push_back("file:///" + path);
				}

				m_window->signal_send<S::drop_uri>(*m_window, uris);
			})
			|| work(m_format_text, [this](const void* buf, size_t size)
			{
				Str text(StrV(reinterpret_cast<const char*>(buf), size));
				m_window->signal_send<S::drop_text>(*m_window, text);
			})
			|| work(CF_UNICODETEXT, [this](const void* buf, size_t size)
			{
				auto wcstr = reinterpret_cast<const wchar_t*>(buf);
				auto text = StringConvertor_WChar().convert({wcstr, size});

				m_window->signal_send<S::drop_text>(*m_window, text);
			})
			|| work(CF_TEXT, [this](const void* buf, size_t size)
			{
				Str text(StrV(reinterpret_cast<const char*>(buf), size));
				m_window->signal_send<S::drop_text>(*m_window, text);
			});

			m_window->signal_send<S::drop_done>(*m_window);
			return S_OK;
		}

	private:
		static Opt<FORMATETC>
			check_format(CLIPFORMAT format, IDataObject* data) noexcept
		{
			FORMATETC fetc;
			fetc.cfFormat = format;
			fetc.ptd = nullptr;
			fetc.dwAspect = DVASPECT_CONTENT;
			fetc.lindex = -1;
			fetc.tymed = TYMED_HGLOBAL;

			if (SUCCEEDED(data->QueryGetData(&fetc)))
				return std::move(fetc);
			else
				return nullopt;
		}

		ULONG m_refcount = 0;
		Window* m_window;
		HWND m_hwnd;
		UINT m_format_text;
		UINT m_format_file;
	};
}
