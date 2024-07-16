#include "ImguiFacetDetails.h"
#include "ImguiExtensions.h"
#include "Facet_shared.h"
#include "Helper/MathTools.h"
#include "Simulation/MolflowSimFacet.h"

void ImFacetDetails::Draw()
{
	if (!drawn) return;
	ImGui::SetNextWindowSizeConstraints(ImVec2(txtW * 60, txtH * 15), ImVec2(txtW * 400, txtH * 150));
	ImGui::Begin("Facet details", &drawn, ImGuiWindowFlags_NoSavedSettings);
	ImGui::BeginChild("###TableSettingsBar", ImVec2(txtW*21,0), ImGuiChildFlags_Border);
    bool columnsChanged = false;
    //toggles
    {
        ImGui::PushStyleCompact();

        ImGui::TextDisabled("Column Toggles");
        ImGui::SameLine();
        if (ImGui::Button("Invert")) Invert();

        columnsChanged |= ImGui::Checkbox("Sticking", &sticking);
        columnsChanged |= ImGui::Checkbox("Opacity", &opacity);
        columnsChanged |= ImGui::Checkbox("Structure", &structure);
        columnsChanged |= ImGui::Checkbox("Link", &link);
        columnsChanged |= ImGui::Checkbox("Desorption", &desorption);
        columnsChanged |= ImGui::Checkbox("Reflection", &reflection);
        columnsChanged |= ImGui::Checkbox("Two Sided", &twoSided);
        columnsChanged |= ImGui::Checkbox("NbVertex", &nbVertex);
        columnsChanged |= ImGui::Checkbox("Area", &area);
        columnsChanged |= ImGui::Checkbox("Temp. [K]", &tempK);
        columnsChanged |= ImGui::Checkbox("Facet 2D Box", &facet2DBox);
        columnsChanged |= ImGui::Checkbox("Texture [u,v]", &textureUV);
        columnsChanged |= ImGui::Checkbox("Mesh Sample", &meshSample);
        columnsChanged |= ImGui::Checkbox("Texture Recolor", &textureRecord);
        columnsChanged |= ImGui::Checkbox("Memory", &memory);
        columnsChanged |= ImGui::Checkbox("Planarity", &planarity);
        columnsChanged |= ImGui::Checkbox("Profile", &profile);
        columnsChanged |= ImGui::Checkbox("Imping Rate", &impingRate);
        columnsChanged |= ImGui::Checkbox("Density [1/m3]", &particleDensity);
        columnsChanged |= ImGui::Checkbox("Density [kg/m3]", &gasDensity);
        columnsChanged |= ImGui::Checkbox("Pressure [mbar]", &pressure);
        columnsChanged |= ImGui::Checkbox("Av.mol.speed [m/s]", &speed);
        columnsChanged |= ImGui::Checkbox("MC Hits", &mcHits);
        columnsChanged |= ImGui::Checkbox("Equiv hits", &equivHits);
        columnsChanged |= ImGui::Checkbox("Des", &des);
        columnsChanged |= ImGui::Checkbox("Equiv Abs.", &equivAbs);
        columnsChanged |= ImGui::Checkbox("Force", &force);
        columnsChanged |= ImGui::Checkbox("Force^2", &force2);
        columnsChanged |= ImGui::Checkbox("Torque", &torque);

        if (columnsChanged) Update();

        ImGui::PopStyleCompact();
    }
	ImGui::EndChild();
	ImGui::SameLine();
	ImGui::BeginChild("###TableWrapper", ImVec2(0, 0), ImGuiChildFlags_Border);
    DrawTable();
	ImGui::EndChild();
	ImGui::End();
}

void ImFacetDetails::OnShow()
{
    Update();
}

void ImFacetDetails::Invert()
{
    {
        sticking = !sticking;
        opacity = !opacity;
        structure = !structure;
        link = !link;
        desorption = !desorption;
        reflection = !reflection;
        twoSided = !twoSided;
        nbVertex = !nbVertex;
        area = !area;
        tempK = !tempK;
        facet2DBox = !facet2DBox;
        textureUV = !textureUV;
        meshSample = !meshSample;
        textureRecord = !textureRecord;
        memory = !memory;
        planarity = !planarity;
        profile = !profile;
        impingRate = !impingRate;
        particleDensity = !particleDensity;
        gasDensity = !gasDensity;
        pressure = !pressure;
        speed = !speed;
        mcHits = !mcHits;
        equivHits = !equivHits;
        des = !des;
        equivAbs = !equivAbs;
        force = !force;
        force2 = !force2;
        torque = !torque;
    }
    Update();
}

