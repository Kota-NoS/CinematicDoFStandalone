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

CDoF::MenuFramework::WindowInterface* CDoF::MenuFramework::GetMainWindow()
{
	using Function = WindowInterface* (*)();
	const auto function = Resolve<Function>("GetMainWindow");
	return function ? function() : nullptr;
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
	using AddConvexPolyFilledFunction = void (*)(void*, const ImVec2*, int, std::uint32_t);
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
	const auto addConvexPolyFilled = Resolve<AddConvexPolyFilledFunction>("ImDrawList_AddConvexPolyFilled");
	const auto addRectFilled = Resolve<AddRectFilledFunction>("ImDrawList_AddRectFilled");
	const auto addTriangleFilled = Resolve<AddTriangleFilledFunction>("ImDrawList_AddTriangleFilled");
	const auto canDraw = getItemMin && getItemMax && getDrawList && addCircleFilled && addConvexPolyFilled &&
	                     addRectFilled && addTriangleFilled;
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

		// Repaint the upper-left half as one convex polygon. Drawing a triangle
		// fan produces anti-aliased internal seams and a bright center pinhole.
		constexpr auto pi = 3.14159265358979323846F;
		constexpr auto arcSegments = 16;
		// Screen-space Y grows downward, so 135..315 degrees is the
		// upper-left semicircle bounded by the rising diagonal.
		constexpr auto startAngle = pi * 0.75F;
		std::array<ImVec2, arcSegments + 1> upperLeftHalf{};
		for (auto point = 0; point <= arcSegments; ++point) {
			const auto angle = startAngle + pi * static_cast<float>(point) / static_cast<float>(arcSegments);
			upperLeftHalf[point] = ImVec2{
				circleCenter.x + std::cos(angle) * circleRadius,
				circleCenter.y + std::sin(angle) * circleRadius
			};
		}
		addConvexPolyFilled(drawList, upperLeftHalf.data(), static_cast<int>(upperLeftHalf.size()), white);
		if (a_active) {
			const auto lampRadius = std::max(1.4F, tileWidth * 0.085F);
			const ImVec2 lampCenter{ whiteMaximum.x - lampRadius * 1.35F, whiteMinimum.y + lampRadius * 1.35F };
			addCircleFilled(drawList, lampCenter, lampRadius, orange, 12);
		}
	}
	return pressed;
}

bool CDoF::MenuFramework::FocusPickerIconButton(const char* a_id)
{
	using ButtonFunction = bool (*)(const char*, ImVec2);
	using GetFrameHeightFunction = float (*)();
	using GetItemRectFunction = void (*)(ImVec2*);
	using GetDrawListFunction = void* (*)();
	using AddLineFunction = void (*)(void*, ImVec2, ImVec2, std::uint32_t, float);
	using AddRectFunction = void (*)(void*, ImVec2, ImVec2, std::uint32_t, float, int, float);

	const auto button = Resolve<ButtonFunction>("igButton");
	if (!button) {
		return false;
	}
	const auto getItemMin = Resolve<GetItemRectFunction>("igGetItemRectMin");
	const auto getItemMax = Resolve<GetItemRectFunction>("igGetItemRectMax");
	const auto getFrameHeight = Resolve<GetFrameHeightFunction>("igGetFrameHeight");
	const auto getDrawList = Resolve<GetDrawListFunction>("igGetWindowDrawList");
	const auto addLine = Resolve<AddLineFunction>("ImDrawList_AddLine");
	const auto addRect = Resolve<AddRectFunction>("ImDrawList_AddRect");
	const auto canDraw = getItemMin && getItemMax && getDrawList && addLine && addRect;
	const auto fallbackLabel = std::string("+\x20") + a_id;
	const auto buttonSide = getFrameHeight ? std::max(getFrameHeight(), 1.0F) : 28.0F;
	const auto pressed = button(canDraw ? a_id : fallbackLabel.c_str(), ImVec2{ buttonSide, buttonSide });
	if (!canDraw) {
		return pressed;
	}

	ImVec2 minimum{};
	ImVec2 maximum{};
	getItemMin(&minimum);
	getItemMax(&maximum);
	const auto center = ImVec2{ (minimum.x + maximum.x) * 0.5F, (minimum.y + maximum.y) * 0.5F };
	const auto side = std::max(1.0F, std::min(maximum.x - minimum.x, maximum.y - minimum.y));
	const auto arm = side * 0.30F;
	const auto box = side * 0.105F;
	const auto shadow = 0xFF000000U;
	const auto white = 0xFFFFFFFFU;
	if (auto* drawList = getDrawList()) {
		for (const auto [color, thickness] :
			{ std::pair{ shadow, 3.5F }, std::pair{ white, 1.6F } }) {
			addLine(drawList, ImVec2{ center.x - arm, center.y }, ImVec2{ center.x - box, center.y }, color, thickness);
			addLine(drawList, ImVec2{ center.x + box, center.y }, ImVec2{ center.x + arm, center.y }, color, thickness);
			addLine(drawList, ImVec2{ center.x, center.y - arm }, ImVec2{ center.x, center.y - box }, color, thickness);
			addLine(drawList, ImVec2{ center.x, center.y + box }, ImVec2{ center.x, center.y + arm }, color, thickness);
			addRect(drawList, ImVec2{ center.x - box, center.y - box }, ImVec2{ center.x + box, center.y + box }, color, 0.0F, 0, thickness);
		}
	}
	return pressed;
}

