#include <Nyaanwork/HighOpenGL.hpp>

#include <print>

import Nyaanwork.HighOpenGL.Core330;
import Nyaanwork.WindowSystem.OpenGL;
import Nyaanwork.WindowSystem;

using namespace Nyaanwork;
namespace HOGL = HighOpenGL::Core330;

int main(int argc, char** argv)
{
	WindowSystem::Instance instance("nyaanwork_test");
	auto gl_instance =
		instance->create_opengl_instance(WindowSystem::OpenGL::Instance::API::gl);

	auto window = instance->create_window({
		.title = "HighOpenGL: Limits check",
		.bounds = {
			.position = {100, 100},
			.resolution = {.value = {1280, 720}},
		},
		.resizable = true,
		.widnow_instance_name = "main_window"
	});

	HOGL::Context context(gl_instance);
	auto surface = window->create_opengl_surface(gl_instance);
	context.swap_interval(1);
	context.make_current(surface);

	std::println("GPU:");
	std::println("  vendor: {}", context.vendor());
	std::println("  renderer: {}", context.renderer());

	auto lims = context.limits();
	std::println("\nLimits:");
	#define print(name) std::println("  " #name ": {}", lims.name)
	print(texture_slots);
	print(texture_size);
	print(uniform_buffer_bindings);
	print(uniform_block_size);

	#ifndef NYAANWORK_HIGH_OPENGL_CORE330_DYNAMIC_LIMITS
	{
		auto lims = context.dynamic_limits();
		std::println("\nGPU Limits:");
		print(texture_slots);
		print(texture_size);
		print(uniform_buffer_bindings);
		print(uniform_block_size);
	}
	#endif

	return 0;
}