void ImFacetDetails::DrawTable()
{
    if (ImGui::BeginTable("###FacetDetailsTable", columns,
        ImGuiTableFlags_SizingFixedFit |
        ImGuiTableFlags_NoKeepColumnsVisible |
        ImGuiTableFlags_RowBg |
        ImGuiTableFlags_ScrollX |
        ImGuiTableFlags_Borders
    )) {
        // headers
        {
            ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, txtW * 7);
            if (sticking) {
                ImGui::TableSetupColumn("Sticking");
            }
            if (opacity) {
                ImGui::TableSetupColumn("Opacity");
            }
            if (structure) {
                ImGui::TableSetupColumn("Structure");
            }
            if (link) {
                ImGui::TableSetupColumn("Link");
            }
            if (desorption) {
                ImGui::TableSetupColumn("Desorption");
            }
            if (reflection) {
                ImGui::TableSetupColumn("Reflection");
            }
            if (twoSided) {
                ImGui::TableSetupColumn("Two Sided");
            }
            if (nbVertex) {
                ImGui::TableSetupColumn("nbVertex");
            }
            if (area) {
                ImGui::TableSetupColumn("Area");
            }
            if (tempK) {
                ImGui::TableSetupColumn("Temp. [K]");
            }
            if (facet2DBox) {
                ImGui::TableSetupColumn("Facet 2D Box");
            }
            if (textureUV) {
                ImGui::TableSetupColumn("Texture [u,v]");
            }
            if (meshSample) {
                ImGui::TableSetupColumn("Mesh Sample/cm");
            }
            if (textureRecord) {
                ImGui::TableSetupColumn("Texture Record");
            }
            if (memory) {
                ImGui::TableSetupColumn("Memory");
            }
            if (planarity) {
                ImGui::TableSetupColumn("Planarity");
            }
            if (profile) {
                ImGui::TableSetupColumn("Profile");
            }
            if (impingRate) {
                ImGui::TableSetupColumn("Imping. Rate");
            }
            if (particleDensity) {
                ImGui::TableSetupColumn("Density [1/m3]");
            }
            if (gasDensity) {
                ImGui::TableSetupColumn("Density [kg/m3]");
            }
            if (pressure) {
                ImGui::TableSetupColumn("Pressure [mbar]");
            }
            if (speed) {
                ImGui::TableSetupColumn("Avg. mol. speed [m/s]");
            }
            if (mcHits) {
                ImGui::TableSetupColumn("MC Hits");
            }
            if (equivHits) {
                ImGui::TableSetupColumn("Equiv. Hits");
            }
            if (des) {
                ImGui::TableSetupColumn("Des");
            }
            if (equivAbs) {
                ImGui::TableSetupColumn("Equiv. Abs");
            }
            if (force) {
                ImGui::TableSetupColumn("Force");
            }
            if (force2) {
                ImGui::TableSetupColumn("Force^2");
            }
            if (torque) {
                ImGui::TableSetupColumn("Torque");
            }

            ImGui::TableHeadersRow(); // Draw header row
        }

        ImGui::TableNextRow();
        for (auto& row : content) {
            for (auto& cell : row) {
                ImGui::TableNextColumn();
                ImGui::Text(cell.c_str());
            }
        }
        ImGui::EndTable();
    }
}

