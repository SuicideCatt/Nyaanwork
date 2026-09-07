#include <print>

import Nyaanwork.Core.Types;
import Nyaanwork.Core.Utils.Signals;

using namespace Nyaanwork;

enum class Slots
{
	a, b
};

struct Test
{
	Test() { std::println("Test: Constructor"); }
	Test(const Test&) { std::println("Test: Copy"); }
	Test(Test&&) { std::println("Test: Move"); }
};

void cref(const Test&) noexcept { std::println("call cref"); }
void copy(Test) { std::println("call copy"); }

struct Callback
{
	void cref(const Test& t) const noexcept
		{ ::cref(t); }

	void copy(Test t) const
		{ ::copy(t); }
};

template<bool single, template<typename...> typename SenderType>
void test(StrV name)
{
	std::print("{}", name);
	if constexpr (single)
		std::print(": SingleSlotInfo", name);
	std::println();

	using BaseSender = std::conditional_t<single,
			SenderType<Slots, SingleSlotInfo<false, Test>>,
			SignalsSender<Slots,
						  SlotInfo<Slots::a, false, Test>,
						  SlotInfo<Slots::b, false, Test>>>;

	struct Sender : BaseSender
	{
		using BaseSender::signal_send;
	} t;

	Callback cb;
	const Callback ccb;

	usize calls = 0;

	auto global_lambda = [](const Test&)
		{ std::println("global lambda"); };
	auto global_lambda_members = [&calls](const Test&)
		{ std::println("global lambda with ref members: calls = {}", ++calls); };

	t.template signal_connect<Slots::a, &cref>();
	t.template signal_connect<Slots::a, &copy>();
	t.template signal_connect<Slots::a, &Callback::cref>(cb);
	t.template signal_connect<Slots::a, &Callback::copy>(cb);
	t.template signal_connect<Slots::a>([](const Test&)
		{ std::println("lambda"); });
	t.template signal_connect<Slots::a>([&calls](const Test&)
		{ std::println("lambda with ref members: calls = {}", ++calls); } );
	t.template signal_connect<Slots::a>(global_lambda);
	t.template signal_connect<Slots::a>(global_lambda_members);

	t.template signal_connect<Slots::b, &cref>();
	t.template signal_connect<Slots::b, &copy>();
	t.template signal_connect<Slots::b, &Callback::cref>(ccb);
	t.template signal_connect<Slots::b, &Callback::copy>(ccb);
	t.template signal_connect<Slots::b>([](const Test&)
		{ std::println("lambda"); });
	t.template signal_connect<Slots::b>([&calls](const Test&)
		{ std::println("lambda with ref members: calls = {}", ++calls); } );
	t.template signal_connect<Slots::b>(global_lambda);
	t.template signal_connect<Slots::b>(global_lambda_members);

	std::println("--- call a ---");
	if constexpr (single)
		t.signal_send(Slots::a, Test());
	else
		t.template signal_send<Slots::a>(Test());

	std::println("\n--- call b ---");
	Test lt;
	t.template signal_send<Slots::b>(lt);

	std::println("\n\n");
}

int main(int argc, char** argv)
{
	test<false, SignalsSender>("SignalsSender");
	test<true, SignalsSender>("SignalsSender");
	test<false, SyncedSignalsSender>("SyncedSignalsSender");
	test<true, SyncedSignalsSender>("SyncedSignalsSender");

	return 0;
}
