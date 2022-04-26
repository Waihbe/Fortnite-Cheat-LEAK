#include "stdafx.h"
#define reseller_name xorstr("Fortnite")
int boxmodepos = 0;
static int tab = 0;
ImFont* DefaultFont;

static const char* aimmodes[]
{
	"Memory",
	"Mouse",
	"Silent"
};
     
static const char* hitboxes[]
{
	"head",
	"neck",
	"body",
	"pelvis"
};

static const char* boxmodes[]
{
	"2D",
	"2D-Filled"
};

static const char* linemodes[]
{
	"bottom",
	"top",
	"center"
};

static const char* spinpitchmodes[]
{
	"static",
	"down",
	"up",
	"jitter",
	"jitter v2",
	"jitter v3",
	"random"
};

static const char* spinyawmodes[]
{
	"static",
	"spin slow",
	"spin fast",
	"jitter",
	"random"
};

namespace ImGui
{
	IMGUI_API bool Tab(unsigned int index, const char* label, int* selected, float width = 0, float height = 0)
	{
		ImGuiStyle& style = ImGui::GetStyle();
		ImVec4 color = style.Colors[ImGuiCol_Button];
		ImVec4 colorActive = style.Colors[ImGuiCol_ButtonActive];
		ImVec4 colorHover = style.Colors[ImGuiCol_ButtonHovered];

		if (index > 0)
			ImGui::SameLine();

		if (index == *selected)
		{
			style.Colors[ImGuiCol_Button] = colorActive;
			style.Colors[ImGuiCol_ButtonActive] = colorActive;
			style.Colors[ImGuiCol_ButtonHovered] = colorActive;
		}
		else
		{
			style.Colors[ImGuiCol_Button] = color;
			style.Colors[ImGuiCol_ButtonActive] = colorActive;
			style.Colors[ImGuiCol_ButtonHovered] = colorHover;
		}

		if (ImGui::Button(label, ImVec2(width, height)))
			*selected = index;

		style.Colors[ImGuiCol_Button] = color;
		style.Colors[ImGuiCol_ButtonActive] = colorActive;
		style.Colors[ImGuiCol_ButtonHovered] = colorHover;

		return *selected == index;
	}
}

ImVec4 backgroundColorRGB = ImVec4(255, 255, 255, 255);
ImVec4 innerColorRGB = ImVec4(219, 9, 72, 255);

ImVec4 backgroundColor = ImVec4(backgroundColorRGB.x / 255.f, backgroundColorRGB.y / 255.f, backgroundColorRGB.z / 255.f, backgroundColorRGB.w / 255.f);
ImVec4 innerColor = ImVec4(innerColorRGB.x / 255.f, innerColorRGB.y / 255.f, innerColorRGB.z / 255.f, innerColorRGB.w / 255.f);
ImVec4 textColor = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);


namespace utils {


