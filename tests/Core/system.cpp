#include <print>

import Nyaanwork.Core.System;

using namespace Nyaanwork;

int main(int argc, char** argv)
{
	std::println("Executable path: {}", Nyaanwork::System::executable_path().string());

	return 0;
}
