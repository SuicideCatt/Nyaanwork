#include <iostream>
#include <print>

import Nyaanwork.Core.Math;
import Nyaanwork.Core.Types;
import Nyaanwork.WindowSystem;

using namespace Nyaanwork;

int main(int argc, char** argv)
{
	WindowSystem::Instance instance1;
	WindowSystem::Instance instance2;

	auto win1_ptr = instance1->create_window({
		.title = "instance1",
		.bounds = {
			.position = {200, 200},
			.resolution = {
				.value = {400, 400}
			},
		},
		.resizable = false,
	});
	auto& win1 = *win1_ptr;

	auto win2_ptr = instance2->create_window({
		.title = "instance2",
		.bounds = {
			.position = {600, 600},
			.resolution = {
				.value = {400, 400}
			},
		},
		.resizable = false,
	});
	auto& win2 = *win2_ptr;

	usize size = 0;
	while (!win1.should_close() || !win2.should_close())
	{
		auto get_str = [](StrV name, WindowSystem::Window& win)
		{
			return std::format(
				"{}: {} {} {:1d}{:1d}", name,
				win.position(), win.resolution(),
				win.keyboard_focused(), win.mouse_focused()
			);
		};

		auto win1_str = get_str("win1", win1);
		auto win2_str = get_str("win2", win2);

		size = max(size, win1_str.size() + win2_str.size() + 1);

		std::print(std::cout, "\r{1: <{0}} ", size, win1_str + ' ' + win2_str);
		std::cout.flush();
	}

	return 0;
}
