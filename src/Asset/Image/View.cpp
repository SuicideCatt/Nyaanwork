module;

#include <Core/Defines.hpp>

#include <variant>

export module Nyaanwork.Asset.Image:Image.View;
import :Pixel;
import :Pixel.std_integration;
import :Colors;
import :Image;

import Nyaanwork.Asset.Exception;
import Nyaanwork.Core.Types;
import Nyaanwork.Core.Utils.FunctionTraits;
import Nyaanwork.Core.Utils.PtrContainer;

export namespace Nyaanwork::Asset
{
	template<typename>
	class ImageView;

	template<Pixels::is_pixel _Pixel>
	class ImageView<_Pixel> final
	{
	public:
		using Pixel = _Pixel;
		using Image = Asset::Image<_Pixel>;

		using iterator = Image::iterator;
		using const_iterator = Image::const_iterator;

		using reverse_iterator = Image::reverse_iterator;
		using const_reverse_iterator = Image::const_reverse_iterator;

		ImageView(NullPtr = nullptr) noexcept
			: m_image(Colors<_Pixel>::null) {}
		ImageView& operator=(NullPtr) noexcept
		{
			m_image = Colors<_Pixel>::null;
			return *this;
		}

		ImageView(Ptr<Image> image) noexcept
			: m_image(image) {}
		ImageView& operator=(Ptr<Image> image) noexcept
		{
			m_image = image;
			return *this;
		}

		ImageView(Image* image) noexcept
			: m_image(PtrContainer(image)) {}
		ImageView& operator=(Image* image) noexcept
		{
			m_image = PtrContainer(image);
			return *this;
		}

		ImageView(Image& image) noexcept
			: m_image(PtrContainer(image)) {}
		ImageView& operator=(Image& image) noexcept
		{
			m_image = PtrContainer(image);
			return *this;
		}

		ImageView(const ImageView&) = default;
		ImageView& operator=(const ImageView&) = default;

		ImageView(ImageView&&) = default;
		ImageView& operator=(ImageView&&) = default;

		bool has_image() NYAAN_CNOEX
			{ return get(); }
		operator bool() NYAAN_CNOEX
			{ return has_image(); }

		Image& image()
		{
			if (auto img = get())
				return *img;

			throw Error::Exception(Error::Code::no_image);
		}
		const Image& image() const
		{
			if (auto img = get())
				return *img;

			throw Error::Exception(Error::Code::no_image);
		}

		bool empty() NYAAN_CNOEX
			{ return call_get<&Image::empty, false>(true); }

		vec2<u16> resolution() NYAAN_CNOEX
			{ return call_get<&Image::resolution, false>(vec2<u16>(0)); }

		usize size() NYAAN_CNOEX
			{ return call_get<&Image::size, false>(0_usize); }

		_Pixel* data() noexcept
		{
			using MPtr = GetMethodPtr<false, true, Image, _Pixel*>;
			constexpr MPtr ptr = &Image::data;
			return call_get<ptr>([this] { return &std::get<0>(m_image); });
		}
		const _Pixel* data() NYAAN_CNOEX
		{
			using MPtr = GetMethodPtr<true, true, Image, const _Pixel*>;
			constexpr MPtr ptr = &Image::data;
			return call_get<ptr>([this] { return &std::get<0>(m_image); });
		}

		_Pixel& at(usize i) noexcept
		{
			using MPtr = GetMethodPtr<false, true, Image, _Pixel&, usize>;
			constexpr MPtr ptr = &Image::at;
			return call_get<ptr>(
				[this](auto) -> _Pixel& { return std::get<0>(m_image); },
				i);
		}
		const _Pixel& at(usize i) NYAAN_CNOEX
		{
			using MPtr = GetMethodPtr<true, true, Image, const _Pixel&, usize>;
			constexpr MPtr ptr = &Image::at;
			return call_get<ptr>(
				[this](auto) -> const _Pixel& { return std::get<0>(m_image); },
				i);
		}

		_Pixel& at(u16 x, u16 y) noexcept
		{
			using MPtr = GetMethodPtr<false, true, Image, _Pixel&, u16, u16>;
			constexpr MPtr ptr = &Image::at;
			return call_get<ptr>(
				[this](auto, auto) -> _Pixel& { return std::get<0>(m_image); },
				x, y);
		}
		const _Pixel& at(u16 x, u16 y) NYAAN_CNOEX
		{
			using MPtr = GetMethodPtr<true, true, Image, const _Pixel&, u16, u16>;
			constexpr MPtr ptr = &Image::at;
			return call_get<ptr>(
				[this](auto, auto) -> const _Pixel& { return std::get<0>(m_image); },
				x, y);
		}