bool CDoF::MenuFramework::TargetLockIconButton(const char* a_id, bool a_active, bool a_available)
{
	using ButtonFunction = bool (*)(const char*, ImVec2);
	using GetFrameHeightFunction = float (*)();
	using GetItemRectFunction = void (*)(ImVec2*);
	using GetDrawListFunction = void* (*)();
	using AddLineFunction = void (*)(void*, ImVec2, ImVec2, std::uint32_t, float);
	using AddRectFunction = void (*)(void*, ImVec2, ImVec2, std::uint32_t, float, int, float);
	using AddRectFilledFunction = void (*)(void*, ImVec2, ImVec2, std::uint32_t, float, int);

	const auto button = Resolve<ButtonFunction>("igButton");
	if (!button) {
		return false;
	}
	const auto getItemMin = Resolve<GetItemRectFunction>("igGetItemRectMin");
	const auto getItemMax = Resolve<GetItemRectFunction>("igGetItemRectMax");
	const auto getFrameHeight = Resolve<GetFrameHeightFunction>("igGetFrameHeight");
	const auto getDrawList = Resolve<GetDrawListFunction>("igGetWindowDrawList");
	const auto addLine = Resolve<AddLineFunction>("ImDrawList_AddLine");
	const auto addRect = Resolve<AddRectFunction>("ImDrawList_AddRect");
	const auto addRectFilled = Resolve<AddRectFilledFunction>("ImDrawList_AddRectFilled");
	const auto canDraw = getItemMin && getItemMax && getDrawList && addLine && addRect && addRectFilled;
	const auto fallbackLabel = std::string(a_active ? "L*" : "L") + a_id;
	const auto buttonSide = getFrameHeight ? std::max(getFrameHeight(), 1.0F) : 28.0F;
	const auto pressed = button(canDraw ? a_id : fallbackLabel.c_str(), ImVec2{ buttonSide, buttonSide });
	if (!canDraw) {
		return a_available && pressed;
	}

	ImVec2 minimum{};
	ImVec2 maximum{};
	getItemMin(&minimum);
	getItemMax(&maximum);
	const auto center = ImVec2{ (minimum.x + maximum.x) * 0.5F, (minimum.y + maximum.y) * 0.5F };
	const auto side = std::max(1.0F, std::min(maximum.x - minimum.x, maximum.y - minimum.y));
	const auto half = side * 0.31F;
	const auto corner = side * 0.14F;
	const auto shadow = 0xFF000000U;
	const auto white = 0xFFFFFFFFU;
	const auto disabled = 0xFF777777U;
	const auto orange = 0xFF1F9EFFU;
	const auto color = !a_available ? disabled : (a_active ? orange : white);

	if (auto* drawList = getDrawList()) {
		for (const auto [strokeColor, thickness] :
			{ std::pair{ shadow, 3.4F }, std::pair{ color, 1.7F } }) {
			addLine(drawList, { center.x - half, center.y - half }, { center.x - half + corner, center.y - half }, strokeColor, thickness);
			addLine(drawList, { center.x - half, center.y - half }, { center.x - half, center.y - half + corner }, strokeColor, thickness);
			addLine(drawList, { center.x + half, center.y - half }, { center.x + half - corner, center.y - half }, strokeColor, thickness);
			addLine(drawList, { center.x + half, center.y - half }, { center.x + half, center.y - half + corner }, strokeColor, thickness);
			addLine(drawList, { center.x - half, center.y + half }, { center.x - half + corner, center.y + half }, strokeColor, thickness);
			addLine(drawList, { center.x - half, center.y + half }, { center.x - half, center.y + half - corner }, strokeColor, thickness);
			addLine(drawList, { center.x + half, center.y + half }, { center.x + half - corner, center.y + half }, strokeColor, thickness);
			addLine(drawList, { center.x + half, center.y + half }, { center.x + half, center.y + half - corner }, strokeColor, thickness);
		}

		const auto lockLeft = center.x - side * 0.005F;
		const auto lockRight = center.x + side * 0.330F;
		const auto lockTop = center.y - side * 0.005F;
		const auto lockBodyTop = center.y + side * 0.105F;
		const auto lockBottom = center.y + side * 0.315F;
		addRect(drawList, { lockLeft + side * 0.065F, lockTop }, { lockRight - side * 0.065F, lockBodyTop + side * 0.050F }, shadow, side * 0.065F, 0, 3.4F);
		addRect(drawList, { lockLeft + side * 0.065F, lockTop }, { lockRight - side * 0.065F, lockBodyTop + side * 0.050F }, color, side * 0.065F, 0, 1.7F);
		addRectFilled(drawList, { lockLeft, lockBodyTop }, { lockRight, lockBottom }, shadow, side * 0.035F, 0);
		const auto inset = side * 0.035F;
		addRectFilled(drawList, { lockLeft + inset, lockBodyTop + inset }, { lockRight - inset, lockBottom - inset }, color, side * 0.02F, 0);
	}
	return a_available && pressed;
}

