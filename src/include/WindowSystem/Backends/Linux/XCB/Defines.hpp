#include <Core/Defines.hpp>

#define XCB_FLAG_HELPER(name, suf, X) name = suf##_##X

#define XCB_REQUEST_TYPE(name, func) \
	using name = Helper::Request<func, func##_reply>
