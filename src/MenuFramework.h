#pragma once

namespace CDoF::MenuFramework
{
	struct ImVec2
	{
		float x;
		float y;
	};

	struct WindowInterface
	{
		std::atomic<bool> isOpen{ false };
		std::atomic<bool> blockUserInput{ true };
	};

	using RenderCallback = void(__stdcall*)();

	bool IsLoaded();
	float GetVersion();
	bool AddSectionItem(std::string_view a_section, std::string_view a_item, RenderCallback a_callback);
	WindowInterface* AddWindow(RenderCallback a_callback, bool a_blockUserInput);

	bool Begin(const char* a_name, bool* a_open = nullptr, int a_flags = 0);
	void End();
	void Text(const char* a_text);
	void SeparatorText(const char* a_text);
	bool Checkbox(const char* a_label, bool* a_value);
	bool ColorEdit3(const char* a_label, float* a_color);
	bool SliderFloat(const char* a_label, float* a_value, float a_min, float a_max, const char* a_format);
	bool Button(const char* a_label);
	void SameLine();
	void ItemTooltip(const char* a_text);
	bool PushFont(const char* a_name);
	void PopFont();
}