		_Pixel& at(vec2<u16> position) noexcept
		{
			using MPtr = GetMethodPtr<false, true, Image, _Pixel&, vec2<u16>>;
			constexpr MPtr ptr = &Image::at;
			return call_get<ptr>(
				[this](auto) -> _Pixel& { return std::get<0>(m_image); },
				position);
		}
		const _Pixel& at(vec2<u16> position) NYAAN_CNOEX
		{
			using MPtr = GetMethodPtr<true, true, Image, const _Pixel&, vec2<u16>>;
			constexpr MPtr ptr = &Image::at;
			return call_get<ptr>(
				[this](auto) -> const _Pixel& { return std::get<0>(m_image); },
				position);
		}

		iterator begin() noexcept
		{
			using MPtr = GetMethodPtr<false, true, Image, iterator>;
			constexpr MPtr ptr = &Image::begin;
			return call_get<ptr>(
				[this] { return iterator(&std::get<0>(m_image)); });
		}
		iterator end() noexcept
		{
			using MPtr = GetMethodPtr<false, true, Image, iterator>;
			constexpr MPtr ptr = &Image::end;
			return call_get<ptr>(
				[this] { return iterator(&std::get<0>(m_image)); });
		}

		const_iterator begin() NYAAN_CNOEX
		{
			using MPtr = GetMethodPtr<true, true, Image, const_iterator>;
			constexpr MPtr ptr = &Image::begin;
			return call_get<ptr>(
				[this] { return const_iterator(&std::get<0>(m_image)); });
		}
		const_iterator end() NYAAN_CNOEX
		{
			using MPtr = GetMethodPtr<true, true, Image, const_iterator>;
			constexpr MPtr ptr = &Image::end;
			return call_get<ptr>(
				[this] { return const_iterator(&std::get<0>(m_image)); });
		}
		const_iterator cbegin() NYAAN_CNOEX
			{ return begin(); }
		const_iterator cend() NYAAN_CNOEX
			{ return end(); }

		reverse_iterator rbegin() noexcept
		{
			using MPtr = GetMethodPtr<false, true, Image, reverse_iterator>;
			constexpr MPtr ptr = &Image::rbegin;
			return call_get<ptr>(
				[this] { return reverse_iterator(iterator(&std::get<0>(m_image))); });
		}
		reverse_iterator rend() noexcept
		{
			using MPtr = GetMethodPtr<false, true, Image, reverse_iterator>;
			constexpr MPtr ptr = &Image::rend;
			return call_get<ptr>(
				[this] { return reverse_iterator(iterator(&std::get<0>(m_image))); });
		}

		const_reverse_iterator rbegin() NYAAN_CNOEX
		{
			using MPtr = GetMethodPtr<true, true, Image, const_reverse_iterator>;
			constexpr MPtr ptr = &Image::rbegin;
			return call_get<ptr>(
				[this]
				{
					using CRI = const_reverse_iterator;
					using CI = const_iterator;
					return CRI(CI(&std::get<0>(m_image)));
				});
		}
		const_reverse_iterator rend() NYAAN_CNOEX
		{
			using MPtr = GetMethodPtr<true, true, Image, const_reverse_iterator>;
			constexpr MPtr ptr = &Image::rbegin;
			return call_get<ptr>(
				[this]
				{
					using CRI = const_reverse_iterator;
					using CI = const_iterator;
					return CRI(CI(&std::get<0>(m_image)));
				});
		}
		const_reverse_iterator crbegin() NYAAN_CNOEX
			{ return rbegin(); }
		const_reverse_iterator crend() NYAAN_CNOEX
			{ return rend(); }

		ImageView<const _Pixel> to_const() NYAAN_CNOEX;

	private:
		Image* get() noexcept
		{
			switch (m_image.index())
			{
			case 1:
				return std::get<1>(m_image).get();
			case 2:
				return std::get<2>(m_image).get();

			default:
				return nullptr;
			}
		}
		const Image* get() NYAAN_CNOEX
		{
			switch (m_image.index())
			{
			case 1:
				return std::get<1>(m_image).get();
			case 2:
				return std::get<2>(m_image).get();

			default:
				return nullptr;
			}
		}

