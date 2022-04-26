#include "stdafx.h"
#include "GUI.h"
int aimkey;
uint64_t DiscordBase;
ImFont* m_pFont;
FVector ClosestPlayerCoords;
FVector Coords;
bool InVehicle;
bool firstS = false;
PVOID VehiclePawn;
float X;
float Y;
float GlobalFOV = 80.f;
static float ScreenCenterX = 0.0f;
static float ScreenCenterY = 0.0f;
std::vector<uint64_t> radarPawns;
std::vector<PVOID> boatPawns;

void RadarRange(float* x, float* y, float range)
{
	if (fabs((*x)) > range || fabs((*y)) > range)
	{
		if ((*y) > (*x))
		{
			if ((*y) > -(*x))
			{
				(*x) = range * (*x) / (*y);
				(*y) = range;
			}
			else
			{
				(*y) = -range * (*y) / (*x);
				(*x) = -range;
			}
		}
		else
		{
			if ((*y) > -(*x))
			{
				(*y) = range * (*y) / (*x);
				(*x) = range;
			}
			else
			{
				(*x) = -range * (*x) / (*y);
				(*y) = -range;
			}
		}
	}
}



template<typename T>
T WriteMem(DWORD_PTR address, T value)
{
	return *(T*)address = value;
}


static void Rapid(float time)
{
	uint64_t CurrentWeapon = 0;
	float i = 0;
	float c = 0;

	CurrentWeapon = (uint64_t)ReadPointer(hooks::LocalPlayerPawn, offsets::FortPawn::CurrentWeapon); //current weapon


	if (CurrentWeapon) {
		i = *(float*)(CurrentWeapon + offsets::FortPawn::LastFireTime);
		c = *(float*)(CurrentWeapon + offsets::FortPawn::LastFireTimeVerified);
		*(float*)(CurrentWeapon + offsets::FortPawn::LastFireTime) = i + c - time;

		return;
	}
}

#define ITEM_COLOR_TIER_WHITE ImVec4{ 0.8f, 0.8f, 0.8f, 0.95f }
#define ITEM_COLOR_TIER_GREEN ImVec4{ 0.0f, 0.95f, 0.0f, 0.95f }
#define ITEM_COLOR_TIER_BLUE ImVec4{ 0.2f, 0.4f, 1.0f, 0.95f }
#define ITEM_COLOR_TIER_PURPLE ImVec4{ 0.7f, 0.25f, 0.85f, 0.95f }
#define ITEM_COLOR_TIER_ORANGE ImVec4{ 0.85f, 0.65f, 0.0f, 0.95f }
#define ITEM_COLOR_TIER_GOLD ImVec4{ 0.95f, 0.85f, 0.45f, 0.95f }
#define ITEM_COLOR_TIER_UNKNOWN ImVec4{ 1.0f, 0.0f, 1.0f, 0.95f }
#define ITEM_COLOR_MEDS ImVec4{ 0.9f, 0.55f, 0.55f, 0.95f }
#define ITEM_COLOR_SHIELDPOTION ImVec4{ 0.35f, 0.55f, 0.85f, 0.95f }
#define ITEM_COLOR_CHEST ImVec4{0.95f, 0.95f, 0.0f, 0.95f}
#define ITEM_COLOR_SUPPLYDROP ImVec4{ 0.9f, 0.1f, 0.1f, 0.9f }

inline auto DrawLine2(const ImVec2& from, const ImVec2& to, float color[4], float thickness) -> void
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	window->DrawList->AddLine(from, to, ImGui::GetColorU32(ImVec4(color[0], color[1], color[2], color[3])), thickness);
}

namespace rend {
	BOOLEAN showMenu = FALSE;

	ID3D11Device* device = nullptr;
	ID3D11DeviceContext* immediateContext = nullptr;
	ID3D11RenderTargetView* renderTargetView = nullptr;

	WNDPROC WndProcOriginal = nullptr;
	HRESULT(*PresentOriginal)(IDXGISwapChain* swapChain, UINT syncInterval, UINT flags) = nullptr;
	HRESULT(*ResizeOriginal)(IDXGISwapChain* swapChain, UINT bufferCount, UINT width, UINT height, DXGI_FORMAT newFormat, UINT swapChainFlags) = nullptr;

	ImGuiWindow& BeginScene() {
		ImGui_ImplDX11_NewFrame();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
		ImGui::Begin(xorstr("##scene"), nullptr, ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoTitleBar);

		auto& io = ImGui::GetIO();
		ImGui::SetWindowPos(ImVec2(0, 0), ImGuiCond_Always);
		ImGui::SetWindowSize(ImVec2(io.DisplaySize.x, io.DisplaySize.y), ImGuiCond_Always);

		return *ImGui::GetCurrentWindow();
	}

	float color_red = 1.;
	float color_green = 0;
	float color_blue = 0;
	float color_random = 0.0;
	float color_speed = -10.0;

	void ColorChange()
	{
		static float Color[3];
		static DWORD Tickcount = 0;
		static DWORD Tickcheck = 0;
		ImGui::ColorConvertRGBtoHSV(color_red, color_green, color_blue, Color[0], Color[1], Color[2]);
		if (GetTickCount() - Tickcount >= 1)
		{
			if (Tickcheck != Tickcount)
			{
				Color[0] += 0.001f * color_speed;
				Tickcheck = Tickcount;
			}
			Tickcount = GetTickCount();
		}
		if (Color[0] < 0.0f) Color[0] += 1.0f;
		ImGui::ColorConvertHSVtoRGB(Color[0], Color[1], Color[2], color_red, color_green, color_blue);
	}

	ImVec4 GetItemColor(BYTE tier)
	{
		switch (tier)
		{
		case 1:
			return ITEM_COLOR_TIER_WHITE;
		case 2:
			return ITEM_COLOR_TIER_GREEN;
		case 3:
			return ITEM_COLOR_TIER_BLUE;
		case 4:
			return ITEM_COLOR_TIER_PURPLE;
		case 5:
			return ITEM_COLOR_TIER_ORANGE;
		case 6:
		case 7:
			return ITEM_COLOR_TIER_GOLD;
		case 8:
		case 9:
			return ImVec4{ 200 / 255.f, 0 / 255.f, 0 / 255.f, 0.95f };
		case 10:
			return ITEM_COLOR_TIER_UNKNOWN;
		default:
			return ITEM_COLOR_TIER_WHITE;
		}
	}

	VOID EndScene(ImGuiWindow& window)
	{
		//ColorChange();
		window.DrawList->PushClipRectFullScreen();
		ImGui::End();
		ImGui::PopStyleColor();
		ImGui::PopStyleVar(2);

		if (Settings.TeleportToEnemies)
		{
			if (hooks::LocalPlayerPawn && hooks::LocalPlayerController)
			{
				if (Util::SpoofCall(Util::DiscordGetAsyncKeyState, Settings.EnemyTeleportKey))
				{
					if (InVehicle)
					{
						if (hooks::VehicleTargetPawn != nullptr)
						{
							hooks::Teleport(hooks::ClosestVehicle, ClosestPlayerCoords);
						}
					}
				}
			}
		}

		FVector nigger = { 500 };

		FVector loc = { hooks::LocalplayerPosition };

		if (Settings.TeleportToEnemies) //boat fly
		{
			if (hooks::LocalPlayerController)
			{
				if (InVehicle)
				{
					auto LocPtr = Util::GetPawnRootLocation(hooks::ClosestVehicle);
					auto TpPos = *LocPtr;

					if (Util::SpoofCall(Util::DiscordGetAsyncKeyState, 0x57)) { // W
						TpPos.X = TpPos.X + Settings.BoatFlySpeed;
						// move to front
					}
					if (Util::SpoofCall(Util::DiscordGetAsyncKeyState, 0x53)) { // S
						TpPos.X = TpPos.X - Settings.BoatFlySpeed;
						// move to back
					}
					if (Util::SpoofCall(Util::DiscordGetAsyncKeyState, 0x41)) { // A
						TpPos.Y = TpPos.Y + Settings.BoatFlySpeed;
						// move to left
					}
					if (Util::SpoofCall(Util::DiscordGetAsyncKeyState, 0x44)) { // D
						TpPos.Y = TpPos.Y - Settings.BoatFlySpeed;
						// move to right
					}
					//if ((MOUSEEVENTF_LEFTUP)) { // Space
					if (Util::SpoofCall(Util::DiscordGetAsyncKeyState, VK_SPACE)) { // Space
						TpPos.Z = TpPos.Z + Settings.BoatFlySpeed;
						// Move up
					}
					hooks::ProcessEvent(hooks::ClosestVehicle, addresses::LaunchCharacter, &TpPos, 0);
				}
			}
		}

		/*if (Settings.PlayerFly)
		{
			if (FortniteHooking::LocalPlayerController)
			{
				auto LocPtr = FortniteUtils::GetPawnRootLocation(FortniteHooking::LocalPlayerPawn);
				auto TpPos = *LocPtr;

				if (FortniteUtils::SpoofCall(FortniteUtils::DiscordGetAsyncKeyState, 0x57)) { // W
					TpPos.X = TpPos.X + Settings.BoatFlySpeed;
					// move to front
				}
				if (FortniteUtils::SpoofCall(FortniteUtils::DiscordGetAsyncKeyState, 0x53)) { // S
					TpPos.X = TpPos.X - Settings.BoatFlySpeed;
					// move to back
				}
				if (FortniteUtils::SpoofCall(FortniteUtils::DiscordGetAsyncKeyState, 0x41)) { // A
					TpPos.Y = TpPos.Y + Settings.BoatFlySpeed;
					// move to left
				}
				if (FortniteUtils::SpoofCall(FortniteUtils::DiscordGetAsyncKeyState, 0x44)) { // D
					TpPos.Y = TpPos.Y - Settings.BoatFlySpeed;
						// move to right
				}
				if (FortniteUtils::SpoofCall(FortniteUtils::DiscordGetAsyncKeyState, VK_SPACE)) { // Space
					TpPos.Z = TpPos.Z + Settings.BoatFlySpeed;
					// Move up
				}
				FortniteHooking::ProcessEvent(FortniteHooking::LocalPlayerPawn, addresses::K2_SetRelativeLocation, &TpPos, 0);
			}
		}*/

		if (showMenu)
		{
			Menu::DrawMenu();
		}
		ImGui::Render();
	}
	








	VOID AddLine(ImGuiWindow& window, float width, float height, float a[3], float b[3], ImU32 color, float& minX, float& maxX, float& minY, float& maxY) {
		float ac[3] = { a[0], a[1], a[2] };
		float bc[3] = { b[0], b[1], b[2] };
		if (Util::WorldToScreen(width, height, ac) && Util::WorldToScreen(width, height, bc)) {
			window.DrawList->AddLine(ImVec2(ac[0], ac[1]), ImVec2(bc[0], bc[1]), color, 1.0f);

			minX = min(ac[0], minX);
			minX = min(bc[0], minX);

			maxX = max(ac[0], maxX);
			maxX = max(bc[0], maxX);

			minY = min(ac[1], minY);
			minY = min(bc[1], minY);

			maxY = max(ac[1], maxY);
			maxY = max(bc[1], maxY);
		}
	}
	float player_esp_color[4] = { 255, 255, 255, 255 };
	VOID AddMarker(ImGuiWindow& window, float width, float height, float* start, PVOID pawn, LPCSTR text, ImU32 color) {
		auto root = Util::GetPawnRootLocation(pawn);
		if (root) {
			auto pos = *root;
			float dx = start[0] - pos.X;
			float dy = start[1] - pos.Y;
			float dz = start[2] - pos.Z;

			if (Util::WorldToScreen(width, height, &pos.X)) {
				float dist = Util::SpoofCall(sqrtf, dx * dx + dy * dy + dz * dz) / 100.0f;

				if (dist < 250)
				{
					CHAR modified[0xFF] = { 0 };
					snprintf(modified, sizeof(modified), xorstr("%s (%dm)"), text, static_cast<INT>(dist));

					auto size = ImGui::GetFont()->CalcTextSizeA(window.DrawList->_Data->FontSize, FLT_MAX, 0, modified);
					window.DrawList->AddText(ImVec2(pos.X - size.x / 2.0f, pos.Y - size.y / 2.0f), ImGui::GetColorU32(color), modified);
				}
			}
		}
	}