bool CDoF::MenuFramework::DepthFrameIconButton(const char* a_id, bool a_active)
{
	using ButtonFunction = bool (*)(const char*, ImVec2);
	using GetFrameHeightFunction = float (*)();
	using GetItemRectFunction = void (*)(ImVec2*);
	using GetDrawListFunction = void* (*)();
	using AddConvexPolyFilledFunction = void (*)(void*, const ImVec2*, int, std::uint32_t);

	const auto button = Resolve<ButtonFunction>("igButton");
	if (!button) {
		return false;
	}
	const auto getItemMin = Resolve<GetItemRectFunction>("igGetItemRectMin");
	const auto getItemMax = Resolve<GetItemRectFunction>("igGetItemRectMax");
	const auto getFrameHeight = Resolve<GetFrameHeightFunction>("igGetFrameHeight");
	const auto getDrawList = Resolve<GetDrawListFunction>("igGetWindowDrawList");
	const auto addConvexPolyFilled = Resolve<AddConvexPolyFilledFunction>("ImDrawList_AddConvexPolyFilled");
	const auto canDraw = getItemMin && getItemMax && getDrawList && addConvexPolyFilled;
	const auto fallbackLabel = std::string(a_active ? "=*" : "=") + a_id;
	const auto buttonSide = getFrameHeight ? std::max(getFrameHeight(), 1.0F) : 28.0F;
	const auto pressed = button(canDraw ? a_id : fallbackLabel.c_str(), ImVec2{ buttonSide, buttonSide });
	if (!canDraw) {
		return pressed;
	}

	ImVec2 minimum{};
	ImVec2 maximum{};
	getItemMin(&minimum);
	getItemMax(&maximum);
	const auto center = ImVec2{ (minimum.x + maximum.x) * 0.5F, (minimum.y + maximum.y) * 0.5F };
	const auto side = std::max(1.0F, std::min(maximum.x - minimum.x, maximum.y - minimum.y));
	const auto white = 0xFFFFFFFFU;
	const auto orange = 0xFF1F9EFFU;
	const auto color = a_active ? orange : white;

    if (auto* drawList = getDrawList()) {
        const auto halfWidth = side * 0.29F;
        const auto halfHeight = side * 0.07F;
        for (const auto offset : { -0.21F, 0.21F }) {
            const auto y = center.y + offset * side;
            const ImVec2 corners[]{
                { center.x - halfWidth, y - halfHeight },
                { center.x + halfWidth, y - halfHeight },
                { center.x + halfWidth, y + halfHeight },
                { center.x - halfWidth, y + halfHeight }
            };
            addConvexPolyFilled(drawList, corners, 4, color);
        }
    }
    return pressed;
}

