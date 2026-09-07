#include <Nyaanwork/Asset.hpp>

#include <algorithm>
#include <chrono>
#include <iostream>
#include <print>
#include <thread>
#include <unordered_map>

import Nyaanwork.Core.Math;
import Nyaanwork.Core.Types;
import Nyaanwork.WindowSystem;

#ifdef NYAANWORK_ASSET_IMAGE
import Nyaanwork.Asset.Image;
#endif

#ifdef NYAANWORK_ASSET_TILE
import Nyaanwork.Asset.Tile;
#endif

using namespace Nyaanwork;
using namespace WindowSystem::Codes;

struct State
{
	bool press = false, hold = false, release = false;
};

template<typename Code>
using StateMap = std::unordered_map<Code, State>;

using KeyStateMap = StateMap<Key>;
using ButtonStateMap = StateMap<Button>;

using IS = WindowSystem::Instance::Slots;
template<IS slot>
using ISPos = WindowSystem::Instance::SignalPosition<slot>;

using WS = WindowSystem::Window::Slots;
template<WS slot>
using WSPos = WindowSystem::Window::SignalPosition<slot>;

enum class Mode
{
	none = 0,
	window,
	tablet, mouse,
	string,
	dnd,
};

struct Context
{
	using WindowPtr = Ptr<WindowSystem::Window>;

	virtual ~Context() = default;

	virtual Mode update(WindowSystem::Instance& instance, WindowPtr window,
						WindowSystem::InputState& state,
						KeyStateMap& keyboard, ButtonStateMap& mouse) = 0;
};

struct NoneContext : Context
{
	Mode update(WindowSystem::Instance& instance, WindowPtr window,
				WindowSystem::InputState& state,
				KeyStateMap& keyboard, ButtonStateMap& mouse) override
	{
		if (once)
		{
			once = false;
			std::println(std::cout);
		}

		auto res = Mode::none;

		if (keyboard[Key::escape].release) [[unlikely]]
			window->close();

		if (keyboard[Key::w].release)
			res = Mode::window;

		if (keyboard[Key::t].release)
			res = Mode::tablet;

		if (keyboard[Key::m].release)
			res = Mode::mouse;

		if (keyboard[Key::s].release)
			res = Mode::string;

		if (keyboard[Key::d].release)
			res = Mode::dnd;

		if (res != Mode::none)
			once = true;

		return res;
	}

	bool once = false;
};

