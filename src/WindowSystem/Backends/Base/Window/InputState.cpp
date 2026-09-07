module;

#include <array>
#include <utility>
#include <vector>

export module Nyaanwork.WindowSystem.Backends.Base:Window.InputState;

import Nyaanwork.Core.Types;
import Nyaanwork.Core.Utils.Flags;
import Nyaanwork.WindowSystem.Codes;

namespace Nyaanwork::WindowSystem::Backends::Base
{
	struct WindowInputState
	{
		template<typename Key>
		struct State
		{
			using MaskType = EnumTraits<Key>::MaskType;

			struct ConstView
			{
				operator bool() const
					{ return (*pos) & (1 << offset); }

				const u8* pos;
				u8 offset;
			};

			struct View
			{
				operator bool() const
					{ return (*pos) & (1 << offset); }

				View& operator=(bool val)
				{
					*pos = ((*pos) & ~(1 << offset)) | (val << offset);
					return *this;
				}

				u8* pos;
				u8 offset;
			};

			ConstView operator[](MaskType i) const
				{ return {data.data() + (i/8), static_cast<u8>(i%8)}; }

			View operator[](MaskType i)
				{ return {data.data() + (i/8), static_cast<u8>(i%8)}; }

			ConstView operator[](Key code) const
				{ return operator[](EnumTraits<Key>::cast(code)); }

			View operator[](Key code)
				{ return operator[](EnumTraits<Key>::cast(code)); }

			static constexpr usize size = Codes::Traits<Key>::count;
			std::array<u8, (size/8) + (size%8? 1 : 0)> data = {};
		};

		struct Keyboard
		{
			struct RecordedString
			{
				RecordedString() = default;

				RecordedString(const RecordedString&) = default;
				RecordedString& operator=(const RecordedString&) = default;

				RecordedString(RecordedString&& oth)
					: string(std::move(oth.string)), char_sizes(std::move(oth.char_sizes)),
					  over_remove(std::exchange(oth.over_remove, 0)) {};

				RecordedString& operator=(RecordedString&& oth)
				{
					if (this != &oth)
					{
						string = std::move(oth.string);
						char_sizes = std::move(oth.char_sizes);
						over_remove = std::exchange(oth.over_remove, 0);
					}

					return *this;
				}

				Str string;
				std::vector<u8> char_sizes;
				usize over_remove = 0;
			};

			Keyboard() = default;

			State<Codes::Key> state;
			RecordedString recorded;
		};

		struct Mouse
		{
			Mouse() = default;

			Mouse(const Mouse&) = default;
			Mouse& operator=(const Mouse&) = default;

			Mouse(Mouse&& oth)
				: position(oth.position),
				  motion(std::exchange(oth.motion, vec2<f32>(0))),
				  wheel(std::exchange(oth.wheel, vec2<f32>(0))),
				  state(oth.state) {};

			Mouse& operator=(Mouse&& oth)
			{
				if (this != &oth)
				{
					position = oth.position,
					motion = std::exchange(oth.motion, vec2<f32>(0)),
					wheel = std::exchange(oth.wheel, vec2<f32>(0)),
					state = oth.state;
				}

				return *this;
			}

			vec2<f32> position = {};
			vec2<f32> motion = {};
			vec2<f32> wheel = {};
			State<Codes::Button> state;
		};

		Keyboard keyboard;
		Mouse mouse;

		struct
		{
			bool on_surface = false;
			f32 pressure = 0.f;
			struct
			{
				vec2<f32> degrees = {};
				vec2<f32> radians = {};
			} tilt;
		} tablet;
	};
}
