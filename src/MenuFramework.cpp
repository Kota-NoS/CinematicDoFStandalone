#include "MenuFramework.h"

namespace
{
	HMODULE GetFrameworkModule()
	{
		if (auto module = GetModuleHandleW(L"SKSEMenuFramework.dll")) {
			return module;
		}
		return GetModuleHandleW(L"SKSEMenuFramework");
	}

	template <class T>
	T Resolve(const char* a_name)
	{
		const auto module = GetFrameworkModule();
		return module ? reinterpret_cast<T>(GetProcAddress(module, a_name)) : nullptr;
	}
}

bool CDoF::MenuFramework::IsLoaded()
{
	return GetFrameworkModule() != nullptr;
}

float CDoF::MenuFramework::GetVersion()
{
	using Function = float (*)();
	const auto function = Resolve<Function>("GetMenuFrameworkVersion");
	return function ? function() : 0.0F;
}

bool CDoF::MenuFramework::AddSectionItem(
	std::string_view a_section,
	std::string_view a_item,
	RenderCallback a_callback)
{
	using Function = void (*)(const char*, RenderCallback);
	const auto function = Resolve<Function>("AddSectionItem");
	if (!function) {
		return false;
	}
	const auto path = std::string(a_section) + "/" + std::string(a_item);
	function(path.c_str(), a_callback);
	return true;
}

CDoF::MenuFramework::WindowInterface* CDoF::MenuFramework::AddWindow(
	RenderCallback a_callback,
	bool a_blockUserInput)
{
	using Function = WindowInterface* (*)(RenderCallback);
	const auto function = Resolve<Function>("AddWindow");
	if (!function) {
		return nullptr;
	}
	const auto window = function(a_callback);
	if (window) {
		window->blockUserInput = a_blockUserInput;
	}
	return window;
}

bool CDoF::MenuFramework::Begin(const char* a_name, bool* a_open, int a_flags)
{
	using Function = bool (*)(const char*, bool*, int);
	const auto function = Resolve<Function>("igBegin");
	return function ? function(a_name, a_open, a_flags) : false;
}

void CDoF::MenuFramework::End()
{
	using Function = void (*)();
	if (const auto function = Resolve<Function>("igEnd")) {
		function();
	}
}

void CDoF::MenuFramework::Text(const char* a_text)
{
	using Function = void (*)(const char*, const char*);
	if (const auto function = Resolve<Function>("igTextUnformatted")) {
		function(a_text, nullptr);
	}
}

void CDoF::MenuFramework::SeparatorText(const char* a_text)
{
	using Function = void (*)(const char*);
	if (const auto function = Resolve<Function>("igSeparatorText")) {
		function(a_text);
	}
}

bool CDoF::MenuFramework::Checkbox(const char* a_label, bool* a_value)
{
	using Function = bool (*)(const char*, bool*);
	const auto function = Resolve<Function>("igCheckbox");
	return function ? function(a_label, a_value) : false;
}

bool CDoF::MenuFramework::ColorEdit3(const char* a_label, float* a_color)
{
	using Function = bool (*)(const char*, float*, int);
	const auto function = Resolve<Function>("igColorEdit3");
	return function ? function(a_label, a_color, 0) : false;
}

bool CDoF::MenuFramework::SliderFloat(
	const char* a_label,
	float* a_value,
	float a_min,
	float a_max,
	const char* a_format)
{
	using Function = bool (*)(const char*, float*, float, float, const char*, int);
	const auto function = Resolve<Function>("igSliderFloat");
	return function ? function(a_label, a_value, a_min, a_max, a_format, 0) : false;
}

bool CDoF::MenuFramework::Button(const char* a_label)
{
	using Function = bool (*)(const char*, ImVec2);
	const auto function = Resolve<Function>("igButton");
	return function ? function(a_label, ImVec2{}) : false;
}

void CDoF::MenuFramework::SameLine()
{
	using Function = void (*)(float, float);
	if (const auto function = Resolve<Function>("igSameLine")) {
		function(0.0F, -1.0F);
	}
}

void CDoF::MenuFramework::ItemTooltip(const char* a_text)
{
	using IsItemHoveredFunction = bool (*)(int);
	using BeginTooltipFunction = bool (*)();
	using EndTooltipFunction = void (*)();
	using PushTextWrapFunction = void (*)(float);
	using PopTextWrapFunction = void (*)();
	const auto isItemHovered = Resolve<IsItemHoveredFunction>("igIsItemHovered");
	const auto beginTooltip = Resolve<BeginTooltipFunction>("igBeginTooltip");
	const auto endTooltip = Resolve<EndTooltipFunction>("igEndTooltip");
	const auto pushTextWrap = Resolve<PushTextWrapFunction>("igPushTextWrapPos");
	const auto popTextWrap = Resolve<PopTextWrapFunction>("igPopTextWrapPos");
	if (!isItemHovered || !beginTooltip || !endTooltip || !isItemHovered(0) || !beginTooltip()) {
		return;
	}
	if (pushTextWrap && popTextWrap) {
		pushTextWrap(450.0F);
		Text(a_text);
		popTextWrap();
	} else {
		Text(a_text);
	}
	endTooltip();
}

bool CDoF::MenuFramework::PushFont(const char* a_name)
{
	using Function = void (*)(const char*);
	if (const auto function = Resolve<Function>("PushFont")) {
		function(a_name);
		return true;
	}
	return false;
}

void CDoF::MenuFramework::PopFont()
{
	using Function = void (*)();
	if (const auto function = Resolve<Function>("Pop")) {
		function();
	}
}
