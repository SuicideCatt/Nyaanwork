<p align="center">
	<img src="logo.svg" width=100 alt=Nyaanwork>
</p>

# Nyaanwork

Windowing framework in C++23 and C++ Modules *(only)*
for make games or game engines, nyaa~~~~

If you want to help, see [`TODO.md`](TODO.md) or issues.

## How to install and init

You'll want to create base **C++**/**CMake** project and initialize git.

1. Install dependencies:

	On ArchLinux:
	```bash
	# Core
	sudo pacman -Sy ninja cmake python clang glm

	# WindowSystem
	sudo pacman -Sy libglvnd vulkan-headers vulkan-icd-loader

	# WindowSystem/Linux/XCB
	sudo pacman -Sy libx11 libxcb libxkbcommon libxkbcommon-x11
	```

2. Add Nyaanwork as git submodule:
```bash
git submodule add $URL path/to/third-party/modules/Nyaanwork
git submodule update --init --recursive
```

3. Add to main `CMakeLists.txt`:
```cmake
cmake_minimum_required(VERSION 3.30.9)

project(ProjectName LANGUAGES CXX) # init you project

add_subdirectory(path/to/Nyaanwork) # init Nyaanwork and cmake-kconfig

# You can add self made Kconfig menu
# read:
# 1. https://github.com/SuicideCatt/cmake-kconfig
# 2. https://www.kernel.org/doc/html/latest/kbuild/kconfig-language.html
#kconfig_add_menu(name Kconfig)

add_executable(${PROJECT_NAME} main.cpp) # add you executable
target_link_libraries(${PROJECT_NAME} PUBLIC Nyaanwork) # link Nyaanwork

# ninja run-main: rebuild(if needed) and run executable target
nyaanwork_run_target(main ${PROJECT_NAME} [args...])

# ninja run # auto call to 'ninja run-main', very convenient when you have a lot of run targets!
nyaanwork_run_target_default(main)
```

4. Init build environment:
```bash
export CC=/usr/bin/clang CXX="/usr/bin/clang++"
mkdir build && cd build
cmake .. -G Ninja
ninja menuconfig
```

5. Configure Nyaanwork on Kconfig setup menu.

## Hello Nyaanwork window and input

Nyaanwork has Core module, this module always enabled,
WindowSystem module provide Linux (X11) and Windows backeds,
and Input module provide HLI.

```cpp
import Nyaanwork.Core.Types;
import Nyaanwork.WindowSystem;

int main()
{
	using namespace Nyaanwork;

	WindowSystem::Instance instance;
	auto window_ptr = instance->create_window({
		.title = "Hello Nyaanwork!",
		.bounds = {
			.position = {300, 500},
			.resolution = {.value = {400, 400}}
		},
	});
	WindowSystem::Window& window = *window_ptr;

    Input::HLI hli;
	while (window->should_close())
	{
		hli.update(window);

		if (hli.state(Key::escape).release)
		{
			window->close();
			break;
		}
	}

	return 0;
}
```
