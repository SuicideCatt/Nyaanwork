module;

#include <Core/Defines.hpp>

#include <array>

export module Nyaanwork.Asset.Tile:TileView;

import Nyaanwork.Asset.Exception;
import Nyaanwork.Asset.Image;
import Nyaanwork.Core.Types;

namespace Nyaanwork::Asset
{
	template<typename _Pixel>
	class Iterator final : public std::bidirectional_iterator_tag
	{
	public:
		using Pixel = _Pixel;
		using ImageView = Asset::ImageView<_Pixel>;

		using value_type = _Pixel;
		using difference_type = isize;
		using reference = _Pixel&;
		using pointer = _Pixel*;

		Iterator()
			: m_data(nullptr), m_end_x(0),
			  m_res_x(0), m_full_x(0), m_x(0) {}

		Iterator(ImageView image,
				 i16 end_x, u16 res_x,
				 u16 full_x, i16 curr_x) noexcept
			: m_data(image.data()), m_end_x(end_x),
			  m_res_x(res_x), m_full_x(full_x), m_x(curr_x) {}

		Iterator(const Iterator&) = default;
		Iterator& operator=(const Iterator&) = default;

		Iterator& operator++() noexcept
		{
			if (m_x == m_end_x) NYAAN_UNLIK
				m_x = (m_end_x += m_full_x) - (m_res_x - 1);
			else
				++m_x;

			return *this;
		}
		Iterator operator++(int) noexcept
		{
			auto itr = *this;
			++(*this);

			return itr;
		}

		Iterator& operator--() noexcept
		{
			if (m_x == m_end_x - (m_res_x - 1)) NYAAN_UNLIK
				m_x = (m_end_x -= m_full_x);
			else
				--m_x;

			return *this;
		}
		Iterator operator--(int) noexcept
		{
			auto itr = *this;
			--(*this);

			return itr;
		}

		Iterator& operator+=(isize i) noexcept
		{
			if (i > 0)
				add(i);
			else if (i < 0)
				sub(-i);

			return *this;
		}
		Iterator& operator-=(isize i) noexcept
		{
			if (i > 0)
				sub(i);
			else if (i < 0)
				add(-i);

			return *this;
		}

		Iterator operator+(isize i) NYAAN_CNOEX
		{
			auto itr = *this;
			itr += i;

			return itr;
		}
		Iterator operator-(isize i) NYAAN_CNOEX
		{
			auto itr = *this;
			itr -= i;

			return itr;
		}

		reference operator*() NYAAN_CNOEX
			{ return m_data[m_x]; }

		bool operator==(const Iterator& oth) NYAAN_CNOEX
		{
			return m_full_x == oth.m_full_x
				&& m_res_x == oth.m_res_x
				&& m_x == oth.m_x;
		}

	private:
		void add(isize i) noexcept
		{
			auto d = (m_end_x + 1) - m_x;
			if (i > d)
			{
				auto l = i / m_res_x;

				i -= d + (m_res_x * (l - 1));
				if (d == 1 && i >= m_res_x)
				{
					i -= m_res_x;
					++l;
				}

				m_x = (m_end_x += m_full_x * l) - (m_res_x - 1);
				m_x += i;
			}
			else if (i == d)
			{
				m_x = (m_end_x += m_full_x) - (m_res_x - 1);
			}
			else
			{
				m_x += i;
			}
		}

		void sub(isize i) noexcept
		{
			auto d = m_x - (m_end_x - m_res_x);
			if (i > d)
			{
				auto l = i / m_res_x;
				if (l == 0)
					++l;

				i -= d + (m_res_x * (l - 1));
				if (d == 1 && i >= m_res_x)
				{
					i -= m_res_x;
					++l;
				}

				m_x = (m_end_x -= m_full_x * l);
				m_x -= i;
			}
			else if (i == d)
			{
				m_x = (m_end_x -= m_full_x);
			}
			else
			{
				m_x -= i;
			}
		}

		_Pixel* m_data;
		i16 m_end_x;
		u16 m_res_x, m_full_x;
		i16 m_x;
	};

	template class Iterator<Pixels::R>;
	template class Iterator<Pixels::RA>;
	template class Iterator<Pixels::RGB>;
	template class Iterator<Pixels::RGBA>;
}

export namespace Nyaanwork::Asset
{
	template<Pixels::is_pixel _Pixel>
	class TileView final
	{
	public:
		using Pixel = _Pixel;
		using ImageView = Asset::ImageView<_Pixel>;
		using Image = ImageView::Image;

		using iterator = Iterator<Pixel>;
		using const_iterator = Iterator<const Pixel>;

		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		TileView(ImageView image, vec2<u16> offset, vec2<u16> resolution)
			{ this->image(image, offset, resolution); }

		TileView(NullPtr = nullptr)
			: m_image(nullptr), m_offset(0), m_resolution(0) {}
		TileView& operator=(NullPtr)
		{
			m_image = nullptr;
			m_offset = {0, 0};
			m_resolution = {0, 0};

			return *this;
		}

		TileView(const TileView&) = default;
		TileView& operator=(const TileView&) = default;

		TileView(TileView&& oth)
			: m_image(oth.image()), m_offset(oth.offset()),
			  m_resolution(oth.resolution())
			{ oth = nullptr; }
		TileView& operator=(TileView&& oth)
		{
			if (this != &oth)
			{
				m_image = oth.image();
				m_offset = oth.offset();
				m_resolution = oth.resolution();

				oth = nullptr;
			}

			return *this;
		}

