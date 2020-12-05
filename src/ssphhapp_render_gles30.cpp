#include "pch.hpp"
#include <fluxions_renderer_utilities.hpp>
#include <ssphhapp.hpp>

namespace SSPHH {
	void SSPHH_Application::SetupRenderGLES30() {
		FxSetErrorMessage(__FILE__, __LINE__, "%s", __FUNCTION__);
		SSG_OptimizeClippingPlanes();
		UpdateSPHLs();
		UploadSPHLs();
	}


	void SSPHH_Application::RenderGLES30() {
		FxSetErrorMessage(__FILE__, __LINE__, "%s", __FUNCTION__);

		// Implementation Plan
		// -------------------
		// Render Sky Box into FBO
		// Render PBR into FBO
		// Render VIZ into FBO
		// Render POST with FBO to screen

		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, 0);
		//glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

		if (Interface.drawRectShadows) {
			RenderGLES30_RectShadows();
		}

		if (Interface.drawCubeShadows) {
			RenderGLES30_CubeShadows();
		}

		if (Interface.drawSkyBox) {
			RenderGLES30_SkyBox();
		}

		if (Interface.drawPBR) {
			RenderGLES30_SceneGraph();
		}

		if (Interface.drawVIZ) {
			RenderGLES30_VIZ();
		}

		if (Interface.drawPOST) {
			RenderGLES30_PostProcess();
		}

		auroraViz.RenderFrame(GetElapsedTime());

		FxSetDefaultErrorMessage();
	}


	void SSPHH_Application::RenderGLES30_SkyBox() {
		FxSetErrorMessage(__FILE__, __LINE__, "%s", __FUNCTION__);

		const std::string renderername{ "skybox" };
		if (rendererContext.renderers.count(renderername)) {
			const std::string& renderconfigname = rendererContext.renderers[renderername].renderconfigname;
			Fluxions::RenderImage(rendererContext, ssg, renderername, renderconfigname);
		}

		if (FxCheckLogErrors()) {
			Interface.drawSkyBox = false;
			HFLOGERROR("Disabling SKY stage due to OpenGL Error-- Please rectify!");
		}
	}


	void SSPHH_Application::RenderGLES30_SceneGraph() {
		FxSetErrorMessage(__FILE__, __LINE__, "%s", __FUNCTION__);

		const std::string renderername{ "pbr" };
		if (rendererContext.renderers.count(renderername)) {
			const std::string& renderconfigname = rendererContext.renderers[renderername].renderconfigname;
			auto& rc = rendererContext.rendererConfigs[renderconfigname];
			rc.shaderDebugChoice = Interface.tools.shaderDebugChoice;
			rc.shaderDebugLight = Interface.tools.shaderDebugLight;
			rc.shaderDebugSphl = Interface.tools.shaderDebugSphl;
			Fluxions::RenderImage(rendererContext, ssg, renderername, renderconfigname);
		}

		if (FxCheckLogErrors()) {
			Interface.drawPBR = false;
			HFLOGERROR("Disabling PBR stage due to OpenGL Error-- Please rectify!");
		}
	}


	void SSPHH_Application::RenderGLES30_VIZ() {
		FxSetErrorMessage(__FILE__, __LINE__, "%s", __FUNCTION__);

		static double viz_lag{ 0 };
		static constexpr double MS_PER_VIZ_UPDATE = 1.0 / 30.0;
		viz_lag += GetFrameTime();
		bool updateViz{ false };
		while (viz_lag >= MS_PER_VIZ_UPDATE) {
			viz_lag -= MS_PER_VIZ_UPDATE;
			updateViz = true;
		}

		const std::string renderername{ "viz" };
		if (rendererContext.renderers.count(renderername)) {
			const std::string& renderconfigname = rendererContext.renderers[renderername].renderconfigname;
			if (updateViz) {
				rendererContext.renderers[renderername].update(RendererGLES30::UPDATE_VIZ);
			}
			Fluxions::RenderImage(rendererContext, ssg, renderername, renderconfigname);
		}

		if (FxCheckLogErrors()) {
			Interface.drawVIZ = false;
			HFLOGERROR("Disabling VIZ stage due to OpenGL Error-- Please rectify!");
		}
	}


	void SSPHH_Application::RenderGLES30_PostProcess() {
		FxSetErrorMessage(__FILE__, __LINE__, "%s", __FUNCTION__);

		const std::string renderername{ "postprocess" };
		if (rendererContext.renderers.count(renderername)) {
			const std::string& renderconfigname = rendererContext.renderers[renderername].renderconfigname;
			Fluxions::RenderImage(rendererContext, ssg, renderername, renderconfigname);
		}

		if (FxCheckLogErrors()) {
			Interface.drawPOST = false;
			HFLOGERROR("Disabling POST stage due to OpenGL Error-- Please rectify!");
		}
	}


	void SSPHH_Application::RenderGLES30Scene() {
		FxSetErrorMessage(__FILE__, __LINE__, "%s", __FUNCTION__);

		while (glGetError());
		const std::string renderername{ "skybox" };
		if (rendererContext.renderers.count(renderername)) {
			const std::string& renderconfigname = rendererContext.renderers[renderername].renderconfigname;
			Fluxions::RenderImage(rendererContext, ssg, renderername, renderconfigname);
		}
		while (glGetError()) HFLOGWARN("AdvancedRender() ERROR!");

		if (Interface.drawSkyBox) {
			FxSetErrorMessage(__FILE__, __LINE__, "skybox");
			glEnable(GL_DEPTH_TEST);
			Sky_Render();
			glDisable(GL_DEPTH_TEST);
			FxSetErrorMessage("ssphh.cpp", __LINE__, __FUNCTION__);
			while (glGetError()) HFLOGWARN("Draw SkyBox ERROR!");
		}

		glUseProgram(0);
		glDisable(GL_DEPTH_TEST);
	}
}