struct WindowContext : Context
{
	Mode update(WindowSystem::Instance& instance, WindowPtr window,
				WindowSystem::InputState& state,
				KeyStateMap& keyboard, ButtonStateMap& mouse) override
	{
		if (keyboard[Key::escape].release) [[unlikely]]
		{
			instance->signal_disconnect(displays_changed);
			displays_changed.id = -1;

			return Mode::none;
		}

		if (displays_changed.id == -1)
		{
			using I = WindowSystem::Instance::Base;

			displays_changed = instance->signal_connect<IS::displays_changed>(
				[](I& instance)
				{
					std::println("s-> displays changed");

					const auto [displays, primary] = instance.displays();
					for (usize i = 0; i < displays.size(); ++i)
					{
						auto& display = displays[i];
						std::println("  {}:", display.name);
						std::println("    is_primary: {}", i == primary);
						std::println("    position: {}", display.position);
						std::println("    resolution: {}", display.resolution);
						std::println("    refresh_rate: {}", display.refresh_rate);
						std::println("    size:");
						std::println("      mm: {}", display.size.mm);
						std::println("      inch: {}", display.size.inch);
					}
				});
		}

		if (print)
		{	std::print(std::cout, "\r{}:", window->title());
			std::print(std::cout, "  position: {}", window->position());
			std::print(std::cout, "  resolution: {}", window->resolution());
			std::print(std::cout, "  keyboard_focused: {}", window->keyboard_focused());
			std::print(std::cout, "  mouse_focused: {}", window->mouse_focused());
			std::print(std::cout, "  minimized: {}", window->minimized());
			std::print(std::cout, "  display: {}", window->display());
			std::print(std::cout, "{: >6}", ' ');
			std::cout.flush();
		}

		if (auto win = instance->focused_window())
		{
			auto t = win->title();
			if (title_keyboard != t)
			{
				title_keyboard = std::move(t);
				std::println(std::cout, "\nInstance keyboard focused window: {}",
							 title_keyboard);
			}
		}
		else if (!title_keyboard.empty())
		{
			std::println(std::cout, "\nInstance keyboard focused window: {}",
						 title_keyboard = "");
		}

		if (auto win = instance->mouse_focused_window())
		{
			auto t = win->title();
			if (title_mouse != t)
			{
				title_mouse = std::move(t);
				std::println(std::cout, "\nInstance mouse focused window: {}",
							 title_mouse);
			}
		}
		else if (!title_mouse.empty())
		{
			std::println(std::cout, "\nInstance mouse focused window: {}",
						 title_mouse = "");
		}

		if (keyboard[Key::f].release)
			window->fullscreen(!window->fullscreen());

		if (keyboard[Key::b].release)
			window->borderless(!window->borderless());

		if (keyboard[Key::r].release)
			window->resizable(!window->resizable());

		if (keyboard[Key::c].release)
		{
			WindowSystem::Window::Resolution res(640);
			window->resolution(res);

			auto d = instance->displays().displays[window->display()];
			auto pos = d.position;
			pos += d.resolution/2_u16 - res/2_u16;
			window->position(pos);
		}

		if (keyboard[Key::p].release)
		{
			if (print)
			{
				print = false;
				std::println(std::cout);
			}
			else
			{
				print = true;
			}
		}

		if (keyboard[Key::x].release)
		{
			auto display = instance->displays().displays[window->display()];

			vec2<i16> pos = (display.resolution-window->resolution())/2_u16;
			pos += display.position;

			window->position(pos);
		}

		if (keyboard[Key::m].release)
		{
			using P = WindowSystem::Window::Position;

			if (window2)
			{
				window2.reset();
			}
			else
			{
				window2 = instance->create_window({
					.title = "Test2",
					.bounds = {
						.position = window->position() + P(window->resolution()),
						.resolution = {
							.value = {100, 100},
						},
					},
					.resizable = false,
					.widnow_instance_name = "test_window"
				});
			}
		}

		return Mode::window;
	}

	bool print = true;
	Ptr<WindowSystem::Window> window2;
	Str title_keyboard = "";
	Str title_mouse = "";
	ISPos<IS::displays_changed> displays_changed = {.id = static_cast<usize>(-1)};
};

struct TabletContext : Context
{
	Mode update(WindowSystem::Instance& instance, WindowPtr window,
				WindowSystem::InputState& state,
				KeyStateMap& keyboard, ButtonStateMap& mouse) override
	{
		if (keyboard[Key::escape].release) [[unlikely]]
		{
			close = false;

			instance->signal_disconnect(connected);
			instance->signal_disconnect(disconnected);

			connected.id = -1;

			return Mode::none;
		}

		if (connected.id == -1)
		{
			using I = WindowSystem::Instance::Base;

			connected = instance->signal_connect<IS::tablet_connected>([](I&)
				{ std::println("s-> tablet connected"); });

			disconnected = instance->signal_connect<IS::tablet_disconnected>([](I&)
				{ std::println("s-> tablet disconnected"); });
		}

		if (close && !state.is_pen_on_surface())
		{
			close = false;
			std::println(std::cout);
		}

		if (state.is_pen_on_surface())
		{
			// std::println("ERR");
			std::print(std::cout, "tablet:");
			std::print(std::cout, "  pressure: {}", state.pen_pressure());
			std::print(std::cout, "  tilt: {} {}",
					   state.pen_tilt_degrees(), state.pen_tilt_radians());
			std::print(std::cout, "{: >20}\r", ' ');
			close = true;
		}
		else if (instance->have_tablet())
		{
			std::print(std::cout, "tablet: not on surface\r");
		}
		else
		{
			std::print(std::cout, "tablet: no\r");
		}
		std::cout.flush();

		return Mode::tablet;
	}

	bool close = false;
	ISPos<IS::tablet_connected> connected = {.id = static_cast<usize>(-1)};
	ISPos<IS::tablet_disconnected> disconnected = {};
};

