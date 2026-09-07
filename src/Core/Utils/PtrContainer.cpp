module;

#include <Core/Defines.hpp>

#include <utility>

export module Nyaanwork.Core.Utils.PtrContainer;

#ifndef NDEBUG
import Nyaanwork.Core.Types;
import Nyaanwork.Core.Exception;

namespace Nyaanwork::Utils::Debug::Error
{
	enum class Code : ErrorCodeMask
	{
		no_error,

		null_ptr
	};
}

template<>
struct std::is_error_code_enum<Nyaanwork::Utils::Debug::Error::Code>
	: public std::true_type {};

namespace Nyaanwork::Utils::Debug::Error
{
	struct Category : BaseCategory<Code>
	{
		static constexpr std::array errors = {
			ErrorInfo{Code::no_error, Condition::no_error, "No error"},
			ErrorInfo{Code::null_ptr, Condition::invalid, "NullPtr"},
		};

		const char* name() const noexcept override
			{ return "Nyaanwork::Utils::Debug"; }

		Str message(int errc) const override
		{
			check_error_infos(errors);
			return message_helper(errors, errc);
		}

		bool equivalent(int i, const error_condition& c) const noexcept override
			{ return equivalent_helper(errors, i, c); }

		error_condition to_condition(int i) const noexcept override
			{ return to_condition_helper(errors, i); }
	};

	const Category& get_category() noexcept
	{
		static Category category;
		return category;
	}

	std::error_code make_error_code(Code err) noexcept
		{ return {static_cast<ErrorCodeMask>(err), get_category()}; }

	struct Exception : BaseException
	{
		Exception(Code error, src_loc src = src_loc::current())
			: BaseException(error, src) {}
	};

	#define DEBUG_NULLPTR \
		if constexpr (is_debug) \
		{ \
			if (!m_ptr) \
				throw Debug::Error::Exception(Debug::Error::Code::null_ptr); \
		}
}
#else
	#define DEBUG_NULLPTR
#endif

export namespace Nyaanwork::Utils
{
	template<typename _Type>
	class PtrContainer
	{
	public:
		using Type = _Type;

		PtrContainer(_Type& ref) noexcept
			: m_ptr(&ref) {}

		PtrContainer& operator=(Type& ref) noexcept
		{
			m_ptr = &ref;
			return *this;
		}

		PtrContainer(_Type* ptr = nullptr) noexcept
			: m_ptr(ptr) {}

		PtrContainer& operator=(Type* ptr) noexcept
		{
			m_ptr = ptr;
			return *this;
		}

		PtrContainer(const PtrContainer&) = default;
		PtrContainer& operator=(const PtrContainer&) = default;

		PtrContainer(PtrContainer&& oth) noexcept
			: m_ptr(std::exchange(oth.m_ptr, nullptr)) {}

		PtrContainer& operator=(PtrContainer&& oth) noexcept
		{
			if (this != &oth)
				m_ptr = std::exchange(oth.m_ptr, nullptr);

			return *this;
		}

		Type* get() NYAAN_CNOEX
			{ return m_ptr; }

		operator bool() NYAAN_CNOEX
			{ return m_ptr; }

		Type* operator->() noexcept(!is_debug)
		{
			DEBUG_NULLPTR;
			return m_ptr;
		}

		Type& operator*() noexcept(!is_debug)
		{
			DEBUG_NULLPTR;
			return *m_ptr;
		}

		const Type* operator->() NYAAN_CNOEX(!is_debug)
		{
			DEBUG_NULLPTR;
			return m_ptr;
		}

		const Type& operator*() NYAAN_CNOEX(!is_debug)
		{
			DEBUG_NULLPTR;
			return *m_ptr;
		}

		Type* ptr() noexcept(!is_debug)
		{
			DEBUG_NULLPTR;
			return m_ptr;
		}

		const Type* ptr() NYAAN_CNOEX(!is_debug)
		{
			DEBUG_NULLPTR;
			return m_ptr;
		}

	private:
		Type* m_ptr;
	};

	template<typename Type>
	requires std::is_void_v<Type>
	class PtrContainer<Type>
	{
	public:
		PtrContainer(Type* ptr = nullptr) noexcept
			: m_ptr(ptr) {}

		PtrContainer& operator=(Type* ptr) noexcept
		{
			m_ptr = ptr;
			return *this;
		}

		PtrContainer(const PtrContainer&) = default;
		PtrContainer& operator=(const PtrContainer&) = default;

		PtrContainer(PtrContainer&& oth) noexcept
			: m_ptr(std::exchange(oth.m_ptr, nullptr)) {}

		PtrContainer& operator=(PtrContainer&& oth) noexcept
		{
			if (this != &oth)
				m_ptr = std::exchange(oth.m_ptr, nullptr);

			return *this;
		}

		template<typename From>
		PtrContainer(const PtrContainer<From>& oth) noexcept
			: m_ptr(oth.get()) {}

		template<typename From>
		PtrContainer& operator=(const PtrContainer<From>& oth) noexcept
		{
			m_ptr = oth.get();
			return *this;
		}

		template<typename From>
		PtrContainer(PtrContainer<From>&& oth) noexcept
			: m_ptr(std::exchange(oth, nullptr).get()) {}

		template<typename From>
		PtrContainer& operator=(PtrContainer<From>&& oth) noexcept
		{
			m_ptr = std::exchange(oth, nullptr).get();
			return *this;
		}

		Type* get() NYAAN_CNOEX
			{ return m_ptr; }

		operator bool() NYAAN_CNOEX
			{ return m_ptr; }

		const Type* ptr() NYAAN_CNOEX(!is_debug)
		{
			DEBUG_NULLPTR;
			return m_ptr;
		}

		template<typename To>
		To* reinterpret_ptr_cast() noexcept(!is_debug)
		{
			DEBUG_NULLPTR;
			return reinterpret_cast<To*>(m_ptr);
		}

		template<typename To>
		const To* reinterpret_ptr_cast() NYAAN_CNOEX(!is_debug)
		{
			DEBUG_NULLPTR;
			return reinterpret_cast<const To*>(m_ptr);
		}

		template<typename To>
		PtrContainer<To> reinterpret_container_cast() noexcept(!is_debug)
			{ return PtrContainer<To>(reinterpret_ptr_cast<To>()); }

		template<typename To>
		PtrContainer<const To> reinterpret_container_cast() NYAAN_CNOEX(!is_debug)
			{ return PtrContainer<To>(reinterpret_ptr_cast<To>()); }

		template<typename To>
		To* static_ptr_cast() noexcept(!is_debug)
		{
			DEBUG_NULLPTR;
			return static_cast<To*>(m_ptr);
		}

		template<typename To>
		const To* static_ptr_cast() NYAAN_CNOEX(!is_debug)
		{
			DEBUG_NULLPTR;
			return static_cast<const To*>(m_ptr);
		}

		template<typename To>
		PtrContainer<To> static_container_cast() noexcept(!is_debug)
			{ return PtrContainer<To>(static_ptr_cast<To>()); }

		template<typename To>
		PtrContainer<const To> static_container_cast() NYAAN_CNOEX(!is_debug)
			{ return PtrContainer<const To>(static_ptr_cast<To>()); }

	private:
		Type* m_ptr;
	};
}

export namespace Nyaanwork
{
	using namespace Utils;
}
