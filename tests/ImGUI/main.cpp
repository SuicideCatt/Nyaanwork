#include <GL/gl.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>

#include <atomic>
#include <vector>

import Nyaanwork.Core.Math;
import Nyaanwork.Core.Types;
import Nyaanwork.ImGUI;
import Nyaanwork.WindowSystem.OpenGL;
import Nyaanwork.WindowSystem;

using namespace Nyaanwork;
using namespace WindowSystem::Codes;

int main()
{
	WindowSystem::Instance instance("nyaanwork_test");

	auto [displays, primary] = instance->displays();
	auto gli =
		instance->create_opengl_instance(WindowSystem::OpenGL::Instance::API::gl);

	auto& display = displays[primary];

	vec2<u16> res = {640, 640};
	auto pos = vec2<i16>((display.resolution-res)/2_u16) + display.position;

	auto window = instance->create_window({
		.title = "Hello Nyaanwork + ImGui",
		.bounds = {
			.position = pos,
			.resolution = {
				.value = res,
			}
		},
		.resizable = false,
		.widnow_instance_name = "main_window"
	});

	using GLP = WindowSystem::OpenGL::Context::Profile;
	auto gl = window->create_opengl_surface_context(gli, GLP::core, {3, 3});
	gl->make_current();
	gl->swap_interval(1);
	glClearColor(0, 0, 0, 1);

	ImGUI nyaanwork_imgui;
	ImGui_ImplOpenGL3_Init();

	ImGui::StyleColorsDark();

	vec2<int> position;
	vec2<int> resolution;
	std::atomic_bool position_changed = true;
	std::atomic_bool resolution_changed = true;
	window->signal_connect<WindowSystem::Window::Slots::resized>(
		[&resolution_changed](WindowSystem::Window&, WindowSystem::Window::Resolution)
		{
			resolution_changed = true;
		});
	window->signal_connect<WindowSystem::Window::Slots::moved>(
		[&position_changed](WindowSystem::Window&, WindowSystem::Window::Position)
		{
			position_changed = true;
		});

	auto title = window->title();
	std::vector<char> input_name(max<usize>(title.size(), 25));
	std::ranges::copy(title, input_name.begin());

	bool keyboard_window = false;
	bool mouse_window = false;
	bool demo_window = false;

	while (!window->should_close())
	{
		auto input = window->input_state();

		bool keyboard_focused = window->keyboard_focused();
		bool mouse_focused = window->mouse_focused();
		bool minimized = window->minimized();
		bool resizable = window->resizable();
		bool fullscreen = window->fullscreen();
		bool borderless = window->borderless();
		bool string_recording = window->string_recording();
		bool should_close = window->should_close();

		vec2<float> mouse_pos = input.position();
		vec2<float> mouse_wheel = input.wheel();
		vec2<float> mouse_motion = input.motion();
		bool show_mouse = window->show_mouse();
		bool grab_mouse = window->grab_mouse();
		bool centralize_mouse = window->centralize_mouse();
		bool capture_mouse = window->capture_mouse();

		nyaanwork_imgui.update(window, input);

		ImGui_ImplOpenGL3_NewFrame();
		nyaanwork_imgui.new_frame(window);

		ImGui::Begin("Hello Nyaanwork + ImGui");
		if (input.key(Key::r) || resolution_changed)
		{
			ImGui::SetWindowPos({40, 40});
			ImGui::SetWindowSize({440, 360});
		}

		if (resolution_changed)
		{
			auto& [w, h] = resolution = window->resolution();
			glViewport(0, 0, w, h);

			resolution_changed = false;
		}

		if (position_changed)
			position = window->position();

		glClear(GL_COLOR_BUFFER_BIT);

		ImGui::Text("Backend: %s (%s)",
					instance->short_name().c_str(),
					instance->full_name().c_str());
		ImGui::Text("");

		ImGui::Text("Base");
		if (ImGui::InputText("Name", input_name.data(), input_name.size()))
			window->title(Str(input_name.data()));
		if (ImGui::DragInt2("resolution", &resolution.x))
			window->resolution(resolution);
		if (ImGui::DragInt2("position", &position.x))
			window->position(position);
		ImGui::Checkbox("keyboard_focused", &keyboard_focused);
		ImGui::Checkbox("mouse_focused", &mouse_focused);
		ImGui::Checkbox("minimized", &minimized);
		if (ImGui::Checkbox("resizable", &resizable))
			window->resizable(resizable);
		if (ImGui::Checkbox("fullscreen", &fullscreen))
			window->fullscreen(fullscreen);
		if (ImGui::Checkbox("borderless", &borderless))
			window->borderless(borderless);
		if (ImGui::Checkbox("string_recording", &string_recording))
			window->string_recording(string_recording);
		if (ImGui::Checkbox("should_close", &should_close) && should_close)
			window->close();

		auto cpos = ImGui::GetCursorPos();
		if (ImGui::Button("Keyboard"))
			keyboard_window = !keyboard_window;

		cpos.x += 70;
		ImGui::SetCursorPos(cpos);
		if (ImGui::Button("Mouse"))
			mouse_window = !mouse_window;

		cpos.x += 49;
		ImGui::SetCursorPos(cpos);
		if (ImGui::Button("ImGui::Demo"))
			demo_window = !demo_window;

		ImGui::End();

		if (demo_window)
			ImGui::ShowDemoWindow();

		if (keyboard_window)
		{
			ImGui::Begin("Keyboard");
			ImGui::SetWindowSize({595, 370});

			ImGui::Text("Maybe later");

			ImGui::End();
		}

		if (mouse_window)
		{
			bool left = input.button(Button::left);
			bool middle = input.button(Button::middle);
			bool right = input.button(Button::right);
			bool x1 = input.button(Button::x1);
			bool x2 = input.button(Button::x2);

			ImGui::Begin("Mouse");
			ImGui::SetWindowSize({220, 240});

			auto cpos = ImGui::GetCursorPos();
			auto& [x, y] = cpos;
			ImGui::Checkbox("l", &left);
			x += 50;
			ImGui::SetCursorPos(cpos);
			ImGui::Checkbox("m", &middle);
			x += 50;
			ImGui::SetCursorPos(cpos);
			ImGui::Checkbox("r", &right);

			cpos = ImGui::GetCursorPos();
			x += 20;
			ImGui::SetCursorPos(cpos);
			ImGui::Checkbox("x1", &x1);
			x += 50;
			ImGui::SetCursorPos(cpos);
			ImGui::Checkbox("x2", &x2);

			if (ImGui::DragFloat2("position", &mouse_pos.x))
				window->warp_mouse(mouse_pos);

			ImGui::DragFloat2("whell", &mouse_wheel.x);
			ImGui::DragFloat2("motion", &mouse_motion.x);

			if (ImGui::Checkbox("show_mouse", &show_mouse))
				window->show_mouse(show_mouse);
			if (ImGui::Checkbox("grab_mouse", &grab_mouse))
				window->grab_mouse(grab_mouse);
			if (ImGui::Checkbox("centralize_mouse", &centralize_mouse))
				window->centralize_mouse(centralize_mouse);
			if (ImGui::Checkbox("capture_mouse", &capture_mouse))
				window->capture_mouse(capture_mouse);

			ImGui::End();
		}

		if (input.is_pen_on_surface())
		{
			ImGui::Begin("Tablet");
			ImGui::SetWindowPos({40, 40});
			ImGui::SetWindowSize({320, 130});

			ImGui::Text("Pressure");
			ImGui::ProgressBar(input.pen_pressure());

			auto tilt_d = input.pen_tilt_degrees();
			ImGui::SliderFloat2("Tilt degrees", &tilt_d.x, 0, 60);

			auto tilt_r = input.pen_tilt_radians();
			ImGui::SliderFloat2("Tilt radians", &tilt_r.x, 0, radians(60.f));

			ImGui::End();
		}

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		gl->swap_buffers();
	}

	ImGui_ImplOpenGL3_Shutdown();

	return 0;
}
