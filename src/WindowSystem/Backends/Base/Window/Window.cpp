module;

#include <Core/Defines.hpp>
#include <Nyaanwork/Asset.hpp>
#include <Nyaanwork/WindowSystem.hpp>

#ifdef NYAANWORK_WINDOW_SYSTEM_VULKAN
#include <vulkan/vulkan.hpp>
#endif

#include <mutex>
#include <shared_mutex>
#include <vector>

export module Nyaanwork.WindowSystem.Backends.Base:Window.decl;
import :InputState;
import :Window.InputState;
import :decl;

import Nyaanwork.Core.Types;
import Nyaanwork.Core.Utils.Flags;
import Nyaanwork.Core.Utils.Signals;

#ifdef NYAANWORK_ASSET_IMAGE
import Nyaanwork.Asset.Image;
#endif

#ifdef NYAANWORK_WINDOW_SYSTEM_OPENGL
import Nyaanwork.WindowSystem.OpenGL;
#endif

namespace Nyaanwork::WindowSystem::Backends::Base
{
	enum class Slots
	{
		moved, resized, should_close,
		focus_gained, focus_lost,
		mouse_enter, mouse_leave,
		minimized, unminimized,

		drop_begin, drop_done, drop_cancel,
		drop_uri, drop_text,
	};

	enum class DropDataType
	{
		uri, text
	};

	enum class WindowState : u16
	{
		should_close = 1 << 0,

		fullscreen = 1 << 1,
		borderless = 1 << 2,
		resizable = 1 << 3,

		focus_keyboard = 1 << 4,
		focus_mouse = 1 << 5,

		minimized = 1 << 6,

		mouse_grab = 1 << 7,
		mouse_show = 1 << 8,
		mouse_centralize = 1 << 9,

		string_recording = 1 << 10,
	};

	using WindowPosition = vec2<i16>;
	using WindowResolution = vec2<u16>;
}

export namespace Nyaanwork
{
	template<>
	struct FlagTraits<WindowSystem::Backends::Base::WindowState>
		: FlagInfo<WindowSystem::Backends::Base::WindowState>
	{
		static constexpr bool is_bit_type = true;

		static constexpr Flags main_state =
			Type::should_close | Type::fullscreen
			| Type::borderless | Type::resizable;
		static constexpr Flags focus =
			Type::focus_keyboard | Type::focus_mouse;
		static constexpr Flags mouse =
			Type::mouse_grab | Type::mouse_show | Type::mouse_centralize;

		static constexpr Flags all =
			main_state | focus | Type::minimized | mouse | Type::string_recording;
	};
}

