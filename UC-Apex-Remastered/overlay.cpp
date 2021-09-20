#include "overlay.h"
#include <string>
#include <filesystem>
#include <thread>
#include "imgui/ksk.h"
#include "imgui/unknownController.h"
#include "imgui/FontAwesome.h"
#include "imgui/IconsFontAwesome.h"
#include "imgui/Comfortaa-Regular.h"
#include "imgui/Comfortaa-Bold.h"
#include "View.h"

// set up these variables here in case we need them later
HWND overlayWindow;
IDirect3D9Ex* p_Object;
IDirect3DDevice9Ex* p_Device;
D3DPRESENT_PARAMETERS p_Params;
ID3DXLine* p_Line;
ID3DXFont* pFont;
ID3DXFont* pModuleFont;
ID3DXFont* pESPFont;
RECT rc;
MARGINS margin = { -1 };
ImFont* titleFont;
ImFont* bigFont;
MSG message;
IDirect3DTexture9* ksk;
IDirect3DTexture9* unknownCtrl;
float oldDelay = 0;
View view;

// winprochandler
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WinProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, Message, wParam, lParam))
		return true;

	switch (Message)
	{
	case WM_SIZE:
		if (p_Device != NULL && wParam != SIZE_MINIMIZED)
		{
			ImGui_ImplDX9_InvalidateDeviceObjects();
			p_Params.BackBufferWidth = LOWORD(lParam);
			p_Params.BackBufferHeight = HIWORD(lParam);
			HRESULT hr = p_Device->Reset(&p_Params);
			if (hr == D3DERR_INVALIDCALL)
				IM_ASSERT(0);
			ImGui_ImplDX9_CreateDeviceObjects();
		}
		break;

	case WM_DESTROY:
		overlay::DirectXShutdown();
		PostQuitMessage(0);
		return 0;
		break;

	default:
		return DefWindowProcW(hWnd, Message, wParam, lParam);
		break;
	}
	return 0;
}

void HandleMessages(std::wstring lClassName, std::wstring lWindowName)
{
	// create the window and set window attributes
	overlayWindow = CreateWindowExW(/*WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED*/NULL, lClassName.c_str(), lWindowName.c_str(), WS_POPUP | WS_VISIBLE, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), 0, 0, 0, 0);
	SetWindowLongA(overlayWindow, GWL_EXSTYLE, (WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT));
	DwmExtendFrameIntoClientArea(overlayWindow, &margin);
	ShowWindow(overlayWindow, SW_SHOW);
	UpdateWindow(overlayWindow);

	while (true)
	{
		if (PeekMessageW(&message, overlayWindow, 0, 0, PM_REMOVE))
		{
			DispatchMessageW(&message);
			TranslateMessage(&message);
		}

		HWND hwnd_active = GetForegroundWindow();

		if (hwnd_active == globals.hWnd)
		{
			HWND hwndtest = GetWindow(hwnd_active, GW_HWNDPREV);
			SetWindowPos(overlayWindow, hwndtest, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		}

		Sleep(1);
	}
}

bool overlay::InitWindow()
{
	// create a window class
	std::string r1 = Util::RandomString(16);
	std::string r2 = Util::RandomString(16);

	std::wstring lClassName = std::wstring(r1.begin(), r1.end());
	std::wstring lWindowName = std::wstring(r2.begin(), r2.end());

	WNDCLASSEXW wClass;
	wClass.cbSize = sizeof(WNDCLASSEXW);
	wClass.style = 0;
	wClass.lpfnWndProc = WinProc;
	wClass.cbClsExtra = NULL;
	wClass.cbWndExtra = NULL;
	wClass.hInstance = nullptr;
	wClass.hIcon = LoadIcon(0, IDI_APPLICATION);
	wClass.hCursor = LoadCursor(0, IDC_ARROW);
	wClass.hbrBackground = nullptr;
	wClass.lpszMenuName = lWindowName.c_str();
	wClass.lpszClassName = lClassName.c_str();
	wClass.hIconSm = LoadIcon(0, IDI_APPLICATION);

	if (!RegisterClassExW(&wClass))
		return false;

	std::thread msgThread(HandleMessages, lClassName, lWindowName);
	msgThread.detach();

	return true;
}

