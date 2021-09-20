#include "View.h"
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
void View::drawMenu()
{
	// imgui stuff

	ImGui::SetNextWindowSize(ImVec2(600, 700));
	ImGui::Begin(xorstr_("menu"), (bool*)0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse);


	ImGui::Spacing();

	ImGui::Checkbox(xorstr_("Aimbot"), &globals.aimbot);
	ImGui::Checkbox(xorstr_("Aimbot Visibility Check"), &globals.aimbotVisibleCheck);
	ImGui::SliderFloat(xorstr_(u8"自瞄强度"), &globals.aimRcsIntensity, 0.f, 100.f, "%.1f%%");
	ImGui::SliderInt(xorstr_(u8"压枪延迟"), &globals.delayTime, 1, 200, "%dms");
	ImGui::Checkbox(xorstr_(u8"启用预判"), &globals.forceAim);
	ImGui::Checkbox(xorstr_(u8"平滑模式"), &globals.smoothMode);
	ImGui::SliderFloat(xorstr_(u8"平滑自瞄强度"), &globals.smoothAimIntensity, 0.f, 100.f, "%.1f%%");
	ImGui::SliderFloat(xorstr_(u8"最大瞄准距离"), &globals.maxAimbotDistance, 0.f, 1000.f);
	ImGui::SliderFloat(xorstr_("Aimbot FOV"), &globals.aimbotFOV, 0.1f, 2000.f, "%.1f");
	ImGui::Checkbox(xorstr_("Draw Aimbot FOV"), &globals.drawAimbotFOV);

	ImGui::Spacing();

	ImGui::Checkbox(xorstr_(u8"后坐力控制Recoil Control"), &globals.rcs);
	ImGui::SliderFloat(xorstr_("RCS Intensity"), &globals.rcsIntensity, 0.f, 100.f, "%.1f%%");

	ImGui::Spacing();

	ImGui::Checkbox(xorstr_("ESP"), &globals.esp);
	ImGui::ColorEdit4(xorstr_("Head Circle Color | Visible"), (float*)&globals.headCircleColorVisible, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_AlphaBar);
	ImGui::ColorEdit4(xorstr_("Head Circle Color | Invisible"), (float*)&globals.headCircleColorInvisible, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_AlphaBar);
	ImGui::SliderInt(xorstr_("ESP Thickness"), &globals.espThickness, 1, 5, "%dpx");
	ImGui::ColorEdit4(xorstr_("ESP Color | Visible"), (float*)&globals.espColorVisible, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_AlphaBar);
	ImGui::ColorEdit4(xorstr_("ESP Color | Invisible"), (float*)&globals.espColorInvisible, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_AlphaBar);
	ImGui::Checkbox(xorstr_("Fill Boxes"), &globals.fillBox);
	ImGui::ColorEdit4(xorstr_("ESP Fill Color | Visible"), (float*)&globals.espFillColorVisible, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_AlphaBar);
	ImGui::ColorEdit4(xorstr_("ESP Fill Color | Invisible"), (float*)&globals.espFillColorInvisible, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_AlphaBar);
	ImGui::ColorEdit4(xorstr_("Head Line"), (float*)&globals.headLineColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_AlphaBar);
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

void View::drawMonitor()
{

}


void View::DrawBoxESP(float x, float y, float width, float height, int thickness, D3DCOLOR fillColor, D3DCOLOR boxColor)
{
	// self explaining
	if (globals.fillBox)
		DrawFilledRectangle(x, y, width, height, fillColor);
	DrawBorderBox(x, y, width, height, thickness, boxColor);
}

void View::DrawHealthbars(float x, float y, float width, float height, int thickness, int health, int shieldHealth)
{
	// shitty calculation but it works
	DrawFilledRectangle(x - (thickness + 5), y, thickness, height, ARGB(255, 0, 255, 0));
	DrawFilledRectangle(x - (thickness + 5), y, thickness, height - ((height / 100) * health), ARGB(120, 255, 0, 0));
	DrawFilledRectangle(x - (thickness * 2 + 12), y, thickness, height, ARGB(255, 155, 0, 252));
	DrawFilledRectangle(x - (thickness * 2 + 12), y, thickness, height - ((height / 125) * shieldHealth), ARGB(0, 0, 0, 0));

	if (shieldHealth > 0)
		DrawBorderBox(x - (thickness * 2 + 12) - 1, y - 1, thickness + 1, height + 1, 1, ARGB(255, 0, 0, 0));
	DrawBorderBox(x - (thickness + 5) - 1, y - 1, thickness + 1, height + 1, 1, ARGB(255, 0, 0, 0));
}