bool CDoF::MenuFramework::SneakIconButton(const char* a_id, bool a_active)
{
	using ButtonFunction = bool (*)(const char*, ImVec2);
	using GetFrameHeightFunction = float (*)();
	using GetItemRectFunction = void (*)(ImVec2*);
	using GetDrawListFunction = void* (*)();
	using AddConvexPolyFilledFunction = void (*)(void*, const ImVec2*, int, std::uint32_t);

	const auto button = Resolve<ButtonFunction>("igButton");
	if (!button) {
		return false;
	}
	const auto getItemMin = Resolve<GetItemRectFunction>("igGetItemRectMin");
	const auto getItemMax = Resolve<GetItemRectFunction>("igGetItemRectMax");
	const auto getFrameHeight = Resolve<GetFrameHeightFunction>("igGetFrameHeight");
	const auto getDrawList = Resolve<GetDrawListFunction>("igGetWindowDrawList");
	const auto addConvexPolyFilled = Resolve<AddConvexPolyFilledFunction>("ImDrawList_AddConvexPolyFilled");
	const auto canDraw = getItemMin && getItemMax && getDrawList && addConvexPolyFilled;
	const auto fallbackLabel = std::string(a_active ? "F*" : "F") + a_id;
	const auto buttonSide = getFrameHeight ? std::max(getFrameHeight(), 1.0F) : 28.0F;
	const auto pressed = button(canDraw ? a_id : fallbackLabel.c_str(), ImVec2{ buttonSide, buttonSide });
	if (!canDraw) {
		return pressed;
	}

	ImVec2 minimum{};
	ImVec2 maximum{};
	getItemMin(&minimum);
	getItemMax(&maximum);
	const auto center = ImVec2{ (minimum.x + maximum.x) * 0.5F, (minimum.y + maximum.y) * 0.5F };
	const auto side = std::max(1.0F, std::min(maximum.x - minimum.x, maximum.y - minimum.y));
	const auto shadow = 0xFF000000U;
	const auto white = 0xFFFFFFFFU;
	const auto orange = 0xFF1F9EFFU;
	const auto color = a_active ? orange : white;

	if (auto* drawList = getDrawList()) {
		const auto drawFootprint = [&](ImVec2 a_center, float a_angle) {
			const auto transform = [&](float a_x, float a_y, float a_scale) {
				const auto x = a_x * side * a_scale;
				const auto y = a_y * side * a_scale;
				return ImVec2{
					a_center.x + x * std::cos(a_angle) - y * std::sin(a_angle),
					a_center.y + x * std::sin(a_angle) + y * std::cos(a_angle)
				};
			};
			const auto drawShapes = [&](float a_scale, std::uint32_t a_color) {
				const std::array sole{
					transform(-0.090F, -0.190F, a_scale), transform(0.000F, -0.230F, a_scale),
					transform(0.090F, -0.190F, a_scale), transform(0.110F, -0.080F, a_scale),
					transform(0.075F, 0.090F, a_scale), transform(-0.075F, 0.090F, a_scale),
					transform(-0.110F, -0.080F, a_scale)
				};
				const std::array heel{
					transform(-0.070F, 0.145F, a_scale), transform(0.070F, 0.145F, a_scale),
					transform(0.080F, 0.235F, a_scale), transform(0.040F, 0.280F, a_scale),
					transform(-0.040F, 0.280F, a_scale), transform(-0.080F, 0.235F, a_scale)
				};
				addConvexPolyFilled(drawList, sole.data(), static_cast<int>(sole.size()), a_color);
				addConvexPolyFilled(drawList, heel.data(), static_cast<int>(heel.size()), a_color);
			};
			drawShapes(1.16F, shadow);
			drawShapes(1.00F, color);
		};

		drawFootprint({ center.x - side * 0.115F, center.y - side * 0.045F }, -0.14F);
		drawFootprint({ center.x + side * 0.125F, center.y + side * 0.045F }, 0.14F);
	}
	return pressed;
}

