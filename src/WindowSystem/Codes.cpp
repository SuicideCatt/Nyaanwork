export module Nyaanwork.WindowSystem.Codes;

import Nyaanwork.Core.Types;

namespace Nyaanwork::WindowSystem::Codes
{
	template<typename _Type>
	struct BaseTraits
	{
		using Type = _Type;
		static constexpr bool is_key = false;
		static constexpr bool is_button = false;
		static constexpr usize count = 0;
	};
}

export namespace Nyaanwork::WindowSystem::Codes
{
	enum class Key : u8
	{
		a, b, c, d, e, f, g, h, i, j, k, l, m,
		n, o, p, q, r, s, t, u, v, w, x, y, z,

		num_0, num_1, num_2, num_3, num_4, num_5, num_6, num_7, num_8, num_9,

		up, left, down, right,

		shift_left, shift_right,
		ctrl_left,  ctrl_right,
		alt_left,   alt_right,

		return_, escape, backspace, space, tab,

		minus, equals,
		bracket_left, bracket_right,
		slash, backslash,
		semicolon, apostrophe,
		grave, comma, period,

		f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12,

		insert, delete_, home, end, pageup, pagedown,

		np_0, np_1, np_2, np_3, np_4, np_5, np_6, np_7, np_8, np_9,
		np_divide, np_multiply, np_minus, np_plus, np_enter, np_period,
	};

	enum class Button : u8
	{
		left, middle, right,
		x1, x2,
	};

	template<typename Type>
	struct Traits;

	template<>
	struct Traits<Key> : BaseTraits<Key>
	{
		static constexpr bool is_key = true;
		static constexpr Nyaanwork::usize count =
			static_cast<Nyaanwork::u8>(Key::np_period) + 1;

		static bool is_printable(Key key)
		{
			switch (key)
			{
			case Key::a:
			case Key::b:
			case Key::c:
			case Key::d:
			case Key::e:
			case Key::f:
			case Key::g:
			case Key::h:
			case Key::i:
			case Key::j:
			case Key::k:
			case Key::l:
			case Key::m:
			case Key::n:
			case Key::o:
			case Key::p:
			case Key::q:
			case Key::r:
			case Key::s:
			case Key::t:
			case Key::u:
			case Key::v:
			case Key::w:
			case Key::x:
			case Key::y:
			case Key::z:
			case Key::num_0:
			case Key::num_1:
			case Key::num_2:
			case Key::num_3:
			case Key::num_4:
			case Key::num_5:
			case Key::num_6:
			case Key::num_7:
			case Key::num_8:
			case Key::num_9:
			case Key::space:
			case Key::tab:
			case Key::minus:
			case Key::equals:
			case Key::bracket_left:
			case Key::bracket_right:
			case Key::slash:
			case Key::backslash:
			case Key::semicolon:
			case Key::apostrophe:
			case Key::grave:
			case Key::comma:
			case Key::period:
				return true;

			default:
				return false;
			}
		}
	};

	template<>
	struct Traits<Button> : BaseTraits<Button>
	{
		static constexpr bool is_button = true;
		static constexpr Nyaanwork::usize count =
			static_cast<Nyaanwork::u8>(Button::x2) + 1;
	};
}