struct MouseContext : Context
{
	Mode update(WindowSystem::Instance& instance, WindowPtr window,
				WindowSystem::InputState& state,
				KeyStateMap& keyboard, ButtonStateMap& mouse) override
	{
		std::print(std::cout, "mouse:");
		std::print(std::cout, "  position: {}", state.position());
		std::print(std::cout, "  motion: {}", state.motion());
		std::print(std::cout, "  wheel: {}", state.wheel());
		std::print(std::cout, "{: >25}\r", ' ');

		if (keyboard[Key::escape].release) [[unlikely]]
			return Mode::none;

		if (keyboard[Key::s].release)
			window->show_mouse(!window->show_mouse());

		if (keyboard[Key::g].release)
			window->grab_mouse(!window->grab_mouse());

		if (keyboard[Key::c].release)
			window->centralize_mouse(!window->centralize_mouse());

		if (keyboard[Key::p].release)
			window->capture_mouse(!window->capture_mouse());

		if (mouse[Button::left].press)
			window->cursor(WindowSystem::Window::Cursor::crosshair);
		else if (mouse[Button::left].release)
			window->cursor(WindowSystem::Window::Cursor::default_);

		if (mouse[Button::middle].release)
		{
			using C = WindowSystem::Window::Cursor;
			switch (window->cursor())
			{
			case C::default_:
				window->cursor(C::wait);
				break;
			case C::wait:
				window->cursor(C::default_);
				break;
			default:
				break;
			}
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(16));

		return Mode::mouse;
	}
};

struct StringContext : Context
{
	Mode update(WindowSystem::Instance& instance, WindowPtr window,
				WindowSystem::InputState& state,
				KeyStateMap& keyboard, ButtonStateMap& mouse) override
	{
		if (keyboard[Key::escape].release) [[unlikely]]
		{
			window->string_recording(false);

			instance->signal_disconnect(kl_changed);
			kl_changed.id = -1;

			return Mode::none;
		}

		if (!window->string_recording()) [[unlikely]]
		{
			using I = WindowSystem::Instance::Base;

			window->string_recording(true);
			rstr.string.clear();

			kl_changed = instance->signal_connect<IS::keyboard_layout_changed>([](I&)
				{ std::println("s-> layout changed"); });
		}

		auto buf = state.string();
		while (buf.over_remove-- && !rstr.char_sizes.empty())
		{
			auto pop = rstr.char_sizes.back();
			rstr.char_sizes.pop_back();

			while (pop-- && !rstr.string.empty())
				rstr.string.pop_back();
		}

		bool apply = false;
		auto nl = std::ranges::find(buf.string, '\n');
		if ((apply = nl != buf.string.end()))
		{
			buf.string.resize(nl-buf.string.begin());

			usize ic = 0;
			usize i = 0;
			while (ic < buf.string.size())
				ic += buf.char_sizes[i++];
			buf.char_sizes.resize(i);
		}

		rstr.string += buf.string;
		rstr.char_sizes.insert(rstr.char_sizes.end(),
							   buf.char_sizes.begin(), buf.char_sizes.end());
		rbuf_size = max(rbuf_size, rstr.char_sizes.size()+1);

		if (apply)
			window->title(rstr.string);

		std::print(std::cout, "\r{1:_>{0}}", rbuf_size, '_');
		std::print(std::cout, "{}", instance->keyboard_layout());
		std::print(std::cout, "{: >10}\r", ' ');
		std::cout.flush();
		std::print(std::cout, "{}", rstr.string);
		std::cout.flush();

		return Mode::string;
	}

	WindowSystem::Window::RecordedString rstr;
	usize rbuf_size = 20;
	ISPos<IS::keyboard_layout_changed> kl_changed = {.id = static_cast<usize>(-1)};
};

struct DND_Context : Context
{
	Mode update(WindowSystem::Instance& instance, WindowPtr window,
				WindowSystem::InputState& state,
				KeyStateMap& keyboard, ButtonStateMap& mouse) override
	{
		if (keyboard[Key::escape].release) [[unlikely]]
		{
			window->signal_disconnect(begin);
			window->signal_disconnect(cancel);
			window->signal_disconnect(done);
			window->signal_disconnect(text);
			window->signal_disconnect(uri);

			begin.id = -1;

			return Mode::none;
		}

		if (begin.id == -1)
		{
			begin = window->signal_connect<WS::drop_begin>(
				[](WindowSystem::Window&, WindowSystem::Window::DropDataType t)
				{
					std::println("dnd: begin {}",
								 t == decltype(t)::text? "text" : "uri");
				});

			cancel = window->signal_connect<WS::drop_cancel>(
				[](WindowSystem::Window&)
					{ std::println("dnd: cancel"); });

			done = window->signal_connect<WS::drop_done>(
				[](WindowSystem::Window&)
					{ std::println("dnd: done"); });

			text = window->signal_connect<WS::drop_text>(
				[](WindowSystem::Window&, const Str& text)
				{
					std::println("dnd:");
					std::println("  text \"{}\"", text);
					std::println("  syms({}):", text.size());
					for (auto c : text)
					{
						std::println("    \'{0}\' -> {0:d}", c);
					}
				});

			uri = window->signal_connect<WS::drop_uri>(
				[](WindowSystem::Window&, const std::vector<Str>& uris)
				{
					std::println("dnd: uris({}):", uris.size());
					for (usize i = 0; i < uris.size(); ++i)
						std::println("  {}: {}", i+1, uris[i]);
				});
		}

		return Mode::dnd;
	}

