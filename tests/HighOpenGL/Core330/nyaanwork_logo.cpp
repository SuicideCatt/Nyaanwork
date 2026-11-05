#include <array>
#include <atomic>
#include <cstddef>
#include <mutex>

import Nyaanwork.Asset.Image;
import Nyaanwork.Core.Math;
import Nyaanwork.Core.Types;
import Nyaanwork.Core.Utils.Ptr;
import Nyaanwork.HighOpenGL.Core330;
import Nyaanwork.WindowSystem.OpenGL;
import Nyaanwork.WindowSystem;

using namespace Nyaanwork;
namespace HOGL = HighOpenGL::Core330;

using Asset::Pixels::RGB;

using SimpleVertex = vec2<f32>;

struct Vertex
{
	vec2<f32> position;
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
layout (location = 0) in vec2 vertex_pos;
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
	vec3 color;
} to_fragment;

void main()
{
	fragment_color = vec4(to_fragment.color, 1);
})";

	static constexpr StrV fragment_frame =
R"(#version 330 core
out vec4 fragment_color;

in VS_OUT
{
	vec3 color;
} to_fragment;

void main()
{
	fragment_color = vec4(0, 0, 0, 1);
})";

	static constexpr StrV fragment_screen =
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

	static constexpr std::array frame = {
		// body
		vec2<f32>(0.15625, 0.8623046875),
		vec2<f32>(0.8447265625, 0.8623046875),
		vec2<f32>(0.5, 0.3984375),

		// finger1
		vec2<f32>(0.01953125, 0.318359375),
		vec2<f32>(0.0947265625, 0.591796875),
		vec2<f32>(0.2763671875, 0.408203125),

		// finger4 (hmirrored finger1)
		vec2<f32>(1-0.01953125, 0.318359375),
		vec2<f32>(1-0.0947265625, 0.591796875),
		vec2<f32>(1-0.2763671875, 0.408203125),

		// finger2
		vec2<f32>(0.3232421875, 0.0390625),
		vec2<f32>(0.271484375, 0.3623046875),
		vec2<f32>(0.486328125, 0.3037109375),

		// finger3 (hmirrored finger2)
		vec2<f32>(1-0.3232421875, 0.0390625),
		vec2<f32>(1-0.271484375, 0.3623046875),
		vec2<f32>(1-0.486328125, 0.3037109375),
	};

	static constexpr std::array colored = {
		// body
		Vertex{
			vec2<f32>(0.1787109375, 0.8505859375),
			static_cast<vec3<f32>>(RGB(203, 108, 130)),
		},
		Vertex{
			vec2<f32>(0.8212890625, 0.8505859375),
			static_cast<vec3<f32>>(RGB(203, 108, 130)),
		},
		Vertex{
			vec2<f32>(0.5, 0.4189453125),
			static_cast<vec3<f32>>(RGB(255, 240, 255)),
		},

		// finger1
		Vertex{
			vec2<f32>(0.0380859375, 0.3408203125),
			static_cast<vec3<f32>>(RGB(255, 184, 200)),
		},
		Vertex{
			vec2<f32>(0.099609375, 0.564453125),
			static_cast<vec3<f32>>(RGB(255, 184, 200)),
		},
		Vertex{
			vec2<f32>(0.1796875, 0.4228515625),
			static_cast<vec3<f32>>(RGB(255, 184, 200)),
		},

		Vertex{
			vec2<f32>(0.0830078125, 0.3525390625),
			static_cast<vec3<f32>>(RGB(255, 240, 255)),
		},
		Vertex{
			vec2<f32>(0.1904296875, 0.4150390625),
			static_cast<vec3<f32>>(RGB(255, 240, 255)),
		},
		Vertex{
			vec2<f32>(0.2412109375, 0.4072265625),
			static_cast<vec3<f32>>(RGB(255, 240, 255)),
		},

		Vertex{
			vec2<f32>(0.1923828125, 0.42578125),
			static_cast<vec3<f32>>(RGB(203, 108, 130)),
		},
		Vertex{
			vec2<f32>(0.24609375, 0.4189453125),
			static_cast<vec3<f32>>(RGB(203, 108, 130)),
		},
		Vertex{
			vec2<f32>(0.125, 0.5419921875),
			static_cast<vec3<f32>>(RGB(203, 108, 130)),
		},

		// finger4 (hmirrored finger1)
		Vertex{
			vec2<f32>(1-0.0380859375, 0.3408203125),
			static_cast<vec3<f32>>(RGB(255, 184, 200)),
		},
		Vertex{
			vec2<f32>(1-0.099609375, 0.564453125),
			static_cast<vec3<f32>>(RGB(255, 184, 200)),
		},
		Vertex{
			vec2<f32>(1-0.1796875, 0.4228515625),
			static_cast<vec3<f32>>(RGB(255, 184, 200)),
		},

		Vertex{
			vec2<f32>(1-0.0830078125, 0.3525390625),
			static_cast<vec3<f32>>(RGB(255, 240, 255)),
		},
		Vertex{
			vec2<f32>(1-0.1904296875, 0.4150390625),
			static_cast<vec3<f32>>(RGB(255, 240, 255)),
		},
		Vertex{
			vec2<f32>(1-0.2412109375, 0.4072265625),
			static_cast<vec3<f32>>(RGB(255, 240, 255)),
		},

		Vertex{
			vec2<f32>(1-0.1923828125, 0.42578125),
			static_cast<vec3<f32>>(RGB(203, 108, 130)),
		},
		Vertex{
			vec2<f32>(1-0.24609375, 0.4189453125),
			static_cast<vec3<f32>>(RGB(203, 108, 130)),
		},
		Vertex{
			vec2<f32>(1-0.125, 0.5419921875),
			static_cast<vec3<f32>>(RGB(203, 108, 130)),
		},

		// finger2
		Vertex{
			vec2<f32>(0.328125, 0.0869140625),
			static_cast<vec3<f32>>(RGB(255, 184, 200)),
		},
		Vertex{
			vec2<f32>(0.2880859375, 0.3369140625),
			static_cast<vec3<f32>>(RGB(255, 184, 200)),
		},
		Vertex{
			vec2<f32>(0.3759765625, 0.2646484375),
			static_cast<vec3<f32>>(RGB(255, 184, 200)),
		},

		Vertex{
			vec2<f32>(0.341796875, 0.091796875),
			static_cast<vec3<f32>>(RGB(255, 240, 255)),
		},
		Vertex{
			vec2<f32>(0.3876953125, 0.2626953125),
			static_cast<vec3<f32>>(RGB(255, 240, 255)),
		},
		Vertex{
			vec2<f32>(0.462890625, 0.2890625),
			static_cast<vec3<f32>>(RGB(255, 240, 255)),
		},

		Vertex{
			vec2<f32>(0.3828125, 0.2744140625),
			static_cast<vec3<f32>>(RGB(203, 108, 130)),
		},
		Vertex{
			vec2<f32>(0.4541015625, 0.2998046875),
			static_cast<vec3<f32>>(RGB(203, 108, 130)),
		},
		Vertex{
			vec2<f32>(0.3017578125, 0.341796875),
			static_cast<vec3<f32>>(RGB(203, 108, 130)),
		},

		// finger3 (hmirrored finger2)
		Vertex{
			vec2<f32>(1-0.328125, 0.0869140625),
			static_cast<vec3<f32>>(RGB(255, 184, 200)),
		},
		Vertex{
			vec2<f32>(1-0.2880859375, 0.3369140625),
			static_cast<vec3<f32>>(RGB(255, 184, 200)),
		},
		Vertex{
			vec2<f32>(1-0.3759765625, 0.2646484375),
			static_cast<vec3<f32>>(RGB(255, 184, 200)),
		},

		Vertex{
			vec2<f32>(1-0.341796875, 0.091796875),
			static_cast<vec3<f32>>(RGB(255, 240, 255)),
		},
		Vertex{
			vec2<f32>(1-0.3876953125, 0.2626953125),
			static_cast<vec3<f32>>(RGB(255, 240, 255)),
		},
		Vertex{
			vec2<f32>(1-0.462890625, 0.2890625),
			static_cast<vec3<f32>>(RGB(255, 240, 255)),
		},

		Vertex{
			vec2<f32>(1-0.3828125, 0.2744140625),
			static_cast<vec3<f32>>(RGB(203, 108, 130)),
		},
		Vertex{
			vec2<f32>(1-0.4541015625, 0.2998046875),
			static_cast<vec3<f32>>(RGB(203, 108, 130)),
		},
		Vertex{
			vec2<f32>(1-0.3017578125, 0.341796875),
			static_cast<vec3<f32>>(RGB(203, 108, 130)),
		},
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
		.title = "HighOpenGL: Nyaanwork Logo",
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

	using FramebufferInfo = HOGL::Framebuffer::Info<RGB>;
	static constexpr HOGL::Texture2D<RGB>::Info texture_info
	{
		.filter = {
			.mag = HOGL::Filter::Type::linear,
		},
		.target = HOGL::TextureTarget::framebuffer,
	};

	static constexpr u16 multiplier = 2;
	struct Screen
	{
		Screen(Ptr<W> window, PtrContainer<HOGL::Context> context)
			: resolution(window->resolution()),
			  color(context, texture_info, {8, 8}, {}),
			  framebuffer(context, FramebufferInfo{.colors = {color}})
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
				color.update(res, {});
				updated = false;

				return true;
			}

			return false;
		}

		std::atomic_bool updated = true;
		W::Resolution resolution;
		std::mutex mutex;

		HOGL::Texture2D<RGB> color;
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
			.type_size = sizeof(SimpleVertex),
			.attributes = {
				{
					.count = 2,
					.type = HOGL::VertexDescriptor::AttributeDescriptor::Type::f32,
				},
			},
		};
		HOGL::Shader::Program program;
		HOGL::StaticMesh<SimpleVertex, void> mesh;
	} frame;

	struct
	{
		HOGL::VertexDescriptor vertex_descriptor = {
			.type_size = sizeof(Vertex),
			.attributes = {
				{
					.count = 2,
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
		HOGL::StaticMesh<Vertex, void> mesh;
	} colored;

	screen.mesh = {
		context, screen.vertex_descriptor,
		HOGL::PrimitiveTopology::triangles, Data::Square()
	};

	frame.mesh = {
		context, frame.vertex_descriptor,
		HOGL::PrimitiveTopology::triangles, Data::frame
	};

	colored.mesh = {
		context, colored.vertex_descriptor,
		HOGL::PrimitiveTopology::triangles, Data::colored
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
				.vertex_descriptor = screen.vertex_descriptor
			},
			.uniform_layout = {
				.textures = {
					{"texture_color", 0},
				},
			},
		};

		screen.program = {context, program_info};

		program_info.layout.vertex_descriptor = frame.vertex_descriptor;
		program_info.uniform_layout.preload_locations = {"model"};
		program_info.uniform_layout.textures = {};
		program_info.uniform_layout.buffers = {{"Camera", 0}};

		vertex = {context, {.source = Data::vertex}};
		fragment = {context, {
			.type = HOGL::Shader::Module::Type::fragment,
			.source = Data::fragment_frame
		}};

		frame.program = {context, program_info};

		program_info.layout.vertex_descriptor = colored.vertex_descriptor;

		fragment = {context, {
			.type = HOGL::Shader::Module::Type::fragment,
			.source = Data::fragment
		}};

		colored.program = {context, program_info};
	}

	struct Camera
	{
		Matrix4<f32> projection;
		Matrix4<f32> view;
	};

	HOGL::UniformBuffer<Camera> ubo(context);

	HOGL::CommandBuffer cmd_buf(surface, context);
	cmd_buf.clear_config({
		.flags = HOGL::CommandBuffer::Clear::Flags::color,
		.color = Asset::Colors<Asset::Pixels::RGBA>::dark_gray,
	});

	auto logo_matrix = translate(Matrix4<f32>(1.f), vec3<f32>(-0.5f, 0.5f, 0.f));
	logo_matrix = scale(logo_matrix, vec3<f32>(1.f, -1.f, 0.f));

	cmd_buf.bind_shader_program(frame.program);
	cmd_buf->set("model", logo_matrix);
	cmd_buf.bind_shader_program(colored.program);
	cmd_buf->set("model", logo_matrix);

	while (!window->should_close())
	{
		if (screen.update()) [[unlikely]]
		{
			vec2<f32> res = screen.resolution;
			f32 aspect = res.x/res.y;

			f32 scale = 2.f;
			vec2<f32> lr = scale * vec2<f32>(-1.f, 1.f);
			vec2<f32> td = lr/aspect;

			ubo->view = translate(Matrix4<f32>(1.f), vec3<f32>(0.f, 0.f, -10.f));
			ubo->projection = ortho(lr.x, lr.y, td.x, td.y, 0.1f, 100.f);
			ubo.sync();

			cmd_buf.reset();
		}

		cmd_buf.bind_framebuffer(screen.framebuffer);
		cmd_buf.viewport(screen.resolution*multiplier);
		cmd_buf.clear();

		cmd_buf.bind_uniform_buffer(ubo, 0);

		cmd_buf.bind_shader_program(frame.program);
		cmd_buf.bind_mesh(frame.mesh);
		cmd_buf.draw();

		cmd_buf.bind_shader_program(colored.program);
		cmd_buf.bind_mesh(colored.mesh);
		cmd_buf.draw();

		cmd_buf.bind_framebuffer();
		cmd_buf.viewport(screen.resolution);
		cmd_buf.clear();

		cmd_buf.bind_shader_program(screen.program);
		cmd_buf.bind_texture_2d(screen.color, 0);
		cmd_buf.bind_mesh(screen.mesh);
		cmd_buf.draw();

		cmd_buf.swap();
	}

	return 0;
}
