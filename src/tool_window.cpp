#include "pch.hpp"
#include <ssphhapp.hpp>
#include <tool_window.hpp>

ToolWindow::ToolWindow(const std::string& name)
	: Vf::Window(name) {}

ToolWindow::~ToolWindow() {}

void ToolWindow::OnUpdate(double timestamp) {
	if (!ssphh_widget_ptr) {
		ssg = nullptr;
		return;
	}
	else if (ssg != &ssphh_widget_ptr->ssg) {
		ssg = &ssphh_widget_ptr->ssg;
	}
	if (!ssg) return;
	Vf::Window::OnUpdate(timestamp);
}

void ToolWindow::OnRenderDearImGui() {
	if (!ssg) return;
	if (!beginWindow()) return;
	Vf::Window::OnRenderDearImGui();

	ImGui::TextColored(Colors::Cyan, "POST");
	ImGui::SliderFloat("Simple Exposure", &ssg->environment.base.toneMap.x, -12.0f, 12.0f);
	ImGui::SliderFloat("Gamma Compression", &ssg->environment.base.toneMap.y, 0.0f, 2.2f);
	ImGui::SliderFloat("Filmic Highlights", &ssg->environment.base.toneMap.z, 0.0f, 1.0f);
	ImGui::SliderFloat("Filmic Shadows", &ssg->environment.base.toneMap.w, 0.0f, 1.0f);
	ImGui::SliderFloat("Fade Image", &ssg->environment.base.CompareBlend.r, 0.0f, 1.0f);
	ImGui::SliderFloat("Fade Color", &ssg->environment.base.CompareBlend.g, 0.0f, 1.0f);
	ImGui::SliderFloat("Fade Reference", &ssg->environment.base.CompareBlend.b, 0.0f, 1.0f);
	ImGui::SliderFloat("Compare Reference", &ssg->environment.base.CompareBlend.a, 0.0f, 1.0f);
	ImGui::ColorEdit3("Fade Color", ssg->environment.base.FadeColor.ptr());
	ImGui::SliderFloat("Fade Dissolve", &ssg->environment.base.FadeColor.a, 0.0f, 1.0f);
	ImGui::SliderFloat("Ref Exposure", &ssg->environment.base.ReferenceOps.r, -12.0f, 12.0f);
	ImGui::SliderFloat("Ref Gamma", &ssg->environment.base.ReferenceOps.g, 0.0f, 2.2f);
	ImGui::SliderFloat("Ref F.Highlights", &ssg->environment.base.ReferenceOps.b, 0.0f, 1.0f);
	ImGui::SliderFloat("Ref F.Shadows", &ssg->environment.base.ReferenceOps.a, 0.0f, 1.0f);
	ImGui::SliderFloat4("Flips", ssg->environment.base.Flips.ptr(), 0.0f, 1.0f);
	endWindow();
}
