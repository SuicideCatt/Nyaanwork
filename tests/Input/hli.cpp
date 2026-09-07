#include <print>

import Nyaanwork.Core.Math;
import Nyaanwork.Core.Types;
import Nyaanwork.WindowSystem;
import Nyaanwork.Input.HLI;

using namespace Nyaanwork;
using namespace WindowSystem::Codes;

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
	}

	Input::HLI::Action old_action;
	Input::HLI::Axis2D old_axis2d = {};
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
	}

	return 0;
}