void ImFacetDetails::Update()
{
    // count enabled columns
    {
        columns = 1; // start with 1 (ID always visible)
        columns += sticking ? 1 : 0;
        columns += opacity ? 1 : 0;
        columns += structure ? 1 : 0;
        columns += link ? 1 : 0;
        columns += desorption ? 1 : 0;
        columns += reflection ? 1 : 0;
        columns += twoSided ? 1 : 0;
        columns += nbVertex ? 1 : 0;
        columns += area ? 1 : 0;
        columns += tempK ? 1 : 0;
        columns += facet2DBox ? 1 : 0;
        columns += textureUV ? 1 : 0;
        columns += meshSample ? 1 : 0;
        columns += textureRecord ? 1 : 0;
        columns += memory ? 1 : 0;
        columns += planarity ? 1 : 0;
        columns += profile ? 1 : 0;
        columns += impingRate ? 1 : 0;
        columns += particleDensity ? 1 : 0;
        columns += gasDensity ? 1 : 0;
        columns += pressure ? 1 : 0;
        columns += speed ? 1 : 0;
        columns += mcHits ? 1 : 0;
        columns += equivHits ? 1 : 0;
        columns += des ? 1 : 0;
        columns += equivAbs ? 1 : 0;
        columns += force ? 1 : 0;
        columns += force2 ? 1 : 0;
        columns += torque ? 1 : 0;
    }
    std::vector<std::vector<std::string>>().swap(content);
    auto selFacets = interfGeom->GetSelectedFacets();
    for (auto& fId : selFacets) {
        auto f = interfGeom->GetFacet(fId);
        std::vector<std::string> row;
        row.push_back(fmt::format("{}", fId+1));
        if (sticking) {
            //row.push_back(f->sh.stickingParam == "" ? fmt::format("{}",  f->sh.sticking) : f->sh.stickingParam);
            row.push_back(fmt::format("{:.5g}",  f->sh.sticking));
        }
        if (opacity) {
            //row.push_back(f->sh.opacityParam == "" ? fmt::format("{}", f->sh.opacity) : f->sh.opacityParam);
            row.push_back(fmt::format("{:.5g}", f->sh.opacity));
        }
        if (structure) {
            row.push_back(f->sh.superIdx != -1 ? fmt::format("{}", f->sh.superIdx+1) : "All");
        }
        if (link) {
            row.push_back(fmt::format("{}", f->sh.superDest));
        }
        if (desorption) {
            std::string des = "";
            switch (f->sh.desorbType)
            {
            case(0):
                des = "None";
                break;
            case(1): 
                des = "Uniform";
                break;
            case(2):
                des = "Cosine";
                break;
            case(3):
                des = fmt::format("Cosine^{}", f->sh.desorbTypeN);
                break;
            default:
                des = "ERROR";
                break;
            }
            row.push_back(des);
        }
        if (reflection) {
            row.push_back(fmt::format("{:.5g} diff. {:.5g} spec. {:.5g} cos^{:.5g}", 
                f->sh.reflection.diffusePart, f->sh.reflection.specularPart,
                1.0 - f->sh.reflection.diffusePart - f->sh.reflection.specularPart,
                f->sh.reflection.cosineExponent));
        }
        if (twoSided) {
            row.push_back(f->sh.is2sided ? "Yes" : "No");
        }
        if (nbVertex) {
            row.push_back(fmt::format("{}", f->sh.nbIndex));
        }
        if (area) {
            if (f->sh.is2sided) {
                row.push_back(fmt::format("2*{:.5g}", f->sh.area));
            }
            else {
                row.push_back(fmt::format("{:.5g}", f->sh.area));
            }
        }
        if (tempK) {
            if (f->sh.temperatureParam.empty()) {
                row.push_back(fmt::format("{:.5g}", f->sh.temperature));
            }
            else {
                // copied from legacy
                double time;
                auto mfModel = std::static_pointer_cast<MolflowSimulationModel>(mApp->worker.model);
                if (mApp->worker.displayedMoment != 0) time = mApp->worker.interfaceMomentCache[mApp->worker.displayedMoment - 1].time;
                else time = mfModel->sp.latestMoment;
                if (mApp->worker.needsReload || !mApp->worker.model->initialized) {
                    //Don't dereference facets, maybe they werent' yet passed to model
                    throw Error(fmt::format("Evaluating time-dependent temperature of facet {} but model not yet synchronized.", fId + 1));
                }
                auto mfFacet = std::static_pointer_cast<MolflowSimFacet>(mApp->worker.model->facets[fId]);
                row.push_back(fmt::format("{:.5g}", mfModel->GetTemperatureAt(mfFacet.get(), time)));
            }
        }
        if (facet2DBox) {
            row.push_back(fmt::format("{:.5g} x {:.5g}", f->sh.U.Norme(), f->sh.V.Norme()));
        }
        if (textureUV) {
            if (f->sh.isTextured) {
                row.push_back(fmt::format("{:.3g}x{:.3g} ({:.3g} x {:.3g})", f->sh.texWidth, f->sh.texHeight, f->sh.texWidth_precise, f->sh.texHeight_precise));
            }
            else {
                row.push_back("None");
            }
        }
        if (meshSample) {
            if (IsEqual(f->tRatioU, f->tRatioV)) {
                row.push_back(fmt::format("{:.5g}", f->tRatioU));
            }
            else {
                row.push_back(fmt::format("{:.3g} x {:.3g}", f->tRatioU, f->tRatioV));
            }
        }
        if (textureRecord) {
            std::string rec = "";
            if (f->sh.countDes) rec += "DES";
            if (f->sh.countAbs) rec += rec == "" ? "ABS" : "+ABS";
            if (f->sh.countRefl) rec += rec == "" ? "REFL" : "+REFL";
            if (f->sh.countTrans) rec += rec == "" ? "TRANS" : "+TRANS";
            row.push_back(rec == "" ? "None" : rec);;
        }
        if (memory) {
            row.push_back(FormatMemory(f->GetTexRamSize(1 + mApp->worker.interfaceMomentCache.size())));
        }
        if (planarity) {
            if (f->planarityError < 1E-15) {
                row.push_back("0");
            }
            else {
                row.push_back(fmt::format("{:.3g}", f->planarityError));
            }
        }
        if (profile) {
            std::string prof = "";
            switch (f->sh.profileType)
            {
            case 0:
                prof = "None"; break;
            case 1:
                prof = u8"Pressure (u)"; break; // TODO Unicode
            case 2:
                prof = u8"Pressure (v)"; break;
            case 3:
                prof = "Angular"; break;
            case 4:
                prof = "Speed distr."; break;
            case 5:
                prof = "Ort. velocity"; break;
            case 6:
                prof = "Ten. velocity"; break;
            default:
                prof = "ERROR"; break;
            }
            row.push_back(prof);
        }
        if (impingRate) {
            row.push_back(fmt::format("{:.5g}", f->facetHitCache.nbHitEquiv / f->GetArea() * (1E4 * mApp->worker.GetMoleculesPerTP(mApp->worker.displayedMoment))));
        }
        if (particleDensity) {
            row.push_back(fmt::format("{:.5g}", f->facetHitCache.nbHitEquiv / f->GetArea() * 
                (1E4*mApp->worker.GetMoleculesPerTP(mApp->worker.displayedMoment)*f->DensityCorrection())));
        }
        if (gasDensity) {
            row.push_back(fmt::format("{:.5g}", f->facetHitCache.sum_1_per_ort_velocity / f->GetArea()
                * (1E4 * mApp->worker.GetMoleculesPerTP(mApp->worker.displayedMoment) * f->DensityCorrection())
                * mApp->worker.model->sp.gasMass / 1000.0 / 6E23
            ));
        }
        if (pressure) {
            row.push_back(fmt::format("{:.5g}", (f->facetHitCache.sum_v_ort
                * 1E4 * mApp->worker.GetMoleculesPerTP(mApp->worker.displayedMoment)
                * (mApp->worker.model->sp.gasMass / 1000 / 6E23) * 0.0100)
                / f->GetArea()
            ));
        }
        if (speed) {
            row.push_back(fmt::format("{:.5g}", (f->facetHitCache.nbHitEquiv + static_cast<double>(f->facetHitCache.nbDesorbed)) / f->facetHitCache.sum_1_per_velocity));
        }
        if (mcHits) {
            row.push_back(fmt::format("{}", f->facetHitCache.nbMCHit));
        }
        if (equivHits) {
            row.push_back(fmt::format("{}", f->facetHitCache.nbHitEquiv));
        }
        if (des) {
            row.push_back(fmt::format("{}", f->facetHitCache.nbDesorbed));
        }
        if (equivAbs) {
            row.push_back(fmt::format("{}", f->facetHitCache.nbAbsEquiv));
        }
        if (force) {
            auto force = f->facetHitCache.impulse * mApp->worker.GetMoleculesPerTP(mApp->worker.displayedMoment) * (mApp->worker.model->sp.gasMass / 1000 / 6E23);
            row.push_back(fmt::format("{:.4g} N ({:.4g},{:.4g},{:.4g})", force.Norme(), force.x, force.y, force.z));
        }
        if (force2) {
            auto force_sqr = f->facetHitCache.impulse_square * mApp->worker.GetMoleculesPerTP(mApp->worker.displayedMoment) * Square(mApp->worker.model->sp.gasMass / 1000 / 6E23);
            if (mApp->worker.displayedMoment != 0) {
                force_sqr = 1.0 / mApp->worker.interfaceMomentCache[mApp->worker.displayedMoment - 1].window * force_sqr; //force2 divided by dt^2 to get N^2 
            }
            row.push_back(fmt::format("{:.4g} N^2 ({:.4g},{:.4g},{:.4g})", force_sqr.Norme(), force_sqr.x, force_sqr.y, force_sqr.z));
        }
        if (torque) {
            auto torque = f->facetHitCache.impulse_momentum * mApp->worker.GetMoleculesPerTP(mApp->worker.displayedMoment) * (mApp->worker.model->sp.gasMass / 1000 / 6E23) * 0.01; //0.01: N*cm to Nm
            row.push_back(fmt::format("{:.4g} Nm ({:.4g},{:.4g},{:.4g})", torque.Norme(), torque.x, torque.y, torque.z));
        }

        content.push_back(row);
    }
}
