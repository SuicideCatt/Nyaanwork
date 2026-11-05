<p align="center">
	<img src="logo.svg" width=100 alt=Nyaanwork>
</p>

# Nyaanwork

Windowing framework in C++23 and C++ Modules *(only)*
for make games or game engines, nyaa~~~~

If you want to help, see [`TODO.md`](TODO.md) or issues.

## How to install and init

You'll want to create base **C++**/**CMake** project and initialize git.

1. Install dependencies:

	On ArchLinux:
	```bash
	# Core
	sudo pacman -Sy ninja cmake python clang

	# WindowSystem
	sudo pacman -Sy libglvnd vulkan-headers vulkan-icd-loader

	# WindowSystem/Linux/XCB
	sudo pacman -Sy libx11 libxcb libxkbcommon libxkbcommon-x11 xcb-util-cursor
	```

2. Add Nyaanwork as git submodule:
```bash
git submodule add https://codeberg.org/SuicideCatt/Nyaanwork path/to/third-party/modules/Nyaanwork
# or
git submodule add https://github.com/SuicideCatt/Nyaanwork path/to/third-party/modules/Nyaanwork

git submodule update --init --recursive --depth=1
```

3. Add to main `CMakeLists.txt`:
```cmake
cmake_minimum_required(VERSION 3.30.9)

project(ProjectName LANGUAGES CXX) # init you project

add_subdirectory(path/to/Nyaanwork) # init Nyaanwork and cmake-kconfig

# You can add self made Kconfig menu
# read:
# 1. https://github.com/SuicideCatt/cmake-kconfig
# 2. https://www.kernel.org/doc/html/latest/kbuild/kconfig-language.html
#kconfig_add_menu(name Kconfig)

add_executable(${PROJECT_NAME} main.cpp) # add you executable
target_link_libraries(${PROJECT_NAME} PUBLIC Nyaanwork) # link Nyaanwork

# ninja run-main: rebuild(if needed) and run executable target
nyaanwork_run_target(main ${PROJECT_NAME} [args...])

# ninja run # auto call to 'ninja run-main', very convenient when you have a lot of run targets!
nyaanwork_run_target_default(main)
```

4. Init build environment:
```bash
export CC=/usr/bin/clang CXX="/usr/bin/clang++"
mkdir build && cd build
cmake .. -G Ninja
ninja menuconfig
```

5. Configure Nyaanwork on Kconfig setup menu.

## Hello Nyaanwork window and input

Nyaanwork has Core module, this module always enabled,
WindowSystem module provide Linux (X11) and Windows backeds,
and Input module provide HLI.

```cpp
import Nyaanwork.Input.HLI;
import Nyaanwork.WindowSystem;

int main()
{
	using namespace Nyaanwork;

	WindowSystem::Instance instance;
	auto window = instance->create_window({
		.title = "Hello Nyaanwork!",
		.bounds = {
			.position = {300, 500},
			.resolution = {.value = {400, 400}}
		},
		.resizable = false,
	});

	Input::HLI hli;
	while (!window->should_close())
	{
		using namespace Input::Codes;

		hli.update(window);

		if (hli.state(Key::escape).release)
		{
			window->close();
			break;
		}
	}

	return 0;
}
```

### ImGUI (with OpenGL)

Nyaanwork containts Dear ImGui like git submodule, and automatically link ImGui
with render backeds enabled in WindowSystem

Enable ImGUI module in Kconfig: `(Top)` -> `Nyaanwork` -> `Enable ImGUI module`

