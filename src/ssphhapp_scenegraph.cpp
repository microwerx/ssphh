#include "pch.hpp"
#include <ssphhapp.hpp>
#include <hatchetfish_stopwatch.hpp>

namespace SSPHH
{
	const std::string ssg_default_scene_path{"ssphh-data/resources/scenes/test_texture_scene/"};

	void SSPHH_Application::SSG_ReloadScene()
	{
		ssg.reset();
		Hf::StopWatch stopwatch;
		SSG_LoadScene();
		stopwatch.Stop();
		HFLOGINFO("SSG reload took %4.2f milliseconds", stopwatch.GetMillisecondsElapsed());
		Interface.lastScenegraphLoadTime = stopwatch.GetMillisecondsElapsed();
	}

	void SSPHH_Application::Sun_AdvanceClock(double numSeconds, bool recomputeSky)
	{
		pbsky_timeOffsetInSeconds += numSeconds;
		ssg.environment.pbsky.SetTime(pbsky_localtime, (float)pbsky_timeOffsetInSeconds);
		Interface.recomputeSky = recomputeSky;
	}

	void SSPHH_Application::Sun_ResetClock()
	{
		ssg.environment.pbsky.SetCivilDateTime(ssg.environment.pbsky_dtg);
		ssg.environment.pbsky.computeAstroFromLocale();
		pbsky_localtime = ssg.environment.pbsky.GetTime();
		pbsky_timeOffsetInSeconds = 0.0;
		Interface.recomputeSky = true;
		Sun_AdvanceClock(0.0, true);
		Sky_RegenHosekWilkieTextures();
	}

	void SSPHH_Application::Sun_UseCurrentTime()
	{
		pbsky_localtime = time(NULL);
		pbsky_timeOffsetInSeconds = 0.0;

		ssg.environment.pbsky.SetTime(time(NULL), 0.0);
		ssg.environment.pbsky.computeAstroFromLocale();
		Interface.recomputeSky = true;
		Sky_RegenHosekWilkieTextures();
	}

	void SSPHH_Application::Sun_SetLights()
	{
		if (!sun)
			sun = &ssg.dirToLights.getPtr("sun")->ublock;
		if (sun)
			sun->dirTo = ssg.environment.curSunDirTo;
		if (!moon)
		{
			moon = &ssg.dirToLights.getPtr("moon")->ublock;
			moonGG = ssg.geometryGroups.getPtr("moon");
		}
		if (moon)
		{
			moon->dirTo = ssg.environment.curMoonDirTo;
			moonGG->transform = Matrix4f::MakeTranslation(moon->dirTo.xyz() * 95.0f);
		}
	}

	void SSPHH_Application::SSG_LoadScene()
	{
		if (Interface.uf.uf_type == UfType::Broker)
		{
			HFLOGINFO("configured to be a broker, so not loading scene");
			return;
		}

		ssg.addPath("ssphh-data/resources/models/");
		ssg.addPath("ssphh-data/resources/textures/");
		ssg.addPath("ssphh-data/resources/scenes/");
		ssg.addPath("ssphh-data/resources/scenes/test_texture_scene/");
		ssg.Load(sceneFilename);

		// Configure astronomical models/textures/lights
		ssg.environment.hasMoon = true;
		ssg.environment.hasSun = true;
		ssg.addDirToLight("sun");
		ssg.addDirToLight("moon");
		ssg.currentTransform.LoadIdentity();
		ssg.currentTransform.Translate(0.0f, 2.0f, 0.0f);
		ssg.addGeometryGroup("moon", "ssphh-data/resources/models/moon.obj");

		Interface.sceneName = ssg.name_str();
	}

	void SSPHH_Application::SSG_OptimizeClippingPlanes()
	{
		//Matrix4f cameraMatrix_ = defaultRenderConfig.preCameraMatrix * ssg.camera.actualViewMatrix * defaultRenderConfig.postCameraMatrix;
		//cameraMatrix_.AsInverse().col4()
		Matrix4f cameraMatrix = ssg.camera.actualViewMatrix.AsInverse();
		const BoundingBoxf &bbox = ssg.getBoundingBox();
		const Matrix4f &frameOfReference = cameraMatrix;
		const Vector3f &position = frameOfReference.col4().xyz();
		float znear;
		float zfar;

		float distanceToBoxCenter = (position - bbox.Center()).length() + 1.0f;
		float boxRadius = bbox.RadiusRounded();
		znear = std::max(0.1f, distanceToBoxCenter - boxRadius);
		zfar = distanceToBoxCenter + 2 * boxRadius; // min(1000.0f, distanceToBoxCenter + boxRadius);

		//rendererContext.rendererConfigs["default"].znear = znear;
		//rendererContext.rendererConfigs["default"].zfar = zfar;
		//rendererContext.rendererConfigs["rectShadow"].znear = std::max(0.1f, ssg.environment.sunShadowMapNearZ);
		//rendererContext.rendererConfigs["rectShadow"].zfar = std::min(1000.0f, ssg.environment.sunShadowMapFarZ);
	}

	void SSPHH_Application::SSG_ProcessInterfaceTasks()
	{
		if (Interface.ssg.saveScene)
		{
			Interface.ssg.saveScene = false;
			sceneFilename = ssg_default_scene_path;
			sceneFilename += Interface.ssg.scenename;
			ssg.Save(sceneFilename);
		}

		if (Interface.ssg.resetScene)
		{
			Interface.ssg.resetScene = false;
			ssg.reset();
		}

		if (Interface.ssg.createScene)
		{
			Interface.ssg.createScene = false;
		}

		if (Interface.ssg.loadScene)
		{
			Interface.ssg.loadScene = false;
			sceneFilename = ssg_default_scene_path;
			sceneFilename += Interface.ssg.scenename;
			SSG_ReloadScene();
		}
	}
} // namespace SSPHH