export namespace Nyaanwork::WindowSystem::Backends::Base
{
	class Window
		: public SyncedSignalsSender<Slots,
					SlotInfo<Slots::moved, false, Window&, WindowPosition>,
					SlotInfo<Slots::resized, false, Window&, WindowResolution>,
					SlotInfo<Slots::should_close, false, Window&>,
					SlotInfo<Slots::focus_gained, true, Window&>,
					SlotInfo<Slots::focus_lost, true, Window&>,
					SlotInfo<Slots::mouse_enter, true, Window&>,
					SlotInfo<Slots::mouse_leave, true, Window&>,
					SlotInfo<Slots::minimized, true, Window&>,
					SlotInfo<Slots::unminimized, true, Window&>,
					SlotInfo<Slots::drop_begin, false, Window&, DropDataType>,
					SlotInfo<Slots::drop_cancel, false, Window&>,
					SlotInfo<Slots::drop_uri, false, Window&,
							   const std::vector<Str>&>,
					SlotInfo<Slots::drop_text, false,
							   Window&, const Str&>,
					SlotInfo<Slots::drop_done, false, Window&>>
	{
	public:
		using SyncedSignalsSender::Slots;
		using SyncedSignalsSender::SignalPosition;
		using DropDataType = Base::DropDataType;
		using RecordedString = WindowInputState::Keyboard::RecordedString;

		using Position = WindowPosition;
		using Resolution = WindowResolution;

		struct Info
		{
			Str title = "No name";

			struct {

				Position position;

				struct
				{
					Resolution min = {1, 1};
					Resolution max =
						Resolution(limits<Resolution::value_type>::max());

					Resolution value;
				} resolution;
			} bounds;

			bool resizable = true;
			bool borderless = false;
			bool fullscreen = false;
			struct
			{
				bool show = true;
				bool grab = false;
				bool centralize = false;
			} mouse;

			Opt<Str> widnow_instance_name = nullopt;
		};

		enum class Cursor : u8
		{
			default_,
			progress, wait,
			text, vertical_text,
			crosshair,

			copy, alias,

			grab, grabbing,
			no_drop, not_allowed,

			pointer, context_menu, help,
			cell,

			trdl_resize, tldr_resize,
			t_resize, d_resize, td_resize, row_resize,
			l_resize, r_resize, lr_resize, col_resize,
			tl_resize, tr_resize,
			dl_resize, dr_resize,
			move,
			all_scroll,

			zoom_in, zoom_out,
		};

		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;

		Window(Window&&) = delete;
		Window& operator=(Window&&) = delete;

		virtual ~Window() = default;

		virtual Ptr<Instance> instance() const = 0;

		Str title() const;
		void title(Str title);

		#ifdef NYAANWORK_ASSET_IMAGE
		Asset::Image<Asset::Pixels::RGBA> icon() const;
		void icon(Asset::Image<Asset::Pixels::RGBA> icon);
		#endif

		Position position() const;
		void position(Position position);

		Resolution resolution() const;
		void resolution(Resolution resolution);

		Resolution min_resolution() const;
		void min_resolution(Resolution resolution);

		Resolution max_resolution() const;
		void max_resolution(Resolution resolution);

		bool resizable() const;
		void resizable(bool mode);

		bool fullscreen() const;
		void fullscreen(bool mode);

		bool borderless() const;
		void borderless(bool mode);

		Cursor cursor() const;
		void cursor(Cursor cursor);

		bool show_mouse() const;
		void show_mouse(bool mode);

		bool grab_mouse() const;
		void grab_mouse(bool mode);

		bool centralize_mouse() const;
		void centralize_mouse(bool mode);

		bool capture_mouse() const;
		void capture_mouse(bool mode);

		void warp_mouse(Position position);

		bool keyboard_focused() const;
		bool mouse_focused() const;
		bool minimized() const;

		bool should_close() const;
		void cancel_close();
		void close();

		Base::InputState input_state();

		bool key_state(Codes::Key key) const;

		vec2<f32> mouse_position() const;
		vec2<f32> mouse_motion();
		vec2<f32> mouse_wheel();
		bool button_state(Codes::Button button) const;

		bool tablet_on_surface() const;
		f32 tablet_pressure() const;
		vec2<f32> tablet_tilt_degrees() const;
		vec2<f32> tablet_tilt_radians() const;

		bool string_recording() const;
		void string_recording(bool mode);
		RecordedString recorded_string();

		usize display() const;

		#ifdef NYAANWORK_WINDOW_SYSTEM_OPENGL
		virtual Ptr<OpenGL::Surface>
			create_opengl_surface(Ptr<OpenGL::Instance> instance) = 0;
		virtual Ptr<OpenGL::SurfaceContext>
			create_opengl_surface_context(Ptr<OpenGL::Instance> instance,
										  OpenGL::Context::Profile profile,
										  vec2<u8> version) = 0;
		#endif

		#ifdef NYAANWORK_WINDOW_SYSTEM_VULKAN
		NYAAN_ND virtual vk::SurfaceKHR
			create_vulkan_surface(
				vk::Instance instance,
				vk::Optional<vk::AllocationCallbacks const> allocator = nullptr) = 0;
		#endif

	protected:
		using InputState = WindowInputState;
		using State = WindowState;

		Window() = default;

		std::shared_lock<std::shared_mutex> shared_lock() const;
		std::unique_lock<std::shared_mutex> unique_lock();

		Opt<std::shared_lock<std::shared_mutex>> try_shared_lock() const;
		Opt<std::unique_lock<std::shared_mutex>> try_unique_lock();

		bool should_close_no_lock() NYAAN_CNOEX;

		bool resizable_no_lock() NYAAN_CNOEX;
		bool fullscreen_no_lock() NYAAN_CNOEX;
		bool borderless_no_lock() NYAAN_CNOEX;

		bool keyboard_focused_no_lock() NYAAN_CNOEX;
		bool mouse_focused_no_lock() NYAAN_CNOEX;
		bool minimized_no_lock() NYAAN_CNOEX;

		bool show_mouse_no_lock() NYAAN_CNOEX;
		bool grab_mouse_no_lock() NYAAN_CNOEX;
		bool centralize_mouse_no_lock() NYAAN_CNOEX;
		bool capture_mouse_no_lock() NYAAN_CNOEX;

		bool string_recording_no_lock() NYAAN_CNOEX;

		virtual void title_impl(const Str& title) = 0;

		#ifdef NYAANWORK_ASSET_IMAGE
		virtual void icon_impl(const Asset::Image<Asset::Pixels::RGBA>& icon) = 0;
		#endif

		virtual void position_impl(Position position) = 0;
		virtual void resolution_impl(Resolution resolution) = 0;
		virtual void min_resolution_impl(Resolution resolution) = 0;
		virtual void max_resolution_impl(Resolution resolution) = 0;

		virtual void resizable_impl(bool mode) = 0;
		virtual void fullscreen_impl(bool mode) = 0;
		virtual void borderless_impl(bool mode) = 0;

		virtual void cursor_impl(Cursor cursor) = 0;
		virtual void show_mouse_impl(bool mode) = 0;
		virtual void grab_mouse_impl(bool mode) = 0;
		// virtual void centralize_mouse_impl(bool mode) = 0;

		virtual void warp_mouse_impl(Position position) = 0;

		virtual usize display_impl() const;

		Str p_title;
		#ifdef NYAANWORK_ASSET_IMAGE
		Asset::Image<Asset::Pixels::RGBA> p_icon;
		#endif

		InputState p_input;
		Flags<State> p_state = State::resizable | State::mouse_show;
		Position p_position;
		struct
		{
			Resolution min = {1, 1};
			Resolution max = Resolution(limits<u16>::max());

			Resolution value;
		} p_resolution;

		Cursor p_cursor = Cursor::default_;

	private:
		mutable std::shared_mutex m_mutex;
	};
}
