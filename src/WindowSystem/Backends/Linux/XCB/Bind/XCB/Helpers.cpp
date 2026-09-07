// i write this module only for Request and Reply pattern, but add Range and View

module;

#include <Core/Defines.hpp>

#include <xcb/xcb.h>

#include <cstdlib>
#include <iterator>

export module Nyaanwork.WindowSystem.Backends.Linux.XCB:Bind.XCB.Helper;

import Nyaanwork.Core.Types;
import Nyaanwork.Core.Utils.ClearType;
import Nyaanwork.Core.Utils.PtrContainer;
import Nyaanwork.Core.Utils.UPtr;

export namespace XCB::Helper
{
	template<typename Who>
	struct View
	{
		using Type = void;
	};
}

namespace XCB::Helper
{
	// my GetUPtr not work with std::free
	template<typename Who>
	void free(Who* ptr)
		{ return std::free(ptr); }

	template<typename Who>
	struct ViewOrT
	{
		using Type = Who;
	};

	template<typename Who>
	requires (!std::is_void_v<typename View<Who>::Type>)
	struct ViewOrT<Who>
	{
		using Type = View<Who>;
	};

	// if View containt Type and is not void Request or Range(Iterator) can use View
	template<typename Who>
	using ViewOr = ViewOrT<Who>::Type;

	template<typename Itr, auto next>
	class Iterator : public std::forward_iterator_tag
	{
	public:
		using xvalue_type = Nyaanwork::ClearType<decltype(Itr::data)>;
		using value_type = ViewOr<xvalue_type>;
		using difference_type = Nyaanwork::isize;

		static constexpr bool viewed = !std::same_as<value_type, xvalue_type>;

		using reference = std::conditional_t<viewed, value_type, value_type&>;
		using pointer = std::conditional_t<viewed, value_type, value_type*>;

		Iterator()
			: m_itr{} {}

		Iterator(Itr itr) noexcept
			: m_itr(itr) {}

		Iterator(const Iterator&) = default;
		Iterator& operator=(const Iterator&) = default;

		Iterator& operator++() noexcept
		{
			next(&m_itr);
			return *this;
		}
		Iterator operator++(int) noexcept
		{
			auto itr = *this;
			++(*this);

			return itr;
		}

		reference operator*() NYAAN_CNOEX
			{ return *m_itr.data; }

		pointer operator->() NYAAN_CNOEX
		{
			if constexpr (viewed)
				return *m_itr.data;
			else
				return m_itr.data;
		}

		bool operator==(const Iterator& oth) NYAAN_CNOEX
			{ return oth.m_itr.rem == m_itr.rem; }

	private:
		Itr m_itr;
	};
}

export namespace XCB::Helper
{
	template<typename Who>
	using GetUPtr = Nyaanwork::GetUPtr<Who, &free<Who>>;

	template<auto init, auto next,
			 typename InitSignature = decltype(init),
			 typename NextSignature = decltype(next)>
	struct Range;

	template<auto init, auto next,
			 typename Itr,
			 typename... InitArgs>
	struct Range<init, next, Itr(*)(InitArgs...), void(*)(Itr*)>
	{
		using iterator = Iterator<Itr, next>;

		using xvalue_type = iterator::xvalue_type;
		using value_type = iterator::value_type;

		Range() noexcept
			: itr{} {}

		Range(InitArgs... args) noexcept
			: itr(init(args...)) {}

		iterator begin() NYAAN_CNOEX
			{ return iterator(itr); }

		iterator end() NYAAN_CNOEX
			{ return iterator(Itr{}); }

		Itr itr;
	};

	template<auto _request, auto _reply,
			 typename RequestSignature = decltype(_request),
			 typename ReplySignature = decltype(_reply)>
	struct Request;

	template<auto _request, auto _reply,
			 typename _Cookie, typename _Reply,
			 typename... RequestArgs>
	struct Request<_request, _reply,
				   _Cookie(*)(xcb_connection_t*, RequestArgs...),
				   _Reply*(*)(xcb_connection_t*, _Cookie, xcb_generic_error_t**)>
	{
		using Cookie = _Cookie;

		using ReplyType = _Reply;
		using ReplyPtr = GetUPtr<_Reply>;
		using Reply = ViewOr<ReplyPtr>;

		static Cookie request(xcb_connection_t* c, RequestArgs... args) noexcept
			{ return _request(c, args...); }

		static Reply reply(xcb_connection_t* c, Cookie cookie) noexcept
			{ return ReplyPtr(_reply(c, cookie, nullptr)); }

		static Reply request_reply(xcb_connection_t* c, RequestArgs... args) noexcept
			{ return reply(c, request(c, args...)); }

		struct Package
		{
			using Request = Helper::Request<_request, _reply>;

			Package()
				: connection(nullptr), cookie{} {}

			Package(xcb_connection_t* connection, RequestArgs... args)
				: connection(connection), cookie(request(connection, args...)) {}

			Package(const Package&) = delete;
			Package& operator=(const Package&) = delete;

			Package(Package&&) = default;
			Package& operator=(Package&&) = default;

			~Package()
			{
				if (connection)
					wait();
			}

			void wait() noexcept
			{
				if (!reply) NYAAN_UNLIK
					reply = Request::reply(connection.ptr(), cookie);
			}

			Reply& get() noexcept
			{
				wait();
				return reply;
			}

			Nyaanwork::PtrContainer<xcb_connection_t> connection;
			Cookie cookie;
			Reply reply;
		};
	};
}
