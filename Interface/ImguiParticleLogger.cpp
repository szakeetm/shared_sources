#include "ImguiParticleLogger.h"
#include "ImguiExtensions.h"
#include "Helper/StringHelper.h"
#include "Helper/FormatHelper.h"
#include "imgui_stdlib/imgui_stdlib.h"
#include "ImguiPopup.h"
#include "Interface.h"
#include "SimulationModel.h"
#include "ImguiWindow.h"
#include "NativeFileDialog/molflow_wrapper/nfd_wrapper.h"

#include "ParticleLogger.h" // to update legazy gui

#if defined(MOLFLOW)
#include "../../src/MolFlow.h"
#include "../../src/Simulation/MolflowSimGeom.h"
#endif

void ImParticleLogger::Draw()
{
	if (!drawn) return;

	ImGui::SetNextWindowPos(ImVec2(5*txtW, 3*txtH), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(50*txtW, 17*txtH));
	ImGui::Begin("Particle Logger", &drawn, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize);

	ImGui::TextWrapped("This tool allows to record all test particles hitting a chosen facet.\nOnly one facet can be recorded at a time.\nThe recording must be exported, it is not saved with the file.");

	ImGui::BeginChild("##RecSet", ImVec2(0, 7 * txtH), true);
	{
		ImGui::TextDisabled("Recording settings");
		ImGui::Checkbox("Enable logging", &enableLogging);
		if (ImGui::BeginTable("##PL", 2)) {
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::InputTextRightSide("Facet number:", &facetNumInput);
			if (ImGui::InputTextRightSide("Max recorded:", &maxRecInput)) {
				UpdateMemoryEstimate();
			}
			ImGui::TableNextColumn();
			if (ImGui::Button("<- Get selected")) {
				auto selFacets = interfGeom->GetSelectedFacets();
				if (selFacets.size() != 1) ImIOWrappers::InfoPopup("Error","Select exactly one facet");
				else {
					facetNum = selFacets[0];
					facetNumInput = fmt::format("{}", facetNum+1);
					enableLogging = true;
				}
			}
			ImGui::AlignTextToFramePadding();
			ImGui::Text(memUse);
			ImGui::EndTable();
		}
		ImGui::PlaceAtRegionCenter("Apply");
		if (ImGui::Button("Apply")) {
			ApplyButtonPress();
		}
	}
	ImGui::EndChild();
	ImGui::BeginChild("##Result", ImVec2(0, 4 * txtH), true);
	{
		ImGui::TextDisabled("Result");
		ImGui::Text(statusLabel);
		if (ImGui::Button("Copy to clipboard")) {
			std::string text = LogToText();
			std::function<void()> f = [text]() {
				SDL_SetClipboardText(text.c_str());
				};
			if (sizeof(text[0])*text.size() > 50 * 1024 * 1024) {
				mApp->imWnd->popup.Open("Large log size", fmt::format("Careful! You're putting {} to the clipboard.\nMaybe it's a better idea to save it as a file. Try anyway?", Util::formatSize(sizeof(text[0]) * text.size())),
					{ std::make_shared<ImIOWrappers::ImButtonFunc>("Yes", f, ImGuiKey_Enter, ImGuiKey_KeypadEnter),
					  std::make_shared<ImIOWrappers::ImButtonInt>("Cancel", 0, ImGuiKey_Escape) });
			}
			else {
				f();
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Export to CSV")) {
			std::string fn = NFD_SaveFile_Cpp("csv", "");
			if (!fn.empty()) {
				FILE* f = fopen(fn.c_str(), "w");
				if (f == NULL) {
					ImIOWrappers::InfoPopup("Error", "Cannot open file\nFile: " + fn);
				}
				else {
					LogToText("\t",f);
					fclose(f);
				}
			}
		}
	}
	ImGui::EndChild();

	ImGui::End();
}

void ImParticleLogger::Init(Interface* mApp_)
{
	mApp = mApp_;
	interfGeom = mApp->worker.GetGeometry();
	UpdateMemoryEstimate();
}

void ImParticleLogger::ApplyButtonPress()
{
	LockWrapper lW(mApp->imguiRenderLock);
	if (!Util::getNumber(&facetNum, facetNumInput)) {
		ImIOWrappers::InfoPopup("Error", "Invalid facet number");
		return;
	}
	if (!Util::getNumber(&maxRec, maxRecInput)) {
		ImIOWrappers::InfoPopup("Error", "Invalid max rec. number");
		return;
	}
	if (facetNum-1 < 0 || facetNum-1 > interfGeom->GetNbFacet()) {
		ImIOWrappers::InfoPopup("Error", "No such facet");
		return;
	}
	mApp->worker.model->otfParams.enableLogging = enableLogging;
	mApp->worker.model->otfParams.logFacetId = facetNum-1;
	mApp->worker.model->otfParams.logLimit = maxRec;
	mApp->worker.ChangeSimuParams();
	UpdateStatus();
}

void ImParticleLogger::UpdateMemoryEstimate()
{
	int nbRec;
	if (Util::getNumber(&nbRec, maxRecInput) && nbRec > 0) {
		memUse = (Util::formatSize(2 * nbRec * sizeof(ParticleLoggerItem)));
	}
}

void ImParticleLogger::UpdateStatus()
{
	enableLogging = (mApp->worker.model->otfParams.enableLogging);
	LockWrapper lW(mApp->imguiRenderLock);
	if (!enableLogging) {
		statusLabel = "No recording.";
		return;
	}
	log = mApp->worker.GetLog();
	if (log == nullptr) {
		statusLabel = "Log data missing";
	}
	else {
		statusLabel = fmt::format("Recording on facet {}: {} particles logged", mApp->worker.model->otfParams.logFacetId + 1 , log->pLog.size());
	}
	if (mApp->particleLogger != nullptr) mApp->particleLogger->UpdateStatus(); // update legacy gui
	mApp->worker.UnlockLog();
}

void ImParticleLogger::OnShow()
{
	UpdateStatus();
}

std::string ImParticleLogger::LogToText(const std::string& separator, FILE* file)
{
	if (log->pLog.size() == 0) return "";
	/* 
	aborting not yet supported
	might be possible in the future, after reworking the main ImGui Window manager (ImguiWindow class)
	*/

	bool directToFile = file != nullptr;
	std::string out;

	// header
	std::string tmp;
	tmp.append("Pos_X_[cm]" + separator);
	tmp.append("Pos_Y_[cm]" + separator);
	tmp.append("Pos_Z_[cm]" + separator);
	tmp.append("Pos_u" + separator);
	tmp.append("Pos_v" + separator);
	tmp.append("Dir_X" + separator);
	tmp.append("Dir_Y" + separator);
	tmp.append("Dir_Z" + separator);
	tmp.append("Dir_theta_[rad]" + separator);
	tmp.append("Dir_phi_[rad]" + separator);
	tmp.append("LowFluxRatio" + separator);
#ifdef MOLFLOW
	tmp.append("Velovity_[m/s}" + separator);
	tmp.append("HitTime_[s]" + separator);
	tmp.append("ParticleDecayMoment_[s]" + separator);
#endif
#ifdef SYNRAD
	tmp.append("Energy_[eV]" + separator);
	tmp.append("Flux_[photon/s]" + separator);
	tmp.append("Power_[W]" + separator);
#endif
	tmp.append("\n");
	if (directToFile) {
		fprintf(file,tmp.c_str());
	}
	else {
		out.append(tmp);
	}
	tmp.clear();
	/*
	mApp->imWnd->progress.SetMessage("Assembling text");
	mApp->imWnd->progress.SetTitle("Particle logger");
	mApp->imWnd->progress.Show();
	mApp->imWnd->progress.SetProgress(0.0);
	*/
	// using legacy progress bar, can be changed to the ImGui version after ImGui progress is properly implemented
	LockWrapper lW(mApp->imguiRenderLock);
	GLProgress_GUI prg = GLProgress_GUI("Assembling text", "Particle logger");
	prg.SetVisible(true);

	InterfaceFacet* f = mApp->worker.GetGeometry()->GetFacet(mApp->worker.model->otfParams.logFacetId);
	for (size_t i = 0; i < log->pLog.size(); i++) {
		
		prg.SetProgress((double)i / (double)log->pLog.size());
		
		Vector3d hitPos = f->sh.O + log->pLog[i].facetHitPosition.u * f->sh.U + log->pLog[i].facetHitPosition.v * f->sh.V;
		double u = sin(log->pLog[i].hitTheta) * cos(log->pLog[i].hitPhi);
		double v = sin(log->pLog[i].hitTheta) * sin(log->pLog[i].hitPhi);
		double n = cos(log->pLog[i].hitTheta);
		Vector3d hitDir = u * f->sh.nU + v * f->sh.nV + n * f->sh.N;

		tmp.append(fmt::format("{:.8g}", hitPos.x) + separator);
		tmp.append(fmt::format("{:.8g}", hitPos.y) + separator);
		tmp.append(fmt::format("{:.8g}", hitPos.z) + separator);
		tmp.append(fmt::format("{:.8g}", log->pLog[i].facetHitPosition.u) + separator);
		tmp.append(fmt::format("{:.8g}", log->pLog[i].facetHitPosition.v) + separator);
		tmp.append(fmt::format("{:.8g}", hitDir.x) + separator);
		tmp.append(fmt::format("{:.8g}", hitDir.y) + separator);
		tmp.append(fmt::format("{:.8g}", hitDir.z) + separator);
		tmp.append(fmt::format("{:.8g}", log->pLog[i].hitTheta) + separator);
		tmp.append(fmt::format("{:.8g}", log->pLog[i].hitPhi) + separator);
		tmp.append(fmt::format("{:.8g}", log->pLog[i].oriRatio) + separator);
#ifdef MOLFLOW
		tmp.append(fmt::format("{:.8g}", log->pLog[i].velocity) + separator);
		tmp.append(fmt::format("{:.8g}", log->pLog[i].time) + separator);
		tmp.append(fmt::format("{:.8g}", log->pLog[i].particleDecayMoment) + separator);
#endif
#ifdef SYNRAD
		tmp.append(fmt::format("{:.8g}", log->pLog[i].energy) + separator);
		tmp.append(fmt::format("{:.8g}", log->pLog[i].dF/numScans) + separator);
		tmp.append(fmt::format("{:.8g}", log->pLog[i].dP/numScans) + separator);
#endif
		tmp.append("\n");
		if (directToFile) {
			fprintf(file, tmp.c_str());
		}
		else {
			out.append(tmp);
		}
		tmp.clear();

		mApp->imWnd->progress.SetProgress((double)i/(double)log->pLog.size());
	}
	mApp->imWnd->progress.Hide();
	mApp->worker.UnlockLog();
	return out;
}
