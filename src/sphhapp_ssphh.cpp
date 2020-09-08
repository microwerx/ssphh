#include "pch.hpp"
#include <ssphhapp.hpp>

namespace SSPHH
{
	void SSPHH_Application::DirtySPHLs() {
		int i = 0;
		for (auto& sphl : ssgUserData->ssphhLights) {
			sphls[i++];
			sphl.dirty = true;
		}
	}

	void SSPHH_Application::UpdateSPHLs() {
		if (!coefs_init) {
			geosphere.load("ssphh-data/resources/models/icos4.txt");
			//sphl.randomize();
			//sphl.createMesh(geosphere);
			//sphl.createLightProbe();
			rendererContext.renderers["sph_renderer"].setSceneGraph(&ssg);
			coefs_init = true;
		}

		if (ssgUserData->ssphhLights.size() != sphls.size()) {
			sphls.clear();
			DirtySPHLs();
		}

		// Determine which sphls we need to recalculate and actually draw
		for (auto& sphl : sphls) {
			sphl.second.enabled = false;
		}

		int i = 0;
		for (auto& sphl : ssgUserData->ssphhLights) {
			if (sphl.randomize) {
				//sphls[i].randomize();
				//for (int lm = 0; lm < sphl.GetMaxCoefficients(); lm++)
				//{
				//	sphl.msph[0][lm] = sphls[i].v_coefs[Sphl::RedComponent][lm];
				//	sphl.msph[1][lm] = sphls[i].v_coefs[Sphl::GreenComponent][lm];
				//	sphl.msph[2][lm] = sphls[i].v_coefs[Sphl::BlueComponent][lm];
				//}
				sphl.randomizeCoefficients();
				sphl.standardize();
				sphl.randomize = false;
				sphl.dirty = true;
			}
			if (sphl.randomizePosition) {
				sphl.position.reset(randomSampler(-5.0f, 5.0f), randomSampler(2.0f, 3.0f), randomSampler(-6.0f, 6.0f));
				sphl.randomizePosition = false;
				sphl.depthSphlMap.dirty = true;
			}
			if (sphl.dirty) {
				// Copy coefficients from the scene graph SPHL list
				sphls[i].copyCoefficients(sphl, Interface.ssphh.MaxDegrees, Interface.ssphh.enableBasicShowSPHLs);
				sphls[i].createMesh(geosphere);
				//sphls[i].createLightProbe();
				sphl.dirty = false;
				sphl.depthSphlMap.dirty = true;
			}
			sphls[i].position = sphl.position;
			sphls[i].enabled = sphl.enabled;
			i++;
		}
	}

	void SSPHH_Application::UploadSPHLs() {
		unsigned i = 0;
		for (auto& sphl : ssgUserData->ssphhLights) {
			if (sphl.ptrcLightProbeImage.empty()) {
				sphl.uploadLightProbe(sphl.ptrcLightProbeImage, sphl.ptrcLightProbeTexture);
				sphl.uploadLightProbe(sphl.msphLightProbeImage, sphl.msphLightProbeTexture);
				sphl.uploadLightProbe(sphl.hierLightProbeImage, sphl.hierLightProbeTexture);
			}

			if (Interface.ssphh.enableShadowColorMap) {
				sphls[i].lightProbeTexIds[0] = sphl.colorSphlMap.getTexture();
			}
			else {
				sphls[i].lightProbeTexIds[0] = sphl.hierLightProbeTexture.getTexture();
			}
			sphls[i].lightProbeTexIds[1] = sphl.ptrcLightProbeTexture.getTexture();
			sphls[i].lightProbeTexIds[2] = sphl.msphLightProbeTexture.getTexture();
			i++;
		}

	}

