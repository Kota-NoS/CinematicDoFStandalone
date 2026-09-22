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

bool CDoF::MenuFramework::SilhouetteIconButton(const char* a_id, bool a_active)
{
	using ButtonFunction = bool (*)(const char*, ImVec2);
	using GetFrameHeightFunction = float (*)();
	using GetItemRectFunction = void (*)(ImVec2*);
	using GetDrawListFunction = void* (*)();
	using AddCircleFilledFunction = void (*)(void*, ImVec2, float, std::uint32_t, int);
	using AddRectFilledFunction = void (*)(void*, ImVec2, ImVec2, std::uint32_t, float, int);
	using AddTriangleFilledFunction = void (*)(void*, ImVec2, ImVec2, ImVec2, std::uint32_t);

	const auto button = Resolve<ButtonFunction>("igButton");
	if (!button) {
		return false;
	}
	const auto getItemMin = Resolve<GetItemRectFunction>("igGetItemRectMin");
	const auto getItemMax = Resolve<GetItemRectFunction>("igGetItemRectMax");
	const auto getFrameHeight = Resolve<GetFrameHeightFunction>("igGetFrameHeight");
	const auto getDrawList = Resolve<GetDrawListFunction>("igGetWindowDrawList");
	const auto addCircleFilled = Resolve<AddCircleFilledFunction>("ImDrawList_AddCircleFilled");
	const auto addRectFilled = Resolve<AddRectFilledFunction>("ImDrawList_AddRectFilled");
	const auto addTriangleFilled = Resolve<AddTriangleFilledFunction>("ImDrawList_AddTriangleFilled");
	const auto canDraw = getItemMin && getItemMax && getDrawList && addCircleFilled && addRectFilled && addTriangleFilled;
	const auto fallbackLabel = std::string("S") + a_id;

	// Keep the clickable frame in ImGui so hover, active, navigation, and input
	// behavior match every other Menu Framework button. The visible contrast
	// mark is drawn over an ID-only label and therefore does not depend on fonts.
	// An ASCII S remains as a compatibility fallback for older frameworks.
	const auto buttonSide = getFrameHeight ? std::max(getFrameHeight(), 1.0F) : 28.0F;
	const auto pressed = button(canDraw ? a_id : fallbackLabel.c_str(), ImVec2{ buttonSide, buttonSide });
	if (!canDraw) {
		return pressed;
	}

	ImVec2 minimum{};
	ImVec2 maximum{};
	getItemMin(&minimum);
	getItemMax(&maximum);
	const auto width = std::max(maximum.x - minimum.x, 1.0F);
	const auto height = std::max(maximum.y - minimum.y, 1.0F);
	const auto side = std::min(width, height);
	const auto left = minimum.x + (width - side) * 0.5F;
	const auto top = minimum.y + (height - side) * 0.5F;
	const auto outerInset = std::max(1.0F, side * 0.055F);
	const auto activeBorder = a_active ? std::max(1.5F, side * 0.075F) : 0.0F;
	const ImVec2 tileMinimum{ left + outerInset, top + outerInset };
	const ImVec2 tileMaximum{ left + side - outerInset, top + side - outerInset };
	const ImVec2 whiteMinimum{ tileMinimum.x + activeBorder, tileMinimum.y + activeBorder };
	const ImVec2 whiteMaximum{ tileMaximum.x - activeBorder, tileMaximum.y - activeBorder };
	const auto tileWidth = std::max(whiteMaximum.x - whiteMinimum.x, 1.0F);
	const auto tileHeight = std::max(whiteMaximum.y - whiteMinimum.y, 1.0F);
	const auto centerX = (whiteMinimum.x + whiteMaximum.x) * 0.5F;
	const auto black = 0xFF000000U;
	const auto white = 0xFFFFFFFFU;
	const auto orange = 0xFF1F9EFFU;  // ABGR
	const auto circleRadius = std::max(2.5F, std::min(tileWidth, tileHeight) * 0.34F);
	const ImVec2 circleCenter{ centerX, (whiteMinimum.y + whiteMaximum.y) * 0.5F };

	if (auto* drawList = getDrawList()) {
		if (a_active) {
			addRectFilled(drawList, tileMinimum, tileMaximum, orange, 2.0F, 0);
		}
		addRectFilled(drawList, whiteMinimum, whiteMaximum, white, 1.5F, 0);
		addTriangleFilled(
			drawList,
			whiteMinimum,
			ImVec2{ whiteMaximum.x, whiteMinimum.y },
			ImVec2{ whiteMinimum.x, whiteMaximum.y },
			black);
		addCircleFilled(drawList, circleCenter, circleRadius, black, 24);

		// Repaint the upper-left half of the circle white. Its diameter follows
		// the same bottom-left-to-top-right diagonal as the background split.
		constexpr auto pi = 3.14159265358979323846F;
		constexpr auto arcSegments = 16;
		// Screen-space Y grows downward, so 135..315 degrees is the
		// upper-left semicircle bounded by the rising diagonal.
		constexpr auto startAngle = pi * 0.75F;
		for (auto segment = 0; segment < arcSegments; ++segment) {
			const auto angle0 = startAngle + pi * static_cast<float>(segment) / static_cast<float>(arcSegments);
			const auto angle1 = startAngle + pi * static_cast<float>(segment + 1) / static_cast<float>(arcSegments);
			const ImVec2 point0{
				circleCenter.x + std::cos(angle0) * circleRadius,
				circleCenter.y + std::sin(angle0) * circleRadius
			};
			const ImVec2 point1{
				circleCenter.x + std::cos(angle1) * circleRadius,
				circleCenter.y + std::sin(angle1) * circleRadius
			};
			addTriangleFilled(drawList, circleCenter, point0, point1, white);
		}
		if (a_active) {
			const auto lampRadius = std::max(1.4F, tileWidth * 0.085F);
			const ImVec2 lampCenter{ whiteMaximum.x - lampRadius * 1.35F, whiteMinimum.y + lampRadius * 1.35F };
			addCircleFilled(drawList, lampCenter, lampRadius, orange, 12);
		}
	}
	return pressed;
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