bool overlay::DirectXInit()
{
	// initialize directx
	while (!overlayWindow) {}

	if (FAILED(Direct3DCreate9Ex(D3D_SDK_VERSION, &p_Object)))
		return false;

	GetClientRect(globals.hWnd, &rc);

	globals.windowWH = { rc.right - rc.left, rc.bottom - rc.top };

	ZeroMemory(&p_Params, sizeof(p_Params));
	p_Params.Windowed = TRUE;
	p_Params.SwapEffect = D3DSWAPEFFECT_DISCARD;
	p_Params.hDeviceWindow = overlayWindow;
	p_Params.MultiSampleQuality = D3DMULTISAMPLE_NONE;
	p_Params.BackBufferFormat = D3DFMT_A8R8G8B8;
	p_Params.BackBufferWidth = globals.windowWH.x;
	p_Params.BackBufferHeight = globals.windowWH.y;
	p_Params.EnableAutoDepthStencil = TRUE;
	p_Params.AutoDepthStencilFormat = D3DFMT_D16;
	p_Params.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	if (FAILED(p_Object->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, overlayWindow, D3DCREATE_HARDWARE_VERTEXPROCESSING, &p_Params, 0, &p_Device)))
		return false;

	D3DXCreateLine(p_Device, &p_Line);

	D3DXCreateFont(p_Device, 50, 0, FW_NORMAL, 1, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Helvetica", &pFont); // Create the font used for text rendering
	D3DXCreateFont(p_Device, 14, 0, FW_NORMAL, 1, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Helvetica", &pESPFont); // Create the font used for esp rendering

	if (FAILED(D3DXCreateTextureFromFileInMemory(p_Device, kskPng, sizeof(kskPng), &ksk)))
		return false;

	if (FAILED(D3DXCreateTextureFromFileInMemory(p_Device, unknownController, sizeof(unknownController), &unknownCtrl)))
		return false;

	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGuiIO& io = ImGui::GetIO();

	auto fonts = io.Fonts;//中文
	fonts->AddFontFromFileTTF(
		//"c:/windows/fonts/simhei.ttf",
		"fonts/simhei.ttf",
		13.0f,
		NULL,
		fonts->GetGlyphRangesChineseFull()
	);

	// add default font and merge fontawesome icons
	io.Fonts->AddFontFromMemoryTTF(comfortaaRegular, sizeof(comfortaaRegular), 13.f);

	// you can uncomment this if you want to load the icon font/bold font
	//static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
	//ImFontConfig icons_config;
	//icons_config.MergeMode = true;
	//icons_config.PixelSnapH = true;
	//icons_config.GlyphOffset = ImVec2(0.f, 2.f);
	//io.Fonts->AddFontFromMemoryTTF(font_awesome_data, sizeof(font_awesome_data), 16.f, &icons_config, icons_ranges); // icons font

	//// add title font and merge fontawesome icons
	//ImFontConfig titleConfig;
	//titleConfig.GlyphOffset = ImVec2(0.f, -2.f);
	//titleFont = io.Fonts->AddFontFromMemoryTTF(comfortaaBold, sizeof(comfortaaBold), 19.f, &titleConfig); // bold font

	//io.Fonts->AddFontFromMemoryTTF(font_awesome_data, sizeof(font_awesome_data), 21.f, &icons_config, icons_ranges);

	//// add big font and merge fontawesome icons
	//bigFont = io.Fonts->AddFontFromMemoryTTF(comfortaaRegular, sizeof(comfortaaRegular), 16.f);

	//icons_config.GlyphOffset = ImVec2(0.f, 1.f);
	//io.Fonts->AddFontFromMemoryTTF(font_awesome_data, sizeof(font_awesome_data), 18.f, &icons_config, icons_ranges);

	ImGui_ImplWin32_Init(overlayWindow);
	ImGui_ImplDX9_Init(p_Device);

	return true;
}

void InputHandler() {
	for (int i = 0; i < 5; i++) ImGui::GetIO().MouseDown[i] = false;
	int button = -1;
	if (GetAsyncKeyState(VK_LBUTTON)) button = 0;
	if (button != -1) ImGui::GetIO().MouseDown[button] = true;
}

void DrawImGui()
{
	// pass input to imgui
	InputHandler();

	// create an imgui frame
	
	// set the right window flags
	if (globals.menuActive)
		SetWindowLongA(overlayWindow, GWL_EXSTYLE, (WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT));
	else
		SetWindowLongA(overlayWindow, GWL_EXSTYLE, (WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT));

	if (globals.menuActive)//主菜单
	{
		view.drawMenu();

	}
	/*原菜单originMenu
	if (globals.menuActive)//主菜单
	{
		// imgui stuff

		ImGui::SetNextWindowSize(ImVec2(600, 700));
		ImGui::Begin(xorstr_("menu"), (bool*)0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse);

		ImGui::SliderFloat(xorstr_("Rainbow Speed"), &globals.rainbowSpeed, 0.0005f, 0.5f);

		ImGui::Spacing();

		ImGui::Checkbox(xorstr_("Aimbot"), &globals.aimbot);
		ImGui::Checkbox(xorstr_("Aimbot Visibility Check"), &globals.aimbotVisibleCheck);
		ImGui::SliderFloat(xorstr_("Aimbot RCS Intensity"), &globals.aimRcsIntensity, 0.f, 100.f, "%.1f%%");
		ImGui::SliderFloat(xorstr_("Max Aimbot Distance"), &globals.maxAimbotDistance, 0.f, 1000.f);
		ImGui::SliderFloat(xorstr_("Aimbot FOV"), &globals.aimbotFOV, 0.1f, 2000.f, "%.1f");
		ImGui::Checkbox(xorstr_("Draw Aimbot FOV"), &globals.drawAimbotFOV);
		ImGui::ColorEdit4(xorstr_("FOV Circle Color"), (float*)&globals.fovCircleColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_AlphaBar);
		ImGui::Checkbox(xorstr_("Rainbow FOV Circle"), &globals.rainbowFOV);

		ImGui::Spacing();

		ImGui::Checkbox(xorstr_("Recoil Control"), &globals.rcs);
		ImGui::SliderFloat(xorstr_("RCS Intensity"), &globals.rcsIntensity, 0.f, 100.f, "%.1f%%");

		ImGui::Spacing();

		ImGui::Checkbox(xorstr_("ESP"), &globals.esp);
		ImGui::ColorEdit4(xorstr_("Head Circle Color | Visible"), (float*)&globals.headCircleColorVisible, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_AlphaBar);
		ImGui::Checkbox(xorstr_("Rainbow"), &globals.headCircleColorVisibleRainbow);
		ImGui::ColorEdit4(xorstr_("Head Circle Color | Invisible"), (float*)&globals.headCircleColorInvisible, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_AlphaBar);
		ImGui::Checkbox(xorstr_("Rainbow"), &globals.headCircleColorInvisibleRainbow);
		ImGui::SliderInt(xorstr_("ESP Thickness"), &globals.espThickness, 1, 5, "%dpx");
		ImGui::ColorEdit4(xorstr_("ESP Color | Visible"), (float*)&globals.espColorVisible, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_AlphaBar);
		ImGui::Checkbox(xorstr_("Rainbow"), &globals.espColorVisibleRainbow);
		ImGui::ColorEdit4(xorstr_("ESP Color | Invisible"), (float*)&globals.espColorInvisible, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_AlphaBar);
		ImGui::Checkbox(xorstr_("Rainbow"), &globals.espColorInvisibleRainbow);
		ImGui::Checkbox(xorstr_("Fill Boxes"), &globals.fillBox);
		ImGui::ColorEdit4(xorstr_("ESP Fill Color | Visible"), (float*)&globals.espFillColorVisible, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_AlphaBar);
		ImGui::ColorEdit4(xorstr_("ESP Fill Color | Invisible"), (float*)&globals.espFillColorInvisible, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_AlphaBar);
		ImGui::ColorEdit4(xorstr_("Head Line"), (float*)&globals.headLineColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_AlphaBar);
		ImGui::Checkbox(xorstr_("Rainbow"), &globals.headLineRainbow);
		ImGui::SliderInt(xorstr_("Head Line Thickness"), &globals.headLineThickness, 1, 5, "%dpx");
		
		ImGui::Spacing();

		ImGui::ColorEdit4(xorstr_("Text Color | Visible"), (float*)&globals.textColorVisible, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_AlphaBar);
		ImGui::Checkbox(xorstr_("Rainbow"), &globals.textColorVisibleRainbow);
		ImGui::ColorEdit4(xorstr_("Text Color | Invisible"), (float*)&globals.textColorInvisible, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_AlphaBar);
		ImGui::Checkbox(xorstr_("Rainbow"), &globals.textColorInvisibleRainbow);
		ImGui::SliderFloat(xorstr_("Max Box Distance"), &globals.maxBoxDistance, 0.f, 1000.f, "%.1fm");
		ImGui::SliderFloat(xorstr_("Max Text Distance"), &globals.maxTextDistance, 0.f, 1000.f, "%.1fm");
		ImGui::SliderFloat(xorstr_("Max Head Circle Distance"), &globals.maxHeadCircleDistance, 0.f, 1000.f, "%.1fm");
		ImGui::SliderFloat(xorstr_("Max Healthbars Circle Distance"), &globals.maxHealthDistance, 0.f, 1000.f, "%.1fm");
		ImGui::SliderFloat(xorstr_("Max Head Line Distance"), &globals.maxHeadLineDistance, 0.f, 1000.f, "%.1fm");
		ImGui::SliderFloat(xorstr_("KSK ESP Max Distance"), &globals.maxKskDistance, 0.f, 1000.f, "%.1fm");

		ImGui::End();
	}
	*/
}

/*auto Rainbow(float delay)
{
	static uint32_t cnt = 0;
	float freq = delay;

	if (++cnt >= (uint32_t)-1)
	{
		cnt = 0;
	}
	if (delay != oldDelay)
	{
		cnt = 0;
		oldDelay = delay;
	}

	return std::make_tuple(std::sin(freq * cnt + 0) * 2.f, std::sin(freq * cnt + 2) * 2.3f, std::sin(freq * cnt + 4) * 2.6f);
}//原作者可能是个gay
*/

void overlay::Render()//透视
{
	static ImDrawList* drawList;

	while (!GetAsyncKeyState(VK_END))
	{
		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		DrawImGui();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
		ImGui::Begin(xorstr_("##scene"), nullptr, ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoTitleBar);
		ImGui::SetWindowPos(ImVec2(0, 0), ImGuiCond_Always);
		ImGui::SetWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y), ImGuiCond_Always);

		drawList = ImGui::GetWindowDrawList();

		p_Device->Clear(0, 0, D3DCLEAR_TARGET, 0, 1.f, 0);
		p_Device->BeginScene();

		//globals.currentRainbowColor = { std::get<0>(Rainbow(globals.rainbowSpeed)), std::get<1>(Rainbow(globals.rainbowSpeed)), std::get<2>(Rainbow(globals.rainbowSpeed)), 1.f };

		// render here
		if (Player::IsPlayer(globals.localPlayer))
		{
			// get view matrix every frame
			globals.viewMatrix = Driver.rpm<viewMatrix_t>(Driver.rpm<uintptr_t>(globals.viewRenderer + OFFSET_MATRIX));

			int index = 0; // we need the index for the visibility check
			// loop through the entity list
			for (uintptr_t& player : Player::GetPlayers())
			{
				int pic = Player::GetPic(index);
				bool visible = Player::IsVisible(player, index);
				index++;

				if (!Player::IsValidEnemy(player)) continue; // check if its a valid enemy

				vec3 targetHead = Util::GetBonePos(player, 8); // get head position
				vec2 targetHeadScreen;
				if (!Util::WorldToScreen(targetHead, targetHeadScreen)) continue; // convert to screen coordinates

				vec3 targetBody = Driver.rpm<vec3>(player + OFFSET_ORIGIN); // get vecOrigin
				vec2 targetBodyScreen;
				if (!Util::WorldToScreen(targetBody, targetBodyScreen)) continue; // convert to screen coordinates
				
				vec3 targetNeck = Util::GetBonePos(player, 7); // get neck bone
				vec2 targetNeckScreen;
				if (!Util::WorldToScreen(targetNeck, targetNeckScreen)) continue; // convert to screen coordinates

				if (globals.esp)
				{
					// calculate the stuff needed for the boxes
					float height = abs(abs(targetHeadScreen.y) - abs(targetBodyScreen.y));
					float width = height / 2.f;
					float middle = targetBodyScreen.x - (width / 2.f);
					float dist = Driver.rpm<vec3>(globals.localPlayer + OFFSET_ORIGIN).DistTo(targetHead);
					float distM = Util::ToMeters(dist);
					float neckHeadDistance = targetHeadScreen.Dist2D(targetNeckScreen);

					if (distM <= globals.maxBoxDistance)
					{
						D3DCOLOR espColor;
						D3DCOLOR espFillColor;

						// get the appropriate colors
						if (visible)
						{
							espColor = globals.espColorVisibleRainbow ? ImGui::ColorConvertFloat4ToU32(globals.currentRainbowColor) : Util::Vec4toARGB(globals.espColorVisible);
							espFillColor = Util::Vec4toARGB(globals.espFillColorVisible);
						}
						else
						{
							espColor = globals.espColorInvisibleRainbow ? ImGui::ColorConvertFloat4ToU32(globals.currentRainbowColor) : Util::Vec4toARGB(globals.espColorInvisible);
							espFillColor = Util::Vec4toARGB(globals.espFillColorInvisible);
						}

						// call the function to draw the box
						view.DrawBoxESP(middle, targetHeadScreen.y, width, height, globals.espThickness, espFillColor, espColor);
					}

					if (distM <= globals.maxHealthDistance)
					{
						// call the function to draw the healthbars
						view.DrawHealthbars(middle, targetHeadScreen.y, width, height, 4, Driver.rpm<int>(player + OFFSET_HEALTH), Driver.rpm<int>(player + OFFSET_SHIELD));
					}

					if (distM <= globals.maxTextDistance)
					{
						D3DCOLOR textColor;

						// get the appropriate colors
						if (visible)
							textColor = globals.textColorVisibleRainbow ? ImGui::ColorConvertFloat4ToU32(globals.currentRainbowColor) : Util::Vec4toARGB(globals.textColorVisible);
						else
							textColor = globals.textColorInvisibleRainbow ? ImGui::ColorConvertFloat4ToU32(globals.currentRainbowColor) : Util::Vec4toARGB(globals.textColorInvisible);

						// draw text
						DrawShadowString(Player::GetName(player).c_str(), middle, targetHeadScreen.y - 15, 1, textColor, ARGB(255, 0, 0, 0), pESPFont);
						std::string distance = std::to_string(distM);
						distance = distance.substr(0, distance.find('.')) + "m";
						DrawShadowString(distance.c_str(), middle, targetBodyScreen.y + 1, 1, textColor, ARGB(255, 0, 0, 0), pESPFont);
					}

					if (distM <= globals.maxHeadCircleDistance)
					{
						D3DCOLOR circleColor;

						// get the appropriate colors
						if (visible)
							circleColor = globals.headCircleColorVisibleRainbow ? ImGui::ColorConvertFloat4ToU32(globals.currentRainbowColor) : Util::Vec4toARGB(globals.headCircleColorVisible);
						else
							circleColor = globals.headCircleColorInvisibleRainbow ? ImGui::ColorConvertFloat4ToU32(globals.currentRainbowColor) : Util::Vec4toARGB(globals.headCircleColorInvisible);

						// draw the head circle
						DrawCircle(targetHeadScreen.x, targetHeadScreen.y, neckHeadDistance, 1.f, 20.f, circleColor); /* we need the distance from head to neck
																																				so we can size it depending on distance*/
					}

					/*if (distM <= globals.maxHeadLineDistance)
					{
						// get the color
						D3DCOLOR lineColor = globals.headLineRainbow ? ImGui::ColorConvertFloat4ToU32(globals.currentRainbowColor) : Util::Vec4toARGB(globals.headLineColor);

						vec3 targetAngles = Driver.rpm<vec3>(player + OFFSET_VIEWANGLES); // get enemy angles
						vec3 targetPos = targetHead.TransformVec(targetAngles, 30.f); // eye position + 30 in direction of the eyes
						vec2 targetPosScreen;
						if (Util::WorldToScreen(targetPos, targetPosScreen))
							DrawLine(targetHeadScreen.x, targetHeadScreen.y, targetPosScreen.x, targetPosScreen.y, globals.headLineThickness, lineColor);
					}*/

					if (distM <= globals.maxKskDistance)
					{
						switch (pic)
						{
						case 1:
							drawList->AddImage(ksk, ImVec2(targetHeadScreen.x - neckHeadDistance, targetHeadScreen.y - neckHeadDistance), ImVec2(targetHeadScreen.x + neckHeadDistance, targetHeadScreen.y + neckHeadDistance));
							break;

						case 2:
							drawList->AddImage(unknownCtrl, ImVec2(targetHeadScreen.x - neckHeadDistance, targetHeadScreen.y - neckHeadDistance), ImVec2(targetHeadScreen.x + neckHeadDistance, targetHeadScreen.y + neckHeadDistance));
							break;

						default:
							break;
						}
					}
				}
			}

		}

		if (globals.drawAimbotFOV)//显示fov范围
		{
			// draw fov circle
			DrawCircle(globals.windowWH.x / 2, globals.windowWH.y / 2, globals.aimbotFOV, 1.f, 50.f, globals.rainbowFOV ? ImGui::ColorConvertFloat4ToU32(globals.currentRainbowColor) : Util::Vec4toARGB(globals.fovCircleColor));
		}

		DrawString((std::string(xorstr_("自瞄:\t")) + (globals.aimbot ? xorstr_("ON") : xorstr_("OFF"))).c_str(), 5, 5, globals.aimbot ? ARGB(255, 0, 255, 0) : ARGB(255, 255, 0, 0), pESPFont);
		DrawString((std::string(xorstr_("无后座力:\t")) + (globals.rcs ? xorstr_("ON") : xorstr_("OFF"))).c_str(), 5, 19, globals.rcs ? ARGB(255, 0, 255, 0) : ARGB(255, 255, 0, 0), pESPFont);
		DrawString((std::string(xorstr_("透视:\t")) + (globals.esp ? xorstr_("ON") : xorstr_("OFF"))).c_str(), 5, 33, globals.esp ? ARGB(255, 0, 255, 0) : ARGB(255, 255, 0, 0), pESPFont);
		DrawString((std::string(xorstr_("启用预判:\t")) + ((globals.forceAim == true) ? xorstr_("强行") : xorstr_("预判"))).c_str(), 5, 47, (globals.forceAim == true) ? ARGB(255, 0, 255, 0) : ARGB(255, 255, 0, 0), pESPFont);
		DrawString((std::string(xorstr_("自瞄模式:\t")) + (globals.smoothMode ? xorstr_("平滑") : xorstr_("暴力"))).c_str(), 5, 61, globals.smoothMode ? ARGB(255, 0, 255, 0) : ARGB(255, 255, 0, 0), pESPFont);
		DrawString((std::string(xorstr_("自瞄判断延迟:\t")) + (std::to_string(globals.delayTime))).c_str(), 5, 75, ARGB(255, 0, 255, 0), pESPFont);
		DrawString((std::string(xorstr_("平滑压枪强度:\t")) + (std::to_string(globals.smoothAimIntensity))).c_str(), 5, 89, ARGB(255, 0, 255, 0), pESPFont);

		
		//DrawString((std::string(xorstr_("Target:\t")) + ((globals.currentAimTarget != NULL) ? xorstr_("YES") : xorstr_("NO"))).c_str(), 5, 47, ARGB(255, 0, 255, 0), pESPFont);
		

		drawList->PushClipRectFullScreen();
		ImGui::End();
		ImGui::PopStyleColor();
		ImGui::PopStyleVar(2);

		ImGui::EndFrame();
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

		p_Device->EndScene();
		p_Device->PresentEx(0, 0, 0, 0, 0);
	}
}

void overlay::DirectXShutdown()
{
	// destroy everything
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();

	p_Object->Release();
	p_Device->Release();
	pFont->Release();
	pESPFont->Release();
	p_Line->Release();

	DestroyWindow(overlayWindow);
}
/*
void overlay::UpdateWindowPos()
{
	RECT oldRC = rc;
	while (!GetAsyncKeyState(VK_END))
	{
		// update the window position every 100 ms
		globals.hWnd = FindWindowA(NULL, xorstr_("Apex Legends"));

		if (globals.hWnd)
		{
			GetWindowRect(globals.hWnd, &rc);
			globals.windowWH = { rc.right - rc.left, rc.bottom - rc.top };

			DWORD dwStyle = GetWindowLong(globals.hWnd, GWL_STYLE);
			if (dwStyle & WS_BORDER)
			{
				rc.top += 32;
				globals.windowWH.y -= 39;
			}

			MoveWindow(overlayWindow, rc.left, rc.top, globals.windowWH.x, globals.windowWH.y, true);
		}

		Sleep(100);
	}
}*/