		bool has_image() NYAAN_CNOEX
			{ return m_image.has_image(); }
		operator bool() NYAAN_CNOEX
			{ return has_image(); }

		ImageView image() NYAAN_CNOEX
			{ return m_image; }
		void image(ImageView image, vec2<u16> offset, vec2<u16> resolution)
		{
			m_image = image;
			move_view(offset, resolution);
		}
		void image(ImageView image)
			{ this->image(image, m_offset, m_resolution); }

		usize size() NYAAN_CNOEX
			{ return m_resolution.x * m_resolution.y; }

		vec2<u16> offset() NYAAN_CNOEX
			{ return m_offset; }
		vec2<u16> resolution() NYAAN_CNOEX
			{ return m_resolution; }

		void move_view(vec2<u16> offset, vec2<u16> resolution)
		{
			vec2<u16> res = m_image.resolution();

			auto check = [](u16 x, u16 rx, u16 o)
				{ return x < rx || x < rx+o; };

			bool wrong_x = check(res.x, resolution.x, offset.x);
			bool wrong_y = check(res.y, resolution.y, offset.y);

			if (wrong_x || wrong_y) NYAAN_UNLIK
				throw Error::Exception(Error::Code::wrong_resolution_or_offset);

			m_offset = offset;
			m_resolution = resolution;
		}

		_Pixel& at(usize i) noexcept
			{ return *(begin() + i); }
		const _Pixel& at(usize i) NYAAN_CNOEX
			{ return *(begin() + i); }

		_Pixel& at(u16 x, u16 y) noexcept
		{
			return at(static_cast<usize>(x)
						+ (static_cast<usize>(y) * m_resolution.x));
		}
		const _Pixel& at(u16 x, u16 y) NYAAN_CNOEX
		{
			return at(static_cast<usize>(x)
						+ (static_cast<usize>(y) * m_resolution.x));
		}

		_Pixel& at(vec2<u16> position) noexcept
			{ return at(position.x, position.y); }
		const _Pixel& at(vec2<u16> position) NYAAN_CNOEX
			{ return at(position.x, position.y); }

		_Pixel& operator[](usize i) noexcept
			{ return at(i); }
		const _Pixel& operator[](usize i) NYAAN_CNOEX
			{ return at(i); }

		_Pixel& operator[](u16 x, u16 y) noexcept
			{ return at(x, y); }
		const _Pixel& operator[](u16 x, u16 y) NYAAN_CNOEX
			{ return at(x, y); }

		_Pixel& operator[](vec2<u16> position) noexcept
			{ return at(position); }
		const _Pixel& operator[](vec2<u16> position) NYAAN_CNOEX
			{ return at(position); }

		template<is_floating Float>
		std::array<vec2<Float>, 4> texcoords() NYAAN_CNOEX
		{
			if constexpr (std::same_as<Float, f128>)
				return texcoords128();

			auto tcds = texcoords128();
			return {
				vec2<Float>(tcds[0]), vec2<Float>(tcds[1]),
				vec2<Float>(tcds[2]), vec2<Float>(tcds[3]),
			};
		}

		iterator begin() noexcept
		{
			u16 real_x = m_image.resolution().x;
			i16 pos = m_offset.x + (m_offset.y * real_x);
			i16 res = m_resolution.x;
			i16 end_x = pos + (res - 1);

			return {m_image, end_x, m_resolution.x, real_x, pos};
		}
		iterator end() noexcept
			{ return begin() + size(); }

		const_iterator begin() NYAAN_CNOEX
		{
			u16 real_x = m_image.resolution().x;
			i16 pos = m_offset.x + (m_offset.y * real_x);
			i16 res = m_resolution.x;
			i16 end_x = pos + (res - 1);

			return {m_image, end_x, m_resolution.x, real_x, pos};
		}
		const_iterator end() NYAAN_CNOEX
			{ return begin() + size(); }
		const_iterator cbegin() NYAAN_CNOEX
			{ return begin(); }
		const_iterator cend() NYAAN_CNOEX
			{ return end(); }

		reverse_iterator rbegin() noexcept
			{ return reverse_iterator(end()); }
		reverse_iterator rend() noexcept
			{ return reverse_iterator(begin()); }

		const_reverse_iterator rbegin() NYAAN_CNOEX
			{ return const_reverse_iterator(end()); }
		const_reverse_iterator rend() NYAAN_CNOEX
			{ return const_reverse_iterator(begin()); }
		const_reverse_iterator crbegin() NYAAN_CNOEX
			{ return rbegin(); }
		const_reverse_iterator crend() NYAAN_CNOEX
			{ return rend(); }

		Image create_image()
			{ return {m_resolution, begin(), end()}; }
		Ptr<Image> create_image_ptr()
			{ return make_shared<Image>(std::move(create_image())); }

	private:
		std::array<vec2<f128>, 4> texcoords128() NYAAN_CNOEX
		{
			vec2<f128> res(m_image.resolution());
			vec2<f128> pos = vec2<f128>(m_offset) / res;
			vec2<f128> end = pos + (vec2<f128>(resolution()) / res);

			return {vec2<f128>(pos.x, end.y), pos, end, {end.x, pos.y}};
		}

		ImageView m_image;
		vec2<u16> m_offset, m_resolution;
	};

	template class TileView<Pixels::R>;
	template class TileView<Pixels::RA>;
	template class TileView<Pixels::RGB>;
	template class TileView<Pixels::RGBA>;
}
