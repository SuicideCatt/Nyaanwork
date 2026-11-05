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
	WindowSystem::Instance instance("nyaanwork_test");
	auto gl_instance =
		instance->create_opengl_instance(WindowSystem::OpenGL::Instance::API::gl);

	auto window = instance->create_window({
		.title = "HighOpenGL: Hello triangle",
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
