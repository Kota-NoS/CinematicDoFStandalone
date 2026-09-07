#pragma once

#include "Settings.h"

namespace CDoF::UI
{
	void Initialize(Settings a_settings);
	void TryRegister();
	void HandleKeyboardKey(std::uint32_t a_keyCode);
}
