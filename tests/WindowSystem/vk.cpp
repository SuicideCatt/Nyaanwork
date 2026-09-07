#include "vk.hpp"

using namespace WindowSystem::Codes;

int main(int argc, char** argv)
{
	WindowSystem::Instance instance("nyaanwork_test");
	auto vk_instance = make_instance(instance);

	auto [displays, primary] = instance->displays();
	auto& display = displays[primary];

	vec2<u16> res = {640, 640};
	auto pos = vec2<i16>((display.resolution-res)/2_u16) + display.position;

	auto window = instance->create_window({
		.title = "Test",
		.bounds = {
			.position = pos,
			.resolution = {
				.value = res,
			}
		},
		.resizable = false,
		.widnow_instance_name = "main_window"
	});

	UPtr<Context> context = make_unique<Context>(*vk_instance, window);

	while (!window->should_close())
	{
		if (window->key_state(Key::escape))
			window->close();

		context->render_frame();
	}

	return 0;
}