```cpp
#include <GL/gl.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>

import Nyaanwork.Core.Types;
import Nyaanwork.ImGUI;
import Nyaanwork.Input.HLI;
import Nyaanwork.WindowSystem.OpenGL;
import Nyaanwork.WindowSystem;

int main()
{
	using namespace Nyaanwork;
	namespace OpenGL = Nyaanwork::WindowSystem::OpenGL;

	WindowSystem::Instance instance;
	auto gl_instance = instance->create_opengl_instance(OpenGL::Instance::API::gl);

	auto window = instance->create_window({
		.title = "Hello Nyaanwork + Dear ImGui!",
		.bounds = {
			.position = {100, 100},
			.resolution = {.value = {800, 800}}
		},
		.resizable = true,
	});
	auto gl = window->create_opengl_surface_context(gl_instance, OpenGL::Context::Profile::core, {3, 3});
	gl->make_current();
	gl->swap_interval(1);
	glClearColor(0, 0, 0, 1);

	ImGUI nyaanwork_imgui;
	ImGui_ImplOpenGL3_Init();

	ImGui::StyleColorsDark();

	Input::HLI hli;
	while (!window->should_close())
	{
		using namespace Input::Codes;

		auto input_state = window->input_state();

		hli.update(window, input_state);
		nyaanwork_imgui.update(window, input_state);

		if (hli.state(Key::escape).release)
		{
			window->close();
			break;
		}

		auto [w, h] = window->resolution();
		glViewport(0, 0, w, h);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_NewFrame();
		nyaanwork_imgui.new_frame(window);

		ImGui::ShowDemoWindow();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		gl->swap_buffers();
	}

	ImGui_ImplOpenGL3_Shutdown();

	return 0;
}
```

## Hello triangle with HighOpenGL (HOGL)
```cpp
#include <array>

import Nyaanwork.Asset.Image;
import Nyaanwork.Core.Types;
import Nyaanwork.HighOpenGL.Core330;
import Nyaanwork.WindowSystem.OpenGL;
import Nyaanwork.WindowSystem;

using namespace Nyaanwork;
namespace HOGL = HighOpenGL::Core330;

using Vertex = vec2<f32>;

struct Data
{
	static constexpr StrV vertex =
R"(#version 330 core
layout (location = 0) in vec2 vertex_pos;

void main()
{
	gl_Position = vec4(vertex_pos, 0, 1);
})";

	static constexpr StrV fragment =
R"(#version 330 core
out vec4 fragment_color;

void main()
{
	fragment_color = vec4(1, 1, 1, 1);
})";

	static constexpr std::array vertices = {
		Vertex(-0.5f, -0.5f),
		Vertex(0.5f, -0.5f),
		Vertex(0.0f, 0.5f),
	};
};

int main(int argc, char** argv)
{
	WindowSystem::Instance instance;
	auto gl_instance =
		instance->create_opengl_instance(WindowSystem::OpenGL::Instance::API::gl);

	auto window = instance->create_window({
		.title = "HighOpenGL",
		.bounds = {
			.position = {100, 100},
			.resolution = {.value = {1280, 720}},
		},
		.resizable = true,
	});

	HOGL::Context context(gl_instance);
	auto surface = window->create_opengl_surface(gl_instance);
	context.swap_interval(1);
	context.make_current(surface);

	HOGL::StaticMesh<vec2<f32>, void> mesh(
		context,
		{
			.type_size = sizeof(Vertex),
			.attributes = {
				{
					.count = 2,
					.type = HOGL::VertexDescriptor::AttributeDescriptor::Type::f32,
				}
			}
		},
		HOGL::PrimitiveTopology::triangles, Data::vertices
	);

	HOGL::Shader::Program program;
	{
		HOGL::Shader::Module vertex(context, {.source = Data::vertex});
		HOGL::Shader::Module fragment(context, {
			.type = HOGL::Shader::Module::Type::fragment,
			.source = Data::fragment
		});

		HOGL::Shader::Program::Info program_info = {
			.modules = {vertex, fragment},
			.layout = {
				.vertex_descriptor = mesh.vertex_descriptor(),
			},
		};

		program = {context, program_info};
	}

	HOGL::CommandBuffer cmd_buf(surface, context);
	cmd_buf.clear_config({
		.flags = HOGL::CommandBuffer::Clear::Flags::color,
		.color = Asset::Colors<Asset::Pixels::RGBA>::magenta,
	});

	while (!window->should_close())
	{
		cmd_buf.viewport(window->resolution());
		cmd_buf.clear();

		cmd_buf.bind_shader_program(program);
		cmd_buf.bind_mesh(mesh);
		cmd_buf.draw();

		cmd_buf.swap();
	}

	return 0;
}
```
