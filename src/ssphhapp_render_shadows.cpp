#include "pch.hpp"
#include <ssphhapp.hpp>
#include <fluxions_renderer_utilities.hpp>

namespace SSPHH {
	namespace {
		RendererConfig* rectShadowRC{ nullptr };
		RendererConfig* cubeShadowRC{ nullptr };
		RendererProgram* cubeShadowProgram{ nullptr };
		Fluxions::RendererGLES30* rectShadowRenderer{ nullptr };
		Fluxions::RendererGLES30* cubeShadowRenderer{ nullptr };
		Fluxions::RendererConfig* pbrRC{ nullptr };

		Fluxions::RendererFramebuffer* cubeShadowFbos[Fluxions::MaxLights] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

		std::string rectShadowFboName{ "rectShadow_fbo" };
		Fluxions::RendererFramebuffer* rectShadowFbo{ nullptr };
	}

	void SSPHH_Application::RenderGLES30_RectShadows() {
		if (!Interface.drawRectShadows) return;

		constexpr int shadow_debugging = 0;

		if (!pbrRC) {
			pbrRC = rendererContext.getRendererConfig("pbr_rc");
		}

		if (!rectShadowFbo) {
			rectShadowFbo = rendererContext.getFramebuffer("rectShadow_fbo");
		}

		if (!rectShadowRC) {
			rectShadowRC = rendererContext.getRendererConfig("rectShadow_rc");
			if (rectShadowRC && rectShadowFbo) {
				// one time setup
				rectShadowRC->isCubeMap = false;
				rectShadowRC->renderSkyBox = false;
				rectShadowRC->renderSceneGraph = true;
				rectShadowRC->useSceneCamera = false;
				rectShadowRC->useSceneProjection = false;
				rectShadowRC->clearColor.reset(1.0f, 0.0f, 1.0f, 1.0f);
				rectShadowRC->clearColorBuffer = (shadow_debugging != 0);
				rectShadowRC->clearDepthBuffer = true;
				rectShadowRC->viewportRect.w = rectShadowFbo->width();
				rectShadowRC->viewportRect.h = rectShadowFbo->height();
				rectShadowRC->writeFBO = rectShadowFbo;
			}
		}

		if (!rectShadowRenderer) {
			rectShadowRenderer = rendererContext.getRenderer("rectShadow_renderer");
			if (rectShadowRenderer) {
				// one time setup
				rectShadowRenderer->setSceneGraph(&ssg);
				rectShadowRenderer->setRenderConfig(rectShadowRC);
				if (!rectShadowRenderer->validate()) {
					rectShadowRenderer = nullptr;
				}
			}
		}

		FxSetErrorMessage(__FILE__, __LINE__, "%s Rect Shadows", __func__);

		if (rectShadowRenderer && rectShadowRC && rectShadowFbo) {
			// INFO: To render rectangular shadows we need to:
			// [x] Create FBO if it doesn't already exist
			// [x] Record how much time it takes to render the shadow
			// [x] Set viewport to the size of the frame buffer
			// [ ] Bind frame buffer to shadow fbo (a render to texture "DirToLightNDepth" with GL_DEPTH_ATTACHMENT of type GL_FLOAT)
			// [ ] 
			// [ ] Set uniform block variables:
			//     [ ] Camera Matrix is inverse directional light position matrix
			//     [ ] Projection Matrix is a ortho matrix which tightly wraps a sphere which encloses the scene
			// [ ] Reading Uniforms
			//     [ ] shadow view matrices
			// [x] Render to depth texture
			// [x] Render to color texture (when debugging)

			for (int dtli = 0; dtli < Fluxions::MaxLights; dtli++) {
				double rectShadowT0 = HFLOG_MS_ELAPSED();

				Fluxions::SimpleDirToLight* dirToLight = ssg.dirToLights.getPtr(dtli + 1);
				if (!dirToLight) continue;

				// Determine best projection for shadow map based on size of scene
				float zfar = ssg.getBoundingBox().radiusRounded() + 1.0f;
				Vector3f center = ssg.getBoundingBox().center();
				Vector3f eye = center + dirToLight->ublock.dirTo.xyz() * zfar;
				dirToLight->viewMatrix = Matrix4f::MakeLookAt(eye, center, { 0.0f, 1.0f, 0.0f });
				dirToLight->ublock.shadow.x = 1.0f;
				dirToLight->ublock.shadow.y = 2.0f * zfar;
				dirToLight->projMatrix = Matrix4f::MakeOrtho(-zfar, zfar, -zfar, zfar, 1.0f, 2.0f * zfar);
				dirToLight->projviewMatrix = dirToLight->projMatrix * dirToLight->viewMatrix;

				rectShadowRC->preCameraMatrix = dirToLight->viewMatrix;
				rectShadowRC->postCameraMatrix.LoadIdentity();
				rectShadowRC->viewportProjectionMatrix = dirToLight->projMatrix;

				rectShadowFbo->setCurrentLayer(dtli);
				rectShadowRenderer->render();

				// TODO: Create uniform buffer block with all necessary shadow parameters

				my_hud_info.rectShadowTimes[dtli] = HFLOG_MS_ELAPSED() - rectShadowT0;

				if (Interface.captureShadows) {
					//	SaveTextureMap(GL_TEXTURE_2D, ssg.environment.sunColorMapId, "sun_color.ppm");
					//	SaveTextureMap(GL_TEXTURE_2D, ssg.environment.sunDepthMapId, "sun_depth.ppm");
				}
			}
		}

		while (FxCheckLogErrors()) {
			HFLOGERROR("Error rendering Cube Shadows");
			Interface.drawRectShadows = false;
		}

		// Set draw rect shadows to false so we don't continuously generate shadow maps
		Interface.drawRectShadows = false;
		Interface.captureShadows = false;
	}

