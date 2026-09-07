module;

#include <Core/Defines.hpp>

#include <concepts>
#include <span>
#include <vector>

export module Nyaanwork.Asset.Image:Image;
import :Pixel;
import :Pixel.std_integration;
import :Colors;

import Nyaanwork.Asset.Exception;
import Nyaanwork.Core.Types;

export namespace Nyaanwork::Asset
{
	template<Pixels::is_pixel _Pixel>
	class Image final
	{
	public:
		using Pixel = _Pixel;

		using iterator = std::vector<_Pixel>::iterator;
		using const_iterator = std::vector<_Pixel>::const_iterator;

		using reverse_iterator = std::vector<_Pixel>::reverse_iterator;
		using const_reverse_iterator = std::vector<_Pixel>::const_reverse_iterator;

		Image(vec2<u16> resolution = {0, 0},
			  _Pixel color = Colors<_Pixel>::null) noexcept
			: m_resolution(resolution),
			  m_memory(resolution.x * resolution.y, color) {}

		template<typename Itr>
		requires (std::same_as<typename std::iterator_traits<Itr>::value_type, _Pixel>)
		Image(vec2<u16> resolution, Itr begin, Itr end)
			: m_resolution(resolution)
		{
			m_memory = {begin, end};

			if (m_memory.size() != (resolution.x * resolution.y)) NYAAN_UNLIK
				throw Error::Exception("resolution.{x*y} != memory.size()",
									   Error::Code::wrong_resolution);
		}

		Image(vec2<u16> resolution, std::span<_Pixel const> memory)
			: m_resolution(resolution)
		{
			if (memory.size() != (resolution.x * resolution.y)) NYAAN_UNLIK
				throw Error::Exception("resolution.{x*y} != memory.size()",
									   Error::Code::wrong_resolution);

			m_memory = {memory.begin(), memory.end()};
		}

		Image(vec2<u16> resolution, std::initializer_list<_Pixel> memory)
			: Image(resolution, std::span(memory)) {}

		Image(const Image&) = default;
		Image& operator=(const Image&) = default;

		Image(Image&&) = default;
		Image& operator=(Image&&) = default;

		template<Pixels::is_pixel OthPixel>
		Image(const Image<OthPixel>& oth) noexcept
			{ *this = oth; }

		template<Pixels::is_pixel OthPixel>
		Image& operator=(const Image<OthPixel>& oth) noexcept
		{
			m_resolution = oth.resolution();
			m_memory.resize(oth.size());
			Colors<_Pixel>::convert(oth.begin(), oth.end(), m_memory.begin());

			return *this;
		}

		bool empty() NYAAN_CNOEX
			{ return m_memory.empty(); }

		usize size() NYAAN_CNOEX
			{ return m_memory.size(); }

		vec2<u16> resolution() NYAAN_CNOEX
			{ return m_resolution; }

		_Pixel* data() noexcept
			{ return m_memory.data(); }
		const _Pixel* data() NYAAN_CNOEX
			{ return m_memory.data(); }

		_Pixel& at(usize i) noexcept
			{ return m_memory[i]; }
		const _Pixel& at(usize i) NYAAN_CNOEX
			{ return m_memory[i]; }

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

		iterator begin() noexcept
			{ return m_memory.begin(); }
		iterator end() noexcept
			{ return m_memory.end(); }

		const_iterator begin() NYAAN_CNOEX
			{ return m_memory.begin(); }
		const_iterator end() NYAAN_CNOEX
			{ return m_memory.end(); }
		const_iterator cbegin() NYAAN_CNOEX
			{ return begin(); }
		const_iterator cend() NYAAN_CNOEX
			{ return end(); }

		reverse_iterator rbegin() noexcept
			{ return m_memory.rbegin(); }
		reverse_iterator rend() noexcept
			{ return m_memory.rend(); }

		const_reverse_iterator rbegin() NYAAN_CNOEX
			{ return m_memory.rbegin(); }
		const_reverse_iterator rend() NYAAN_CNOEX
			{ return m_memory.rend(); }
		const_reverse_iterator crbegin() NYAAN_CNOEX
			{ return m_memory.crbegin(); }
		const_reverse_iterator crend() NYAAN_CNOEX
			{ return m_memory.crend(); }

	private:
		vec2<u16> m_resolution;
		std::vector<_Pixel> m_memory;
	};

	template class Image<Pixels::R>;
	template class Image<Pixels::RA>;
	template class Image<Pixels::RGB>;
	template class Image<Pixels::RGBA>;
}