	void ToggleButton(const char* str_id, bool* v)
	{
		ImVec2 p = ImGui::GetCursorScreenPos();
		ImDrawList* draw_list = ImGui::GetOverlayDrawList();

		float height = ImGui::GetFrameHeight() - 10;
		float width = height * 1.45f;
		float radius = height * 0.50f;

		ImGui::InvisibleButton(str_id, ImVec2(width, height));
		if (ImGui::IsItemClicked())
			*v = !*v;

		float t = *v ? 1.0f : 0.0f;

		ImGuiContext& g = *GImGui;
		float ANIM_SPEED = 0.05f;
		if (g.ActiveId == g.CurrentWindow->GetID(str_id))// && g.LastActiveIdTimer < ANIM_SPEED)
		{
			float t_anim = ImSaturate(g.ActiveIdTimer / ANIM_SPEED);
			t = *v ? (t_anim) : (1.0f - t_anim);
		}

		ImU32 col_bg;
		if (ImGui::IsItemHovered())
			col_bg = ImGui::GetColorU32(ImLerp(ImVec4(0.85f, 0.85f, 0.85f, 1.0f), innerColor, t));
		else
			col_bg = ImGui::GetColorU32(ImLerp(ImVec4(0.85f, 0.85f, 0.85f, 1.0f), innerColor, t));

		draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), col_bg, height * 0.5f);
		draw_list->AddCircleFilled(ImVec2(p.x + radius + t * (width - radius * 2.0f), p.y + radius), radius - 1.5f + 1.0f, IM_COL32(0, 0, 0, 255));
		draw_list->AddCircleFilled(ImVec2(p.x + radius + t * (width - radius * 2.0f), p.y + radius), radius - 1.5f, IM_COL32(255, 255, 255, 255));
	}

	void ToggleButtonA(const char* str_id, bool* v, int A)
	{
		ImVec2 p = ImGui::GetCursorScreenPos();
		ImDrawList* draw_list = ImGui::GetOverlayDrawList();

		float height = ImGui::GetFrameHeight() - 10;
		float width = height * 1.45f;
		float radius = height * 0.50f;

		ImGui::InvisibleButton(str_id, ImVec2(width, height));
		if (ImGui::IsItemClicked()) {
			if (A == 1) {
				Settings.SilentAimbot = false;
			}
			else if (A == 2) {
				Settings.Aimbot = false;
			}
			else if (A == 3) {
				Settings.Aimbot = false;
				Settings.SilentAimbot = false;
			}
			*v = !*v;
		}

		float t = *v ? 1.0f : 0.0f;

		ImGuiContext& g = *GImGui;
		float ANIM_SPEED = 0.05f;
		if (g.ActiveId == g.CurrentWindow->GetID(str_id))// && g.LastActiveIdTimer < ANIM_SPEED)
		{
			float t_anim = ImSaturate(g.ActiveIdTimer / ANIM_SPEED);
			t = *v ? (t_anim) : (1.0f - t_anim);
		}

		ImU32 col_bg;
		if (ImGui::IsItemHovered())
			col_bg = ImGui::GetColorU32(ImLerp(ImVec4(0.85f, 0.85f, 0.85f, 1.0f), innerColor, t));
		else
			col_bg = ImGui::GetColorU32(ImLerp(ImVec4(0.85f, 0.85f, 0.85f, 1.0f), innerColor, t));

		draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), col_bg, height * 0.5f);
		draw_list->AddCircleFilled(ImVec2(p.x + radius + t * (width - radius * 2.0f), p.y + radius), radius - 1.5f + 1.0f, IM_COL32(0, 0, 0, 255));
		draw_list->AddCircleFilled(ImVec2(p.x + radius + t * (width - radius * 2.0f), p.y + radius), radius - 1.5f, IM_COL32(255, 255, 255, 255));
	}

	void PushDisabled(bool disabled = true) {
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, disabled);
	}
	void PopDisabled(int num = 1) {
		while (num--) {
			ImGui::PopItemFlag();
		}
	}
}

