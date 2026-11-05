#include <array>

import Nyaanwork.Asset.Image;
import Nyaanwork.Core.Math;
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

struct Camera
{
	mat4 view;
	mat4 projection;
};

uniform Camera camera;
uniform mat4 model;

void main()
{
	mat4 proj_view = camera.projection * camera.view;
	gl_Position = proj_view * model * vec4(vertex_pos, 0, 1);
})";

	static constexpr StrV fragment =
R"(#version 330 core
out vec4 fragment_color;

uniform vec3 color;

void main()
{
	fragment_color = vec4(color, 1);
})";

	static constexpr std::array triangle = {
		vec2<f32>(-0.5f, -0.5f),
		vec2<f32>(0.5f, -0.5f),
		vec2<f32>(0.0f, 0.5f),
	};

	struct Square
	{
		static constexpr std::array vertices = {
			vec2<f32>(-0.5f, 0.5f),
			vec2<f32>(0.5f),
			vec2<f32>(-0.5f),
			vec2<f32>(0.5f, -0.5f),
		};

		static constexpr std::array indices = {
			0_u8, 1_u8, 2_u8,
			1_u8, 3_u8, 2_u8,
		};
	};
};

int main(int argc, char** argv)
{
	WindowSystem::Instance instance("nyaanwork_test");
	auto gl_instance =
		instance->create_opengl_instance(WindowSystem::OpenGL::Instance::API::gl);

	auto window = instance->create_window({
		.title = "HighOpenGL: Multi-mesh",
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

	HOGL::VertexDescriptor vertex_descriptor = {
		.type_size = sizeof(Vertex),
		.attributes = {
			{
				.count = 2,
				.type = HOGL::VertexDescriptor::AttributeDescriptor::Type::f32,
			}
		}
	};

	HOGL::StaticMesh<Vertex, void> triangle(
		context, vertex_descriptor,
		HOGL::PrimitiveTopology::triangles, Data::triangle
	);

	HOGL::StaticMesh<Vertex, u8> square(
		context, vertex_descriptor,
		HOGL::PrimitiveTopology::triangles, Data::Square()
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
				.vertex_descriptor = vertex_descriptor
			},
			.uniform_layout = {
				.preload_locations = {
					"camera.projection", "camera.view", "model", "color",
				},
			},
		};

		program = {context, program_info};
	}

	HOGL::CommandBuffer cmd_buf(surface, context);
	cmd_buf.clear_config({
		.flags = HOGL::CommandBuffer::Clear::Flags::color,
		.color = Asset::Colors<Asset::Pixels::RGBA>::dark_gray,
	});

	while (!window->should_close())
	{
		cmd_buf.viewport(window->resolution());
		cmd_buf.clear();

		cmd_buf.bind_shader_program(program);
		{
			vec2<f32> res = window->resolution();
			f32 aspect = res.x/res.y;

			f32 scale = 2.f;
			vec2<f32> lr = scale * vec2<f32>(-1.f, 1.f);
			vec2<f32> td = lr/aspect;

			cmd_buf->set("camera.projection", ortho(lr.x, lr.y, td.x, td.y, 0.1f, 100.f));
			cmd_buf->set("camera.view", translate(Matrix4<f32>(1.f), vec3<f32>(0.f, 0.f, -10.f)));
		}

		cmd_buf->set("model", translate(Matrix4<f32>(1.f), vec3<f32>(-1.f, 0.f, 0.f)));
		cmd_buf->set("color", static_cast<vec3<f32>>(Asset::Colors<Asset::Pixels::RGB>::blue));
		cmd_buf.bind_mesh(triangle);
		cmd_buf.draw();

		cmd_buf->set("model", translate(Matrix4<f32>(1.f), vec3<f32>(1.f, 0.f, 0.f)));
		cmd_buf->set("color", static_cast<vec3<f32>>(Asset::Colors<Asset::Pixels::RGB>::yellow));
		cmd_buf.bind_mesh(square);
		cmd_buf.draw();

		cmd_buf.swap();
	}

	return 0;
}
