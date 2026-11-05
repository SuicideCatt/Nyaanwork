#include <array>
#include <atomic>
#include <cstddef>
#include <mutex>

import Nyaanwork.Asset.Image;
import Nyaanwork.Core.Math;
import Nyaanwork.Core.Types;
import Nyaanwork.Core.Utils.Flags;
import Nyaanwork.Core.Utils.Ptr;
import Nyaanwork.HighOpenGL.Core330;
import Nyaanwork.WindowSystem.OpenGL;
import Nyaanwork.WindowSystem;

using namespace Nyaanwork;
namespace HOGL = HighOpenGL::Core330;

using Asset::Pixels::RGB;

struct Vertex
{
	vec3<f32> position;
	vec3<f32> color;
};

struct ScreenVertex
{
	vec2<f32> position;
	vec2<f32> uv;
};

struct Data
{
	static constexpr StrV vertex =
R"(#version 330 core
layout (location = 0) in vec3 vertex_pos;
layout (location = 1) in vec3 vertex_color;

out VS_OUT
{
	vec3 color;
} to_fragment;

layout (std140) uniform Camera
{
	mat4 projection;
	mat4 view;
} camera;

uniform mat4 model;

void main()
{
	mat4 proj_view = camera.projection * camera.view;
	to_fragment.color = vertex_color;
	gl_Position = proj_view * model * vec4(vertex_pos, 1);
})";

	static constexpr StrV vertex_screen =
R"(#version 330 core
layout (location = 0) in vec2 vertex_pos;
layout (location = 1) in vec2 vertex_uv;

out VS_OUT
{
	vec2 uv;
	float aspect;
} to_fragment;

layout (std140) uniform Camera
{
	mat4 projection;
	mat4 view;
} camera;

void main()
{
	to_fragment.uv = vec2(vertex_uv.x, -vertex_uv.y);
	to_fragment.aspect = camera.projection[1][1]/camera.projection[0][0];
	gl_Position = vec4(vertex_pos*2, 0, 1);
})";

	static constexpr StrV fragment =
R"(#version 330 core
out vec4 fragment_color;

in VS_OUT
{
	vec3 color;
} to_fragment;

uniform float alpha;

void main()
{
	fragment_color = vec4(to_fragment.color, alpha);
})";

	static constexpr StrV fragment_screen =
R"(#version 330 core
out vec4 fragment_color;

in VS_OUT
{
	vec2 uv;
	float aspect;
} to_fragment;

uniform sampler2D texture_color;

vec3 tonemap(vec3 color)
{
	vec3 p1 = color * (0.15 * color + 0.1 * 0.5) + 0.2 * 0.02;
	vec3 p2 = color * (0.15 * color + 0.5) + 0.2 * 0.3;

	return (p1 / p2) - 0.02 / 0.3;
}

vec3 grid()
{
	const float grid_size = 32;

	const vec3 color_a = vec3(1, 0.7215686, 0.7843137);
	const vec3 color_b = vec3(0.7960784, 0.4235294, 0.5098040);

	const vec3 color_diff = color_b-color_a;

	vec2 coord = floor(mod(gl_FragCoord.xy, vec2(grid_size))/(grid_size/2));

	bool is_b = coord.x > 0.1 && coord.y < 0.1 || coord.x < 0.1 && coord.y > 0.1;

	return color_a + color_diff*float(is_b);
}

