#include "pch.hpp"
#include <ssphhapp.hpp>

namespace SSPHH
{
	void SSPHH_Application::InitUnicornfish() {
		if (Interface.uf.uf_type != UfType::None) {
			Interface.tools.showMaterialEditor = false;
			Interface.tools.showSphlEditor = false;
			Interface.tools.showScenegraphEditor = false;
			Interface.tools.showDebugView = false;
			Interface.tools.showSSPHHWindow = false;
			Interface.tools.showRenderConfigWindow = false;
			Interface.tools.showUnicornfishWindow = true;
		}
	}

	void SSPHH_Application::KillUnicornfish() {
		ssphhUf.Stop();
		ssphhUf.Join();
	}

	void SSPHH_Application::GI_ScatterJobs() {}

	int SSPHH_Application::GI_GatherJobs() {
		int numScattered = ssphhUf.GetNumScatteredJobs();
		int numFinished = ssphhUf.GetNumFinishedJobs();
		std::ostringstream ostr;
		ostr << "Scattered " << std::setw(2) << numScattered << "/";
		ostr << "Finished  " << std::setw(2) << numFinished;
		Interface.ssphh.gi_status = ostr.str();
		if (numFinished > 0) {
			if (numScattered > 0) {
				return 0;
			}
			std::map<std::string, Uf::CoronaJob> jobs;
			ssphhUf.GetFinishedJobs(jobs);

			int numVIZ = 0;
			int numGEN = 0;
			int numREF = 0;
			for (auto& job : jobs) {
				if (job.second.IsVIZ())
					numVIZ++;
				if (job.second.IsGEN())
					numGEN++;
				if (job.second.IsREF())
					numREF++;
				// All we get are SPHs, so we do not want to cause a memory
				// issue by reading images from disc that don't exist
				GI_ProcessGatherJob(job.second);
			}

			//if (numGEN)
			//	ssgUserData->ssphh.GEN();
			//if (numVIZ)
			//	ssgUserData->ssphh.VIZ();
			//ssgUserData->ssphh.HIER();
			if (numGEN) {
				ssphh.GEN();
			}
			if (numVIZ) {
				ssphh.VIZ();
			}
			ssphh.HIER();
		}
		return numScattered;
	}

	bool SSPHH_Application::GI_ProcessJob(Uf::CoronaJob& job) {
		bool useEXR = true;
		FilePathInfo fpi(job.GetOutputPath(useEXR));
		if (fpi.notFound()) {
			HFLOGERROR("Could not find rendered light probe %s", job.GetOutputPath(useEXR).c_str());
			return false;
		}

		int sendLight = -1;
		int recvLight = -1;
		if (job.IsGEN()) {
			sendLight = job.GetGENLightIndex();
		}
		else if (job.IsVIZ()) {
			sendLight = job.GetVIZSendLightIndex();
			recvLight = job.GetVIZRecvLightIndex();
		}

		SimpleSSPHHLight& sphl = ssgUserData->ssphhLights[sendLight];
		Sph4f sph;
		if (job.IsVIZ()) {
			if (useEXR) {
				sphl.vizgenLightProbes[recvLight].loadEXR(fpi.shortestPath());
			}
			else {
				sphl.vizgenLightProbes[recvLight].loadPPM(fpi.shortestPath());
			}
			sphl.vizgenLightProbes[recvLight].convertRectToCubeMap();
			sphl.lightProbeToSph(sphl.vizgenLightProbes[recvLight], sph.msph);
			job.CopySPH(sph);
			return true;
		}
		else if (job.IsGEN()) {
			std::string basename = job.GetName() + "_" + std::to_string(sendLight) + "_sph";

			sphl.readPtrcLightProbe(job.GetOutputPath(useEXR));
			sphl.savePtrcLightProbe(basename + ".exr");

			//if (ssgUserData->ssphh.saveJSONs)
			//	sphl.SaveJsonSph(job.GetName() + "_sph.json");
			if (ssphh.saveJSONs)
				sphl.saveJsonSph(basename + ".json");

			if (useEXR) {
				sphl.vizgenLightProbes[sendLight].loadEXR(fpi.shortestPath());
			}
			else {
				sphl.vizgenLightProbes[sendLight].loadPPM(fpi.shortestPath());

			}
			sphl.vizgenLightProbes[sendLight].convertRectToCubeMap();
			sphl.lightProbeToSph(sphl.vizgenLightProbes[sendLight], sph.msph);
			job.CopySPH(sph);
			sphl.uploadLightProbe(sphl.vizgenLightProbes[sendLight], sphl.hierLightProbeTexture);
			return true;
		}
		return false;
	}

	bool SSPHH_Application::GI_ProcessGatherJob(Uf::CoronaJob& job) {
		int sendLight = -1;
		int recvLight = -1;
		if (job.IsGEN()) {
			sendLight = job.GetGENLightIndex();
		}
		else if (job.IsVIZ()) {
			sendLight = job.GetVIZSendLightIndex();
			recvLight = job.GetVIZRecvLightIndex();
		}

		SimpleSSPHHLight& sphl = ssgUserData->ssphhLights[sendLight];
		Sph4f sph;
		job.CopySPHToSph4f(sph);
		if (job.IsVIZ()) {
			sphl.sphToLightProbe(sph.msph, sphl.vizgenLightProbes[recvLight]);
			return true;
		}
		else if (job.IsGEN()) {
			sphl.sphToLightProbe(sph.msph, sphl.vizgenLightProbes[sendLight]);
			sphl.uploadLightProbe(sphl.vizgenLightProbes[sendLight], sphl.hierLightProbeTexture);
			return true;
		}
		return false;
	}

	void SSPHH_Application::RunJob(Uf::CoronaJob& job) {
		job.Start(coronaScene, ssg);
		GI_ProcessJob(job);
	}
}
