module;

#include <Core/Defines.hpp>

#include <algorithm>
#include <array>

export module Nyaanwork.Asset.Tile:Tile;
import :TileView;

import Nyaanwork.Asset.Image;

export namespace Nyaanwork::Asset
{
	template<Pixels::is_pixel _Pixel>
	class Tile final
	{
	public:
		using TileView = Asset::TileView<_Pixel>;

		using Pixel = _Pixel;
		using ImageView = TileView::ImageView;
		using Image = TileView::Image;

		using iterator = TileView::iterator;
		using const_iterator = TileView::const_iterator;

		using reverse_iterator = TileView::reverse_iterator;
		using const_reverse_iterator = TileView::const_reverse_iterator;

		Tile(ImageView image, vec2<u16> offset, vec2<u16> resolution)
			: m_view(image, offset, resolution) {}

		Tile(const Tile&) = delete;
		Tile& operator=(const Tile& oth)
		{
			if (this != &oth)
			{
				if (resolution() != oth.resolution()) NYAAN_UNLIK
					throw Error::Exception(Error::Code::wrong_resolution);

				std::copy(oth.begin(), oth.end(), begin());
			}

			return *this;
		}

		Tile(Tile&&) = default;
		Tile& operator=(Tile&&) = default;

		template<Pixels::is_pixel OthPixel>
		Tile& operator=(const Tile<OthPixel>& oth)
		{
			if (this != &oth)
			{
				if (resolution() != oth.resolution()) NYAAN_UNLIK
					throw Error::Exception(Error::Code::wrong_resolution);

				Colors<Pixel>::convert(oth.begin(), oth.end(), begin());
			}

			return *this;
		}

		usize size() NYAAN_CNOEX
			{ return m_view.size(); }

		vec2<u16> offset() NYAAN_CNOEX
			{ return m_view.offset(); }
		vec2<u16> resolution() NYAAN_CNOEX
			{ return m_view.resolution(); }

		_Pixel& at(usize i) noexcept
			{ return m_view.at(i); }
		const _Pixel& at(usize i) NYAAN_CNOEX
			{ return m_view.at(i); }

		_Pixel& at(u16 x, u16 y) noexcept
			{ return m_view.at(x, y); }
		const _Pixel& at(u16 x, u16 y) NYAAN_CNOEX
			{ return m_view.at(x, y); }

		_Pixel& at(vec2<u16> position) noexcept
			{ return m_view.at(position.x, position.y); }
		const _Pixel& at(vec2<u16> position) NYAAN_CNOEX
			{ return m_view.at(position.x, position.y); }

		_Pixel& operator[](usize i) noexcept
			{ return m_view[i]; }
		const _Pixel& operator[](usize i) NYAAN_CNOEX
			{ return m_view[i]; }

		_Pixel& operator[](u16 x, u16 y) noexcept
			{ return m_view[x, y]; }
		const _Pixel& operator[](u16 x, u16 y) NYAAN_CNOEX
			{ return m_view[x, y]; }

		_Pixel& operator[](vec2<u16> position) noexcept
			{ return m_view[position]; }
		const _Pixel& operator[](vec2<u16> position) NYAAN_CNOEX
			{ return m_view[position]; }

		template<is_floating Float>
		std::array<vec2<Float>, 4> texcoords() NYAAN_CNOEX
			{ return m_view.template texcoords<Float>(); }

		iterator begin() noexcept
			{ return m_view.begin(); }
		iterator end() noexcept
			{ return m_view.end(); }

		const_iterator begin() NYAAN_CNOEX
			{ return m_view.begin(); }
		const_iterator end() NYAAN_CNOEX
			{ return m_view.end(); }
		const_iterator cbegin() NYAAN_CNOEX
			{ return begin(); }
		const_iterator cend() NYAAN_CNOEX
			{ return end(); }

		reverse_iterator rbegin() noexcept
			{ return m_view.rbegin(); }
		reverse_iterator rend() noexcept
			{ return m_view.rend(); }

		const_reverse_iterator rbegin() NYAAN_CNOEX
			{ return m_view.rbegin(); }
		const_reverse_iterator rend() NYAAN_CNOEX
			{ return m_view.rend(); }
		const_reverse_iterator crbegin() NYAAN_CNOEX
			{ return rbegin(); }
		const_reverse_iterator crend() NYAAN_CNOEX
			{ return rend(); }

		Image create_image()
			{ return m_view.create_image(); }
		Ptr<Image> create_image_ptr()
			{ return m_view.create_image_ptr(); }

	private:
		TileView m_view;
	};

	template class Tile<Pixels::R>;
	template class Tile<Pixels::RA>;
	template class Tile<Pixels::RGB>;
	template class Tile<Pixels::RGBA>;
}

export namespace std
{
	template<Nyaanwork::Asset::Pixels::is_pixel Pixel>
	void swap(Nyaanwork::Asset::Tile<Pixel>& a,
			  Nyaanwork::Asset::Tile<Pixel>& b)
	{
		using namespace Nyaanwork;
		using namespace Nyaanwork::Asset;

		if (a.resolution() != b.resolution())
			throw Error::Exception(Error::Code::wrong_resolution);

		auto itr_a = a.begin();
		auto itr_b = b.begin();
		for (usize i = 0; i < a.size(); ++i)
		{
			auto& pixel_a = *itr_a++;
			auto& pixel_b = *itr_b++;
			std::swap(pixel_a, pixel_b);
		}
	}
}
