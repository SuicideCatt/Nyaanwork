#include <print>
#include <string>

import Nyaanwork.Core.Types;
import Nyaanwork.Core.StringConvertor;

using namespace Nyaanwork;

template<typename Char>
bool check(StrV name, std::basic_string_view<Char> from)
{
	StringConvertor<Char> convertor;

	auto to = convertor.convert(from);
	auto from2 = convertor.convert(to);

	auto check = from == from2;
	std::println("{}: {}", name, check);
	if (!check)
	{
		auto print = [](StrV name, auto& str)
		{
			std::println("  {}({}):", name, str.size());
			for (const auto& c : str)
			{
				std::print(" {:0>8x}", static_cast<u32>(c));
			}
			std::println();
		};

		print("orig", from);
		print("converted", from2);
	}

	return check;
}

int main(int argc, char** argv)
{
	#ifdef _WIN64
	// FIXME: In MinGW not work std::locale for en_US.utf8
	// NOTE: WindowSystem automatically set to en_US.utf8 global locale
	// librarty user don't set itself
	std::setlocale(LC_ALL, "en_US.utf8");
	#endif

	check<wchar_t>("wchar", L"Test Тест");
	check<char16_t>("char16", u"Test Тест");
	check<char32_t>("char32", U"Test Тест");

	return 0;
}