		template<auto method, bool is_functor = true,
				 typename Traits = MethodTraits<method>, typename... Args>
		auto call_get(auto&& no_image_call, Args&&... args) noexcept
			-> Traits::Result
		{
			if (auto img = get())
			{
				return (img->*method)(std::forward<Args>(args)...);
			}
			else
			{
				if constexpr (is_functor)
					return no_image_call(std::forward<Args>(args)...);
				else
					return no_image_call;
			}
		}
		template<auto method, bool is_functor = true,
				 typename Traits = MethodTraits<method>, typename... Args>
		auto call_get(auto&& no_image_call, Args&&... args) NYAAN_CNOEX
			-> Traits::Result
		{
			if (auto img = get())
			{
				return (img->*method)(std::forward<Args>(args)...);
			}
			else
			{
				if constexpr (is_functor)
					return no_image_call(std::forward<Args>(args)...);
				else
					return no_image_call;
			}
		}

		std::variant<_Pixel, Ptr<Image>, PtrContainer<Image>> m_image;
	};

	template<Pixels::is_pixel Pixel>
	ImageView(Ptr<Image<Pixel>>) -> ImageView<Pixel>;
	template<Pixels::is_pixel Pixel>
	ImageView(Image<Pixel>*) -> ImageView<Pixel>;
	template<Pixels::is_pixel Pixel>
	ImageView(Image<Pixel>&) -> ImageView<Pixel>;

	template class ImageView<Pixels::R>;
	template class ImageView<Pixels::RA>;
	template class ImageView<Pixels::RGB>;
	template class ImageView<Pixels::RGBA>;

	template<Pixels::is_pixel _Pixel>
	class ImageView<const _Pixel> final
	{
	public:
		using Pixel = _Pixel;
		using Image = Asset::Image<_Pixel>;

		using const_iterator = Image::const_iterator;
		using const_reverse_iterator = Image::const_reverse_iterator;

		ImageView(NullPtr = nullptr) noexcept
			: m_image(Colors<_Pixel>::null) {}
		ImageView& operator=(NullPtr) noexcept
		{
			m_image = Colors<_Pixel>::null;
			return *this;
		}

		ImageView(Ptr<const Image> image) noexcept
			: m_image(image) {}
		ImageView& operator=(Ptr<const Image> image) noexcept
		{
			m_image = image;
			return *this;
		}

		ImageView(const Image* image) noexcept
			: m_image(PtrContainer(image)) {}
		ImageView& operator=(const Image* image) noexcept
		{
			m_image = PtrContainer(image);
			return *this;
		}

		ImageView(const Image& image) noexcept
			: m_image(PtrContainer(image)) {}
		ImageView& operator=(const Image& image) noexcept
		{
			m_image = PtrContainer(image);
			return *this;
		}

		ImageView(const ImageView&) = default;
		ImageView& operator=(const ImageView&) = default;

		ImageView(ImageView&&) = default;
		ImageView& operator=(ImageView&&) = default;

		ImageView(const ImageView<Pixel>& image) noexcept
			: ImageView(image.to_const()) {}
		ImageView& operator=(const ImageView<Pixel>& image) noexcept
			{ return *this = image.to_const(); }

		bool has_image() NYAAN_CNOEX
			{ return get(); }
		operator bool() NYAAN_CNOEX
			{ return has_image(); }

		const Image& image() const
		{
			if (auto img = get())
				return *img;

			throw Error::Exception(Error::Code::no_image);
		}

		bool empty() NYAAN_CNOEX
			{ return call_get<&Image::empty, false>(true); }

		vec2<u16> resolution() NYAAN_CNOEX
			{ return call_get<&Image::resolution, false>(vec2<u16>(0)); }

		usize size() NYAAN_CNOEX
			{ return call_get<&Image::size, false>(0_usize); }

		const _Pixel* data() NYAAN_CNOEX
		{
			using MPtr = GetMethodPtr<true, true, Image, const _Pixel*>;
			constexpr MPtr ptr = &Image::data;
			return call_get<ptr>([this] { return &std::get<0>(m_image); });
		}

		const _Pixel& at(usize i) NYAAN_CNOEX
		{
			using MPtr = GetMethodPtr<true, true, Image, const _Pixel&, usize>;
			constexpr MPtr ptr = &Image::at;
			return call_get<ptr>(
				[this](auto) -> const _Pixel& { return std::get<0>(m_image); },
				i);
		}

