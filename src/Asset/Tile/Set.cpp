module;

#include <Core/Defines.hpp>

#include <vector>

export module Nyaanwork.Asset.Tile:Set;
import :Tile;

import Nyaanwork.Asset.Exception;

export namespace Nyaanwork::Asset
{
	template<Pixels::is_pixel _Pixel>
	class TileSet final
	{
	public:
		using Tile = Asset::Tile<_Pixel>;

		using Pixel = _Pixel;
		using ImageView = Tile::ImageView;

	private:
		using CacheTiles = std::vector<Tile>;

	public:
		using iterator = CacheTiles::iterator;
		using const_iterator = CacheTiles::const_iterator;

		using reverse_iterator = CacheTiles::reverse_iterator;
		using const_reverse_iterator = CacheTiles::const_reverse_iterator;

		TileSet(ImageView image, vec2<u32> tile_resolution, bool generate_cache = true)
			: m_image(image), m_tile_resolution(tile_resolution),
			  m_counts(0), m_tiles()
		{
			if (generate_cache)
				this->generate_cache();
		}

		TileSet(NullPtr = nullptr)
			: m_image(nullptr), m_tile_resolution(0), m_counts(0), m_tiles() {}
		TileSet& operator=(NullPtr)
		{
			m_image = nullptr;
			m_tile_resolution = {0, 0};
			m_counts = {0, 0};
			m_tiles.clear();

			return *this;
		}

		TileSet(const TileSet& oth)
			: m_image(oth.image()), m_tile_resolution(oth.tile_resolution())
			{ generate_cache(); }
		TileSet operator=(const TileSet& oth)
		{
			if (this != &oth)
				image(oth.image(), oth.tile_resolution());

			return *this;
		}

		TileSet(TileSet&&) = default;
		TileSet& operator=(TileSet&&) = default;

		bool has_image() NYAAN_CNOEX
			{ return m_image.has_image(); }
		operator bool() NYAAN_CNOEX
			{ return has_image(); }

		ImageView image() NYAAN_CNOEX
			{ return m_image; }
		template<bool generate_cache = true>
		void image(ImageView image, vec2<u16> tile_resolution) noexcept(generate_cache)
		{
			m_image = image;
			this->tile_resolution<generate_cache>(tile_resolution);
		}
		template<bool generate_cache = true>
		void image(ImageView image) noexcept(generate_cache)
			{ this->image<generate_cache>(image, m_tile_resolution); }

		vec2<u16> tile_resolution() NYAAN_CNOEX
			{ return m_tile_resolution; }
		template<bool generate_cache = true>
		void tile_resolution(vec2<u16> tile_resolution) noexcept(generate_cache)
		{
			m_tile_resolution = tile_resolution;

			if constexpr (generate_cache)
				this->generate_cache();
		}

		void generate_cache()
		{
			const auto ires = m_image.resolution();
			if (m_tile_resolution.x > ires.x
				|| m_tile_resolution.y > ires.y) NYAAN_UNLIK
			{
				throw Error::Exception("tile_resolution > image.resolution()",
									   Error::Code::wrong_resolution);
			}

			auto& c = m_counts = ires / m_tile_resolution;
			usize vc = c.x * c.y;
			m_tiles.clear();
			m_tiles.reserve(vc);
			for (usize i = 0; i < vc; ++i)
			{
				vec2<u16> pos = {i % c.x, i / c.x};
				pos *= m_tile_resolution;
				m_tiles.emplace_back(m_image, pos, m_tile_resolution);
			}
		}

		usize count() NYAAN_CNOEX
			{ return m_tiles.size(); }
		vec2<usize> counts() NYAAN_CNOEX
			{ return m_counts; }

		Tile& at(usize i) noexcept
			{ return m_tiles[i]; }
		const Tile& at(usize i) NYAAN_CNOEX
			{ return m_tiles[i]; }

		Tile& at(usize x, usize y) noexcept
			{ return at(x * (y*counts().x)); }
		const Tile& at(usize x, usize y) NYAAN_CNOEX
			{ return at(x * (y*counts().x)); }

		Tile& at(vec2<usize> position) noexcept
			{ return at(position.x, position.y); }
		const Tile& at(vec2<usize> position) NYAAN_CNOEX
			{ return at(position.x, position.y); }

		Tile& operator[](usize i) noexcept
			{ return at(i); }
		const Tile& operator[](usize i) NYAAN_CNOEX
			{ return at(i); }

		Tile& operator[](u16 x, u16 y) noexcept
			{ return at(x, y); }
		const Tile& operator[](u16 x, u16 y) NYAAN_CNOEX
			{ return at(x, y); }

		Tile& operator[](vec2<u16> position) noexcept
			{ return at(position); }
		const Tile& operator[](vec2<u16> position) NYAAN_CNOEX
			{ return at(position); }

		iterator begin() noexcept
			{ return m_tiles.begin(); }
		iterator end() noexcept
			{ return m_tiles.end(); }

		const_iterator begin() NYAAN_CNOEX
			{ return m_tiles.begin(); }
		const_iterator end() NYAAN_CNOEX
			{ return m_tiles.end(); }
		const_iterator cbegin() NYAAN_CNOEX
			{ return m_tiles.cbegin(); }
		const_iterator cend() NYAAN_CNOEX
			{ return m_tiles.cend(); }

		reverse_iterator rbegin() noexcept
			{ return m_tiles.rbegin(); }
		reverse_iterator rend() noexcept
			{ return m_tiles.rend(); }

		const_reverse_iterator rbegin() NYAAN_CNOEX
			{ return m_tiles.rbegin(); }
		const_reverse_iterator rend() NYAAN_CNOEX
			{ return m_tiles.rend(); }
		const_reverse_iterator crbegin() NYAAN_CNOEX
			{ return m_tiles.crbegin(); }
		const_reverse_iterator crend() NYAAN_CNOEX
			{ return m_tiles.crend(); }

	private:
		ImageView m_image;
		vec2<u16> m_tile_resolution;

		vec2<usize> m_counts;
		CacheTiles m_tiles;
	};

	template class TileSet<Pixels::R>;
	template class TileSet<Pixels::RA>;
	template class TileSet<Pixels::RGB>;
	template class TileSet<Pixels::RGBA>;
}