namespace Menu
{
	void DrawMenu()
	{

		// menu here
		ImGui::StyleColorsLight();

		ImGuiStyle* style = &ImGui::GetStyle();
		style->WindowPadding = ImVec2(15, 15);
		style->WindowRounding = 1.0f;
		style->FramePadding = ImVec2(5, 5);
		style->ItemSpacing = ImVec2(12, 8);
		style->ItemInnerSpacing = ImVec2(8, 6);
		style->IndentSpacing = 25.0f;
		style->ScrollbarSize = 0.0f;
		style->ScrollbarRounding = 1.0f;
		style->GrabMinSize = 5.0f;
		style->GrabRounding = 3.0f;
		style->WindowTitleAlign = ImVec2(0.06, 1.1);
		style->Alpha = 1.f;
		// title 
		ImGui::PushStyleColor(ImGuiCol_TitleBgActive, backgroundColor);
		ImGui::PushStyleColor(ImGuiCol_TitleBg, backgroundColor);
		ImGui::PushStyleColor(ImGuiCol_TitleBgCollapsed, backgroundColor);
		ImGui::PushStyleColor(ImGuiCol_Border, innerColor); // 
		ImGui::PushStyleColor(ImGuiCol_BorderShadow, ImVec4(0.00f, 0.00f, 0.00f, 0.00f)); //
		// window
		ImGui::PushStyleColor(ImGuiCol_Text, textColor);
		ImGui::PushStyleColor(ImGuiCol_TextDisabled, textColor);
		ImGui::PushStyleColor(ImGuiCol_TextSelectedBg, textColor);
		ImGui::PushStyleColor(ImGuiCol_WindowBg, backgroundColor);
		ImGui::PushStyleColor(ImGuiCol_Border, innerColor);
		// button
		ImGui::PushStyleColor(ImGuiCol_Button, backgroundColor);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, backgroundColor);
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, backgroundColor);
		ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4(0.40f, 0.39f, 0.38f, 1.00f));
		ImGui::PushStyleColor(ImGuiCol_PlotLinesHovered, ImVec4(0.25f, 1.00f, 0.00f, 1.00f));
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.40f, 0.39f, 0.38f, 1.00f));
		ImGui::PushStyleColor(ImGuiCol_PlotHistogramHovered, ImVec4(0.25f, 1.00f, 0.00f, 1.00f));
		ImGui::PushStyleColor(ImGuiCol_TextSelectedBg, ImVec4(0.25f, 1.00f, 0.00f, 1.00f));
		ImGui::PushStyleColor(ImGuiCol_ModalWindowDarkening, ImVec4(94 / 255.f, 94 / 255.f, 94 / 255.f, 1.00f));
		// Slider
		ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(94 / 255.f, 94 / 255.f, 94 / 255.f, 1.00f));
		ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(94 / 255.f, 94 / 255.f, 94 / 255.f, 1.00f));
		ImGui::PushStyleColor(ImGuiCol_FrameBg, innerColor);
		ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, innerColor);
		ImGui::PushStyleColor(ImGuiCol_FrameBgActive, innerColor);
		ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(0.48f, 0.48f, 0.48f, 1.00f));

		std::string title = ("FORTNITE (Build: ");
		title += __DATE__;
		title += (" ");
		title += __TIME__;
		title += (")");
		ImGuiWindowFlags TargetFlags;
		TargetFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;
		bool trueSend = false;
		bool falseSend = false;

		if (ImGui::Begin(title.c_str(), 0, ImVec2(0, 600), -1.f, TargetFlags)) {
			ImVec2 ve = ImVec2(ImGui::GetItemRectMin().x + 15, ImGui::GetItemRectMin().y + 80);
			ImVec2 v = ImVec2(ImGui::GetItemRectMax().x - 15, ImGui::GetItemRectMax().y + 560);
			ImGui::GetOverlayDrawList()->AddRect(ve, v, IM_COL32(innerColorRGB.x, innerColorRGB.y, innerColorRGB.z, innerColorRGB.w));

			static short tab = 0;
			ImGui::BeginGroup();
			if (tab == 0) {
				ImGui::PushStyleColor(ImGuiCol_Button, innerColor);
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, innerColor);
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.00f, 1.00f, 1.00f, 1.00f));
			}
			if (tab == 0)
				utils::PushDisabled(true);
			else
				utils::PushDisabled(false);
			bool tab_aimbot = ImGui::Button(("AIMBOT"), ImVec2(150, 23));
			utils::PopDisabled();
			if (tab == 0)
				ImGui::PopStyleColor(3);
			ImGui::SameLine(NULL, 0);
			if (tab == 1) {
				ImGui::PushStyleColor(ImGuiCol_Button, innerColor);
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, innerColor);
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.00f, 1.00f, 1.00f, 1.00f));
			}
			if (tab == 1)
				utils::PushDisabled(true);
			else
				utils::PushDisabled(false);
			bool tab_visuals = ImGui::Button(("VISUALS"), ImVec2(150, 23));
			utils::PopDisabled();
			if (tab == 1)
				ImGui::PopStyleColor(3);
			ImGui::SameLine(NULL, 0);
			if (tab == 2) {
				ImGui::PushStyleColor(ImGuiCol_Button, innerColor);
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, innerColor);
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.00f, 1.00f, 1.00f, 1.00f));
			}
			if (tab == 2)
				utils::PushDisabled(true);
			else
				utils::PushDisabled(false);
			bool tab_misc = ImGui::Button(("MISC & EXPLOITS"), ImVec2(150, 23));
			utils::PopDisabled();
			if (tab == 2)
				ImGui::PopStyleColor(3);
			ImGui::EndGroup();
			ImGui::GetOverlayDrawList()->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(innerColorRGB.x, innerColorRGB.y, innerColorRGB.z, innerColorRGB.w));

			if (tab_aimbot) {
				tab = 0;
			}
			if (tab_visuals) {
				tab = 1;
			}
			if (tab_misc) {
				tab = 2;
			}
			ImGui::Spacing();

			if (tab == 0) { // AIMBOT TAB
				ImGui::BeginGroup();
				ImGui::Dummy(ImVec2(0.0f, 0.0f));
				ImGui::Spacing();
				ImGui::Spacing();

				ImGui::Dummy(ImVec2(0.0f, 0.0f));
				ImGui::SameLine(NULL, 9);
				utils::ToggleButtonA(("Memory Aimbot"), &Settings.Aimbot, 1);
				ImGui::SameLine(NULL, 8);
				ImGui::Text(("Memory Aimbot"));

				if (Settings.Aimbot) {
					ImGui::Dummy(ImVec2(0.0f, 0.0f));
					ImGui::SameLine(NULL, 9);
					utils::ToggleButton(("[OPTION] Aimbot Lock"), &Settings.AimHelper);
					ImGui::SameLine(NULL, 8);
					ImGui::Text(("[OPTION] Aimbot Lock"));
				}

				ImGui::Dummy(ImVec2(0.0f, 0.0f));
				ImGui::SameLine(NULL, 9);
				utils::ToggleButtonA(("Silent Aimbot"), &Settings.SilentAimbot, 2);
				ImGui::SameLine(NULL, 8);
				ImGui::Text(("Silent Aimbot"));

				ImGui::Dummy(ImVec2(0.0f, 0.0f));
				ImGui::SameLine(NULL, 9);
				utils::ToggleButton(("Draw crosshair"), &Settings.Crosshair);
				ImGui::SameLine(NULL, 8);
				ImGui::Text(("Draw crosshair"));

				ImGui::Dummy(ImVec2(0.0f, 0.0f));
				ImGui::SameLine(NULL, 9);
				utils::ToggleButton(("Draw FOV"), &Settings.ESP.AimbotFOV);
				ImGui::SameLine(NULL, 8);
				ImGui::Text(("Draw FOV"));

				ImGui::Dummy(ImVec2(0.0f, 0.0f));
				ImGui::SameLine(NULL, 9);
				utils::ToggleButton(("Random aim pos"), &Settings.ESP.RandomAim);
				ImGui::SameLine(NULL, 8);
				ImGui::Text(("Random aim pos"));

				ImGui::Dummy(ImVec2(6, 6));
				ImGui::Dummy(ImVec2(0.0f, 0.0f));
				ImGui::SameLine(NULL, 9);
				ImGui::PushItemWidth(200);
				ImGui::SliderFloat(("Aim FOV"), &Settings.AimbotFOV, 1.0f, 80.f);

				ImGui::Dummy(ImVec2(6, 6));
				ImGui::Dummy(ImVec2(0.0f, 0.0f));
				ImGui::SameLine(NULL, 9);
				ImGui::PushItemWidth(200);
				ImGui::SliderFloat(("Aim Smooth"), &Settings.AimbotSlow, 0.0f, 25.f);



				ImGui::Dummy(ImVec2(6, 6));
				ImGui::Dummy(ImVec2(0.0f, 0.0f));
				ImGui::SameLine(NULL, 9);
				ImGui::PushItemWidth(200);
				ImGui::SliderFloat(("Crosshair scale"), &Settings.crosshairNuclear, 1, 10);
				Settings.crosshairScale = Settings.crosshairNuclear + 7;

				ImGui::EndGroup();

				//////////////////////////////////////////
				ImGui::SameLine(ImGui::GetWindowWidth() - 111/*111: size of the button*/ - 10);
				ImGui::BeginGroup();
				ImGui::Dummy(ImVec2(0.0f, 5.0f));
				ImGui::Spacing();


				ImGui::EndGroup();
			}

			if (tab == 1) { // VISUALS TAB
				ImGui::BeginGroup();
				ImGui::Dummy(ImVec2(0.0f, 0.0f));
				ImGui::Spacing();
				ImGui::Spacing();

				ImGui::Dummy(ImVec2(0.0f, 0.0f));
				ImGui::SameLine(NULL, 9);
				utils::ToggleButton(("Enable visuals (F2)"), &Settings.realplayertags);
				ImGui::SameLine(NULL, 8);
				ImGui::Text(("Enable visuals (F2)"));

				ImGui::Dummy(ImVec2(0.0f, 0.0f));
				ImGui::SameLine(NULL, 9);
				utils::ToggleButton(("Show Distance"), &Settings.ESP.Distance);
				ImGui::SameLine(NULL, 8);
				ImGui::Text(("Show Distance"));

				utils::PushDisabled(true);
				ImGui::Dummy(ImVec2(0.0f, 0.0f));
				ImGui::SameLine(NULL, 9);
				utils::ToggleButton(("Show Player Names [Disabled]"), &Settings.ESP.PlayerNames);
				ImGui::SameLine(NULL, 8);
				ImGui::Text(("Show Player Names [Disabled]"));
				utils::PopDisabled();

				ImGui::Dummy(ImVec2(0.0f, 0.0f));
				ImGui::SameLine(NULL, 9);
				utils::ToggleButton(("Show Player Weapons"), &Settings.ESP.PlayerWeapons);
				ImGui::SameLine(NULL, 8);
				ImGui::Text(("Show Player Weapons"));

				ImGui::Dummy(ImVec2(0.0f, 0.0f));
				ImGui::SameLine(NULL, 9);
				utils::ToggleButton(("Player Skeleton"), &Settings.ESP.Skeleton);
				ImGui::SameLine(NULL, 8);
				ImGui::Text(("Player Skeleton"));

				ImGui::Dummy(ImVec2(0.0f, 0.0f));
				ImGui::SameLine(NULL, 9);
				utils::ToggleButton(("Player Box"), &Settings.ESP.Boxes);
				ImGui::SameLine(NULL, 8);
				ImGui::Text(("Player Box"));

				if (Settings.ESP.Box) {
					ImGui::Dummy(ImVec2(0.0f, 0.0f));
					ImGui::SameLine(NULL, 9);
					utils::ToggleButton(("[OPTION] 3D Box"), &Settings.ESP.ThreeDBox);
					ImGui::SameLine(NULL, 8);
					ImGui::Text(("[OPTION] 3D Box"));
				}

				ImGui::Dummy(ImVec2(0.0f, 0.0f));
				ImGui::SameLine(NULL, 9);
				utils::ToggleButton(("Weapons ESP"), &Settings.ESP.Weapons);
				ImGui::SameLine(NULL, 8);
				ImGui::Text(("Weapons ESP"));

				ImGui::Dummy(ImVec2(0.0f, 0.0f));
				ImGui::SameLine(NULL, 9);
				utils::ToggleButton(("Upgrade Bench ESP"), &Settings.ESP.UpgradeBench);
				ImGui::SameLine(NULL, 8);
				ImGui::Text(("Upgrade Bench ESP"));

				ImGui::Dummy(ImVec2(0.0f, 0.0f));
				ImGui::SameLine(NULL, 9);
				utils::ToggleButton(("Ammo ESP"), &Settings.ESP.Ammo);
				ImGui::SameLine(NULL, 8);
				ImGui::Text(("Ammo ESP"));

				ImGui::Dummy(ImVec2(0.0f, 0.0f));
				ImGui::SameLine(NULL, 9);
				utils::ToggleButton(("Chests ESP"), &Settings.ESP.Containers);
				ImGui::SameLine(NULL, 8);
				ImGui::Text(("Chests ESP"));

				ImGui::Dummy(ImVec2(0.0f, 0.0f));
				ImGui::SameLine(NULL, 9);
				utils::ToggleButton(("Ammo Box ESP"), &Settings.ESP.AmmoBox);
				ImGui::SameLine(NULL, 8);
				ImGui::Text(("Ammo Box ESP"));

				ImGui::Dummy(ImVec2(0.0f, 0.0f));
				ImGui::SameLine(NULL, 9);
				utils::ToggleButton(("Llama ESP"), &Settings.ESP.Llama);
				ImGui::SameLine(NULL, 8);
				ImGui::Text(("Llama ESP"));

				ImGui::Dummy(ImVec2(0.0f, 0.0f));
				ImGui::SameLine(NULL, 9);
				utils::ToggleButton(("Supply Drop ESP"), &Settings.ESP.SupplyDrop);
				ImGui::SameLine(NULL, 8);
				ImGui::Text(("Supply Drop ESP"));

				ImGui::Dummy(ImVec2(0.0f, 0.0f));
				ImGui::SameLine(NULL, 9);
				utils::ToggleButton(("Agents/Boss ESP"), &Settings.ESP.AgentsBoss);
				ImGui::SameLine(NULL, 8);
				ImGui::Text(("Agents/Boss ESP"));

				ImGui::Dummy(ImVec2(0.0f, 0.0f));
				ImGui::SameLine(NULL, 9);
				utils::ToggleButton(("Shark ESP"), &Settings.ESP.Shark);
				ImGui::SameLine(NULL, 8);
				ImGui::Text(("Shark ESP"));

				ImGui::Dummy(ImVec2(0.0f, 0.0f));
				ImGui::SameLine(NULL, 9);
				utils::ToggleButton(("Boats ESP"), &Settings.ESP.Boats);
				ImGui::SameLine(NULL, 8);
				ImGui::Text(("Boats ESP"));

				ImGui::Dummy(ImVec2(0.0f, 0.0f));
				ImGui::SameLine(NULL, 9);
				utils::ToggleButton(("Helicopters ESP"), &Settings.ESP.Helicopters);
				ImGui::SameLine(NULL, 8);
				ImGui::Text(("Helicopters ESP"));

				ImGui::Dummy(ImVec2(0.0f, 0.0f));
				ImGui::SameLine(NULL, 9);
				utils::ToggleButton(("Cars ESP"), &Settings.ESP.Cars);
				ImGui::SameLine(NULL, 8);
				ImGui::Text(("Cars ESP"));

				ImGui::Dummy(ImVec2(0.0f, 0.0f));
				ImGui::SameLine(NULL, 9);
				utils::ToggleButton(("Trucks ESP"), &Settings.ESP.Trucks);
				ImGui::SameLine(NULL, 8);
				ImGui::Text(("Trucks ESP"));

				ImGui::Dummy(ImVec2(0.0f, 0.0f));
				ImGui::SameLine(NULL, 9);
				utils::ToggleButton(("Taxi ESP"), &Settings.ESP.Taxi);
				ImGui::SameLine(NULL, 8);
				ImGui::Text(("Taxi ESP"));

				ImGui::EndGroup();

				//////////////////////////////////////////
				ImGui::SameLine(NULL, 0/*47*/);
				ImGui::BeginGroup();
				ImGui::Dummy(ImVec2(0.0f, 5.0f));
				ImGui::Spacing();

				ImGui::Dummy(ImVec2(6, 6));
				ImGui::Dummy(ImVec2(0.0f, 0.0f));
				ImGui::SameLine(NULL, 9);
				ImGui::PushItemWidth(200);
				ImGui::SliderInt(("Min Items Tier"), &Settings.ESP.MinWeaponTier, 1, 5);

				ImGui::Dummy(ImVec2(6, 6));
				ImGui::Dummy(ImVec2(0.0f, 0.0f));
				ImGui::SameLine(NULL, 9);
				ImGui::PushItemWidth(200);
				ImGui::SliderInt(("Items max distance"), &Settings.OverallDistance, 50, 500);

				ImGui::Dummy(ImVec2(6, 6));
				ImGui::Dummy(ImVec2(0.0f, 0.0f));
				ImGui::SameLine(NULL, 9);
				ImGui::PushItemWidth(200);
				ImGui::SliderFloat(("Line thickness"), &Settings.SnaplinePower, 1.f, 10.f);

				ImGui::Dummy(ImVec2(0.0f, 0.0f));
				ImGui::SameLine(NULL, 6);
				utils::ToggleButton(("Corner box"), &Settings.ESP.CornerBox);
				ImGui::SameLine(NULL, 8);
				ImGui::Text(("Corner box"));

				ImGui::Dummy(ImVec2(0.0f, 0.0f));
				ImGui::SameLine(NULL, 6);
				utils::ToggleButton(("Draw Direction Lazer"), &Settings.ESP.Lazer);
				ImGui::SameLine(NULL, 8);
				ImGui::Text(("Draw Direction Lazer"));

				ImGui::Dummy(ImVec2(0.0f, 0.0f));
				ImGui::SameLine(NULL, 6);
				utils::ToggleButton(("Enable visible check"), &Settings.VisibleCheck);
				ImGui::SameLine(NULL, 8);
				ImGui::Text(("Enable visible check"));

				ImGui::Dummy(ImVec2(0.0f, 0.0f));
				ImGui::SameLine(NULL, 6);
				utils::ToggleButton(("Aim Line"), &Settings.ESP.AimLine);
				ImGui::SameLine(NULL, 8);
				ImGui::Text(("Aim Line"));

				ImGui::Dummy(ImVec2(0.0f, 0.0f));
				ImGui::SameLine(NULL, 6);
				utils::ToggleButton(("XP Coin"), &Settings.ESP.XPCoin);
				ImGui::SameLine(NULL, 8);
				ImGui::Text(("XP Coin"));

				ImGui::Dummy(ImVec2(0.0f, 0.0f));
				ImGui::SameLine(NULL, 6);
				utils::ToggleButton(("Draw snaplines"), &Settings.ESP.PlayerLines);
				ImGui::SameLine(NULL, 8);
				ImGui::Text(("Draw snaplines"));

				ImGui::Dummy(ImVec2(0.0f, 0.0f));
				ImGui::SameLine(NULL, 6);
				utils::ToggleButton(("Square FOV"), &Settings.isRecFov);
				ImGui::SameLine(NULL, 8);
				ImGui::Text(("Square FOV"));

				ImGui::EndGroup();

			}
			if (tab == 2) { // EXPLOITS TAB
				ImGui::BeginGroup();
				ImGui::Dummy(ImVec2(0.0f, 0.0f));
				ImGui::Spacing();
				ImGui::Spacing();

				ImGui::Dummy(ImVec2(0.0f, 0.0f));
				ImGui::SameLine(NULL, 9);
				utils::ToggleButton(("Enable Info Bar"), &Settings.Info);
				ImGui::SameLine(NULL, 8);
				ImGui::Text(("Enable Info Bar"));
				
				ImGui::Dummy(ImVec2(0.0f, 0.0f));
				ImGui::SameLine(NULL, 9);
				utils::ToggleButton(("Show warning about enemies in the area"), &Settings.WarningEnemiesArea);
				ImGui::SameLine(NULL, 8);
				ImGui::Text(("Show warning about enemies in the area"));

				ImGui::Dummy(ImVec2(0.0f, 0.0f));
				ImGui::SameLine(NULL, 9);
				utils::ToggleButton(("Enable Radar ESP"), &Settings.RadarESP);
				ImGui::SameLine(NULL, 8);
				ImGui::Text(("Enable Radar ESP"));

				if (Settings.RadarESP) {
					ImGui::Dummy(ImVec2(6, 6));
					ImGui::Dummy(ImVec2(0.0f, 0.0f));
					ImGui::SameLine(NULL, 9);
					ImGui::PushItemWidth(200);
					ImGui::SliderFloat(("[OPTION] Radar Range"), &Settings.RadarESPRange, 7000.f, 50000.f);
				}

				ImGui::Dummy(ImVec2(0.0f, 0.0f));
				ImGui::SameLine(NULL, 9);
				utils::ToggleButton(("Enable Camera FOV Changer"), &Settings.FovChanger);
				ImGui::SameLine(NULL, 8);
				ImGui::Text(("Enable Camera FOV Changer")); 

				if (Settings.FovChanger) {
					ImGui::Dummy(ImVec2(6, 6));
					ImGui::Dummy(ImVec2(0.0f, 0.0f));
					ImGui::SameLine(NULL, 9);
					ImGui::PushItemWidth(200);
					ImGui::SliderFloat(("[OPTION] Camera FOV Distance"), &Settings.FOV, 20.f, 140.f);
				}

				ImGui::Dummy(ImVec2(0.0f, 0.0f));
				ImGui::SameLine(NULL, 9);
				utils::ToggleButton(("Enable AirStuck"), &Settings.AirStuck);
				ImGui::SameLine(NULL, 8);
				ImGui::Text(("Enable AirStuck"));

				ImGui::Dummy(ImVec2(0.0f, 0.0f));
				ImGui::SameLine(NULL, 9);
				utils::ToggleButton(("Shoot Throuth Walls"), &Settings.NoClip);
				ImGui::SameLine(NULL, 8);
				ImGui::Text(("Shoot Through Walls (WITH VEHICLE)"));

				ImGui::Dummy(ImVec2(0.0f, 0.0f));
				ImGui::SameLine(NULL, 9);
				utils::ToggleButton(("Boat Fly"), &Settings.TeleportToEnemies);
				ImGui::SameLine(NULL, 8);
				ImGui::Text(("Boat Fly"));

				if (Settings.TeleportToEnemies) {
					ImGui::Dummy(ImVec2(6, 6));
					ImGui::Dummy(ImVec2(0.0f, 0.0f));
					ImGui::SameLine(NULL, 9);
					ImGui::PushItemWidth(200);
					ImGui::SliderFloat(("[OPTION] Boat Fly Speed"), &Settings.BoatFlySpeed, 0.f, 1000.f);
				}

				
				ImGui::Dummy(ImVec2(0.0f, 0.0f));
				ImGui::SameLine(NULL, 9);
				utils::ToggleButton(("Instant Reload"), &Settings.InstantReload);
				ImGui::SameLine(NULL, 8);
				ImGui::Text(("Instant Reload"));
				
				
				ImGui::Dummy(ImVec2(0.0f, 0.0f));
				ImGui::SameLine(NULL, 9);
				utils::ToggleButton(("Sniper Bullet TP"), &Settings.SniperHits);
				ImGui::SameLine(NULL, 8);
				ImGui::Text(("Sniper Bullet TP"));

				if (Settings.SniperHits) {
					ImGui::Dummy(ImVec2(0.0f, 0.0f));
					ImGui::SameLine(NULL, 9);
					utils::ToggleButton(("[OPTION] Shoot through walls (Rage)"), &Settings.SniperThroughWalls);
					ImGui::SameLine(NULL, 8);
					ImGui::Text(("[OPTION] Shoot through walls (Rage)"));

					ImGui::Dummy(ImVec2(0.0f, 0.0f));
					ImGui::SameLine(NULL, 9);
					utils::ToggleButton(("[OPTION] Sniper No Aimbot"), &Settings.SniperNoAimbot);
					ImGui::SameLine(NULL, 8);
					ImGui::Text(("[OPTION] Sniper No Aimbot"));
				}
				else {
					Settings.SniperThroughWalls = false;
					Settings.SniperNoAimbot = false;
				}
				
				ImGui::Dummy(ImVec2(0.0f, 0.0f));
				ImGui::SameLine(NULL, 9);
				utils::ToggleButton(("Boat Super Boost"), &Settings.BoatBoost);
				ImGui::SameLine(NULL, 8);
				ImGui::Text(("Boat Super Boost"));

				if (Settings.BoatBoost) {
					ImGui::Dummy(ImVec2(6, 6));
					ImGui::Dummy(ImVec2(0.0f, 0.0f));
					ImGui::SameLine(NULL, 9);
					ImGui::PushItemWidth(200);
					ImGui::SliderFloat(("[OPTION] Boost Speed Multiplier"), &Settings.BoatBoostMultiplier, 1.f, 10.f);
				}
				
				ImGui::Dummy(ImVec2(0.0f, 0.0f));
				ImGui::SameLine(NULL, 9);
				utils::ToggleButton(("Helicopter Infinite Boost"), &Settings.HelicopterInfiniteBoost);
				ImGui::SameLine(NULL, 8);
				ImGui::Text(("Helicopter Infinite Boost"));

				ImGui::Dummy(ImVec2(0.0f, 0.0f));
				ImGui::SameLine(NULL, 9);
				utils::ToggleButton(("Helicopter Super Speed"), &Settings.HelicopterSpeed);
				ImGui::SameLine(NULL, 8);
				ImGui::Text(("Helicopter Super Speed"));

				if (Settings.HelicopterSpeed) {
					ImGui::Dummy(ImVec2(6, 6));
					ImGui::Dummy(ImVec2(0.0f, 0.0f));
					ImGui::SameLine(NULL, 9);
					ImGui::PushItemWidth(200);
					ImGui::SliderFloat(("[OPTION] Super Speed Multiplier"), &Settings.HelicopterSpeedMultiplier, 1.f, 10.f);
				}

				ImGui::Dummy(ImVec2(0.0f, 0.0f));
				ImGui::SameLine(NULL, 9);
				utils::ToggleButton(("Rapid Fire"), &Settings.RapidFire);
				ImGui::SameLine(NULL, 8);
				ImGui::Text(("Rapid Fire"));
				
				ImGui::Dummy(ImVec2(0.0f, 0.0f));
				ImGui::SameLine(NULL, 9);
				utils::ToggleButton(("No Spread"), &Settings.NoSpreadAimbot);
				ImGui::SameLine(NULL, 8);
				ImGui::Text(("No Spread"));
				
				ImGui::Dummy(ImVec2(0.0f, 0.0f));
				ImGui::SameLine(NULL, 9);
				utils::ToggleButton(("Invisible Player"), &Settings.InvisiblePlayer);
				ImGui::SameLine(NULL, 8);
				ImGui::Text(("Invisible Player"));
				
				
				ImGui::Dummy(ImVec2(0.0f, 0.0f));
				ImGui::SameLine(NULL, 9);
				utils::ToggleButton(("Freecam [BETA]"), &Settings.FreeCam);
				ImGui::SameLine(NULL, 8);
				ImGui::Text(("Freecam [BETA]"));
				
				/*
				ImGui::Dummy(ImVec2(0.0f, 0.0f));
				ImGui::SameLine(NULL, 9);
				utils::ToggleButton(("Boat Fly"), &Settings.BoatFly);
				ImGui::SameLine(NULL, 8);
				ImGui::Text(("Boat Fly"));*/

				if (tab == 1) {
					ImGui::Dummy(ImVec2(0.0f, 0.5f));
					ImGui::SameLine(NULL, 9);
					ImGui::Text(("-> Administrator Section:"));
					/*
					ImGui::Dummy(ImVec2(0.0f, 0.0f));
					ImGui::SameLine(NULL, 9);
					utils::ToggleButton(("Annas Nonsense"), &Settings.AnnasMisc);
					ImGui::SameLine(NULL, 8);
					ImGui::Text(("Annas Nonsense"));
					*/
					ImGui::Dummy(ImVec2(0.0f, 0.0f));
					ImGui::SameLine(NULL, 9);
					utils::ToggleButton(("Enable Objects Names Display (Caution: Might cuz FPS drop)"), &Settings.Dev);
					ImGui::SameLine(NULL, 8);
					ImGui::Text(("Enable Objects Names Display (Caution: Might cuz FPS drop)"));

					
					ImGui::Dummy(ImVec2(0.0f, 0.0f));
					ImGui::SameLine(NULL, 9);
					utils::ToggleButton(("[BETA] Player TP"), &Settings.PlrTP);
					ImGui::SameLine(NULL, 8);
					ImGui::Text(("[BETA] Player TP"));


				}
				ImGui::EndGroup();

				//////////////////////////////////////////
				ImGui::SameLine(ImGui::GetWindowWidth() - 111/*111: size of the button*/ - 10);
				ImGui::BeginGroup();
				ImGui::Dummy(ImVec2(0.0f, 5.0f));
				ImGui::Spacing();


				/*
				ImGui::Dummy(ImVec2(0.0f, 0.0f));
				ImGui::SameLine(NULL, 1);
				ImGui::Text(("TP Key"));
				ImGui::Hotkey(("TP Key"), &Settings.BoatTPkey, ImVec2{ 111, 36 }, 0);
				*/
				ImGui::EndGroup();
			}
		}
		ImGui::End();
	}
}