void main()
{
	vec4 real_color = texture(texture_color, to_fragment.uv).rgba;
	vec3 color = real_color.rgb;

	vec3 buf = tonemap(color)*2;
	vec3 white = tonemap(vec3(11.2));

	color = pow(buf*white, vec3(1/2.2));
	vec3 grid_color = grid();

	if (real_color.a < 0.3)
		color = grid_color;
	else if (real_color.a < 1)
		color = (color+grid_color)/2;

	fragment_color = vec4(color, 1);
})";

	struct Cube
	{
		static constexpr std::array vertices = {
			Vertex({0.5f, 0.5f, -0.5f}, vec3<f32>(9.f, 0.f, 0.f)),
			Vertex({0.5f, -0.5f, -0.5f}, static_cast<vec3<f32>>(RGB(0x00FF00))),
			Vertex({0.5f, 0.5f, 0.5f}, static_cast<vec3<f32>>(RGB(0x0000FF))),
			Vertex({0.5f, -0.5f, 0.5f}, static_cast<vec3<f32>>(RGB(0xFFFF00))),
			Vertex({-0.5f, 0.5f, -0.5f}, static_cast<vec3<f32>>(RGB(0x00FFFF))),
			Vertex({-0.5f, -0.5f, -0.5f}, static_cast<vec3<f32>>(RGB(0xFF00FF))),
			Vertex({-0.5f, 0.5f, 0.5f}, static_cast<vec3<f32>>(RGB(0xFFFFFF))),
			Vertex({-0.5f, -0.5f, 0.5f}, static_cast<vec3<f32>>(RGB(0x000000))),
		};

		static constexpr std::array indices = {
			0_u8, 4_u8, 6_u8,
			3_u8, 2_u8, 6_u8,
			6_u8, 4_u8, 5_u8,
			1_u8, 3_u8, 7_u8,
			1_u8, 0_u8, 2_u8,
			4_u8, 0_u8, 1_u8,
			7_u8, 3_u8, 6_u8,
			5_u8, 4_u8, 1_u8,
			3_u8, 1_u8, 2_u8,
			7_u8, 6_u8, 5_u8,
			5_u8, 1_u8, 7_u8,
			2_u8, 0_u8, 6_u8,
		};
	};

	struct Square
	{
		static constexpr std::array vertices = {
			ScreenVertex(vec2<f32>(-0.5f, 0.5f), vec2<f32>(0.f)),
			ScreenVertex(vec2<f32>(0.5f), vec2<f32>(1.f, 0.f)),
			ScreenVertex(vec2<f32>(-0.5f), vec2<f32>(0.f, 1.f)),
			ScreenVertex(vec2<f32>(0.5f, -0.5f), vec2<f32>(1.f)),
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
		.title = "HighOpenGL: Cube3D and friend: Blend",
		.bounds = {
			.position = {100, 100},
			.resolution = {.value = {1280, 720}},
		},
		.resizable = false,
		.widnow_instance_name = "main_window"
	});

	HOGL::Context context(gl_instance);
	auto surface = window->create_opengl_surface(gl_instance);
	context.swap_interval(1);
	context.make_current(surface);

	using W = WindowSystem::Window;

	using FramebufferInfo = HOGL::Framebuffer::Info<vec4<f32>>;
	static constexpr HOGL::Texture2D<vec4<f32>>::Info texture_info
	{
		.filter = {
			.mag = HOGL::Filter::Type::nearest,
		},
		.target = HOGL::TextureTarget::framebuffer,
	};

	static constexpr u16 multiplier = 2;
	struct Screen
	{
		Screen(Ptr<W> window, PtrContainer<HOGL::Context> context)
			: resolution(window->resolution()),
			  color(context, texture_info, {8, 8}, {}),
			  depth(context, texture_info, {8, 8}, {}),
			  framebuffer(context, FramebufferInfo{
					.colors = {color},
					.depth = depth,
				})
		{
			using S = W::Slots;
			window->signal_connect<S::resized>(
				[this](W&, W::Resolution resolution)
				{
					std::lock_guard l(mutex);
					this->resolution = resolution;
					updated = true;
				});
		}

		bool update()
		{
			if (updated) [[unlikely]]
			{
				std::lock_guard l(mutex);
				auto res = resolution*multiplier;
				color.update<true, false>(res, {});
				depth.update(res, {});
				updated = false;

				return true;
			}

			return false;
		}

		std::atomic_bool updated = true;
		W::Resolution resolution;
		std::mutex mutex;

		HOGL::Texture2D<vec4<f32>> color;
		HOGL::Texture2D<HOGL::DepthStencil> depth;
		HOGL::Framebuffer framebuffer;

		HOGL::VertexDescriptor vertex_descriptor = {
			.type_size = sizeof(ScreenVertex),
			.attributes = {
				{
					.count = 2,
					.type = HOGL::VertexDescriptor::AttributeDescriptor::Type::f32,
				},
				{
					.offset = offsetof(ScreenVertex, uv),
					.count = 2,
					.type = HOGL::VertexDescriptor::AttributeDescriptor::Type::f32,
				},
			},
		};
		HOGL::Shader::Program program;
		HOGL::StaticMesh<ScreenVertex, u8> mesh;
	} screen(window, context);

	struct
	{
		HOGL::VertexDescriptor vertex_descriptor = {
			.type_size = sizeof(Vertex),
			.attributes = {
				{
					.count = 3,
					.type = HOGL::VertexDescriptor::AttributeDescriptor::Type::f32,
				},
				{
					.offset = offsetof(Vertex, color),
					.count = 3,
					.type = HOGL::VertexDescriptor::AttributeDescriptor::Type::f32,
				},
			},
		};
		HOGL::Shader::Program program;
		HOGL::StaticMesh<Vertex, u8> mesh;
	} cube;

	screen.mesh = {
		context, screen.vertex_descriptor,
		HOGL::PrimitiveTopology::triangles, Data::Square()
	};

	cube.mesh = {
		context, cube.vertex_descriptor,
		HOGL::PrimitiveTopology::triangles, Data::Cube()
	};

	{
		HOGL::Shader::Module vertex(context, {.source = Data::vertex_screen});
		HOGL::Shader::Module fragment(context, {
			.type = HOGL::Shader::Module::Type::fragment,
			.source = Data::fragment_screen
		});

		HOGL::Shader::Program::Info program_info = {
			.modules = {vertex, fragment},
			.layout = {
				.vertex_descriptor = screen.vertex_descriptor,
			},
			.uniform_layout = {
				.textures = {
					{"texture_color", 0},
				},
				.buffers = {{"Camera", 0}},
			},
		};

		screen.program = {context, program_info};

		using L = HOGL::Layout;

		program_info.layout.vertex_descriptor = cube.vertex_descriptor;
		program_info.layout.blend = L::Blend{
			.color = {
				.factor = {
					.src = HOGL::BlendFactor::src_alpha,
					.dst = HOGL::BlendFactor::one_minus_src_alpha
				},
			},
			.alpha = {
				.operation = HOGL::BlendOp::max,
			}
		};
		program_info.layout.depth = L::Depth{};
		program_info.uniform_layout.preload_locations = {"model", "alpha"};
		program_info.uniform_layout.textures = {};

		vertex = {context, {.source = Data::vertex}};
		fragment = {context, {
			.type = HOGL::Shader::Module::Type::fragment,
			.source = Data::fragment
		}};

		cube.program = {context, program_info};
	}

	struct Camera
	{
		Matrix4<f32> projection;
		Matrix4<f32> view;
	};

	HOGL::UniformBuffer<Camera> ubo(context);

	using C = HOGL::CommandBuffer::Clear::Flags;

	HOGL::CommandBuffer cmd_buf(surface, context);
	cmd_buf.clear_config({
		.flags = C::color | C::depth,
	});

	auto matrix1 = rotate(Matrix4<f32>(1.f), radians(43.f), vec3<f32>(1.f, 0.f, 1.f));
	matrix1 = rotate(matrix1, radians(34.f), vec3<f32>(0.f, 1.f, 0.f));

	auto matrix2 = rotate(Matrix4<f32>(1.f), radians(43.f), vec3<f32>(1.f, 0.f, 0.f));
	matrix2 = rotate(matrix2, radians(34.f), vec3<f32>(0.f, 1.f, 1.f));

	while (!window->should_close())
	{
		if (screen.update()) [[unlikely]]
		{
			vec2<f32> res = screen.resolution;
			f32 aspect = res.x/res.y;

			ubo->view = translate(Matrix4<f32>(1.f), vec3<f32>(0.f, 0.f, -2.5f));
			ubo->projection = perspective(radians(70.f), aspect, 0.1f, 100.f);
			ubo.sync();

			cmd_buf.reset();
		}

		cmd_buf.bind_framebuffer(screen.framebuffer);
		cmd_buf.viewport(screen.resolution*multiplier);
		cmd_buf.clear();

		cmd_buf.bind_uniform_buffer(ubo, 0);

		cmd_buf.bind_shader_program(cube.program);

		cmd_buf->set("model", matrix1);
		cmd_buf->set("alpha", 1.f);
		cmd_buf.bind_mesh(cube.mesh);
		cmd_buf.draw();

		cmd_buf->set("model", matrix2);
		cmd_buf->set("alpha", 0.3f);
		cmd_buf.bind_mesh(cube.mesh);
		cmd_buf.draw();

		cmd_buf.bind_framebuffer();
		cmd_buf.viewport(screen.resolution);

		cmd_buf.bind_shader_program(screen.program);
		cmd_buf.bind_texture_2d(screen.color, 0);
		cmd_buf.bind_mesh(screen.mesh);
		cmd_buf.draw();

		cmd_buf.swap();
	}

	return 0;
}
