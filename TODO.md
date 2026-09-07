# Nyaanwork 1.0.0 TODO

## Legend
- `` `` - todo
- ``B`` - broken
- ``-`` - removed
- ``/`` - work in progress
- ``~`` - complete, but not tested
- ``x`` - complete

## Core
- Maybe restruct
	- Move some Utils to root or other directory
- ``System``
	- get executable path
		- [x] Linux
		- [x] Windows
	- open by uri or path
		- [ ] Linux
		- [ ] Windows
	- *More?*

## Asset
- [ ] ``AudioBuffer``
- [ ] ``Loader``

## WindowSystem

### Instance
|           Feature            |Main interface|XCB|Windows|
|:-----------------------------|:------------:|:-:|:-----:|
|                 create window|       x      |xx | xxxx  |
|                 *get focused*|
|                   -> keyboard|       x      |xx | xxxB  |
|                      -> mouse|       x      |xx | xxBB  |
|      get keyboard layout name|       x      |xx |       |
|           *get displays info*|
|                       -> name|       x      |xx | xxxx  |
|                   -> position|       x      |xx | xxxx  |
|                 -> resolution|       x      |xx | xxxx  |
|               -> refresh rate|       x      |xx | xxxx  |
|              -> physical size|       x      |xx | xxxx  |
|                  -> bit-depth|              |   |       |
|        create opengl instance|       x      |xx | xxxx  |
|get vulkan instance extensions|       x      |xx | xxxx  |
|                   send notify|              |   |       |
|          write/read clipboard|              |   |       |
|                       *More?*|
### Window
|           Feature            |Main interface|XCB|Windows|
|:-----------------------------|:------------:|:-:|:-----:|
|                         title|       x      |xx | xxxx  |
|                          icon|       x      |xx | xxxx  |
|                      position|       x      |xx | xxxB  |
|                  *resolution*|
|                    -> min+max|       x      |xx | xxxx  |
|          -> window resolution|       x      |xx | xxxx  |
|                     resizable|       x      |xx | xxxx  |
|                  is minimized|       x      |xx | xxxB  |
|                  should close|       x      |xx | xxxx  |
|                    borderless|       x      |xx | xxxx  |
|                    fullscreen|       x      |xx | xxx/  |
|           transparent support|              |   |       |
|       progress bar in taskbar|              |   |       |
|                 drag and drop|       x      |xx | B//B  |
|          current display (id)|       x      |xx | xxxB  |
|                      *opengl*|
|             -> create surface|       x      |xx | xxxx  |
|     -> create surface+context|       x      |xx | xxxx  |
|         create surface vulkan|       x      |xx | xxxx  |
|                    *keyboard*|
|                  -> get focus|       x      |xx | xxxB  |
|                  -> key state|       x      |xx | xxxx  |
|              -> record string|       x      |xx | xxxx  |
|                       *mouse*|
|                  -> get focus|       x      |xx | xxBB  |
| -> set icon from system table|       x      |xx | xxxx  |
|          -> change visibility|       x      |xx | xxxx  |
|               -> set position|       x      |xx | xx/x  |
|               -> get position|       x      |xx | xxxx  |
|              -> buttons state|       x      |xx | xxxx  |
|                     -> motion|       x      |xx | /xxx  |
|                      -> wheel|       x      |xx | xxxB  |
|                       -> grab|       x      |xx | xxxx  |
|                      *tablet*|
|                   -> pressure|       x      |xx |       |
|                       -> tilt|       x      |xx |       |
|                       *More?*|

### Tests
Vulkan is only tested on
[DWM](https://dwm.suckless.org) and [Hyprland](https://hypr.land)
because my test PC doesn't support Vulkan

#### ``Linux.XCB``
1. X11 (KDE) (Vulkan tested on DWM)
2. Xwayland (KDE) (Vulkan tested on Hyprland)
#### ``Windows``
1. QEmu with ms windows (VK and GL tested not on any of my PCs)
2. Wine X11 (KDE)
3. Wine Xwayland (KDE)
4. Wine Wayland (KDE)

### Bugs
#### ``Backends.Linux.XCB``
- [ ] *NON CRITICAL* **(ONLY X11)**
	if not enabled grab mode and refocus: mouse not centralize on button press
- [ ] *NON CRITICAL* **(ONLY X11)**
	on keyboard focused: mouse is unfocus and focus again
- [ ] *NON CRITICAL* **(ONLY XWayland (maybe Hyprland))**
	on changing monitor mouse focused again
- [ ] *NON CRITICAL* **(X11 and Xwayland (KDE))**
	super + d: can not set minimized
#### ``Backends.Windows``
- [ ] *NON CRITICAL* why window surface is white?
- [ ] *NON CRITICAL*
	if enable and disable fullscreen window position goes down
- [ ] **CRITICAL** **(ONLY ms windows)**
	drag and drop work but you need move window for handle events
- [ ] *NON CRITICAL* **(ONLY Wine X11 and Xwayland (maybe KDE))**
	should close not work doesn't work the first time,
	need more events (any events)
- [ ] *NON CRITICAL* **(ONLY Wine X11 and Xwayland (maybe KDE))**
	drag and drop: text containt shit
- [ ] *NON CRITICAL* **(ONLY Wine Xwayland (maybe KDE))**
	mouse focus not work
- [ ] *NON CRITICAL* **(ONLY Wine Xwayland (maybe KDE))**
	set position work only if cursor hiden
	(classic Xwayland problem, see XCB backend)
- [ ] *NON CRITICAL* **(ONLY Wine Wayland (maybe KDE))**
	just not work

## Input
- HLI: High-level Interface
	- binary_state
		- [x] keyboard
		- [x] mouse buttons
		- [ ] gamepad buttons
		- [ ] gamepad axises
	- state (press, hold, release)
		- [x] keyboard
		- [x] mouse buttons
		- [ ] gamepad buttons
		- [ ] gamepad axises
	- action
		- [x] keyboard
		- [x] mouse buttons
		- [ ] gamepad buttons
		- [ ] gamepad axises
	- axis
		- [x] keyboard
		- [x] mouse buttons
		- [ ] mouse move
		- [ ] gamepad buttons
		- [ ] gamepad axises
- Gamepad
|           Feature            |Main interface|Linux|Windows|
|-----------------------------:|:------------:|:---:|:-----:|
|                              |              |     |       |

## ImGUI
ImGUI integration, similar to `imgui_impl_sdl2.h` or `imgui_impl_sdl3.h`.
