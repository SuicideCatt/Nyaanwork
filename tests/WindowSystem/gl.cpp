#include <Nyaanwork/WindowSystem.hpp>

#include <GL/gl.h>

#include <memory>
#include <print>
#include <unordered_map>

import Nyaanwork.Core.Math;
import Nyaanwork.Core.Types;

import Nyaanwork.WindowSystem;
import Nyaanwork.WindowSystem.OpenGL;

using namespace Nyaanwork;
using namespace WindowSystem::Codes;

int main(int argc, char** argv)
{
	namespace OpenGL = Nyaanwork::WindowSystem::OpenGL;

	WindowSystem::Instance instance("nyaanwork_test");
	auto gli = instance->create_opengl_instance(OpenGL::Instance::API::gl);

	auto [displays, primary] = instance->displays();
	auto& display = displays[primary];

	vec2<u16> res = {640, 640};
	auto pos = vec2<i16>((display.resolution-res)/2_u16) + display.position;

	using Win = WindowSystem::Window;
	Win::Info window_info = {
		.title = "Test1",
		.bounds = {
			.position = pos,
			.resolution = {
				.value = res,
			}
		},
		.resizable = false,
		.widnow_instance_name = "window1"
	};

	auto window1 = instance->create_window(window_info);
	auto glsurf1 = window1->create_opengl_surface(gli);

	window_info.title.back() = '2';
	window_info.widnow_instance_name.value().back() = '2';
	auto window2 = instance->create_window(window_info);
	auto glsurf2 = window2->create_opengl_surface(gli);

	auto key_focused = [](Win& win) noexcept
		{ std::println("{}: key focused", win.title()); };
	auto key_unfocused = [](Win& win) noexcept
		{ std::println("{}: key unfocused", win.title()); };
	auto mouse_focused = [](Win& win) noexcept
		{ std::println("{}: mouse focused", win.title()); };
	auto mouse_unfocused = [](Win& win) noexcept
		{ std::println("{}: mouse unfocused", win.title()); };

	window1->signal_connect<Win::Slots::focus_gained>(key_focused);
	window1->signal_connect<Win::Slots::focus_lost>(key_unfocused);
	window1->signal_connect<Win::Slots::mouse_enter>(mouse_focused);
	window1->signal_connect<Win::Slots::mouse_leave>(mouse_unfocused);

	window2->signal_connect<Win::Slots::focus_gained>(key_focused);
	window2->signal_connect<Win::Slots::focus_lost>(key_unfocused);
	window2->signal_connect<Win::Slots::mouse_enter>(mouse_focused);
	window2->signal_connect<Win::Slots::mouse_leave>(mouse_unfocused);

	using GLC = OpenGL::Context;
	using GLP = GLC::Profile;
	auto glc1 = gli->create_context(GLP::core, vec2<u8>(3, 2));
	auto glc2 = gli->create_context(GLP::compatibility, vec2<u8>(3, 2));

	glc1->swap_interval(1);
	glc1->make_current(glsurf1);
	glClearColor(1.f, 0.67058823529412f, 0.87058823529412f, 1.f);

	glc2->swap_interval(1);
	glc2->make_current(glsurf2);
	glClearColor(0.3f, 0.3f, 0.3f, 1.f);

	window1->show_mouse(false);

	struct State
	{
		bool press = false, hold = false, release = false;
	};
	std::unordered_map<Key, State> keyboard;
	std::unordered_map<Button, State> mouse;

	auto update = [](auto& states, auto& inputer, auto&& proj)
	{
		for (auto& [kb, state] : states)
		{
			bool kb_state = (inputer.*proj)(kb);
			auto& [p, h, r] = state;

			p = kb_state == true && kb_state != h;
			r = kb_state == false && kb_state != h;
			h = kb_state;
		}
	};

	while (!window1->should_close())
	{
		auto input = window2->input_state();

		update(keyboard, input, &WindowSystem::InputState::key);
		update(mouse, input, &WindowSystem::InputState::button);

		if (keyboard[Key::escape].release)
			window1->close();

		if (keyboard[Key::m].release)
			window1->show_mouse(!window1->show_mouse());

		if (keyboard[Key::g].release)
			window1->grab_mouse(!window1->grab_mouse());

		if (keyboard[Key::c].release)
			window1->centralize_mouse(!window1->centralize_mouse());

		if (keyboard[Key::p].release)
			window1->capture_mouse(!window1->capture_mouse());

		auto draw = [](auto& glc, auto& glsurf, auto& window)
		{
			glc->make_current(glsurf);
			auto [x, y] = window->resolution();
			glViewport(0, 0, x, y);
			glClear(GL_COLOR_BUFFER_BIT);
			glsurf->swap_buffers();
		};

		draw(glc1, glsurf1, window1);
		draw(glc2, glsurf2, window2);
	}

	return 0;
}