		const _Pixel& at(u16 x, u16 y) NYAAN_CNOEX
		{
			using MPtr = GetMethodPtr<true, true, Image, const _Pixel&, u16, u16>;
			constexpr MPtr ptr = &Image::at;
			return call_get<ptr>(
				[this](auto, auto) -> const _Pixel& { return std::get<0>(m_image); },
				x, y);
		}

		const _Pixel& at(vec2<u16> position) NYAAN_CNOEX
		{
			using MPtr = GetMethodPtr<true, true, Image, const _Pixel&, vec2<u16>>;
			constexpr MPtr ptr = &Image::at;
			return call_get<ptr>(
				[this](auto) -> const _Pixel& { return std::get<0>(m_image); },
				position);
		}

		const_iterator begin() NYAAN_CNOEX
		{
			using MPtr = GetMethodPtr<true, true, Image, const_iterator>;
			constexpr MPtr ptr = &Image::begin;
			return call_get<ptr>(
				[this] { return const_iterator(&std::get<0>(m_image)); });
		}
		const_iterator end() NYAAN_CNOEX
		{
			using MPtr = GetMethodPtr<true, true, Image, const_iterator>;
			constexpr MPtr ptr = &Image::end;
			return call_get<ptr>(
				[this] { return const_iterator(&std::get<0>(m_image)); });
		}
		const_iterator cbegin() NYAAN_CNOEX
			{ return begin(); }
		const_iterator cend() NYAAN_CNOEX
			{ return end(); }

		const_reverse_iterator rbegin() NYAAN_CNOEX
		{
			using MPtr = GetMethodPtr<true, true, Image, const_reverse_iterator>;
			constexpr MPtr ptr = &Image::rbegin;
			return call_get<ptr>(
				[this]
				{
					using CRI = const_reverse_iterator;
					using CI = const_iterator;
					return CRI(CI(&std::get<0>(m_image)));
				});
		}
		const_reverse_iterator rend() NYAAN_CNOEX
		{
			using MPtr = GetMethodPtr<true, true, Image, const_reverse_iterator>;
			constexpr MPtr ptr = &Image::rbegin;
			return call_get<ptr>(
				[this]
				{
					using CRI = const_reverse_iterator;
					using CI = const_iterator;
					return CRI(CI(&std::get<0>(m_image)));
				});
		}
		const_reverse_iterator crbegin() NYAAN_CNOEX
			{ return rbegin(); }
		const_reverse_iterator crend() NYAAN_CNOEX
			{ return rend(); }

	private:
		const Image* get() NYAAN_CNOEX
		{
			switch (m_image.index())
			{
			case 1:
				return std::get<1>(m_image).get();
			case 2:
				return std::get<2>(m_image).get();

			default:
				return nullptr;
			}
		}

		template<auto method, bool is_functor = true,
				 typename Traits = MethodTraits<method>, typename... Args>
		auto call_get(auto&& no_image_call, Args&&... args) NYAAN_CNOEX
			-> Traits::Result
		{
			if (auto img = get())
			{
				return (img->*method)(std::forward<Args>(args)...);
			}
			else
			{
				if constexpr (is_functor)
					return no_image_call(std::forward<Args>(args)...);
				else
					return no_image_call;
			}
		}

		std::variant<_Pixel, Ptr<const Image>, PtrContainer<const Image>> m_image;
	};

	template<Pixels::is_pixel Pixel>
	ImageView(Ptr<const Image<Pixel>>) -> ImageView<const Pixel>;
	template<Pixels::is_pixel Pixel>
	ImageView(const Image<Pixel>*) -> ImageView<const Pixel>;
	template<Pixels::is_pixel Pixel>
	ImageView(const Image<Pixel>&) -> ImageView<const Pixel>;

	template class ImageView<const Pixels::R>;
	template class ImageView<const Pixels::RA>;
	template class ImageView<const Pixels::RGB>;
	template class ImageView<const Pixels::RGBA>;

	template<Pixels::is_pixel Pixel>
	ImageView<const Pixel> ImageView<Pixel>::to_const() NYAAN_CNOEX
	{
		switch (m_image.index())
		{
		case 1:
			return Ptr<const Image>(std::get<1>(m_image));
		case 2:
			return std::get<2>(m_image).get();

		default:
			return nullptr;
		}
	}
}