	void SSPHH_Application::RenderGLES30SPHLs() {
		if (!Interface.ssphh.enableShowSPHLs)
			return;
		////RendererContext gl;
		//RendererConfig& rc = rendererContext.rendererConfigs["default"];
		//// FIXME: Are we using rendererContext?
		//rc.rc_program_ptr->reset();// = rendererContext.FindProgram("sphl", "sphl");
		//rc.clearDepthBuffer = false;
		//rc.clearColorBuffer = false;
		//rc.enableBlend = false;
		//rc.blendFuncSrcFactor = GL_SRC_ALPHA;
		//rc.blendFuncDstFactor = GL_ONE;

		//if (!rc.shaderProgram) {
		//	//HFLOGINFO("sphl shader not found");
		//	return;
		//}

		//GLint vloc = rc.shaderProgram->getAttribLocation("aPosition");
		//GLint tloc = rc.shaderProgram->getAttribLocation("aTexCoord");

		//// BEGIN RENDER SPH
		//RendererGLES30&sph_renderer = rendererContext.renderers["sph_renderer"];
		//sph_renderer.setRenderConfig(&rc);
		////auto program1 = rc.shaderProgram = rendererContext.FindProgram("sphl", "sphl");
		////auto program2 = rc.shaderProgram = rendererContext.FindProgram("glut", "cubemap");
		//sph_renderer.saveGLState();
		//if (sph_renderer.applyRenderConfig()) {
		//	glActiveTexture(GL_TEXTURE0);
		//	for (auto& it : sphls) {
		//		auto& sphl = it.second;

		//		if (!sphl.enabled)
		//			continue;

		//		// glutDebugBindTexture(GL_TEXTURE_CUBE_MAP, sphl.lightProbeTexIds[0]);

		//		rc.shaderProgram->applyUniform("SPHL_LightProbeMode", (RendererUniform)0);
		//		rc.shaderProgram->applyUniform("SPHL_NumDegrees", (RendererUniform)2);
		//		rc.shaderProgram->applyUniform("SPHL_Coefs", RendererUniform(GL_FLOAT_VEC4, 9, GL_FALSE, (GLubyte*)sphl.coefs));
		//		rc.shaderProgram->applyUniform("SPHL_LightProbe", (RendererUniform)0);

		//		Matrix4f worldMatrix;
		//		worldMatrix.Translate(sphl.position.x, sphl.position.y, sphl.position.z);
		//		rendererContext.renderers["sph_renderer"].renderMesh(sphl.sph_model, worldMatrix);

		//		if (!Interface.ssphh.enableBasicShowSPHLs) {
		//			Matrix4f identityMatrix;
		//			rc.shaderProgram->applyUniform("SPHL_LightProbeMode", (RendererUniform)1);
		//			rc.shaderProgram->applyUniform("WorldMatrix", (RendererUniform)identityMatrix);
		//			// Render light probe cube maps
		//			float angles[3] = { 270.0f, 135.0f, 45.0f };
		//			for (auto& a : angles)
		//				a *= FX_F32_DEGREES_TO_RADIANS;
		//			float R = 1.0f;
		//			float S = 0.5f;
		//			Matrix4f lpWorldMatrix[3];
		//			rc.shaderProgram->use();
		//			for (int i = 0; i < 3; i++) {

		//				// glutDebugBindTexture(GL_TEXTURE_CUBE_MAP, sphl.lightProbeTexIds[i]);
		//				//if (i == 0)
		//				//	glutDebugBindTexture(GL_TEXTURE_CUBE_MAP, enviroSkyBoxTexture.GetTextureId());
		//				//else if (i == 1)
		//				//	glutDebugBindTexture(GL_TEXTURE_CUBE_MAP, ssg.environment.pbskyColorMapId);
		//				//else if (i == 2)

		//				FxDebugBindTexture(GL_TEXTURE_CUBE_MAP, sphl.lightProbeTexIds[i]);

		//				//sph_renderer.RenderMesh(sphl.lightProbeModel, lpWorldMatrix[i]);
		//				FxDrawGL2CubeMap(
		//					sphl.position.x + R * cos(angles[i]),
		//					sphl.position.y + R * sin(angles[i]),
		//					sphl.position.z,
		//					S, vloc, tloc);
		//			}
		//		}
		//	}

		//	FxDebugBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		//	glActiveTexture(GL_TEXTURE0);
		//}

		//if (0) //(!Interface.ssphh.enableBasicShowSPHLs)
		//{
		//	for (auto& sphl : ssgUserData->ssphhLights) {
		//		double S = 0.25;
		//		double R = 1.0;
		//		FxDebugBindTexture(GL_TEXTURE_CUBE_MAP, sphl.ptrcLightProbeTexture.getTexture());
		//		FxDrawGL2CubeMap(
		//			sphl.position.x + R * 0.707 + S * 1,
		//			sphl.position.y - R * 0.707,
		//			sphl.position.z,
		//			S, vloc, tloc);
		//		FxDebugBindTexture(GL_TEXTURE_CUBE_MAP, sphl.msphLightProbeTexture.getTexture());
		//		FxDrawGL2CubeMap(
		//			sphl.position.x + R * 0.707 + S * 3,
		//			sphl.position.y - R * 0.707,
		//			sphl.position.z,
		//			S, vloc, tloc);
		//		FxDebugBindTexture(GL_TEXTURE_CUBE_MAP, sphl.hierLightProbeTexture.getTexture());
		//		FxDrawGL2CubeMap(
		//			sphl.position.x + R * 0.707 + S * 5,
		//			sphl.position.y - R * 0.707,
		//			sphl.position.z,
		//			S, vloc, tloc);

		//		//auto i = 0;
		//		//for (auto & hier : sphl.hierarchies)
		//		//{
		//		//	GLuint texture = 0;

		//		//	texture = hier.debugLightProbe.texture.GetTexture();
		//		//	glutDebugBindTexture(GL_TEXTURE_CUBE_MAP, texture);
		//		//	FxDrawGL2CubeMap(
		//		//		sphl.position.x + R * 0.707 + S * (i * 2 + 1),
		//		//		sphl.position.y - R * 0.707 - S * 2,
		//		//		sphl.position.z,
		//		//		S, vloc, tloc);

		//		//	texture = hier.debugSphLightProbe.texture.GetTexture();
		//		//	glutDebugBindTexture(GL_TEXTURE_CUBE_MAP, texture);
		//		//	FxDrawGL2CubeMap(
		//		//		sphl.position.x + R * 0.707 + S * (i * 2 + 1),
		//		//		sphl.position.y - R * 0.707 - S * 4,
		//		//		sphl.position.z,
		//		//		S, vloc, tloc);

		//		//	texture = ssgUserData->ssphhLights[hier.index].ptrcLightProbeTexture.GetTexture();
		//		//	glutDebugBindTexture(GL_TEXTURE_CUBE_MAP, texture);
		//		//	FxDrawGL2CubeMap(
		//		//		sphl.position.x + R * 0.707 + S * (i * 2 + 1),
		//		//		sphl.position.y - R * 0.707 - S * 6,
		//		//		sphl.position.z,
		//		//		S, vloc, tloc);

		//		//	texture = ssgUserData->ssphhLights[hier.index].msphLightProbeTexture.GetTexture();
		//		//	glutDebugBindTexture(GL_TEXTURE_CUBE_MAP, texture);
		//		//	FxDrawGL2CubeMap(
		//		//		sphl.position.x + R * 0.707 + S * (i * 2 + 1),
		//		//		sphl.position.y - R * 0.707 - S * 8,
		//		//		sphl.position.z,
		//		//		S, vloc, tloc);
		//		//	i++;
		//		//}

		//		FxDebugBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		//	}
		//}
		//sph_renderer.restoreGLState();
	}

