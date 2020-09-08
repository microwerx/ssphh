#include "pch.hpp"
#include <hatchetfish.hpp>
#include <renderer_window.hpp>
#include <ssphhapp.hpp>

RendererWindow::RendererWindow(const std::string& name)
	: Vf::Window(name) {}

RendererWindow::~RendererWindow() {}

void RendererWindow::OnUpdate(double timeStamp) {
	if (!ssphh_widget_ptr) {
		context = nullptr;
		return;
	}

	Vf::Window::OnUpdate(timeStamp);

	if (context != &ssphh_widget_ptr->rendererContext)
		context = &ssphh_widget_ptr->rendererContext;
}

void RendererWindow::OnRenderDearImGui() {
	if (!context || !beginWindow()) return;
	Vf::Window::OnRenderDearImGui();

	if (ImGui::Button("Reset")) {
		context->reset();
		endWindow();
		return;
	}
	ImGui::SameLine();
	if (ImGui::Button("Defaults")) context->set_default_parameters();

	SSPHH::SSPHH_Application::InterfaceInfo& Interface = ssphh_widget_ptr->Interface;

	ImGui::Value("Render Mode", ssphh_widget_ptr->renderMode);	
	if (ImGui::TreeNode("Render Time(s)")) {
		ImGui::BulletText("UPDATE:    %3.2f ms", ssphh_widget_ptr->my_hud_info.onUpdateTime);
		ImGui::BulletText("PBSKY:     %3.2f ms", ssphh_widget_ptr->my_hud_info.pbskyTime);
		ImGui::BulletText("GBUFFER:   %3.2f ms", ssphh_widget_ptr->my_hud_info.gbufferPassTime);
		ImGui::BulletText("DEFFERRED: %3.2f ms", ssphh_widget_ptr->my_hud_info.deferredPassTime);
		ImGui::BulletText("RENDER:    %3.2f ms", ssphh_widget_ptr->my_hud_info.totalRenderTime);
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Shadow Time(s)")) {
		if (ImGui::TreeNode("Rect Shadow Time(s)")) {
			for (int i = 0; i < Fluxions::MaxLights; i++) {
				ImGui::BulletText("%d %3.2f ms", i, ssphh_widget_ptr->my_hud_info.rectShadowTimes[i]);
			}
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Cube Shadow Time(s)")) {
			for (int i = 0; i < Fluxions::MaxLights; i++) {
				ImGui::BulletText("%d %3.2f ms", i, ssphh_widget_ptr->my_hud_info.cubeShadowTimes[i]);
			}
			ImGui::TreePop();
		}
		ImGui::TreePop();
	}

	ImGui::Separator();

	if (ImGui::Button("Load Configs")) HFCLOCKSf(lastConfigsLoadTime, context->loadConfig(SSPHH::default_renderconfig_path))
		ImGui::Value("configs load", lastConfigsLoadTime);

	if (ImGui::Button("Load Shaders")) HFCLOCKSf(lastShadersLoadTime, context->loadShaders());
	ImGui::Value("shaders load", lastShadersLoadTime);

	if (ImGui::Button("Load Textures")) HFCLOCKSf(lastTextureLoadTime, context->loadTextures());
	ImGui::Value("texture load", lastTextureLoadTime);

	static float lastFramebufferTime{ 0 };
	if (ImGui::Button("Make Framebuffers")) HFCLOCKSf(lastFramebufferTime, context->makeFramebuffers());
	ImGui::Value("framebuffer time", lastFramebufferTime);

	ImGui::Separator();

	ImGui::TextColored(Colors::Azure, "Passes");
	ImGui::Checkbox("RECT SHADOWS", &ssphh_widget_ptr->Interface.drawRectShadows);
	ImGui::Checkbox("CUBE SHADOWS", &ssphh_widget_ptr->Interface.drawCubeShadows);
	ImGui::Checkbox("SKY", &ssphh_widget_ptr->Interface.drawSkyBox);
	ImGui::Checkbox("PBR", &ssphh_widget_ptr->Interface.drawPBR);
	if (ssphh_widget_ptr->Interface.drawPBR)
	{
		ImGui::SliderInt("Shader Debug Choice", &Interface.tools.shaderDebugChoice, 0, 17);
		ImGui::SliderInt("Shader Debug Dirto", &Interface.tools.shaderDebugLight, -1, MaxLights);
		ImGui::SliderInt("Shader Debug SPHL", &Interface.tools.shaderDebugSphl, -1, MaxSphlLights - 1);
	}
	ImGui::Checkbox("VIZ", &ssphh_widget_ptr->Interface.drawVIZ);
	ImGui::Checkbox("POST", &ssphh_widget_ptr->Interface.drawPOST);

	ImGui::Separator();

	if (ImGui::TreeNode("programs")) {
		for (auto& [k, ro] : context->programs) {
			ImGui::Text("%s [%s, %s]", ro.name(), ro.status(), (ro.usable() ? "usable" : "not usable"));
		}
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("renderers")) {
		for (auto& [k, ro] : context->renderers) {
			ImGui::Text("%s [%s, %s]", ro.name(), ro.status(), (ro.usable() ? "usable" : "not usable"));
		}
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("rendererconfigs")) {
		for (auto& [k, ro] : context->rendererConfigs) {
			ImGui::Text("%s [%s, %s]", ro.name(), ro.status(), (ro.usable() ? "usable" : "not usable"));
		}
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("fbos")) {
		for (auto& [k, ro] : context->fbos) {
			ImGui::Text("%s [%s, %s]", ro.name(), ro.status(), (ro.usable() ? "usable" : "not usable"));
		}
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("texture2Ds")) {
		for (auto& [k, ro] : context->texture2Ds) {
			ImGui::Text("%s [%s, %s]", ro.name(), ro.status(), (ro.usable() ? "usable" : "not usable"));
		}
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("textureCubes")) {
		for (auto& [k, ro] : context->textureCubes) {
			ImGui::Text("%s [%s, %s]", ro.name(), ro.status(), (ro.usable() ? "usable" : "not usable"));
		}
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("samplers")) {
		for (auto& [k, ro] : context->samplers) {
			ImGui::Text("%s [%s, %s]", ro.name(), ro.status(), (ro.usable() ? "usable" : "not usable"));
		}
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("vars")) {
		for (auto& [k, v] : context->vars.variables) {
			if (v.IsInteger()) ImGui::Value(k.c_str(), v.ival);
			if (v.IsDouble()) ImGui::Value(k.c_str(), (float)v.dval);
			if (v.IsStringOrIdentifier()) ImGui::Value(k.c_str(), v.sval.c_str());
		}
		ImGui::TreePop();
	}

	static int hflogCurrentItem{ 0 };
	int hflogSize = HFLOG_HISTORYSIZE();
	if (hflogSize) ImGui::ListBox("hflog", &hflogCurrentItem, Hf::Log.getHistoryItems(), hflogSize);

	endWindow();
}
