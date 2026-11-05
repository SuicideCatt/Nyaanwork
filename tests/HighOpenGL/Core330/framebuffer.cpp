#include <array>
#include <atomic>
#include <cstddef>
#include <mutex>

import Nyaanwork.Asset.Image;
import Nyaanwork.Core.Math;
import Nyaanwork.Core.Types;
import Nyaanwork.HighOpenGL.Core330;
import Nyaanwork.WindowSystem.OpenGL;
import Nyaanwork.WindowSystem;

using namespace Nyaanwork;
namespace HOGL = HighOpenGL::Core330;

struct Vertex
{
	vec2<f32> position;
	vec2<f32> uv;
};

struct Data
{
	static constexpr StrV vertex =
R"(#version 330 core
layout (location = 0) in vec2 vertex_pos;
layout (location = 1) in vec2 vertex_uv;

out VS_OUT
{
	vec2 uv;
} to_fragment;

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
	to_fragment.uv = vertex_uv;
	gl_Position = proj_view * model * vec4(vertex_pos, 0, 1);
})";

	static constexpr StrV vertex_screen =
R"(#version 330 core
layout (location = 0) in vec2 vertex_pos;
layout (location = 1) in vec2 vertex_uv;

out VS_OUT
{
	vec2 uv;
} to_fragment;

void main()
{
	to_fragment.uv = vec2(vertex_uv.x, -vertex_uv.y);
	gl_Position = vec4(vertex_pos*2, 0, 1);
})";

	static constexpr StrV fragment =
R"(#version 330 core
out vec4 fragment_color;

in VS_OUT
{
	vec2 uv;
} to_fragment;

uniform sampler2D texture_color;

void main()
{
	fragment_color = vec4(texture(texture_color, to_fragment.uv).rgb, 1);
})";

	struct Square
	{
		static constexpr std::array vertices = {
			Vertex(vec2<f32>(-0.5f, 0.5f), vec2<f32>(0.f)),
			Vertex(vec2<f32>(0.5f), vec2<f32>(1.f, 0.f)),
			Vertex(vec2<f32>(-0.5f), vec2<f32>(0.f, 1.f)),
			Vertex(vec2<f32>(0.5f, -0.5f), vec2<f32>(1.f)),
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
		.title = "HighOpenGL: Framebuffers",
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
			},
			{
				.offset = offsetof(Vertex, uv),
				.count = 2,
				.type = HOGL::VertexDescriptor::AttributeDescriptor::Type::f32,
			}
		}
	};

	HOGL::StaticMesh<Vertex, u8> square(
		context, vertex_descriptor,
		HOGL::PrimitiveTopology::triangles, Data::Square()
	);

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
				"camera.projection", "camera.view", "model",
			},
			.textures = {
				{"texture_color", 0},
			},
		},
	};

	HOGL::Shader::Program program(context, program_info);

	auto screen_slot = context.limits().texture_slots-1;

	HOGL::Shader::Module vertex_screen(context, {.source = Data::vertex_screen});
	HOGL::Shader::Program screen_program;
	{
		program_info.modules[0] = vertex_screen;
		program_info.uniform_layout.textures[0].second = screen_slot;
		program_info.uniform_layout.preload_locations = {};

		screen_program = {context, program_info};
	}

	using Asset::Pixels::RGB;
	Asset::Image<RGB> image({4, 4}, {
		0xFF0000, 0xFF0000, 0x00FF00, 0x00FF00,
		0xFF0000, 0xFF0000, 0x00FF00, 0x00FF00,
		0x0000FF, 0x0000FF, 0xFFFFFF, 0xFFFFFF,
		0x0000FF, 0x0000FF, 0xFFFFFF, 0xFFFFFF,
	});

	HOGL::Texture2D<RGB> texture(context, {
		.filter = {
			.mag = HOGL::Filter::Type::nearest,
		},
	}, image);

	HOGL::Texture2D<RGB> color(context, {
		.filter = {
			.mag = HOGL::Filter::Type::linear,
		},
		.target = HOGL::TextureTarget::framebuffer,
	}, window->resolution(), {});

	HOGL::Texture2D<HOGL::DepthStencil> depth(context, {
		.filter = {
			.mag = HOGL::Filter::Type::linear,
		},
		.target = HOGL::TextureTarget::framebuffer,
	}, window->resolution(), {});

	HOGL::Framebuffer::Info<RGB> framebuffer_info = {
		.colors = {color},
		.depth = depth,
	};
	HOGL::Framebuffer framebuffer(context, framebuffer_info);

	HOGL::CommandBuffer cmd_buf(surface, context);
	cmd_buf.clear_config({
		.flags = HOGL::CommandBuffer::Clear::Flags::color,
		.color = Asset::Colors<Asset::Pixels::RGBA>::dark_gray,
	});

	using W = WindowSystem::Window;

	std::atomic_bool updated = true;
	std::mutex update_mutex;
	W::Resolution resolution = window->resolution();

	using S = W::Slots;
	window->signal_connect<S::resized>(
		[&updated, &update_mutex, &old=resolution](W&, W::Resolution resolution)
		{
			std::lock_guard l(update_mutex);
			old = resolution;
			updated = true;
		});

	while (!window->should_close())
	{
		if (updated) [[unlikely]]
		{
			std::lock_guard l(update_mutex);
			cmd_buf.viewport(resolution);
			color.update<true, false>(resolution, {});
			depth.update(resolution, {});
			updated = false;

			cmd_buf.reset();
		}

		cmd_buf.bind_framebuffer(framebuffer);
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

		cmd_buf->set("model", Matrix4<f32>(1.f));
		cmd_buf.bind_texture_2d(texture, 0);
		cmd_buf.bind_mesh(square);
		cmd_buf.draw();

		cmd_buf.bind_framebuffer();
		cmd_buf.clear();

		cmd_buf.bind_shader_program(screen_program);
		cmd_buf.bind_texture_2d(color, screen_slot);
		cmd_buf.draw();

		cmd_buf.swap();
	}

	return 0;
}
