export module Nyaanwork.WindowSystem.OpenGL:API;

import Nyaanwork.Core.Types;

namespace Nyaanwork::WindowSystem::OpenGL
{
	enum class API : u8
	{
		gl //, gles
	};

	enum class Profile : u8
	{
		core, compatibility
	};
}