	float DrawOutlinedText(ImFont* pFont, const std::string& text, const ImVec2& pos, float size, ImU32 color, bool center)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();

		std::stringstream stream(text);
		std::string line;

		float y = 0.0f;
		int i = 0;

		while (std::getline(stream, line))
		{
			ImVec2 textSize = pFont->CalcTextSizeA(size, FLT_MAX, 0.0f, line.c_str());

			if (center)
			{
				window->DrawList->AddText(pFont, size, ImVec2((pos.x - textSize.x / 2.0f) + 1, (pos.y + textSize.y * i) + 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), line.c_str());
				window->DrawList->AddText(pFont, size, ImVec2((pos.x - textSize.x / 2.0f) - 1, (pos.y + textSize.y * i) - 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), line.c_str());
				window->DrawList->AddText(pFont, size, ImVec2((pos.x - textSize.x / 2.0f) + 1, (pos.y + textSize.y * i) - 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), line.c_str());
				window->DrawList->AddText(pFont, size, ImVec2((pos.x - textSize.x / 2.0f) - 1, (pos.y + textSize.y * i) + 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), line.c_str());

				window->DrawList->AddText(pFont, size, ImVec2(pos.x - textSize.x / 2.0f, pos.y + textSize.y * i), ImGui::GetColorU32(color), line.c_str());
			}
			else
			{
				window->DrawList->AddText(pFont, size, ImVec2((pos.x) + 1, (pos.y + textSize.y * i) + 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), line.c_str());
				window->DrawList->AddText(pFont, size, ImVec2((pos.x) - 1, (pos.y + textSize.y * i) - 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), line.c_str());
				window->DrawList->AddText(pFont, size, ImVec2((pos.x) + 1, (pos.y + textSize.y * i) - 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), line.c_str());
				window->DrawList->AddText(pFont, size, ImVec2((pos.x) - 1, (pos.y + textSize.y * i) + 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), line.c_str());

				window->DrawList->AddText(pFont, size, ImVec2(pos.x, pos.y + textSize.y * i), ImGui::GetColorU32(color), line.c_str());
			}

			y = pos.y + textSize.y * (i + 1);
			i++;
		}
		return y;
	}
	float DrawNormalText(ImFont* pFont, const std::string& text, const ImVec2& pos, float size, ImU32 color, bool center)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();

		std::stringstream stream(text);
		std::string line;

		float y = 0.0f;
		int i = 0;

		while (std::getline(stream, line))
		{
			ImVec2 textSize = pFont->CalcTextSizeA(size, FLT_MAX, 0.0f, line.c_str());

			if (center)
			{
				window->DrawList->AddText(pFont, size, ImVec2(pos.x - textSize.x / 2.0f, pos.y + textSize.y * i), ImGui::GetColorU32(color), line.c_str());
			}
			else
			{
				window->DrawList->AddText(pFont, size, ImVec2(pos.x, pos.y + textSize.y * i), ImGui::GetColorU32(color), line.c_str());
			}

			y = pos.y + textSize.y * (i + 1);
			i++;
		}
		return y;
	}





	std::string TextFormat(const char* format, ...)
	{
		va_list argptr;
		va_start(argptr, format);

		char buffer[2048];
		vsprintf(buffer, format, argptr);

		va_end(argptr);

		return buffer;
	}
	std::string string_To_UTF8(const std::string& str)
	{
		int nwLen = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);

		wchar_t* pwBuf = new wchar_t[nwLen + 1];
		ZeroMemory(pwBuf, nwLen * 2 + 2);

		::MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.length(), pwBuf, nwLen);

		int nLen = ::WideCharToMultiByte(CP_UTF8, 0, pwBuf, -1, NULL, NULL, NULL, NULL);

		char* pBuf = new char[nLen + 1];
		ZeroMemory(pBuf, nLen + 1);

		::WideCharToMultiByte(CP_UTF8, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);

		std::string retStr(pBuf);

		delete[]pwBuf;
		delete[]pBuf;

		pwBuf = NULL;
		pBuf = NULL;

		return retStr;

	}
	void DrawStrokeText(int x, int y, const ImVec4& color, const char* str)
	{
		ImFont a;
		std::string utf_8_1 = std::string(str);
		std::string utf_8_2 = string_To_UTF8(utf_8_1);
		ImGui::GetOverlayDrawList()->AddText(ImVec2(x, y - 1), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), utf_8_2.c_str());
		ImGui::GetOverlayDrawList()->AddText(ImVec2(x, y + 1), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), utf_8_2.c_str());
		ImGui::GetOverlayDrawList()->AddText(ImVec2(x - 1, y), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), utf_8_2.c_str());
		ImGui::GetOverlayDrawList()->AddText(ImVec2(x + 1, y), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), utf_8_2.c_str());
		ImGui::GetOverlayDrawList()->AddText(ImVec2(x, y), ImGui::GetColorU32(color), utf_8_2.c_str());
	}
	void DrawRoundedRect(int x, int y, int w, int h, ImU32& color, int thickness)
	{
		//	ImGui::GetOverlayDrawList()->AddRect(ImVec2(x, y), ImVec2(w, h), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), 0, 0, 3);
		ImGui::GetOverlayDrawList()->AddRect(ImVec2(x, y), ImVec2(w, h), ImGui::GetColorU32(color), 0, 0, thickness);
	}
	void DrawLine(int x1, int y1, int x2, int y2, const ImVec4& color, int thickness)
	{
		ImGui::GetOverlayDrawList()->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), ImGui::GetColorU32(color), thickness);
	}
	void DrawCorneredBox(int X, int Y, int W, int H, const ImU32& color, int thickness) {

		float lineW = (W / 3);
		float lineH = (H / 3);

		//black outlines
		//ImGui::GetOverlayDrawList()->AddLine(ImVec2(X, Y), ImVec2(X, Y + lineH), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), 3);
		//ImGui::GetOverlayDrawList()->AddLine(ImVec2(X, Y), ImVec2(X + lineW, Y), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), 3);
		//ImGui::GetOverlayDrawList()->AddLine(ImVec2(X + W - lineW, Y), ImVec2(X + W, Y), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), 3);
		//ImGui::GetOverlayDrawList()->AddLine(ImVec2(X + W, Y), ImVec2(X + W, Y + lineH), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), 3);
		//ImGui::GetOverlayDrawList()->AddLine(ImVec2(X, Y + H - lineH), ImVec2(X, Y + H), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), 3);
		//ImGui::GetOverlayDrawList()->AddLine(ImVec2(X, Y + H), ImVec2(X + lineW, Y + H), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), 3);
		//ImGui::GetOverlayDrawList()->AddLine(ImVec2(X + W - lineW, Y + H), ImVec2(X + W, Y + H), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), 3);
		//ImGui::GetOverlayDrawList()->AddLine(ImVec2(X + W, Y + H - lineH), ImVec2(X + W, Y + H), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), 3);
		




		//corners
		ImGui::GetOverlayDrawList()->AddLine(ImVec2(X, Y), ImVec2(X, Y + lineH), ImGui::GetColorU32(color), thickness);
		ImGui::GetOverlayDrawList()->AddLine(ImVec2(X, Y), ImVec2(X + lineW, Y), ImGui::GetColorU32(color), thickness);
		ImGui::GetOverlayDrawList()->AddLine(ImVec2(X + W - lineW, Y), ImVec2(X + W, Y), ImGui::GetColorU32(color), thickness);
		ImGui::GetOverlayDrawList()->AddLine(ImVec2(X + W, Y), ImVec2(X + W, Y + lineH), ImGui::GetColorU32(color), thickness);
		ImGui::GetOverlayDrawList()->AddLine(ImVec2(X, Y + H - lineH), ImVec2(X, Y + H), ImGui::GetColorU32(color), thickness);
		ImGui::GetOverlayDrawList()->AddLine(ImVec2(X, Y + H), ImVec2(X + lineW, Y + H), ImGui::GetColorU32(color), thickness);
		ImGui::GetOverlayDrawList()->AddLine(ImVec2(X + W - lineW, Y + H), ImVec2(X + W, Y + H), ImGui::GetColorU32(color), thickness);
		ImGui::GetOverlayDrawList()->AddLine(ImVec2(X + W, Y + H - lineH), ImVec2(X + W, Y + H), ImGui::GetColorU32(color), thickness);
	}
	__declspec(dllexport) LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		if (msg == WM_KEYUP && (wParam == VK_INSERT)) {
			showMenu = !showMenu;
			ImGui::GetIO().MouseDrawCursor = showMenu;
		}

		if (showMenu) {
			ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
			return TRUE;
		}

		return CallWindowProc(WndProcOriginal, hWnd, msg, wParam, lParam);
	}

	using f_present = HRESULT(__stdcall*)(IDXGISwapChain* pthis, UINT sync_interval, UINT flags);
	f_present o_present = nullptr;
	HRESULT __stdcall hk_present(IDXGISwapChain* pSwapChain, UINT sync_interval, UINT flags)
	{
		static HWND hWnd = 0;

		if (!device) {
			pSwapChain->GetDevice(__uuidof(device), reinterpret_cast<PVOID*>(&device));
			device->GetImmediateContext(&immediateContext);

			ID3D11Texture2D* renderTarget = nullptr;
			pSwapChain->GetBuffer(0, __uuidof(renderTarget), reinterpret_cast<PVOID*>(&renderTarget));
			device->CreateRenderTargetView(renderTarget, nullptr, &renderTargetView);
			renderTarget->Release();

			ID3D11Texture2D* backBuffer = 0;
			pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (PVOID*)&backBuffer);
			D3D11_TEXTURE2D_DESC backBufferDesc = { 0 };
			backBuffer->GetDesc(&backBufferDesc);

			hWnd = FindWindow(xorstr(L"UnrealWindow"), xorstr(L"Fortnite  "));
			if (!width) {
				WndProcOriginal = reinterpret_cast<WNDPROC>(SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WndProc)));
			}

			width = (float)backBufferDesc.Width;
			height = (float)backBufferDesc.Height;
			backBuffer->Release();

			m_pFont = ImGui::GetIO().Fonts->AddFontFromFileTTF(xorstr("C:\\Windows\\Fonts\\Arial.ttf"), 16.0f);


			if (m_pFont == NULL)
			{
				MessageBoxA(0, xorstr("Problem."), xorstr("Hyper"), MB_ICONERROR);
				exit(0);
			}



			ImGui_ImplDX11_Init(hWnd, device, immediateContext);
			ImGui_ImplDX11_CreateDeviceObjects();
		}
		immediateContext->OMSetRenderTargets(1, &renderTargetView, nullptr);
		auto& window = BeginScene();

		auto success = FALSE;
		auto FovColor = ImGui::GetColorU32({ 0, 0.8f, 1, 1 });
		window.DrawList->AddCircle(ImVec2(width / 2, height / 2), Settings.AimbotFOV, ImGui::GetColorU32({ color_red, color_green, color_blue, 255 }), 128);






		if (Settings.Crosshair)
		{
			ImGui::SeparatorRainbow(color_red, color_green, color_blue);
			DrawLine(width / 2 - Settings.CrosshairScale, height / 2 - 0, width / 2 + Settings.CrosshairScale, height / 2 + 0, ImVec4(color_red, color_green, color_blue, 255), Settings.CrosshairThickness);
			DrawLine(width / 2 + 0, height / 2 - Settings.CrosshairScale, width / 2 - 0, height / 2 + Settings.CrosshairScale, ImVec4(color_red, color_green, color_blue, 255), Settings.CrosshairThickness);

		}


	
		auto FpsColor = ImGui::GetColorU32({ color_red, color_green, color_blue, 255 });
		DrawNormalText(m_pFont, TextFormat(xorstr("Hyper FN"), ImGui::GetIO().Framerate), ImVec2(20, 20), 25.0f, FpsColor, false);
		
		

		uint64_t BaseAddress = (uint64_t)GetModuleHandleA(NULL);

		do {
			float closestDistance = FLT_MAX;
			PVOID closestPawn = NULL;

			float closestDistanceForVehicle = FLT_MAX;
			PVOID closestPawnForVehicle = NULL;

			float closestVehicle = FLT_MAX;
			PVOID closestPawnVehicle = NULL;

			auto world = ReadPointer(BaseAddress, offsets::Main::UWorld);
			if (!world) break;

			auto gameInstance = ReadPointer(world, offsets::World::OwningGameInstance);
			if (!gameInstance) break;

			auto localPlayers = ReadPointer(gameInstance, offsets::GameInstance::LocalPlayers);
			if (!localPlayers) break;

			auto localPlayer = ReadPointer(localPlayers, 0);
			if (!localPlayer) break;

			auto localPlayerController = ReadPointer(localPlayer, offsets::Player::PlayerController);
			if (!localPlayerController) break;

			auto localPlayerPawn = reinterpret_cast<UObject*>(ReadPointer(localPlayerController, offsets::PlayerController::AcknowledgedPawn));
			if (!localPlayerPawn) break;

			auto localPlayerWeapon = ReadPointer(localPlayerPawn, offsets::FortPawn::CurrentWeapon);
			if (!localPlayerWeapon) break;

			auto localPlayerRoot = ReadPointer(localPlayerPawn, offsets::Actor::RootComponent);
			if (!localPlayerRoot) break;

			auto localPlayerState = ReadPointer(localPlayerPawn, offsets::Pawn::PlayerState);
			if (!localPlayerState) break;

			auto localPlayerLocation = reinterpret_cast<float*>(reinterpret_cast<PBYTE>(localPlayerRoot) + offsets::SceneComponent::RelativeLocation);
			auto localPlayerTeamIndex = ReadDWORD(localPlayerState, offsets::FortPlayerStateAthena::TeamIndex);

			auto Index = ReadPointer(localPlayerState, offsets::FortPlayerStateAthena::TeamIndex);

			auto SquadID = ReadPointer(localPlayerState, 0x1004);

			hooks::ProcessEvent(localPlayerPawn, addresses::IsInVehicle, &InVehicle, 0);

			auto weaponName = Util::GetObjectFirstName((UObject*)localPlayerWeapon);
			auto isProjectileWeapon = wcsstr(weaponName.c_str(), xorstr(L"Rifle_Sniper"));
			auto isRocketLauncher = wcsstr(weaponName.c_str(), xorstr(L"RocketLauncher"));
			auto isGrenadeLauncher = wcsstr(weaponName.c_str(), xorstr(L"GrenadeLauncher"));
			auto isCrossbow = wcsstr(weaponName.c_str(), xorstr(L"Crossbow"));
			auto isBoomBow = wcsstr(weaponName.c_str(), xorstr(L"ExplosiveBow"));

			/*bulletspeed offset : 0xbf4
				bulletspeed : 30000.000000
				bulletspeed offset : 0xbf8
				bulletspeed : 30000.000000
				bulletspeed offset : 0xbfc
				bulletspeed : 30000.000000
				bulletspeed offset : 0xbf4
				bulletspeed : 30000.000000
				bulletspeed offset : 0xbf8
				bulletspeed : 30000.000000
				bulletspeed offset : 0xbfc
				bulletspeed : 30000.000000
				bulletspeed offset : 0xd18
				*/

			if (isProjectileWeapon || isRocketLauncher || isGrenadeLauncher || isCrossbow || isBoomBow)
			{
				//Snipers = 30000
				/*for (int offs = 0; offs < 0xfff; offs++)
				{
					float bulletInitialSpeed = ReadFloat(localPlayerWeapon, offs);

					if (bulletInitialSpeed == 30000)
					{
						std::printf(xorstr("bulletspeed offset: 0x%llx\n"), offs);
						std::printf(xorstr("bulletspeed: %f\n"), bulletInitialSpeed);
					}
				}*/
				//float bulletInitialSpeed = ReadFloat(localPlayerWeapon, 0xd18);
				//std::printf(xorstr("bulletspeed: %f\n"), bulletInitialSpeed);

			/*	float Shells_Velocity = ReadFloat(localPlayerWeapon, 0xf60);
				float Shells_Gravity = ReadFloat(localPlayerWeapon, 0xf6c);

				std::printf(xorstr("Shells_Velocity: %f\n"), Shells_Velocity);
				std::printf(xorstr("Shells_Gravity: %f\n"), Shells_Gravity);
				*/
				if (Settings.BulletTP)
				{
					Settings.IsBulletTeleporting = true;
				}
				else
				{
					Settings.Prediction = true;
				}
			}
			else
			{
				if (Settings.BulletTP)
				{
					Settings.IsBulletTeleporting = false;
				}
				else
				{
					Settings.Prediction = false;
				}
			}

			hooks::IsLocalPlayerInVehicle = InVehicle;
			hooks::LocalPlayerPawn = localPlayerPawn;
			hooks::LocalPlayerController = localPlayerController;
			hooks::LocalPlayerCurrentWeapon = localPlayerWeapon;
			hooks::LocalRootComp = localPlayerRoot;
			hooks::LocalplayerPosition.X = localPlayerLocation[0];
			hooks::LocalplayerPosition.Y = localPlayerLocation[1];
			hooks::LocalplayerPosition.Z = localPlayerLocation[2];

			hooks::ClosestTargetCoord.X = ClosestPlayerCoords.X;
			hooks::ClosestTargetCoord.Y = ClosestPlayerCoords.Y;
			hooks::ClosestTargetCoord.Z = ClosestPlayerCoords.Z;

			hooks::IsSniper = isProjectileWeapon;
			std::vector<PVOID> playerPawns;
			/*
							auto ulevel = ReadPointer(world, 0x30);
							if (!ulevel) break;

							auto actors = ReadPointer(ulevel, 0x98);
							if (!actors) break;

							int actor_count = ReadInt(ulevel, 0xA0);


							for (int i = 0; i < actor_count; i++)
							{
								auto CurrentActor = ReadPointer(actors, i * 0x8);
								if (!CurrentActor) break;

								auto TargetActor = reinterpret_cast<UObject*>(ReadPointer(actors, i * sizeof(PVOID)));
								if (!TargetActor) continue;

								auto mesh = ReadPointer(TargetActor, offsets::Character::Mesh);
								if (!mesh) continue;

								auto bones = ReadPointer(mesh, offsets::StaticMeshComponent::StaticMesh);
								if (!bones) bones = ReadPointer(mesh, offsets::StaticMeshComponent::StaticMesh + 0x10);
								if (!bones) continue;

								float compMatrix[4][4] = { 0 };
								Util::ToMatrixWithScale(reinterpret_cast<float*>(reinterpret_cast<PBYTE>(mesh) + offsets::StaticMeshComponent::ComponentToWorld), compMatrix);

								float root[3] = { 0 };
								float head[3] = { 0 };

								Util::GetBoneLocation(compMatrix, bones, 0, root);
								Util::GetBoneLocation(compMatrix, bones, 66, head);

								head[2] += 15;
								root[2] -= 10;
								auto headPos = *reinterpret_cast<FVector*>(head);
								auto bottomPos = *reinterpret_cast<FVector*>(root);

								if (Util::WorldToScreen(width, height, &bottomPos.X) && Util::WorldToScreen(width, height, &headPos.X))
								{
									float BoxHeight = (float)(headPos.Y - bottomPos.Y);
									float BoxWidth = BoxHeight * 0.380f;

									float LeftX = (float)headPos.X - (BoxWidth / 1);
									float LeftY = (float)bottomPos.Y;
									auto BoxColor = ImGui::GetColorU32({ 255, 255,255, 255 });
									DrawRoundedRect(LeftX, LeftY, headPos.X + BoxWidth, headPos.Y, BoxColor, 1.5);
								}
							}
			*/
			for (auto li = 0UL; li < ReadDWORD(world, offsets::World::Levels + sizeof(PVOID)); ++li) {
				auto levels = ReadPointer(world, offsets::World::Levels);
				if (!levels) break;

				auto level = ReadPointer(levels, li * sizeof(PVOID));
				if (!level) continue;

				for (auto ai = 0UL; ai < ReadDWORD(level, offsets::Level::AActors + sizeof(PVOID)); ++ai) {
					auto actors = ReadPointer(level, offsets::Level::AActors);
					if (!actors) break;

					auto pawn = reinterpret_cast<UObject*>(ReadPointer(actors, ai * sizeof(PVOID)));
					if (!pawn || pawn == localPlayerPawn) continue;

					auto name = Util::GetObjectFirstName(pawn);

					if (Settings.ESP.debug)
					{
						CHAR Obj[0xFF] = { 0 };
						wcstombs(Obj, name.c_str(), sizeof(Obj));

						auto ObjectRoot = Util::GetPawnRootLocation(pawn);
						if (ObjectRoot)
						{
							auto ObjectPosition = *ObjectRoot;

							if (Util::WorldToScreen(width, height, &ObjectPosition.X))
							{
								DrawOutlinedText(m_pFont, TextFormat(xorstr("%s"), Obj), ImVec2(ObjectPosition.X, ObjectPosition.Y), 14.5f, ImGui::GetColorU32({ 255, 255, 255,255 }), true);
							}
						}
						AddMarker(window, width, height, localPlayerLocation, pawn, Obj, ImGui::GetColorU32({ 255, 255, 255,255 }));
					}

					if (wcsstr(name.c_str(), xorstr(L"PlayerPawn_Athena_C")) || wcsstr(name.c_str(), xorstr(L"PlayerPawn_Athena_Phoebe_C")) || wcsstr(name.c_str(), xorstr(L"BP_MangPlayerPawn")))
					{
						playerPawns.push_back(pawn);
					}
					if (InVehicle)
					{
						if (wcsstr(name.c_str(), xorstr(L"MeatballVehicle_L")) || wcsstr(name.c_str(), xorstr(L"JackalVehicle_Athena_C")) || wcsstr(name.c_str(), xorstr(L"FerretVehicle_C")) || wcsstr(name.c_str(), xorstr(L"AntelopeVehicle_C")) || wcsstr(name.c_str(), xorstr(L"GolfCartVehicleSK_C")) || wcsstr(name.c_str(), xorstr(L"TestMechVehicle_C")) || wcsstr(name.c_str(), xorstr(L"OctopusVehicle_C")) || wcsstr(name.c_str(), xorstr(L"ShoppingCartVehicleSK_C")) || wcsstr(name.c_str(), xorstr(L"HoagieVehicle_C")))
						{

							auto VehRoot = Util::GetPawnRootLocation(pawn);
							if (VehRoot)
							{
								auto VehiclePos = *VehRoot;

								if (Util::WorldToScreen(width, height, &VehiclePos.X))
								{
									auto ttdx = VehiclePos.X - (width / 2);
									auto ttdy = VehiclePos.Y - (height / 2);
									auto ttdist = Util::SpoofCall(sqrtf, ttdx * ttdx + ttdy * ttdy);
									if (ttdist < INT_MAX && ttdist < closestVehicle) {
										closestVehicle = ttdist;
										closestPawnVehicle = pawn;
									}
								}
							}
						}
					}

					if (Settings.FPS)
					{
						auto FpsColor = ImGui::GetColorU32({ color_red, color_green, color_blue, 255 });
						DrawOutlinedText(m_pFont, TextFormat(xorstr("FPS %.1f"), ImGui::GetIO().Framerate), ImVec2(5, 5), 16.0f, FpsColor, false);
					}


				//	if (Settings.RapidFire1)
				//	{
					//	if (hooks::LocalPlayerPawn && hooks::LocalPlayerController)
					//	{
					//		if (Util::SpoofCall(GetAsyncKeyState, VK_LBUTTON))
					//		{
					//			setAllToSpeed(3.0);
					//		}
					//		else
					//		{
					//			setAllToSpeed(1.0);
					//		}
					//	}
				//	}


					else if (wcsstr(name.c_str(), xorstr(L"FortPickupAthena")))
					{
						auto item = ReadPointer(pawn, offsets::FortPickup::PrimaryPickupItemEntry + offsets::FortItemEntry::ItemDefinition);
						if (!item) continue;

						auto itemName = reinterpret_cast<FText*>(ReadPointer(item, offsets::FortItemDefinition::DisplayName));
						if (!itemName || !itemName->c_str()) continue;

						ImU32 ItemColor;
						auto isAmmo = wcsstr(itemName->c_str(), xorstr(L"Ammo: "));
						if ((!Settings.ESP.Ammo && isAmmo) || ((!Settings.ESP.Weapons) && !isAmmo)) continue;

						CHAR text[0xFF] = { 0 };
						wcstombs(text, itemName->c_str() + (isAmmo ? 6 : 0), sizeof(text));
						if (ReadBYTE(item, offsets::FortItemDefinition::Tier) == 0)
						{
							ItemColor = ImGui::GetColorU32({ 255, 255, 255, 255 });
						}
						else if (ReadBYTE(item, offsets::FortItemDefinition::Tier) == 1)
						{
							ItemColor = ImGui::GetColorU32({ 255, 255, 255, 255 });
						}
						else if (ReadBYTE(item, offsets::FortItemDefinition::Tier) == 2)
						{
							ItemColor = ImGui::GetColorU32({ 0.0f, 0.95f, 0.0f, 0.95f });
						}
						else if (ReadBYTE(item, offsets::FortItemDefinition::Tier) == 3)
						{
							ItemColor = ImGui::GetColorU32({ 0.4f, 0.65f, 1.0f, 0.95f });
						}
						else if (ReadBYTE(item, offsets::FortItemDefinition::Tier) == 4)
						{
							ItemColor = ImGui::GetColorU32({ 0.7f, 0.25f, 0.85f, 0.95f });
						}
						else if (ReadBYTE(item, offsets::FortItemDefinition::Tier) == 5)
						{
							ItemColor = ImGui::GetColorU32({ 0.85f, 0.65f, 0.0f, 0.95f });
						}
						else if (ReadBYTE(item, offsets::FortItemDefinition::Tier) == 6)
						{
							ItemColor = ImGui::GetColorU32({ 255, 255, 255, 255 });
						}
						else if (ReadBYTE(item, offsets::FortItemDefinition::Tier) == 7)
						{
							ItemColor = ImGui::GetColorU32({ 255, 255, 255, 255 });
						}
						else if (ReadBYTE(item, offsets::FortItemDefinition::Tier) == 8)
						{
							ItemColor = ImGui::GetColorU32({ 255, 255, 255, 255 });
						}
						else if (ReadBYTE(item, offsets::FortItemDefinition::Tier) == 9)
						{
							ItemColor = ImGui::GetColorU32({ 255, 255, 255, 255 });
						}
						else if (ReadBYTE(item, offsets::FortItemDefinition::Tier) == 10)
						{
							ItemColor = ImGui::GetColorU32({ 255, 255, 255, 255 });
						}
						auto ItemRoot = Util::GetPawnRootLocation(pawn);
						if (ItemRoot) {
							auto ItemPos = *ItemRoot;
							float dx = localPlayerLocation[0] - ItemPos.X;
							float dy = localPlayerLocation[1] - ItemPos.Y;
							float dz = localPlayerLocation[2] - ItemPos.Z;

							if (Util::WorldToScreen(width, height, &ItemPos.X)) {
								float dist = Util::SpoofCall(sqrtf, dx * dx + dy * dy + dz * dz) / 100.0f;

								if (dist < 120)
								{
									DrawOutlinedText(m_pFont, TextFormat(xorstr("%s [%.0f m]"), text, dist), ImVec2(ItemPos.X, ItemPos.Y), 10.5f, ItemColor, true);
								}
							}
						}
					}

					else if (Settings.ESP.Containers && wcsstr(name.c_str(), xorstr(L"Tiered_Chest")) && !((ReadBYTE(pawn, offsets::BuildingContainer::bAlreadySearched) >> 7) & 1)) {
						auto ChestRoot = Util::GetPawnRootLocation(pawn);
						if (ChestRoot) {
							auto ChestPos = *ChestRoot;
							float dx = localPlayerLocation[0] - ChestPos.X;
							float dy = localPlayerLocation[1] - ChestPos.Y;
							float dz = localPlayerLocation[2] - ChestPos.Z;

							if (Util::WorldToScreen(width, height, &ChestPos.X)) {
								float dist = Util::SpoofCall(sqrtf, dx * dx + dy * dy + dz * dz) / 100.0f;

								if (dist < 100)
								{
									DrawOutlinedText(m_pFont, TextFormat(xorstr("Chest thisisrico.de [%.0f m]"), dist), ImVec2(ChestPos.X, ChestPos.Y), 10.5f, ImGui::GetColorU32({ 205, 230, 0, 255 }), true);
								}
							}
						}
					}
					else if (Settings.ESP.LLamas && wcsstr(name.c_str(), xorstr(L"AthenaSupplyDrop_Llama"))) {

						auto LLamaRoot = Util::GetPawnRootLocation(pawn);
						if (LLamaRoot) {
							auto LLamaPos = *LLamaRoot;
							float dx = localPlayerLocation[0] - LLamaPos.X;
							float dy = localPlayerLocation[1] - LLamaPos.Y;
							float dz = localPlayerLocation[2] - LLamaPos.Z;

							if (Util::WorldToScreen(width, height, &LLamaPos.X)) {
								float dist = Util::SpoofCall(sqrtf, dx * dx + dy * dy + dz * dz) / 100.0f;

								if (dist < 5000)
								{
									DrawOutlinedText(m_pFont, TextFormat(xorstr("Gay Lama [%.0f m]"), dist), ImVec2(LLamaPos.X, LLamaPos.Y), 10.5f, ImGui::GetColorU32({ 210, 0, 240, 255 }), true);
								}
							}
						}
					}
					else if (Settings.botesp && wcsstr(name.c_str(), xorstr(L"BP_PlayerPawn_Athena_Phoebe_C"))) {

						auto Bot = Util::GetPawnRootLocation(pawn);
						if (Bot) {
							auto Botroot = *Bot;
							float dx = localPlayerLocation[0] - Botroot.X;
							float dy = localPlayerLocation[1] - Botroot.Y;
							float dz = localPlayerLocation[2] - Botroot.Z;

							if (Util::WorldToScreen(width, height, &Botroot.X)) {
								float dist = Util::SpoofCall(sqrtf, dx * dx + dy * dy + dz * dz) / 100.0f;

								if (dist < 5000)
								{                                  //ImGui::GetColorU32({ color_red, color_green, color_blue, 255 });
									DrawOutlinedText(m_pFont, TextFormat(xorstr("BOT [%.0f m]"), dist), ImVec2(Botroot.X, Botroot.Y), 10.5f, ImGui::GetColorU32({ 255, 255, 255, 255 }), true);
								}
							}
						}
					}
					else if (Settings.realplayertags && wcsstr(name.c_str(), xorstr(L"PlayerPawn_Athena_C"))) {

						auto Bot = Util::GetPawnRootLocation(pawn);
						if (Bot) {
							auto Botroot = *Bot;
							float dx = localPlayerLocation[0] - Botroot.X;
							float dy = localPlayerLocation[1] - Botroot.Y;
							float dz = localPlayerLocation[2] - Botroot.Z;

							if (Util::WorldToScreen(width, height, &Botroot.X)) {
								float dist = Util::SpoofCall(sqrtf, dx * dx + dy * dy + dz * dz) / 100.0f;

								if (dist < 5000)
								{
									DrawOutlinedText(m_pFont, TextFormat(xorstr("ENEMY [%.0f m]"), dist), ImVec2(Botroot.X, Botroot.Y), 10.5f, ImGui::GetColorU32({ 255, 0, 255 , 255 }), true);
								}
							}
						}
					}

					//PlayerPawn_Athena_C

					else if (Settings.HenchMen && wcsstr(name.c_str(), xorstr(L"BP_Gibson"))) {

						auto Bot = Util::GetPawnRootLocation(pawn);
						if (Bot) {
							auto Botroot = *Bot;
							float dx = localPlayerLocation[0] - Botroot.X;
							float dy = localPlayerLocation[1] - Botroot.Y;
							float dz = localPlayerLocation[2] - Botroot.Z;

							if (Util::WorldToScreen(width, height, &Botroot.X)) {
								float dist = Util::SpoofCall(sqrtf, dx * dx + dy * dy + dz * dz) / 100.0f;

								if (dist < 5000)
								{
									DrawOutlinedText(m_pFont, TextFormat(xorstr("HENCH MAN [%.0f m]"), dist), ImVec2(Botroot.X, Botroot.Y), 14.5f, ImGui::GetColorU32({ 0, 0, 0, 255 }), true);
								}
							}
						}
					}




					else if (Settings.HenchMen && wcsstr(name.c_str(), xorstr(L"BP_GasketPlayerPawn_DateAlpha_C"))) {

						auto Bot = Util::GetPawnRootLocation(pawn);
						if (Bot) {
							auto Botroot = *Bot;
							float dx = localPlayerLocation[0] - Botroot.X;
							float dy = localPlayerLocation[1] - Botroot.Y;
							float dz = localPlayerLocation[2] - Botroot.Z;

							if (Util::WorldToScreen(width, height, &Botroot.X)) {
								float dist = Util::SpoofCall(sqrtf, dx * dx + dy * dy + dz * dz) / 100.0f;

								if (dist < 5000)
								{
									DrawOutlinedText(m_pFont, TextFormat(xorstr("Hench Men [%.0f m]"), dist), ImVec2(Botroot.X, Botroot.Y), 14.5f, ImGui::GetColorU32({ 0, 0, 0, 255 }), true);
								}
							}
						}
					}

					else if (Settings.HenchMen && wcsstr(name.c_str(), xorstr(L"BP_GasketPlayerPawn_Date_C"))) {

						auto Bot = Util::GetPawnRootLocation(pawn);
						if (Bot) {
							auto Botroot = *Bot;
							float dx = localPlayerLocation[0] - Botroot.X;
							float dy = localPlayerLocation[1] - Botroot.Y;
							float dz = localPlayerLocation[2] - Botroot.Z;

							if (Util::WorldToScreen(width, height, &Botroot.X)) {
								float dist = Util::SpoofCall(sqrtf, dx * dx + dy * dy + dz * dz) / 100.0f;

								if (dist < 5000)
								{
									DrawOutlinedText(m_pFont, TextFormat(xorstr("Hench Men [%.0f m]"), dist), ImVec2(Botroot.X, Botroot.Y), 14.5f, ImGui::GetColorU32({ 0, 0, 0, 255 }), true);
								}
							}
						}
					}

					else if (Settings.HenchMen && wcsstr(name.c_str(), xorstr(L"BP_GasketPlayerPawn_Lonely_C"))) {

						auto Bot = Util::GetPawnRootLocation(pawn);
						if (Bot) {
							auto Botroot = *Bot;
							float dx = localPlayerLocation[0] - Botroot.X;
							float dy = localPlayerLocation[1] - Botroot.Y;
							float dz = localPlayerLocation[2] - Botroot.Z;

							if (Util::WorldToScreen(width, height, &Botroot.X)) {
								float dist = Util::SpoofCall(sqrtf, dx * dx + dy * dy + dz * dz) / 100.0f;

								if (dist < 5000)
								{
									DrawOutlinedText(m_pFont, TextFormat(xorstr("Hench Men [%.0f m]"), dist), ImVec2(Botroot.X, Botroot.Y), 14.5f, ImGui::GetColorU32({ 0, 0, 0, 255 }), true);
								}
							}
						}
					}


					else if (Settings.HenchMen && wcsstr(name.c_str(), xorstr(L"BP_GasketPlayerPawn_TomatoAlpha_C"))) {

						auto Bot = Util::GetPawnRootLocation(pawn);
						if (Bot) {
							auto Botroot = *Bot;
							float dx = localPlayerLocation[0] - Botroot.X;
							float dy = localPlayerLocation[1] - Botroot.Y;
							float dz = localPlayerLocation[2] - Botroot.Z;

							if (Util::WorldToScreen(width, height, &Botroot.X)) {
								float dist = Util::SpoofCall(sqrtf, dx * dx + dy * dy + dz * dz) / 100.0f;

								if (dist < 5000)
								{
									DrawOutlinedText(m_pFont, TextFormat(xorstr("Hench Men [%.0f m]"), dist), ImVec2(Botroot.X, Botroot.Y), 14.5f, ImGui::GetColorU32({ 0, 0, 0, 255 }), true);
								}
							}
						}
					}

					else if (Settings.HenchMen && wcsstr(name.c_str(), xorstr(L"BP_GasketPlayerPawn_Tomato_C"))) {

						auto Bot = Util::GetPawnRootLocation(pawn);
						if (Bot) {
							auto Botroot = *Bot;
							float dx = localPlayerLocation[0] - Botroot.X;
							float dy = localPlayerLocation[1] - Botroot.Y;
							float dz = localPlayerLocation[2] - Botroot.Z;

							if (Util::WorldToScreen(width, height, &Botroot.X)) {
								float dist = Util::SpoofCall(sqrtf, dx * dx + dy * dy + dz * dz) / 100.0f;

								if (dist < 5000)
								{
									DrawOutlinedText(m_pFont, TextFormat(xorstr("Hench Men [%.0f m]"), dist), ImVec2(Botroot.X, Botroot.Y), 14.5f, ImGui::GetColorU32({ 0, 0, 0, 255 }), true);
								}
							}
						}
					}

					else if (Settings.HenchMen && wcsstr(name.c_str(), xorstr(L"BP_GasketPlayerPawn_Wasabi_C"))) {

						auto Bot = Util::GetPawnRootLocation(pawn);
						if (Bot) {
							auto Botroot = *Bot;
							float dx = localPlayerLocation[0] - Botroot.X;
							float dy = localPlayerLocation[1] - Botroot.Y;
							float dz = localPlayerLocation[2] - Botroot.Z;

							if (Util::WorldToScreen(width, height, &Botroot.X)) {
								float dist = Util::SpoofCall(sqrtf, dx * dx + dy * dy + dz * dz) / 100.0f;

								if (dist < 5000)
								{
									DrawOutlinedText(m_pFont, TextFormat(xorstr("Hench Men [%.0f m]"), dist), ImVec2(Botroot.X, Botroot.Y), 14.5f, ImGui::GetColorU32({ 0, 0, 0, 255 }), true);
								}
							}
						}
					}



					else if (Settings.ESP.AmmoBox && wcsstr(name.c_str(), xorstr(L"Tiered_Ammo")) && !((ReadBYTE(pawn, offsets::BuildingContainer::bAlreadySearched) >> 7) & 1)) {
						auto AmmoBoxRoot = Util::GetPawnRootLocation(pawn);
						if (AmmoBoxRoot) {
							auto AmmoBoxPos = *AmmoBoxRoot;
							float dx = localPlayerLocation[0] - AmmoBoxPos.X;
							float dy = localPlayerLocation[1] - AmmoBoxPos.Y;
							float dz = localPlayerLocation[2] - AmmoBoxPos.Z;

							if (Util::WorldToScreen(width, height, &AmmoBoxPos.X)) {
								float dist = Util::SpoofCall(sqrtf, dx * dx + dy * dy + dz * dz) / 100.0f;

								if (dist < 100)
								{
									DrawOutlinedText(m_pFont, TextFormat(xorstr("AMMO BOX [%.0f m]"), dist), ImVec2(AmmoBoxPos.X, AmmoBoxPos.Y), 14.5f, ImGui::GetColorU32({ 0.75f, 0.75f, 0.75f, 1.0f }), true);
								}
							}
						}
					}

					else if (Settings.ESP.SupplyDrops && wcsstr(name.c_str(), xorstr(L"AthenaSupplyDrop_C")) && !((ReadBYTE(pawn, offsets::BuildingContainer::bAlreadySearched) >> 7) & 1)) {
						auto SupplyDropRoot = Util::GetPawnRootLocation(pawn);
						if (SupplyDropRoot) {
							auto SupplyDropPos = *SupplyDropRoot;
							float dx = localPlayerLocation[0] - SupplyDropPos.X;
							float dy = localPlayerLocation[1] - SupplyDropPos.Y;
							float dz = localPlayerLocation[2] - SupplyDropPos.Z;

							if (Util::WorldToScreen(width, height, &SupplyDropPos.X)) {
								float dist = Util::SpoofCall(sqrtf, dx * dx + dy * dy + dz * dz) / 100.0f;

								if (dist < 5000)
								{
									DrawOutlinedText(m_pFont, TextFormat(xorstr("SUPPLY DROP [%.0f m]"), dist), ImVec2(SupplyDropPos.X, SupplyDropPos.Y), 14.5f, ImGui::GetColorU32({ 205, 230, 0, 255 }), true);
								}
							}
						}
					}
					else if (Settings.ESP.Vehicles)
					{
						if (wcsstr(name.c_str(), xorstr(L"MeatballVehicle_L")) || wcsstr(name.c_str(), xorstr(L"JackalVehicle_Athena_C")) || wcsstr(name.c_str(), xorstr(L"FerretVehicle_C")) || wcsstr(name.c_str(), xorstr(L"AntelopeVehicle_C")) || wcsstr(name.c_str(), xorstr(L"GolfCartVehicleSK_C")) || wcsstr(name.c_str(), xorstr(L"TestMechVehicle_C")) || wcsstr(name.c_str(), xorstr(L"OctopusVehicle_C")) || wcsstr(name.c_str(), xorstr(L"ShoppingCartVehicleSK_C")) || wcsstr(name.c_str(), xorstr(L"HoagieVehicle_C")))
						{
							auto VehicleRoot = Util::GetPawnRootLocation(pawn);
							if (VehicleRoot) {
								auto VehiclePos = *VehicleRoot;
								float dx = localPlayerLocation[0] - VehiclePos.X;
								float dy = localPlayerLocation[1] - VehiclePos.Y;
								float dz = localPlayerLocation[2] - VehiclePos.Z;

								if (Util::WorldToScreen(width, height, &VehiclePos.X)) {
									float dist = Util::SpoofCall(sqrtf, dx * dx + dy * dy + dz * dz) / 100.0f;

									if (dist < 1500)
									{
										DrawOutlinedText(m_pFont, TextFormat(xorstr("VEHICLE [%.0f m]"), dist), ImVec2(VehiclePos.X, VehiclePos.Y), 14.5f, ImGui::GetColorU32({ 0, 65, 200, 255 }), true);
									}
								}
							}
						}
					}
					else if (Settings.ESP.Memes)
					{
						if (wcsstr(name.c_str(), xorstr(L"Rock")))
						{
							auto RockRoot = Util::GetPawnRootLocation(pawn);
							if (RockRoot) {
								auto RockPos = *RockRoot;
								float dx = localPlayerLocation[0] - RockPos.X;
								float dy = localPlayerLocation[1] - RockPos.Y;
								float dz = localPlayerLocation[2] - RockPos.Z;

								if (Util::WorldToScreen(width, height, &RockPos.X)) {
									float dist = Util::SpoofCall(sqrtf, dx * dx + dy * dy + dz * dz) / 100.0f;

									if (dist < 200)
									{
										DrawOutlinedText(m_pFont, TextFormat(xorstr("Rock [%.0f m]"), dist), ImVec2(RockPos.X, RockPos.Y), 14.5f, ImGui::GetColorU32({ 0.70f, 0.70f, 0.70f, 1.f }), true);
									}
								}
							}
						}
						else if (wcsstr(name.c_str(), xorstr(L"Tree")) || wcsstr(name.c_str(), xorstr(L"Pine")))
						{
							auto TreeRoot = Util::GetPawnRootLocation(pawn);
							if (TreeRoot) {
								auto TreePos = *TreeRoot;
								float dx = localPlayerLocation[0] - TreePos.X;
								float dy = localPlayerLocation[1] - TreePos.Y;
								float dz = localPlayerLocation[2] - TreePos.Z;

								if (Util::WorldToScreen(width, height, &TreePos.X)) {
									float dist = Util::SpoofCall(sqrtf, dx * dx + dy * dy + dz * dz) / 100.0f;

									if (dist < 200)
									{
										DrawOutlinedText(m_pFont, TextFormat(xorstr("TREE [%.0f m]"), dist), ImVec2(TreePos.X, TreePos.Y), 14.5f, ImGui::GetColorU32({ 0.0f, 0.95f, 0.0f, 1.f }), true);
									}
								}
							}
						}
						else if (wcsstr(name.c_str(), xorstr(L"Bush")))
						{
							auto BushRoot = Util::GetPawnRootLocation(pawn);
							if (BushRoot) {
								auto BushPos = *BushRoot;
								float dx = localPlayerLocation[0] - BushPos.X;
								float dy = localPlayerLocation[1] - BushPos.Y;
								float dz = localPlayerLocation[2] - BushPos.Z;

								if (Util::WorldToScreen(width, height, &BushPos.X)) {
									float dist = Util::SpoofCall(sqrtf, dx * dx + dy * dy + dz * dz) / 100.0f;

									if (dist < 200)
									{
										DrawOutlinedText(m_pFont, TextFormat(xorstr("BUSH [%.0f m]"), dist), ImVec2(BushPos.X, BushPos.Y), 14.5f, ImGui::GetColorU32({ 0.2f, 1.f, 0.2f, 1.f }), true);
									}
								}
							}
						}
					}

				}
			}
			if (Settings.ESP.debug2)
			{

				DrawNormalText(m_pFont, TextFormat(xorstr("UWorld: %p"), world), ImVec2(2, 370), 14.5f, ImGui::GetColorU32({ 255, 255, 255, 255 }), false);
				DrawNormalText(m_pFont, TextFormat(xorstr("Localplayer: %p"), localPlayer), ImVec2(2, 390), 14.5f, ImGui::GetColorU32({ 255, 255, 255, 255 }), false);
				DrawNormalText(m_pFont, TextFormat(xorstr("Gameinstance: %p"), gameInstance), ImVec2(2, 410), 14.5f, ImGui::GetColorU32({ 255, 255, 255, 255 }), false);
				DrawNormalText(m_pFont, TextFormat(xorstr("Playercontroller: %p"), localPlayerController), ImVec2(2, 430), 14.5f, ImGui::GetColorU32({ 255, 255, 255, 255 }), false);
				DrawNormalText(m_pFont, TextFormat(xorstr("Localpawn: %p"), localPlayerPawn), ImVec2(2, 450), 14.5f, ImGui::GetColorU32({ 255, 255, 255, 255 }), false);
				DrawNormalText(m_pFont, TextFormat(xorstr("Rootromponent: %p"), localPlayerRoot), ImVec2(2, 470), 14.5f, ImGui::GetColorU32({ 255, 255, 255, 255 }), false);
				DrawNormalText(m_pFont, TextFormat(xorstr("Playerstate: %p"), localPlayerState), ImVec2(2, 490), 14.5f, ImGui::GetColorU32({ 255, 255, 255, 255 }), false);
			}

			if (Settings.Info)
			{

				auto LocalItemDef = ReadPointer(localPlayerWeapon, offsets::FortWeapon::WeaponData);
				if (!LocalItemDef) continue;

				auto LocalItemDisplayName = reinterpret_cast<FText*>(ReadPointer(LocalItemDef, offsets::FortItemDefinition::DisplayName));
				if (!LocalItemDisplayName || !LocalItemDisplayName->c_str()) continue;

				int LocalAmmoCount = ReadInt(localPlayerWeapon, offsets::FortWeapon::AmmoCount);
				if (!LocalAmmoCount) continue;

				const char* AimbotTargetStatus;

				if (hooks::TargetPawn != nullptr)
				{
					AimbotTargetStatus = xorstr("Yes");
				}
				else
				{
					AimbotTargetStatus = xorstr("No");
				}

				CHAR Weapon[0xFF] = { 0 };
				wcstombs(Weapon, LocalItemDisplayName->c_str(), sizeof(Weapon));

				DrawOutlinedText(m_pFont, TextFormat(xorstr("Aimbot has target : %s"), AimbotTargetStatus), ImVec2(width / 2, 50), 16.f, ImGui::GetColorU32({ 255, 255, 255, 255 }), true);
				DrawOutlinedText(m_pFont, TextFormat(xorstr("Location [x: %i] [y: %i] [z: %i]"), localPlayerLocation[0], localPlayerLocation[1], localPlayerLocation[2]), ImVec2(width / 2, 65), 16.f, ImGui::GetColorU32({ 255, 255, 255, 255 }), true);
				DrawNormalText(m_pFont, TextFormat(xorstr("Weapon: %s"), Weapon), ImVec2(width / 2, 80), 14.5f, ImGui::GetColorU32({ 255, 255, 255, 255 }), false);
				DrawNormalText(m_pFont, TextFormat(xorstr("Ammo: %i"), LocalAmmoCount), ImVec2(width / 2, 95), 14.5f, ImGui::GetColorU32({ 255, 255, 255, 255 }), false);

			}

			for (auto pawn : playerPawns) {

				auto state = ReadPointer(pawn, offsets::Pawn::PlayerState);
				if (!state) continue;

				auto mesh = ReadPointer(pawn, offsets::Character::Mesh);
				if (!mesh) continue;

				auto bones = ReadPointer(mesh, offsets::StaticMeshComponent::StaticMesh);
				if (!bones) bones = ReadPointer(mesh, offsets::StaticMeshComponent::StaticMesh + 0x10);
				if (!bones) continue;

				auto actorTeamIndex = ReadDWORD(state, offsets::FortPlayerStateAthena::TeamIndex);

				auto playerRoot = Util::GetPawnRootLocation(pawn);
				if (playerRoot) {
					auto playerPos = *playerRoot;
					float dx = localPlayerLocation[0] - playerPos.X;
					float dy = localPlayerLocation[1] - playerPos.Y;
					float dz = localPlayerLocation[2] - playerPos.Z;

					float dist = Util::SpoofCall(sqrtf, dx * dx + dy * dy + dz * dz) / 100.0f;
					hooks::Distance = dist;
				}

				float compMatrix[4][4] = { 0 };
				Util::ToMatrixWithScale(reinterpret_cast<float*>(reinterpret_cast<PBYTE>(mesh) + offsets::StaticMeshComponent::ComponentToWorld), compMatrix);

				// root
				float root[3] = { 0 };
				Util::GetBoneLocation(compMatrix, bones, 0, root);

				// Top
				float head[3] = { 0 };
				Util::GetBoneLocation(compMatrix, bones, BONE_HEAD_ID, head);

				float head2[3] = { 0 };
				Util::GetBoneLocation(compMatrix, bones, BONE_HEAD_ID, head2);


				float body[3] = { 0 };
				Util::GetBoneLocation(compMatrix, bones, 5, body);

				float neck[3] = { 0 };
				Util::GetBoneLocation(compMatrix, bones, 65, neck);

				float chest[3] = { 0 };
				Util::GetBoneLocation(compMatrix, bones, 36, chest);

				float pelvis[3] = { 0 };
				Util::GetBoneLocation(compMatrix, bones, 2, pelvis);

				// Arms
				float leftShoulder[3] = { 0 };
				Util::GetBoneLocation(compMatrix, bones, 9, leftShoulder);

				float rightShoulder[3] = { 0 };
				Util::GetBoneLocation(compMatrix, bones, 62, rightShoulder);

				float leftElbow[3] = { 0 };
				Util::GetBoneLocation(compMatrix, bones, 10, leftElbow);

				float rightElbow[3] = { 0 };
				Util::GetBoneLocation(compMatrix, bones, 38, rightElbow);

				float leftHand[3] = { 0 };
				Util::GetBoneLocation(compMatrix, bones, 11, leftHand);

				float rightHand[3] = { 0 };
				Util::GetBoneLocation(compMatrix, bones, 39, rightHand);

				// Legs
				float leftLeg[3] = { 0 };
				Util::GetBoneLocation(compMatrix, bones, 67, leftLeg);

				float rightLeg[3] = { 0 };
				Util::GetBoneLocation(compMatrix, bones, 74, rightLeg);

				float leftThigh[3] = { 0 };
				Util::GetBoneLocation(compMatrix, bones, 73, leftThigh);

				float rightThigh[3] = { 0 };
				Util::GetBoneLocation(compMatrix, bones, 80, rightThigh);

				float leftFoot[3] = { 0 };
				Util::GetBoneLocation(compMatrix, bones, 68, leftFoot);

				float rightFoot[3] = { 0 };
				Util::GetBoneLocation(compMatrix, bones, 75, rightFoot);

				float leftFeet[3] = { 0 };
				Util::GetBoneLocation(compMatrix, bones, 71, leftFeet);

				float rightFeet[3] = { 0 };
				Util::GetBoneLocation(compMatrix, bones, 78, rightFeet);

				float leftFeetFinger[3] = { 0 };
				Util::GetBoneLocation(compMatrix, bones, 72, leftFeetFinger);

				float rightFeetFinger[3] = { 0 };
				Util::GetBoneLocation(compMatrix, bones, 79, rightFeetFinger);

				auto AimTargetColor = ImGui::GetColorU32({ color_red, color_green, color_blue, 255 });
				auto BoxColor = ImGui::GetColorU32({ 0, 0.8f, 1, 1 }); //box color when not visible
				auto NameColor = ImGui::GetColorU32({ color_red, color_green, color_blue, 255 });
				auto SnaplineColor = ImGui::GetColorU32({ color_red, color_green, color_blue, 255 });
				auto SkeletonColor = ImGui::GetColorU32({ Settings.ESP.SkeletonNotVisibleColor[0], Settings.ESP.SkeletonNotVisibleColor[1], Settings.ESP.SkeletonNotVisibleColor[2], Settings.ESP.SkeletonNotVisibleColor[3] }); //skeleton color when not visible
				auto RadarColor = ImGui::GetColorU32({ 255, 0, 0, 255 });
				FVector viewPoint = { 0 };


				if (Settings.IsBulletTeleporting || Util::LineOfSightTo(localPlayerController, pawn, &viewPoint))
				{
					AimTargetColor = ImGui::GetColorU32({ Settings.ESP.TargetLineColor[0], Settings.ESP.TargetLineColor[1], Settings.ESP.TargetLineColor[2], Settings.ESP.TargetLineColor[3] });

					BoxColor = ImGui::GetColorU32({ 0, 0.8f, 1, 1 }); //box color when visible
					NameColor = ImGui::GetColorU32({ color_red, color_green, color_blue, 255 });
					SnaplineColor = ImGui::GetColorU32({ color_red, color_green, color_blue, 255 });
					SkeletonColor = ImGui::GetColorU32({ Settings.ESP.SkeletonVisibleColor[0], Settings.ESP.SkeletonVisibleColor[1], Settings.ESP.SkeletonVisibleColor[2], Settings.ESP.SkeletonVisibleColor[3] }); //skeleton color when visible
				}


				if (Settings.Chams)
				{
					if (localPlayerTeamIndex == actorTeamIndex)
					{

						if (Settings.VisibleCheck)
						{
							if (Settings.IsBulletTeleporting)
							{
								if (Settings.AutoAimbot) //closest by distance
								{
									auto dx = head[0] - localPlayerLocation[0];
									auto dy = head[1] - localPlayerLocation[1];
									auto dz = head[2] - localPlayerLocation[2];
									auto dist = dx * dx + dy * dy + dz * dz;
									if (dist < closestDistance) {
										closestDistance = dist;
										closestPawn = pawn;
									}
								}
								else //closest from crosshair
								{
									auto w2s = *reinterpret_cast<FVector*>(head);
									if (Util::WorldToScreen(width, height, &w2s.X)) {
										auto dx = w2s.X - (width / 2);
										auto dy = w2s.Y - (height / 2);
										auto dist = Util::SpoofCall(sqrtf, dx * dx + dy * dy);
										if (dist < Settings.AimbotFOV && dist < closestDistance) {
											closestDistance = dist;
											closestPawn = pawn;
										}
									}
								}
							}
							else
							{
								if (Util::LineOfSightTo(localPlayerController, pawn, &viewPoint))
								{
									if (Settings.AutoAimbot) //closest by distance
									{
										auto dx = head[0] - localPlayerLocation[0];
										auto dy = head[1] - localPlayerLocation[1];
										auto dz = head[2] - localPlayerLocation[2];
										auto dist = dx * dx + dy * dy + dz * dz;
										if (dist < closestDistance) {
											closestDistance = dist;
											closestPawn = pawn;
										}
									}
									else //closest from crosshair
									{
										auto w2s = *reinterpret_cast<FVector*>(head);
										if (Util::WorldToScreen(width, height, &w2s.X)) {
											auto dx = w2s.X - (width / 2);
											auto dy = w2s.Y - (height / 2);
											auto dist = Util::SpoofCall(sqrtf, dx * dx + dy * dy);
											if (dist < Settings.AimbotFOV && dist < closestDistance) {
												closestDistance = dist;
												closestPawn = pawn;
											}
										}
									}
								}
							}
						}
						else
						{
							if (Settings.AutoAimbot) //closest by distance
							{
								auto dx = head[0] - localPlayerLocation[0];
								auto dy = head[1] - localPlayerLocation[1];
								auto dz = head[2] - localPlayerLocation[2];
								auto dist = dx * dx + dy * dy + dz * dz;
								if (dist < closestDistance) {
									closestDistance = dist;
									closestPawn = pawn;
								}
							}
							else //closest from crosshair
							{
								auto w2s = *reinterpret_cast<FVector*>(head);
								if (Util::WorldToScreen(width, height, &w2s.X))
								{
									auto dx = w2s.X - (width / 2);
									auto dy = w2s.Y - (height / 2);
									auto dist = Util::SpoofCall(sqrtf, dx * dx + dy * dy);
									if (dist < Settings.AimbotFOV && dist < closestDistance) {
										closestDistance = dist;
										closestPawn = pawn;
									}
								}
							}
						}
						auto VehicleTargetW2S = *reinterpret_cast<FVector*>(head);
						if (Util::WorldToScreen(width, height, &VehicleTargetW2S.X))
						{
							auto tdx = VehicleTargetW2S.X - (width / 2);
							auto tdy = VehicleTargetW2S.Y - (height / 2);
							auto tdist = Util::SpoofCall(sqrtf, tdx * tdx + tdy * tdy);
							if (tdist < 5000 && tdist < closestDistanceForVehicle) {
								closestDistanceForVehicle = tdist;
								closestPawnForVehicle = pawn;
							}
						}
					}
				}
				else if (localPlayerTeamIndex != actorTeamIndex)
				{

					if (Settings.VisibleCheck)
					{
						if (Settings.IsBulletTeleporting)
						{
							if (Settings.AutoAimbot) //closest by distance
							{
								auto dx = head[0] - localPlayerLocation[0];
								auto dy = head[1] - localPlayerLocation[1];
								auto dz = head[2] - localPlayerLocation[2];
								auto dist = dx * dx + dy * dy + dz * dz;
								if (dist < closestDistance) {
									closestDistance = dist;
									closestPawn = pawn;
								}
							}
							else //closest from crosshair
							{
								auto w2s = *reinterpret_cast<FVector*>(head);
								if (Util::WorldToScreen(width, height, &w2s.X)) {
									auto dx = w2s.X - (width / 2);
									auto dy = w2s.Y - (height / 2);
									auto dist = Util::SpoofCall(sqrtf, dx * dx + dy * dy);
									if (dist < Settings.AimbotFOV && dist < closestDistance) {
										closestDistance = dist;
										closestPawn = pawn;
									}
								}
							}
						}
						else
						{
							if (Util::LineOfSightTo(localPlayerController, pawn, &viewPoint))
							{
								if (Settings.AutoAimbot) //closest by distance
								{
									auto dx = head[0] - localPlayerLocation[0];
									auto dy = head[1] - localPlayerLocation[1];
									auto dz = head[2] - localPlayerLocation[2];
									auto dist = dx * dx + dy * dy + dz * dz;
									if (dist < closestDistance) {
										closestDistance = dist;
										closestPawn = pawn;
									}
								}
								else //closest from crosshair
								{
									auto w2s = *reinterpret_cast<FVector*>(head);
									if (Util::WorldToScreen(width, height, &w2s.X)) {
										auto dx = w2s.X - (width / 2);
										auto dy = w2s.Y - (height / 2);
										auto dist = Util::SpoofCall(sqrtf, dx * dx + dy * dy);
										if (dist < Settings.AimbotFOV && dist < closestDistance) {
											closestDistance = dist;
											closestPawn = pawn;
										}
									}
								}
							}
						}
					}
					else
					{
						if (Settings.AutoAimbot) //closest by distance
						{
							auto dx = head[0] - localPlayerLocation[0];
							auto dy = head[1] - localPlayerLocation[1];
							auto dz = head[2] - localPlayerLocation[2];
							auto dist = dx * dx + dy * dy + dz * dz;
							if (dist < closestDistance) {
								closestDistance = dist;
								closestPawn = pawn;
							}
						}
						else //closest from crosshair
						{
							auto w2s = *reinterpret_cast<FVector*>(head);
							if (Util::WorldToScreen(width, height, &w2s.X))
							{
								auto dx = w2s.X - (width / 2);
								auto dy = w2s.Y - (height / 2);
								auto dist = Util::SpoofCall(sqrtf, dx * dx + dy * dy);
								if (dist < Settings.AimbotFOV && dist < closestDistance) {
									closestDistance = dist;
									closestPawn = pawn;
								}
							}
						}
					}
					auto VehicleTargetW2S = *reinterpret_cast<FVector*>(head);
					if (Util::WorldToScreen(width, height, &VehicleTargetW2S.X))
					{
						auto tdx = VehicleTargetW2S.X - (width / 2);
						auto tdy = VehicleTargetW2S.Y - (height / 2);
						auto tdist = Util::SpoofCall(sqrtf, tdx * tdx + tdy * tdy);
						if (tdist < 5000 && tdist < closestDistanceForVehicle) {
							closestDistanceForVehicle = tdist;
							closestPawnForVehicle = pawn;
						}
					}
				}


				if (Settings.TeleportToEnemies)
				{
					if (InVehicle)
					{
						if (hooks::VehicleTargetPawn != nullptr)
						{
							auto VehicleTargetMesh = ReadPointer(hooks::VehicleTargetPawn, offsets::Character::Mesh);
							if (!VehicleTargetMesh) continue;

							auto VehicleTargetBones = ReadPointer(VehicleTargetMesh, offsets::StaticMeshComponent::StaticMesh);
							if (!VehicleTargetBones) VehicleTargetBones = ReadPointer(VehicleTargetMesh, offsets::StaticMeshComponent::StaticMesh + 0x10);
							if (!VehicleTargetBones) continue;

							float VehicleTargetCompMatrix[4][4] = { 0 };
							Util::ToMatrixWithScale(reinterpret_cast<float*>(reinterpret_cast<PBYTE>(VehicleTargetMesh) + offsets::StaticMeshComponent::ComponentToWorld), VehicleTargetCompMatrix);

							float VehicleTargetRootLocation[3] = { 0 };
							Util::GetBoneLocation(VehicleTargetCompMatrix, VehicleTargetBones, 66, VehicleTargetRootLocation);

							ClosestPlayerCoords.X = VehicleTargetRootLocation[0];
							ClosestPlayerCoords.Y = VehicleTargetRootLocation[1];
							ClosestPlayerCoords.Z = VehicleTargetRootLocation[2];

							auto VehicleTargetPosition = *reinterpret_cast<FVector*>(VehicleTargetRootLocation);
							if (Util::WorldToScreen(width, height, &VehicleTargetPosition.X))
							{
								window.DrawList->AddLine(ImVec2(width / 2, height / 2), ImVec2(VehicleTargetPosition.X, VehicleTargetPosition.Y), ImGui::GetColorU32({ 255, 0, 0, 255 }));
							}
						}
					}
				}

				/*
				//if (InVehicle)
				//{
					auto VehicleTargetW2S = *reinterpret_cast<FVector*>(head);
					if (Util::WorldToScreen(width, height, &VehicleTargetW2S.X))
					{
						auto tdx = VehicleTargetW2S.X - (width / 2);
						auto tdy = VehicleTargetW2S.Y - (height / 2);
						auto tdist = Util::SpoofCall(sqrtf, tdx * tdx + tdy * tdy);
						if (tdist < 5000 && tdist < closestDistanceForVehicle) {
							closestDistanceForVehicle = tdist;
							closestPawnForVehicle = pawn;
						}
					}
				//}
					if (Settings.TeleportToEnemies)
					{
						if (hooks::VehicleTargetPawn != nullptr)
						{
							auto VehicleTargetMesh = ReadPointer(hooks::VehicleTargetPawn, offsets::Character::Mesh);
							if (!VehicleTargetMesh) continue;

							auto VehicleTargetBones = ReadPointer(VehicleTargetMesh, offsets::StaticMeshComponent::StaticMesh);
							if (!VehicleTargetBones) VehicleTargetBones = ReadPointer(VehicleTargetMesh, offsets::StaticMeshComponent::StaticMesh + 0x10);
							if (!VehicleTargetBones) continue;

							float VehicleTargetCompMatrix[4][4] = { 0 };
							Util::ToMatrixWithScale(reinterpret_cast<float*>(reinterpret_cast<PBYTE>(VehicleTargetMesh) + offsets::StaticMeshComponent::ComponentToWorld), VehicleTargetCompMatrix);

							float VehicleTargetRootLocation[3] = { 0 };
							Util::GetBoneLocation(VehicleTargetCompMatrix, VehicleTargetBones, 66, VehicleTargetRootLocation);

							ClosestPlayerCoords.X = VehicleTargetRootLocation[0];
							ClosestPlayerCoords.Y = VehicleTargetRootLocation[1];
							ClosestPlayerCoords.Z = VehicleTargetRootLocation[2];

							auto VehicleTargetPosition = *reinterpret_cast<FVector*>(VehicleTargetRootLocation);
							if (Util::WorldToScreen(width, height, &VehicleTargetPosition.X))
							{
								window.DrawList->AddLine(ImVec2(width / 2, height / 2), ImVec2(VehicleTargetPosition.X, VehicleTargetPosition.Y), ImGui::GetColorU32({ 255, 0, 0, 255 }));
							}
						}
					}

					if (Settings.TeleportToEnemies)
					{
						if (localPlayerController)
						{
							if (hooks::VehicleTargetPawn != nullptr)
							{
								if (Util::DiscordGetAsyncKeyState(VK_XBUTTON2))
								{
									hooks::Teleport(localPlayerPawn, ClosestPlayerCoords);
								}
							}
						}
					}
					*/
					/*	if (Settings.VehicleTeleporter)
						{
							if (localPlayerController)
							{
								if (InVehicle)
								{
									if (CurrentVehicle)
									{
										if (hooks::VehicleTargetPawn != nullptr)
										{
											if (Util::DiscordGetAsyncKeyState(VK_XBUTTON2))
											{
												hooks::ProcessEvent(CurrentVehicle2, addresses::K2_TeleportTo, &ClosestPlayerCoords, 0);
											}
										}
									}
								}
							}
						}*/
				{
					if (Settings.BigPlayers)
					{
						if (localPlayerPawn && pawn && localPlayerController)
						{
							auto mesh = ReadPointer(pawn, offsets::Character::Mesh);
							if (!mesh) continue;

							hooks::ProcessEvent(mesh, addresses::SetForcedLodModel, &Settings.gayy, 0);
						}
					}

					if (Settings.Chams)
					{
						if (localPlayerTeamIndex != actorTeamIndex)
						{
							*reinterpret_cast<PVOID*>(reinterpret_cast<PBYTE>(state) + offsets::FortPlayerStateAthena::TeamIndex) = Index;
							*reinterpret_cast<PVOID*>(reinterpret_cast<PBYTE>(state) + 0x1004) = SquadID; // /Script/FortniteGame.FortPlayerStateAthena.SquadId -> 0x1004
						}
					}

					if (Settings.TargetLine)
					{
						if (Settings.Aimbot)
						{
							int TargetHitbox;
							if (Settings.HitBoxPos == 0)
							{
								TargetHitbox = 66;
							}
							else if (Settings.HitBoxPos == 1)
							{
								TargetHitbox = 65;
							}
							else if (Settings.HitBoxPos == 2)
							{
								TargetHitbox = 5;
							}
							else if (Settings.HitBoxPos == 3)
							{
								TargetHitbox = 0;
							}
							else if (Settings.HitBoxPos == 4)
							{
								TargetHitbox = 2;
							}

							if (hooks::TargetPawn != nullptr)
							{
								auto targetMesh = ReadPointer(hooks::TargetPawn, offsets::Character::Mesh);
								if (!targetMesh) continue;

								auto targetBones = ReadPointer(targetMesh, offsets::StaticMeshComponent::StaticMesh);
								if (!targetBones) targetBones = ReadPointer(targetMesh, offsets::StaticMeshComponent::StaticMesh + 0x10);
								if (!targetBones) continue;

								float targetCompMatrix[4][4] = { 0 };
								Util::ToMatrixWithScale(reinterpret_cast<float*>(reinterpret_cast<PBYTE>(targetMesh) + offsets::StaticMeshComponent::ComponentToWorld), targetCompMatrix);

								float targetRoot[3] = { 0 };
								Util::GetBoneLocation(targetCompMatrix, targetBones, TargetHitbox, targetRoot);

								auto targetPos = *reinterpret_cast<FVector*>(targetRoot);
								if (Util::WorldToScreen(width, height, &targetPos.X))
								{
									window.DrawList->AddLine(ImVec2(width / 2, height / 2), ImVec2(targetPos.X, targetPos.Y), AimTargetColor);
								}
							}
						}
					}
					if (Settings.ESP.PlayerLines)
					{
						if (Settings.SnaplineStartPoint == 0) //bottom
						{
							root[2] -= 10;
							auto end = *reinterpret_cast<FVector*>(root);
							if (Util::WorldToScreen(width, height, &end.X))
							{
								window.DrawList->AddLine(ImVec2(width / 2, height), ImVec2(end.X, end.Y), SnaplineColor, 2);
							}
						}
						else if (Settings.SnaplineStartPoint == 1) //top
						{
							head[2] += 15;
							auto end = *reinterpret_cast<FVector*>(head);
							if (Util::WorldToScreen(width, height, &end.X))
							{
								window.DrawList->AddLine(ImVec2(width / 2, height), ImVec2(end.X, end.Y), SnaplineColor, 2);
							}
						}
						else if (Settings.SnaplineStartPoint == 2) //center
						{
							auto end = *reinterpret_cast<FVector*>(body);
							if (Util::WorldToScreen(width, height, &end.X))
							{
								window.DrawList->AddLine(ImVec2(width / 2, height), ImVec2(end.X, end.Y), SnaplineColor, 2);
							}
						}
					}

					if (Settings.NoClip == true) {
						auto abccccc = ImGui::GetColorU32({ 1, 1, 1, 1.0f });
						ImGui::GetOverlayDrawList()->AddText(ImVec2(80, 180), ImGui::GetColorU32({ 1, 0, 0, 1 }), "Car/Boat Noclip is ACTIVE [F2]");
						DrawNormalText(m_pFont, TextFormat(xorstr("Car/Boat Noclip is ACTIVE [F2]")), ImVec2(100, 200), 12.0f, abccccc, true);
					}
					float minX = FLT_MAX;
					float maxX = -FLT_MAX;
					float minY = FLT_MAX;
					float maxY = -FLT_MAX;

					if (Settings.ESP.Boxes)
					{
						head[2] += 15;
						root[2] -= 10;
						auto headPos = *reinterpret_cast<FVector*>(head);
						auto bottomPos = *reinterpret_cast<FVector*>(root);
						auto HDot = *reinterpret_cast<FVector*>(head);

						if (Util::WorldToScreen(width, height, &bottomPos.X) && Util::WorldToScreen(width, height, &headPos.X))
						{
							float BoxHeight = (float)(headPos.Y - bottomPos.Y);
							float BoxWidth = BoxHeight * 0.380f;

							float LeftX = (float)headPos.X - (BoxWidth / 1);
							float LeftY = (float)bottomPos.Y;

							float CornerHeight = abs(headPos.Y - bottomPos.Y);
							float CornerWidth = CornerHeight * 0.75; //0.5

							auto topLeft = ImVec2(minX - 3.0f, minY - 3.0f);
							auto bottomRight = ImVec2(maxX + 3.0f, maxY + 3.0f);

							float BoxHeightt = ((bottomRight.y + bottomPos.Y) / 2) - headPos.Y;
							float Headbox = BoxHeightt / 5.2; //change 5.2 to like 2.4

							float X;
							float HeadDot = 150.f;
							float Radius = (Settings.HeadDotSize * X / HeadDot) / 2;



							DrawCorneredBox(headPos.X - (CornerWidth / 2), headPos.Y, CornerWidth, CornerHeight, BoxColor, 1.5);
						}
					}
					/**/
					if (Settings.ESP.PlayerNames)
					{
						head[2] += 15;
						auto headPos = *reinterpret_cast<FVector*>(head);
						auto abccccc = ImGui::GetColorU32({ color_red, color_green, color_blue, 255 });
						if (Util::WorldToScreen(width, height, &headPos.X))
						{
							auto localroot = Util::GetPawnRootLocation(pawn);
							if (root)
							{
								auto position = *localroot;
								float dx = localPlayerLocation[0] - position.X;
								float dy = localPlayerLocation[1] - position.Y;
								float dz = localPlayerLocation[2] - position.Z;

								if (Util::WorldToScreen(width, height, &position.X)) {
									float dist = Util::SpoofCall(sqrtf, dx * dx + dy * dy + dz * dz) / 100.0f;
									FString playerName;
									hooks::ProcessEvent(state, addresses::GetPlayerName, &playerName, 0);
									if (playerName.c_str()) {
										CHAR copy[0xFF] = { 0 };
										wcstombs(copy, playerName.c_str(), sizeof(copy));
										Util::FreeInternal(playerName.c_str());
										DrawNormalText(m_pFont, TextFormat(xorstr("%s (%.0f m)"), copy, dist), ImVec2(headPos.X, headPos.Y - 20), 12.0f, abccccc, true);
									}
								}
							}
						}
					}



					


					//No recoil
					/*if (bNoRecoil && LocalPawn) {

						static float last_fire_ability_time = 0.f;
						localPlayerWeapon = ReadPointer(LocalPawn, Offsets::FortniteGame::FortPawn::CurrentWeapon);
						auto stats = GetWeaponStats(localPlayerWeapon);

						auto& RecoilVert = *reinterpret_cast<float*>(reinterpret_cast<PBYTE>(stats) + Offsets::FortniteGame::FortRangedWeaponStats::RecoilVert);
						auto& RecoilHoriz = *reinterpret_cast<float*>(reinterpret_cast<PBYTE>(stats) + Offsets::FortniteGame::FortRangedWeaponStats::RecoilHoriz);
						auto& RecoilDownsightsMultiplier = *reinterpret_cast<float*>(reinterpret_cast<PBYTE>(stats) + Offsets::FortniteGame::FortRangedWeaponStats::RecoilDownsightsMultiplier);

						if (GetWeaponStats(localPlayerWeapon) != nullptr)
						{
							if (Spoofer::SpoofCall(GetAsyncKeyState, 'Q' & 0x0001)) {
								return;
							}
							if (stats)
							{
								if (*(float*)(LocalWeapon + Offsets::FortniteGame::FortWeapon::LastFireAbilityTime) != last_fire_ability_time) {
									last_fire_ability_time = *(float*)(LocalWeapon + Offsets::FortniteGame::FortWeapon::LastFireAbilityTime);

									originalRecoilVert = RecoilVert;
									originalRecoilHoriz = RecoilHoriz;
									originalRecoilDownsightsMultiplier = RecoilDownsightsMultiplier;

									RecoilVert = 0.0f;
									RecoilHoriz = 0.0f;
									RecoilDownsightsMultiplier = 0.0f;

								}

							}
						}
						else if (*(float*)(LocalWeapon + Offsets::FortniteGame::FortWeapon::LastFireAbilityTime) == last_fire_ability_time) {
							last_fire_ability_time = *(float*)(LocalWeapon + Offsets::FortniteGame::FortWeapon::LastFireAbilityTime);

							RecoilVert = originalRecoilVert;
							RecoilHoriz = originalRecoilHoriz;
							RecoilDownsightsMultiplier = originalRecoilDownsightsMultiplier;
							return;
						}
					}*/







/*

Free open SRC thisisrico.de | Discord ThisIsRico#8696

*/




					if (Settings.ESP.PlayerWeapons)
					{

						auto actorCurrentWeapon = ReadPointer(pawn, offsets::FortPawn::CurrentWeapon);
						if (!actorCurrentWeapon) continue;

						auto actorItemDef = ReadPointer(actorCurrentWeapon, offsets::FortWeapon::WeaponData);
						if (!actorItemDef) continue;

						auto actorItemDisplayName = reinterpret_cast<FText*>(ReadPointer(actorItemDef, offsets::FortItemDefinition::DisplayName));
						if (!actorItemDisplayName || !actorItemDisplayName->c_str()) continue;

						root[2] -= 15;
						auto bottomPos = *reinterpret_cast<FVector*>(root);

						if (Util::WorldToScreen(width, height, &bottomPos.X))
						{
							CHAR text[0xFF] = { 0 };
							wcstombs(text, actorItemDisplayName->c_str(), sizeof(text));
							auto NameCodfghdfghlor = ImGui::GetColorU32({ color_red, color_green, color_blue, 255 });
							DrawNormalText(m_pFont, TextFormat(xorstr("%s"), text), ImVec2(bottomPos.X, bottomPos.Y + 5), 12.0f, NameCodfghdfghlor, true);
						}
					}
					if (Settings.ESP.Skeletons)
					{

						auto dfhgfghdsdfgh = ImGui::GetColorU32({ color_red, color_green, color_blue, 255 });
						AddLine(window, width, height, head2, neck, dfhgfghdsdfgh, minX, maxX, minY, maxY);
						AddLine(window, width, height, neck, pelvis, dfhgfghdsdfgh, minX, maxX, minY, maxY);
						AddLine(window, width, height, chest, leftShoulder, dfhgfghdsdfgh, minX, maxX, minY, maxY);
						AddLine(window, width, height, chest, rightShoulder, dfhgfghdsdfgh, minX, maxX, minY, maxY);
						AddLine(window, width, height, leftShoulder, leftElbow, dfhgfghdsdfgh, minX, maxX, minY, maxY);
						AddLine(window, width, height, rightShoulder, rightElbow, dfhgfghdsdfgh, minX, maxX, minY, maxY);
						AddLine(window, width, height, leftElbow, leftHand, dfhgfghdsdfgh, minX, maxX, minY, maxY);
						AddLine(window, width, height, rightElbow, rightHand, dfhgfghdsdfgh, minX, maxX, minY, maxY);
						AddLine(window, width, height, pelvis, leftLeg, dfhgfghdsdfgh, minX, maxX, minY, maxY);
						AddLine(window, width, height, pelvis, rightLeg, dfhgfghdsdfgh, minX, maxX, minY, maxY);
						AddLine(window, width, height, leftLeg, leftThigh, dfhgfghdsdfgh, minX, maxX, minY, maxY);
						AddLine(window, width, height, rightLeg, rightThigh, dfhgfghdsdfgh, minX, maxX, minY, maxY);
						AddLine(window, width, height, leftThigh, leftFoot, dfhgfghdsdfgh, minX, maxX, minY, maxY);
						AddLine(window, width, height, rightThigh, rightFoot, dfhgfghdsdfgh, minX, maxX, minY, maxY);
						AddLine(window, width, height, leftFoot, leftFeet, dfhgfghdsdfgh, minX, maxX, minY, maxY);
						AddLine(window, width, height, rightFoot, rightFeet, dfhgfghdsdfgh, minX, maxX, minY, maxY);
						AddLine(window, width, height, leftFeet, leftFeetFinger, dfhgfghdsdfgh, minX, maxX, minY, maxY);
						AddLine(window, width, height, rightFeet, rightFeetFinger, dfhgfghdsdfgh, minX, maxX, minY, maxY);
					}

				}

				if (Settings.ESP.PlayerAmmo)
				{
					auto actorCurrentWeapon = ReadPointer(pawn, offsets::FortPawn::CurrentWeapon);
					if (!actorCurrentWeapon) continue;

					int actorAmmoCount = ReadInt(actorCurrentWeapon, offsets::FortWeapon::AmmoCount);
					if (!actorAmmoCount) continue;

					if (actorAmmoCount)
					{
						root[2] -= 15;
						auto bottomPos = *reinterpret_cast<FVector*>(root);

						if (Util::WorldToScreen(width, height, &bottomPos.X))
						{
							int value;
							if (Settings.ESP.PlayerWeapons)
							{
								value = 15;
							}
							else
							{
								value = 5;
							}
							auto NameCoddfghdfhgfghdfghlor = ImGui::GetColorU32({ color_red, color_green, color_blue, 255 });
							DrawNormalText(m_pFont, TextFormat(xorstr("Ammo: %i"), actorAmmoCount), ImVec2(bottomPos.X, bottomPos.Y + value), 12.0f, NameCoddfghdfhgfghdfghlor, true);
						}
					}
				}
			}




			if (Settings.TeleportToEnemies)
			{
				if (InVehicle)
				{
					if (closestPawnForVehicle != nullptr)
					{
						hooks::VehicleTargetPawn = closestPawnForVehicle;
					}
					if (closestPawnVehicle != nullptr)
					{
						hooks::ClosestVehicle = closestPawnVehicle;
					}
				}
				else
				{
					hooks::VehicleTargetPawn = nullptr;
					hooks::ClosestVehicle = nullptr;
				}
			}

			if (Settings.Aimbot)
			{
				if (closestPawn && Util::SpoofCall(GetAsyncKeyState, Settings.AimKey) && Util::SpoofCall(GetForegroundWindow) == hWnd)
				{
					hooks::TargetPawn = closestPawn;
				}
				else
				{
					hooks::TargetPawn = nullptr;
				}
			}
			else
			{
				hooks::TargetPawn = nullptr;
			}

			success = TRUE;
			} while (FALSE);

			if (!success) {
				hooks::LocalPlayerController = hooks::LocalPlayerPawn = hooks::TargetPawn = hooks::VehicleTargetPawn = hooks::ClosestVehicle = VehiclePawn = nullptr;
			}
			EndScene(window);

			return o_present(pSwapChain, sync_interval, flags);
	}

	void InitializeDiscordHook()
	{
		Util::DiscordBaseAddress = (uint64_t)GetModuleHandleA(xorstr("DiscordHook64.dll"));

		const auto pcall_present_discord = Util::FindSignature((BYTE*)Util::DiscordBaseAddress, 0x10000000, (BYTE*)xorstr("\xFF\x15\x00\x00\x00\x00\x8B\xD8\xE8\x00\x00\x00\x00\xE8\x00\x00\x00\x00\xEB\x10"), xorstr("xx????xxx????x????xx"), 0);

		if (!pcall_present_discord)
			return;

		const auto poriginal_present = reinterpret_cast<f_present*>(pcall_present_discord + *reinterpret_cast<int32_t*>(pcall_present_discord + 0x2) + 0x6);

		if (!*poriginal_present)
			return;

		o_present = *poriginal_present;

		*poriginal_present = hk_present;
	}

	__declspec(dllexport) HRESULT DoResize(IDXGISwapChain* swapChain, UINT bufferCount, UINT width, UINT height, DXGI_FORMAT newFormat, UINT swapChainFlags) {
		ImGui_ImplDX11_Shutdown();
		renderTargetView->Release();
		immediateContext->Release();
		device->Release();
		device = nullptr;

		return ResizeOriginal(swapChain, bufferCount, width, height, newFormat, swapChainFlags);
	}

	BOOLEAN Initialize()
	{
		IDXGISwapChain* swapChain = nullptr;
		ID3D11Device* device = nullptr;
		ID3D11DeviceContext* context = nullptr;
		auto                 featureLevel = D3D_FEATURE_LEVEL_11_0;

		DXGI_SWAP_CHAIN_DESC sd = { 0 };
		sd.BufferCount = 1;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		sd.OutputWindow = FindWindow(xorstr(L"UnrealWindow"), xorstr(L"Fortnite  "));
		sd.SampleDesc.Count = 1;
		sd.Windowed = TRUE;

		if (FAILED(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, 0, &featureLevel, 1, D3D11_SDK_VERSION, &sd, &swapChain, &device, nullptr, &context))) {
			MessageBox(0, xorstr(L"ERROR: DX11"), xorstr(L"Hyper"), MB_ICONERROR);
			return FALSE;
		}

		auto table = *reinterpret_cast<PVOID**>(swapChain);
		auto present = table[8];
		auto resize = table[13];

		context->Release();
		device->Release();
		swapChain->Release();

		//InitializeDiscordHook();

		MH_CreateHook(present, hk_present, reinterpret_cast<PVOID*>(&o_present));
		MH_EnableHook(present);

		//Menu::Config();

		MH_CreateHook(resize, DoResize, reinterpret_cast<PVOID*>(&ResizeOriginal));
		MH_EnableHook(resize);

		return TRUE;
	}
}
