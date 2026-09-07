#include "HotkeyInput.h"

#include "DoFUI.h"

namespace CDoF::HotkeyInput
{
	namespace
	{
		void ProcessKeyboardEvents(RE::InputEvent* const* a_events)
		{
			for (auto* event = a_events ? *a_events : nullptr; event; event = event->next) {
				if (event->GetEventType() != RE::INPUT_EVENT_TYPE::kButton ||
					event->GetDevice() != RE::INPUT_DEVICE::kKeyboard) {
					continue;
				}

				auto* button = event->AsButtonEvent();
				if (!button || !button->IsDown()) {
					continue;
				}

				UI::HandleKeyboardKey(button->GetIDCode());
			}
		}

		struct ProcessInputQueueHook
		{
			static void Thunk(
				RE::BSTEventSource<RE::InputEvent*>* a_dispatcher,
				RE::InputEvent* const* a_events)
			{
				// Read the original queue before UI frameworks optionally replace it
				// with an empty list while their menu is open.
				ProcessKeyboardEvents(a_events);
				function(a_dispatcher, a_events);
			}

			static inline REL::Relocation<decltype(Thunk)> function;
		};

		std::atomic_bool registered{};
	}

	void Register()
	{
		if (registered.load(std::memory_order_acquire)) {
			return;
		}

		static REL::Relocation<std::uintptr_t> processInputQueue{
			REL::RelocationID(67315, 68617),
			static_cast<std::ptrdiff_t>(REL::VariantOffset(0x7B, 0x7B, 0x81).offset())
		};
		ProcessInputQueueHook::function = SKSE::GetTrampoline().write_call<5>(
			processInputQueue.address(),
			ProcessInputQueueHook::Thunk);
		registered.store(true, std::memory_order_release);
		spdlog::info("Keyboard hotkey input hook installed at 0x{:X}", processInputQueue.address());
	}
}