	WSPos<WS::drop_begin> begin = {.id = static_cast<usize>(-1)};
	WSPos<WS::drop_cancel> cancel = {};
	WSPos<WS::drop_done> done = {};
	WSPos<WS::drop_text> text = {};
	WSPos<WS::drop_uri> uri = {};
};

int main(int argc, char** argv)
{
	WindowSystem::Instance instance("nyaanwork_test");

	std::println("Backend: {} ({})", instance->full_name(), instance->short_name());
	std::println("  multiwindow_support: {}", instance->multiwindow_support());

	auto [displays, primary] = instance->displays();

	std::println("Displays({}):", displays.size());
	for (usize i = 0; i < displays.size(); ++i)
	{
		auto& display = displays[i];

		std::println("  {}:", display.name);
		std::println("    is_primary: {}", i == primary);
		std::println("    position: {}", display.position);
		std::println("    resolution: {}", display.resolution);
		std::println("    refresh_rate: {}", display.refresh_rate);
		std::println("    size:");
		std::println("      mm: {}", display.size.mm);
		std::println("      inch: {}", display.size.inch);
	}
	std::println();

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
		.widnow_instance_name = "main_window"
	});

	#ifdef NYAANWORK_ASSET_IMAGE
	{
		using namespace Nyaanwork::Asset::Pixels;

		Nyaanwork::Asset::Image image(vec2<u16>(2), {
			RGBA(0xFF0000FF), RGBA(0x00FF00FF),
			RGBA(0x0000FFFF), RGBA(0x00000000),
		});

		#ifdef NYAANWORK_ASSET_TILE
		{
			static constexpr vec2<u16> scale(8);
			Nyaanwork::Asset::Image<RGBA> scaled(image.resolution()*scale);
			Nyaanwork::Asset::TileSet<RGBA> set(scaled, scale);
			for (usize i = 0; i < image.size(); ++i)
				std::ranges::fill(set[i], image[i]);

			window->icon(scaled);
		}
		#else
		{
			window->icon(image);
		}
		#endif

	}
	#endif

	KeyStateMap keyboard;
	ButtonStateMap mouse;

	auto update = [](auto& states, auto& inputer, auto&& proj)
	{
		for (auto& [kb, state] : states)
		{
			bool kb_state = (inputer.*proj)(kb);
			auto& [p, h, r] = state;

			p = kb_state == true && kb_state != h;
			r = kb_state == false && kb_state != h;
			h = kb_state;
		}
	};

	auto mode = Mode::none;
	std::unordered_map<Mode, UPtr<Context>> contexts;
	contexts[Mode::none] = make_unique<NoneContext>();
	contexts[Mode::window] = make_unique<WindowContext>();
	contexts[Mode::tablet] = make_unique<TabletContext>();
	contexts[Mode::mouse] = make_unique<MouseContext>();
	contexts[Mode::string] = make_unique<StringContext>();
	contexts[Mode::dnd] = make_unique<DND_Context>();

	State old_old;
	State old;
	while (!window->should_close())
	{
		auto input = window->input_state();

		update(keyboard, input, &WindowSystem::InputState::key);
		update(mouse, input, &WindowSystem::InputState::button);

		mode = contexts[mode]->update(instance, window, input, keyboard, mouse);

		auto c = keyboard[Key::l];
		if (c.press != old.press || c.hold != old.hold || c.release != old.release)
			std::println("new: {:d} {:d} {:d}", c.press, c.hold, c.release);
		old = c;
	}

	return 0;
}