	void SSPHH_Application::RenderGL11Hierarchies() {
		if (Interface.ssphh.enableShowHierarchies) {
			glMatrixMode(GL_PROJECTION);
			glPushMatrix();
			glLoadIdentity();
			glMultMatrixf(rendererContext.rendererConfigs["default"].projectionMatrix.const_ptr());
			glMultMatrixf(rendererContext.rendererConfigs["default"].cameraMatrix.const_ptr());

			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glLoadIdentity();

			std::vector<SimpleVertex> vertices;
			for (int i = 0; i < ssgUserData->ssphhLights.size(); i++) {
				auto& sphl = ssgUserData->ssphhLights[i];
				if (!sphl.enabled)
					continue;

				Color3f color1;
				Color3f color2;

				for (int j = 0; j < ssgUserData->ssphhLights.size(); j++) {
					if (i == j || j < 0)
						continue;

					float h = 0.0f; // clamp(sphl.hierarchies[j].percentVisible, 0.0f, 1.0f) * 0.5f + 0.5f;
					float s = sphl.enabled ? 1.0f : 0.5f;
					float l;
					if (i >= Interface.ssphh.HierarchiesMaxSphls)
						l = 0.25f;
					else
						l = 0.5f;

					SimpleVertex v1, v2;
					v1.aColor = HLSToRGBf(h, l, s).ToColor4();
					v1.aPosition = ssgUserData->ssphhLights[i].position;
					v2.aColor = v1.aColor;
					v2.aPosition = ssgUserData->ssphhLights[j].position;

					v1.aPosition += Vector3f(0.0f, -0.5f, 0.0f);
					v2.aPosition += Vector3f(0.0f, 0.5f, 0.0f);

					vertices.push_back(v1);
					vertices.push_back(v2);
				}
				SimpleVertex v1, v2;
				v1.aColor = sphl.E0 * sphl.getCoefficientColor(0, 0);
				v2.aColor = v1.aColor;
				v1.aPosition = sphl.position - Vector3f(0.0f, 0.5f, 0.0f);
				v2.aPosition = sphl.position + Vector3f(0.0f, 0.5f, 0.0f);
				vertices.push_back(v1);
				vertices.push_back(v2);
			}

			//glLineWidth(4.0f);
			glEnable(GL_LINE_SMOOTH);
			glBegin(GL_LINES);
			for (auto& v : vertices) {
				glColor4fv(v.aColor.const_ptr());
				glVertex3fv(v.aPosition.const_ptr());
			}
			glEnd();
			//glLineWidth(1.0f);
			glDisable(GL_LINE_SMOOTH);

			glPopMatrix();
			glMatrixMode(GL_PROJECTION);
			glPopMatrix();
			glMatrixMode(GL_MODELVIEW);
		}

		//glMatrixMode(GL_PROJECTION);
		//glPushMatrix();
		//glLoadIdentity();
		//glMultMatrixf(rc.projectionMatrix.const_ptr());
		//glMultMatrixf(rc.cameraMatrix_.const_ptr());

		//glMatrixMode(GL_MODELVIEW);
		//glPushMatrix();
		//glLoadIdentity();
		//for (auto & it : sphls)
		//{
		//	auto & sphl = it.second;
		//	if (!sphl.enabled) continue;

		//	glPushMatrix();
		//	glTranslatef(sphl.position.X, sphl.position.y, sphl.position.z);
		//	FxDrawGL1SolidSphere(1.0, 16, 16);
		//	glPopMatrix();
		//}
		//glPopMatrix();
		//glMatrixMode(GL_PROJECTION);
		//glPopMatrix();
		//glMatrixMode(GL_MODELVIEW);
	}

}
