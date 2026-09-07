#include <Nyaanwork/WindowSystem.hpp>

#include <GL/gl.h>

#include <unordered_map>

import Nyaanwork.WindowSystem.OpenGL;

#include "vk.hpp"

using namespace Nyaanwork;
using namespace WindowSystem::Codes;

int main(int argc, char** argv)
{
	namespace OpenGL = Nyaanwork::WindowSystem::OpenGL;

	WindowSystem::Instance instance("nyaanwork_test");
	auto gli = instance->create_opengl_instance(OpenGL::Instance::API::gl);
	auto vk_instance = make_instance(instance);

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
		.widnow_instance_name = "main_window"
	};

	auto window = instance->create_window(window_info);
	auto glsurf = window->create_opengl_surface(gli);

	using GLC = OpenGL::Context;
	using GLP = GLC::Profile;
	auto glc = make_shared<GLC>(gli, GLP::core, vec2<u8>(3, 2));

	glc->swap_interval(1);
	glc->make_current(glsurf);
	glClearColor(1.f, 0.67058823529412f, 0.87058823529412f, 1.f);

	UPtr<Context> context = make_unique<Context>(*vk_instance, window);

	struct State
	{
		bool press = false, hold = false, release = false;
	};
	std::unordered_map<Key, State> keyboard;

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

	bool vk_render = false;
	while (!window->should_close())
	{
		auto input = window->input_state();

		update(keyboard, input, &WindowSystem::InputState::key);

		if (keyboard[Key::escape].release)
			window->close();

		if (keyboard[Key::r].release)
			vk_render = !vk_render;

		auto draw = [](auto& glc, auto& glsurf, auto& window)
		{
			glc->make_current(glsurf);
			auto [x, y] = window->resolution();
			glViewport(0, 0, x, y);
			glClear(GL_COLOR_BUFFER_BIT);
			glsurf->swap_buffers();
		};

		if (vk_render)
			context->render_frame();
		else
			draw(glc, glsurf, window);
	}

	return 0;
}
