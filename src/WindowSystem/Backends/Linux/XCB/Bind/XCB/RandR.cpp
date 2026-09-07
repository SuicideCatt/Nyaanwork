module;

#include <WindowSystem/Backends/Linux/XCB/Defines.hpp>

#include <xcb/randr.h>

#include <span>

export module Nyaanwork.WindowSystem.Backends.Linux.XCB:Bind.XCB.RandR;
import :Bind.XCB.Base;
import :Bind.XCB.Helper;

import Nyaanwork.Core.Types;
import Nyaanwork.Core.Utils.Flags;

export namespace XCB::RandR
{
	using ModeInfo = xcb_randr_mode_info_t;
	using OutputID = xcb_randr_output_t;
}

export namespace XCB::RandR::Events
{
	enum class Event
		: Nyaanwork::EnumTraits<xcb_randr_notify_t>::MaskType
	{
		#define XCB_FLAG(name, X) XCB_FLAG_HELPER(name, XCB_RANDR_NOTIFY, X)

		XCB_FLAG(crtc_change, CRTC_CHANGE),
		XCB_FLAG(output_change, OUTPUT_CHANGE),
		XCB_FLAG(output_property, OUTPUT_PROPERTY),
		XCB_FLAG(provider_change, PROVIDER_CHANGE),
		XCB_FLAG(provider_property, PROVIDER_PROPERTY),
		XCB_FLAG(resource_change, RESOURCE_CHANGE),
		XCB_FLAG(lease, LEASE),

		#undef XCB_FLAG
	};
	using enum Event;

	void select(Connection *c, Window::ID window, Event event) noexcept
	{
		xcb_randr_select_input(c, window,
							   Nyaanwork::EnumTraits<Event>::cast(event));
	}

	constexpr Nyaanwork::u8 screen_change_notify = XCB_RANDR_SCREEN_CHANGE_NOTIFY;
}

export namespace XCB::RandR::Request
{
	XCB_REQUEST_TYPE(GetCRTCInfo, xcb_randr_get_crtc_info);
	XCB_REQUEST_TYPE(GetOutputInfo, xcb_randr_get_output_info);
	XCB_REQUEST_TYPE(GetOutputPrimary, xcb_randr_get_output_primary);
	XCB_REQUEST_TYPE(GetScreenResources, xcb_randr_get_screen_resources);
	XCB_REQUEST_TYPE(QueryVersion, xcb_randr_query_version);
}

export namespace XCB::Helper
{
	template<>
	struct View<GetUPtr<xcb_randr_get_screen_resources_reply_t>> final
		: GetUPtr<xcb_randr_get_screen_resources_reply_t>
	{
		using unique_ptr::element_type;
		using unique_ptr::pointer;

		using Type = element_type;

		View(unique_ptr reply = nullptr)
			: unique_ptr(std::move(reply)) {}

		View(const View&) = delete;
		View& operator=(const View&) = delete;

		View(View&&) = default;
		View& operator=(View&&) = default;

		~View() = default;

		using unique_ptr::operator bool;

		using unique_ptr::get;

		using unique_ptr::release;
		using unique_ptr::reset;

		using unique_ptr::operator*;
		using unique_ptr::operator->;

		std::span<RandR::ModeInfo> modes() NYAAN_CNOEX
		{
			if (!*this) NYAAN_UNLIK
				return {};

			return std::span(
				xcb_randr_get_screen_resources_modes(get()),
				xcb_randr_get_screen_resources_modes_length(get())
			);
		}

		std::span<RandR::OutputID> outputs() NYAAN_CNOEX
		{
			if (!*this) NYAAN_UNLIK
				return {};

			return std::span(
				xcb_randr_get_screen_resources_outputs(get()),
				xcb_randr_get_screen_resources_outputs_length(get())
			);
		}
	};

	template<>
	struct View<GetUPtr<xcb_randr_get_output_info_reply_t>> final
		: GetUPtr<xcb_randr_get_output_info_reply_t>
	{
		using unique_ptr::element_type;
		using unique_ptr::pointer;

		using Type = element_type;

		View(unique_ptr reply = nullptr)
			: unique_ptr(std::move(reply)) {}

		View(const View&) = delete;
		View& operator=(const View&) = delete;

		View(View&&) = default;
		View& operator=(View&&) = default;

		~View() = default;

		using unique_ptr::operator bool;

		using unique_ptr::get;

		using unique_ptr::release;
		using unique_ptr::reset;

		using unique_ptr::operator*;
		using unique_ptr::operator->;

		bool valid() NYAAN_CNOEX
		{
			auto& ref = *operator->();
			return *this && ref.crtc != XCB_NONE
				&& ref.connection == XCB_RANDR_CONNECTION_CONNECTED;
		}

		Nyaanwork::Str name() NYAAN_CNOEX
		{
			if (!*this) NYAAN_UNLIK
				return "";

			return Nyaanwork::Str(
				reinterpret_cast<const char*>(xcb_randr_get_output_info_name(get())),
				xcb_randr_get_output_info_name_length(get())
			);
		}
	};
}