	void SSPHH_Application::RenderGLES30_CubeShadows() {
		if (!Interface.drawCubeShadows) return;

		constexpr int shadow_debugging = 0;

		if (!pbrRC) {
			pbrRC = rendererContext.getRendererConfig("pbr_rc");
		}

		//////////////////////////////////////////////
		// CUBE SHADOWS //////////////////////////////
		//////////////////////////////////////////////

		FxSetErrorMessage(__FILE__, __LINE__, "%s Cube Shadows", __func__);

		bool hadError = false;

		if (!cubeShadowRC) {
			cubeShadowRC = rendererContext.getRendererConfig("cubeShadow_rc");
			if (cubeShadowRC) {
				// one time setup
				cubeShadowRC->useZOnly = true;
				cubeShadowRC->isCubeMap = false;
				cubeShadowRC->renderSkyBox = false;
				cubeShadowRC->renderSceneGraph = true;
				cubeShadowRC->useSceneCamera = false;
				cubeShadowRC->useSceneProjection = false;
				cubeShadowRC->clearColor.reset(1.0f, 0.0f, 1.0f, 1.0f);
				cubeShadowRC->clearColorBuffer = (shadow_debugging != 0);
				cubeShadowRC->clearDepthBuffer = true;
				cubeShadowRC->viewportRect.w = rectShadowFbo->width();
				cubeShadowRC->viewportRect.h = rectShadowFbo->height();
				cubeShadowRC->writeFBO = nullptr;

				cubeShadowProgram = rendererContext.getRendererProgram("cubeShadow_program");
			}
		}

		if (!cubeShadowRenderer) {
			cubeShadowRenderer = rendererContext.getRenderer("cubeShadow_renderer");
			if (cubeShadowRenderer) {
				// one time setup
				cubeShadowRenderer->setSceneGraph(&ssg);
				cubeShadowRenderer->setRenderConfig(cubeShadowRC);
				if (!cubeShadowRenderer->validate()) {
					cubeShadowRenderer = nullptr;
				}
				if (FxCheckLogErrors()) { hadError = true; }
			}
		}

		if (!cubeShadowFbos[0]) {
			cubeShadowFbos[0] = rendererContext.getFramebuffer("AnisoLight0_fbo");
			cubeShadowFbos[1] = rendererContext.getFramebuffer("AnisoLight1_fbo");
			cubeShadowFbos[2] = rendererContext.getFramebuffer("AnisoLight2_fbo");
			cubeShadowFbos[3] = rendererContext.getFramebuffer("AnisoLight3_fbo");
			cubeShadowFbos[4] = rendererContext.getFramebuffer("AnisoLight4_fbo");
			cubeShadowFbos[5] = rendererContext.getFramebuffer("AnisoLight5_fbo");
			cubeShadowFbos[6] = rendererContext.getFramebuffer("AnisoLight6_fbo");
			cubeShadowFbos[7] = rendererContext.getFramebuffer("AnisoLight7_fbo");
		}

		if (cubeShadowRenderer && cubeShadowRC) {
			// render cube maps
			// version 1.0, just render each face, later we will use a geometry shader to accomplish the same thing
			for (int sphli = 0; sphli < Fluxions::MaxLights; sphli++) {
				double cubeShadowT0 = HFLOG_MS_ELAPSED();

				Fluxions::SimpleAnisoLight* anisoLight = ssg.anisoLights.getPtr(sphli + 1);
				if (!anisoLight) continue;

				Fluxions::RendererFramebuffer* cubeShadowFbo = cubeShadowFbos[sphli];
				if (!cubeShadowFbo) continue;
				cubeShadowRC->writeFBO = cubeShadowFbo;

				for (int face = 0; face < 6; face++) {

					// Determine best projection for shadow map based on size of scene
					float zfar = ssg.getBoundingBox().diameterRounded() + 1.0f;
					constexpr float znear = 1.0f;
					Vector3f lightPosition = anisoLight->ublock.position.xyz();
					anisoLight->ublock.shadow.x = znear;
					anisoLight->ublock.shadow.y = zfar;
					anisoLight->projMatrix = Matrix4f::MakePerspectiveY(90.0f, 1.0f, znear, zfar);
					anisoLight->viewMatrix = Matrix4f::MakeCubeMatrixPosition(face, lightPosition);
					anisoLight->projviewMatrix = anisoLight->projMatrix * anisoLight->viewMatrix;
					cubeShadowRC->preCameraMatrix = anisoLight->viewMatrix;
					cubeShadowRC->postCameraMatrix.LoadIdentity();
					cubeShadowRC->viewportProjectionMatrix = anisoLight->projMatrix;
					if (cubeShadowProgram) {
						cubeShadowProgram->use();
						cubeShadowProgram->uniform1f("ZFar", zfar);
						cubeShadowProgram->uniform3f("LightPosition", lightPosition.const_ptr());
						if (FxCheckLogErrors()) {
							HFLOGERROR("What's up?!");
						}
					}

					cubeShadowFbo->setCurrentLayer(face);
					if (FxCheckLogErrors()) { hadError = true; }
					cubeShadowRenderer->render();
					if (FxCheckLogErrors()) { hadError = true; }
				}

				my_hud_info.cubeShadowTimes[sphli] = HFLOG_MS_ELAPSED() - cubeShadowT0;

				if (Interface.captureShadows) {
					//	SaveTextureMap(GL_TEXTURE_2D, ssg.environment.sunColorMapId, "sun_color.ppm");
					//	SaveTextureMap(GL_TEXTURE_2D, ssg.environment.sunDepthMapId, "sun_depth.ppm");
				}
			}
		}

		while (hadError || FxCheckLogErrors()) {
			HFLOGERROR("Error rendering Cube Shadows");
			Interface.drawCubeShadows = false;
		}

		// Set draw rect shadows to false so we don't continuously generate shadow maps
		Interface.drawCubeShadows = false;


		//RendererConfig& cubeShadow = rendererContext.rendererConfigs["cubeShadow"];
		//RendererConfig& rectShadow = rendererContext.rendererConfigs["rectShadow"];


		//// Render cube shadow map for light 0
		//for (int i = 0; i < ssg.pointLights.size(); i++) {
		//	auto& spl = ssg.pointLights[i];
		//	auto& scs = ssg.pointLights[i].scs;
		//	
		//	scs.zfar = cubeShadow.viewportZFar;
		//	cubeShadow.fbo_gen_color = false;

		//	cubeShadow.renderToFBO = false;
		//	cubeShadow.useSceneCamera = false;
		//	cubeShadow.cameraMatrix_.LoadIdentity();
		//	cubeShadow.cameraPosition = Vector4f(spl.position, 1.0f);

		//	//RenderCubeShadowMap(ssg, scs, cubeShadow);
		//	RenderCube(rendererContext, ssg, "gles30Shadow", "cubeShadow");
		//	FxSetErrorMessage("ssphh.cpp", __LINE__, __FUNCTION__);
		//}

		//for (int i = 0; i < ssgUserData->ssphhLights.size(); i++) {
		//	auto& sphl = ssgUserData->ssphhLights[i];
		//	auto& scs = sphl.depthSphlMap;

		//	scs.zfar = cubeShadow.viewportZFar;
		//	if (Interface.ssphh.enableShadowColorMap) {
		//		sphl.colorSphlMap.createTextureCube();
		//		cubeShadow.clearColor.reset(0.2f, 0.4f, 0.6f, 1.0f);
		//		cubeShadow.fbo_gen_color = true;
		//		cubeShadow.fbo_color_map = sphl.colorSphlMap.getTexture();
		//	}
		//	else {
		//		cubeShadow.fbo_gen_color = false;
		//		cubeShadow.fbo_color_map = 0;
		//	}
		//	cubeShadow.renderToFBO = false;
		//	cubeShadow.useSceneCamera = false;
		//	cubeShadow.cameraMatrix_.LoadIdentity();
		//	cubeShadow.cameraPosition = sphl.position;

		//	//RenderCubeShadowMap(ssg, sphl.depthSphlMap, cubeShadow);
		//	RenderCube(rendererContext, ssg, "gles30Shadow", "cubeShadow");
		//	FxSetErrorMessage("ssphh.cpp", __LINE__, __FUNCTION__);

		//	if (Interface.captureShadows) {
		//		std::ostringstream ostr;
		//		ostr << "sphl" << std::setw(2) << std::setfill('0') << i;
		//		SaveTextureMap(GL_TEXTURE_CUBE_MAP, sphl.colorSphlMap.getTexture(), ostr.str() + "_color.ppm");
		//		SaveTextureMap(GL_TEXTURE_CUBE_MAP, sphl.depthSphlMap.getTexture(), ostr.str() + "_depth.ppm");
		//	}
		//}

		Interface.captureShadows = false;
	}
} // namespace SSPHH