CDoF::MenuFramework::FocusPickerResult CDoF::MenuFramework::RenderFocusPicker(
	float& a_focusX,
	float& a_focusY)
{
	struct Viewport
	{
		std::uint32_t id;
		std::uint32_t flags;
		ImVec2 position;
		ImVec2 size;
	};

	using GetViewportFunction = Viewport* (*)();
	using GetMousePositionFunction = void (*)(ImVec2*);
	using GetDrawListFunction = void* (*)();
	using IsMouseClickedFunction = bool (*)(int, bool);
	using AddLineFunction = void (*)(void*, ImVec2, ImVec2, std::uint32_t, float);
	using AddRectFunction = void (*)(void*, ImVec2, ImVec2, std::uint32_t, float, int, float);

	const auto getViewport = Resolve<GetViewportFunction>("igGetMainViewport");
	const auto getMousePosition = Resolve<GetMousePositionFunction>("igGetMousePos");
	const auto getDrawList = Resolve<GetDrawListFunction>("igGetForegroundDrawList_Nil");
	const auto isMouseClicked = Resolve<IsMouseClickedFunction>("igIsMouseClicked_Bool");
	const auto addLine = Resolve<AddLineFunction>("ImDrawList_AddLine");
	const auto addRect = Resolve<AddRectFunction>("ImDrawList_AddRect");
	if (!getViewport || !getMousePosition || !getDrawList || !isMouseClicked || !addLine || !addRect) {
		return FocusPickerResult::kUnavailable;
	}

	const auto* viewport = getViewport();
	if (!viewport || viewport->size.x <= 1.0F || viewport->size.y <= 1.0F) {
		return FocusPickerResult::kUnavailable;
	}
	ImVec2 mouse{};
	getMousePosition(&mouse);
	a_focusX = std::clamp((mouse.x - viewport->position.x) / viewport->size.x, 0.0F, 1.0F);
	a_focusY = std::clamp((mouse.y - viewport->position.y) / viewport->size.y, 0.0F, 1.0F);

	const auto center = ImVec2{
		viewport->position.x + a_focusX * viewport->size.x,
		viewport->position.y + a_focusY * viewport->size.y
	};
	const auto left = viewport->position.x;
	const auto top = viewport->position.y;
	const auto right = left + viewport->size.x;
	const auto bottom = top + viewport->size.y;
	const auto markerHalfSize = std::clamp(std::min(viewport->size.x, viewport->size.y) * 0.012F, 7.0F, 18.0F);
	const auto shadow = 0xD0000000U;
	const auto white = 0xEFFFFFFFU;
	const auto orange = 0xFF1F9EFFU;
	if (auto* drawList = getDrawList()) {
		addLine(drawList, ImVec2{ left, center.y }, ImVec2{ right, center.y }, shadow, 4.0F);
		addLine(drawList, ImVec2{ center.x, top }, ImVec2{ center.x, bottom }, shadow, 4.0F);
		addLine(drawList, ImVec2{ left, center.y }, ImVec2{ right, center.y }, white, 1.8F);
		addLine(drawList, ImVec2{ center.x, top }, ImVec2{ center.x, bottom }, white, 1.8F);
		addRect(drawList, ImVec2{ center.x - markerHalfSize, center.y - markerHalfSize }, ImVec2{ center.x + markerHalfSize, center.y + markerHalfSize }, shadow, 0.0F, 0, 5.0F);
		addRect(drawList, ImVec2{ center.x - markerHalfSize, center.y - markerHalfSize }, ImVec2{ center.x + markerHalfSize, center.y + markerHalfSize }, orange, 0.0F, 0, 2.2F);
	}

	if (isMouseClicked(1, false)) {
		return FocusPickerResult::kCancelled;
	}
	if (isMouseClicked(0, false)) {
		return FocusPickerResult::kConfirmed;
	}
	return FocusPickerResult::kActive;
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
