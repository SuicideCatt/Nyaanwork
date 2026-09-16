#include <chrono>
#include <print>
#include <thread>

import Nyaanwork.Core.Math;
import Nyaanwork.Core.Types;
import Nyaanwork.WindowSystem;
import Nyaanwork.Input.HLI;

using namespace Nyaanwork;
using namespace Input::Codes;

int main(int argc, char** argv)
{
	WindowSystem::Instance instance;

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
	});

	auto hli = make_shared<Input::HLI>();

	{
		using Input::Action;

		using Input::Axis1D;
		using Input::Axis2D;

		using P1D = Axis1D::Param;
		using P2D = Axis2D::Param;

		hli->action_add("test", Action(Key::l));
		hli->action_slot("test", 1, Action(Button::left));

		hli->axis_add<vec2<f32>>("test");

		auto s0 = Axis2D(P2D(Key::w, {0.f, 1.f}), P2D(Key::s, {0.f, -1.f}),
						 P2D(Key::a, {-1.f, 0.f}), P2D(Key::d, {1.f, 0.f}));
		auto s1 = Axis2D(P2D(Key::up, {0.f, 1.f}), P2D(Key::down, {0.f, -1.f}),
						 P2D(Key::left, {-1.f, 0.f}), P2D(Key::right, {1.f, 0.f}));

		hli->axis_slot("test", 0, s0);
		hli->axis_slot("test", 1, s1);

		hli->axis_add<f32>("zoom", Axis1D(P1D(Key::equals, 1.f), P1D(Key::minus, -1.f)),
								   Axis1D(P1D(Key::num_0, 5.f), P1D(Key::num_9, -5.f)));

		hli->axis_add<vec2<f32>>("mouse", Axis2D(P2D(Mouse::mouse_x, {1.f, 0.f}),
												 P2D(Mouse::mouse_y, {0.f, -1.f})));

		hli->axis_add<f32>("mouse_wheel", Axis1D(P1D(Mouse::mouse_wheel_y, 1.f)));
	}

	Input::HLI::Action old_action;
	Input::HLI::Axis2D old_axis2d = {};

	std::chrono::seconds time(0);
	Input::HLI::Axis1D zoom = {};

	Input::HLI::Axis2D old_mouse = {};
	Input::HLI::Axis1D old_mouse_wheel = {};

	while (!window->should_close())
	{
		hli->update(window);

		if (hli->state(Key::escape).release)
			window->close();

		auto action = hli->action("test");
		if (action.press != old_action.press
			|| action.hold != old_action.hold
			|| action.release != old_action.release)
		{
			std::println("{:d} {:d} {:d}",
						 action.press, action.hold, action.release);
		}
		old_action = action;

		auto axis2d = hli->axis<vec2<f32>>("test");
		if (axis2d != old_axis2d)
			std::println("{}", axis2d);
		old_axis2d = axis2d;

		auto now_clock = std::chrono::steady_clock::now().time_since_epoch();
		auto now = std::chrono::duration_cast<std::chrono::seconds>(now_clock);
		auto new_zoom = zoom + hli->axis<f32>("zoom");
		if (new_zoom != zoom && time != now) // change zoom every second
		{
			time = now;
			std::println("zoom: {}", zoom = new_zoom);
		}

		auto mouse = hli->axis<vec2<f32>>("mouse");
		if (mouse != old_mouse)
			std::println("mouse: {}", mouse);
		old_mouse = mouse;

		auto mouse_wheel = hli->axis<f32>("mouse_wheel");
		if (mouse_wheel != old_mouse_wheel)
			std::println("mouse_wheel: {}", mouse_wheel);
		old_mouse_wheel = mouse_wheel;

		std::this_thread::sleep_until(std::chrono::steady_clock::now() + std::chrono::milliseconds(16));
	}

	return 0;
}
