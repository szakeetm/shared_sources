#include "imguiTesting.h"
#include "imgui_test_engine/imgui_te_ui.h"
#include "imgui_test_engine/imgui_te_internal.h"
#if defined(MOLFLOW)
#include "../src/MolFlow.h"
#else
#include "../src/SynRad.h"
#endif
#include "ImguiWindow.h"
#include "ImguiMenu.h"
#include "ImguiExtensions.h"
#include "imgui_stdlib/imgui_stdlib.h"

void ImTest::Init(Interface* mApp_)
{
    mApp = mApp_;
    interfGeom = mApp->worker.GetGeometry();
    engine = ImGuiTestEngine_CreateContext();
    ImGuiTestEngineIO& test_io = ImGuiTestEngine_GetIO(engine);
    test_io.ConfigVerboseLevel = ImGuiTestVerboseLevel_Info;
    test_io.ConfigVerboseLevelOnError = ImGuiTestVerboseLevel_Debug;
    ImGuiTestEngine_Start(engine, ImGui::GetCurrentContext());
    ImGuiTestEngine_InstallDefaultCrashHandler();
    RegisterTests();
}

bool ImTest::StopEngine()
{
    ImGuiTestEngine_Stop(engine);
    return false;
}

bool ImTest::DestroyContext()
{
    ImGuiTestEngine_DestroyContext(engine);
    return false;
}

void ImTest::Draw()
{
    if (!drawn) return;
    mApp->imWnd->forceDrawNextFrame = true;
    ImGui::StyleColorsDark(); // on light background the log is not readable
    ImGuiTestEngine_ShowTestEngineWindows(engine, &drawn);
    DrawPresetControl();
    ImGui::StyleColorsLight();
}

void ImTest::PostSwap()
{
    ImGuiTestEngine_PostSwap(engine); // normal operation

    // execute queued calls
    {
        LockWrapper lW(mApp->imguiRenderLock);
        while (callQueue.size() > 0) {
            std::function<void()> f = callQueue.front();
            f();
            callQueue.pop();
        }
    }

    // Command-line run test

    if (running && ImGuiTestEngine_IsTestQueueEmpty(engine)) { // test run finished
        int tested, succeeded;
        ImGuiTestEngine_GetResult(engine, tested, succeeded);
        if (tested != succeeded) {
            ImVector<ImGuiTest*> tests;
            ImGuiTestEngine_GetTestList(engine, &tests);
            for (const ImGuiTest* test : tests) {
                bool outputLog = false;
                if (test->Output.Status == 3) {
                    std::cout << fmt::format("Test {}, group {} in scenario {} failed:", test->Name, test->Category, ranScenarios) << std::endl;
                    outputLog = true;
                }
                if (test->Output.Status == 4) {
                    std::cout << fmt::format("Test {}, group {} in scenario {} could not complete:", test->Name, test->Category, ranScenarios) << std::endl;
                    outputLog = true;
                }
                if (outputLog) {
                    std::string log;
                    for (int i = 0; i < test->Output.Log.Buffer.Buf.Size; i++) {
                        log += (test->Output.Log.Buffer.Buf[i]);
                    }
                    std::cout << log << std::endl;
                }
            }
            result = false;
        }
        running = false;
        ranScenarios++;
        if (ranScenarios >= numberOfScenarios) { // completed all scenarios
            std::cout << "Stopping" << std::endl;
            exit(result);
        }
        else {
            RunTests(); // run tests again (changes test scenario at the beginning)
        }
    }
    if (running) {
        mApp->imWnd->forceDrawNextFrame = true;
    }
}

void ImTest::RunTests()
{
    //engine->IO.ConfigRunSpeed = ImGuiTestRunSpeed_Normal;
    ConfigureGeometry(static_cast<Configuration>(ranScenarios));
    std::cout << "Starting tests in scenario " << ranScenarios << std::endl;
    for (int n = 0; n < engine->TestsAll.Size; n++)
    {
        ImGuiTest* test = engine->TestsAll[n];
        ImGuiTestEngine_QueueTest(engine, test, ImGuiTestRunSpeed_Normal | ImGuiTestRunFlags_None);
    }
    running = true;
}

bool ImTest::ConfigureGeometry(Configuration index)
{
    switch (index) {
    case empty:
        if (static_cast<MolFlow*>(mApp)->worker.GetGeometry()->GetNbFacet() == 0) break; // don't do if geometry already is empty
        ImMenu::NewGeometry();
        currentConfig = index;
        break;
    case qPipe:
    {
        LockWrapper myLock(mApp->imguiRenderLock);
        static_cast<MolFlow*>(mApp)->BuildPipe(5, 5);
    }
    currentConfig = index;
    break;
    case profile:
        ConfigureGeometry(qPipe);
        SetFacetProfile(2, 1);
        SetFacetProfile(4, 3);
        SetFacetProfile(6, 5);
        currentConfig = index;
        break;
    default:
        return false;
    }
    return true;
}

void ImTest::DrawPresetControl()
{
    ImGui::Begin("Test preset control", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::AlignTextToFramePadding();
    ImGui::HelpMarker("Select preset in the combo and press apply");
    ImGui::SameLine();
    static std::string previewVal = "Empty";
    static Configuration selection = empty;
    if (ImGui::BeginCombo("###PresetSelector", previewVal)) {
        if (ImGui::Selectable("Empty", selection == empty)) {
            previewVal = "Empty";
            selection = empty;
        }
        if (ImGui::Selectable("Quick Pipe", selection == qPipe)) {
            previewVal = "Quick Pipe";
            selection = qPipe;
        }
        if (ImGui::Selectable("Profile", selection == profile)) {
            previewVal = "Profile";
            selection = profile;
        }
        ImGui::EndCombo();
    } ImGui::SameLine();
    if (ImGui::Button("Apply")) {
        ConfigureGeometry(selection);
    }
    ImGui::End();
}

void ImTest::SelectFacet(size_t idx, bool shift, bool ctrl)
{
    std::function<void()> f = [this, idx, shift, ctrl]() {
        interfGeom->SetSelection({ idx }, shift, ctrl);
        };
    callQueue.push(f);
}

void ImTest::SelectFacet(std::vector<size_t> idxs, bool shift, bool ctrl)
{
    std::function<void()> f = [this, idxs, shift, ctrl]() {
        interfGeom->SetSelection(idxs, shift, ctrl);
        };
    callQueue.push(f);
}

void ImTest::SelectVertex(size_t idx, bool add)
{
    std::function<void()> f = [this, idx, add]() {
        if (!add) interfGeom->EmptySelectedVertexList();
        interfGeom->SelectVertex(idx);
        interfGeom->UpdateSelection();
        };
    callQueue.push(f);
}

void ImTest::SelectVertex(std::vector<size_t> idxs, bool add)
{
    if (!add) {
        std::function<void()> f = [this]() {
            interfGeom->EmptySelectedVertexList();
            };
        callQueue.push(f);
    }
    for (const size_t idx : idxs) {
        SelectVertex(idx, true);
    }
}

void ImTest::DeselectAll()
{
    std::function<void()> f = [this]() {
        interfGeom->UnselectAll();
        };
    callQueue.push(f);
    DeselectAllVerticies();
}

void ImTest::DeselectAllVerticies()
{
    std::function<void()> f = [this]() {
        interfGeom->UnselectAllVertex();
        };
    callQueue.push(f);
}

bool ImTest::SetFacetProfile(size_t facetIdx, int profile)
{
    InterfaceFacet* f = interfGeom->GetFacet(facetIdx);
    if (f == nullptr) return false;
    f->sh.profileType = profile;
    f->sh.isProfile = true;
    {
        LockWrapper lW(mApp->imguiRenderLock);
        mApp->worker.MarkToReload();
        mApp->UpdateFacetParams(false);
        if (mApp->imWnd && mApp->imWnd->profPlot.IsVisible()) mApp->imWnd->profPlot.Refresh();
    }
}

void ImTest::RegisterTests()
{
    ImGuiTest* t = NULL;
    t = IM_REGISTER_TEST(engine, "SelectionMenu", "Smart Selection");
    t->TestFunc = [this](ImGuiTestContext* ctx) {
        // set start state
        mApp->imWnd->smartSelect.planeDiff = 30;
        mApp->imWnd->smartSelect.planeDiffInput = "30";
        mApp->imWnd->smartSelect.enabledToggle = false;
        // navigate to window
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("###Selection/Smart Select facets...");
        ctx->SetRef("Smart Selection");
        // input invalid value
        ctx->ItemClick("##planeDiff");
        ctx->KeyCharsReplaceEnter("abc");
        ctx->ItemClick("###Analyze");
        // assert bahaviour
        IM_CHECK_EQ(false, mApp->imWnd->smartSelect.enabledToggle);
        IM_CHECK_EQ(30, mApp->imWnd->smartSelect.planeDiff);
        // deal with info popup
        ctx->SetRef("Error");
        ctx->ItemClick("  Ok  ");
        ctx->SetRef("Smart Selection");
        // sci notation
        ctx->ItemClick("##planeDiff");
        ctx->KeyCharsReplaceEnter("2e1");
        ctx->ItemClick("###Analyze");
        IM_CHECK_EQ(true, mApp->imWnd->smartSelect.enabledToggle);
        IM_CHECK_EQ(20, mApp->imWnd->smartSelect.planeDiff);
        // input valid value
        ctx->ItemClick("##planeDiff");
        ctx->KeyCharsReplaceEnter("30");
        ctx->ItemClick("###Analyze");
        IM_CHECK_EQ(true, mApp->imWnd->smartSelect.enabledToggle);
        IM_CHECK_EQ(30, mApp->imWnd->smartSelect.planeDiff);
        // toggle checkbox
        ctx->ItemClick("Enable smart selection");
        IM_CHECK_EQ(false, mApp->imWnd->smartSelect.enabledToggle);
        // close window
        ctx->ItemClick("#CLOSE");
        };
    t = IM_REGISTER_TEST(engine, "SelectionMenu", "Select All");
    t->TestFunc = [this](ImGuiTestContext* ctx) {
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("###Selection/Select All Facets");
        IM_CHECK_EQ(interfGeom->GetNbFacet(), interfGeom->GetNbSelectedFacets());
        };
    t = IM_REGISTER_TEST(engine, "SelectionMenu", "Select by Number");
    t->TestFunc = [this](ImGuiTestContext* ctx) {
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("###Selection/Select by Facet Number...");
        ctx->SetRef("Select facet(s) by number");
        if (currentConfig != empty) {
            SelectFacet({ 0,1,2,3,4,5,6 });
            ctx->ItemClick("##1");
            ctx->KeyCharsReplaceEnter("1-7");
            ctx->ItemClick("  Remove from selection  ");
            IM_CHECK_EQ(0, interfGeom->GetNbSelectedFacets());
            ctx->ItemClick("##1");
            ctx->KeyCharsReplaceEnter("1");
            ctx->ItemClick("  Select  ");
            IM_CHECK_EQ(1, interfGeom->GetNbSelectedFacets());
            ctx->ItemClick("##1");
            ctx->KeyCharsReplaceEnter("2");
            ctx->ItemClick("  Add to selection  ");
            IM_CHECK_EQ(2, interfGeom->GetNbSelectedFacets());
        }
        ctx->ItemClick("##1");
        ctx->KeyCharsReplaceEnter("");
        ctx->ItemClick("  Select  ");
        ctx->ItemClick("//Error/  Ok  ");
        ctx->ItemClick("#CLOSE");
        };
    t = IM_REGISTER_TEST(engine, "SelectionMenu", "Select Sticking");
    t->TestFunc = [this](ImGuiTestContext* ctx) {
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("###Selection/Select Sticking");
        if (currentConfig == empty) {
            IM_CHECK_EQ(0, interfGeom->GetNbSelectedFacets());
        }
        else if (currentConfig == qPipe || currentConfig == profile) {
            IM_CHECK_EQ(2, interfGeom->GetNbSelectedFacets());
        }
        };
    t = IM_REGISTER_TEST(engine, "SelectionMenu", "Select Transparent");
    t->TestFunc = [this](ImGuiTestContext* ctx) {
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("###Selection/Select Transparent");
        // TODO - Test Configuration with transparent facets
        if (currentConfig == empty || currentConfig == qPipe || currentConfig == profile) {
            IM_CHECK_EQ(0, interfGeom->GetNbSelectedFacets());
        }
        };
    t = IM_REGISTER_TEST(engine, "SelectionMenu", "Select 2 Sided");
    t->TestFunc = [this](ImGuiTestContext* ctx) {
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("###Selection/Select 2 sided");
        // TODO - Test Configuration with 2-sided facets
        if (currentConfig == empty || currentConfig == qPipe || currentConfig == profile) {
            IM_CHECK_EQ(0, interfGeom->GetNbSelectedFacets());
        }
        };
    t = IM_REGISTER_TEST(engine, "SelectionMenu", "Select Texture");
    t->TestFunc = [this](ImGuiTestContext* ctx) {
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("###Selection/Select Texture");
        // TODO - Test Configuration with textured facets
        if (currentConfig == empty || currentConfig == qPipe || currentConfig == profile) {
            IM_CHECK_EQ(0, interfGeom->GetNbSelectedFacets());
        }
        };
    t = IM_REGISTER_TEST(engine, "SelectionMenu", "Select By Texture Type");
    t->TestFunc = [this](ImGuiTestContext* ctx) {
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("###Selection/Select by Texture type...");
        ctx->SetRef("Select facets by texture properties");
        // TODO test tristate behaviour & test configuration with different texture types
        if (currentConfig == empty || currentConfig == qPipe || currentConfig == profile) {
            IM_CHECK_EQ(0, interfGeom->GetNbSelectedFacets());
        }
        ctx->ItemClick("#CLOSE");
        };
    t = IM_REGISTER_TEST(engine, "SelectionMenu", "Select By Facet Result");
    t->TestFunc = [this](ImGuiTestContext* ctx) {
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("###Selection/Select by facet result...");
        ctx->SetRef("Select facets by simulation result");
        ctx->ItemClick("Select");
        ctx->ItemClick("Add to sel.");
        ctx->ItemClick("Remove from sel.");
        ctx->ItemClick("##SFBSR/##minHits");
        ctx->KeyCharsReplaceEnter("0");
        ctx->ItemClick("Select");
        ctx->ItemClick("##SFBSR/##minHits");
        ctx->KeyCharsReplaceEnter("a");
        ctx->ItemClick("Select");
        ctx->SetRef("Error");
        ctx->ItemClick("  Ok  ");
        ctx->SetRef("Select facets by simulation result");
        ctx->ItemClick("##SFBSR/##minHits");
        ctx->KeyCharsReplaceEnter("");
        ctx->ItemClick("#CLOSE");
        };
    t = IM_REGISTER_TEST(engine, "SelectionMenu", "Select Link facets");
    t->TestFunc = [this](ImGuiTestContext* ctx) {
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("###Selection/Select link facets");
        // TODO test configuration with link facets
        if (currentConfig == empty || currentConfig == qPipe || currentConfig == profile) {
            IM_CHECK_EQ(0, interfGeom->GetNbSelectedFacets());
        }
        };
    t = IM_REGISTER_TEST(engine, "SelectionMenu", "Select teleport facets");
    t->TestFunc = [this](ImGuiTestContext* ctx) {
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("###Selection/Select teleport facets");
        // TODO test configuration with teleport facets
        if (currentConfig == empty || currentConfig == qPipe || currentConfig == profile) {
            IM_CHECK_EQ(0, interfGeom->GetNbSelectedFacets());
        }
        };
    t = IM_REGISTER_TEST(engine, "SelectionMenu", "Select non planar facets");
    t->TestFunc = [this](ImGuiTestContext* ctx) {
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("###Selection/Select non planar facets");
        ctx->SetRef("Select non planar facets");
        ctx->ItemClick("Planarity larger than");
        ctx->KeyCharsReplace("0.001");
        ctx->ItemClick("  OK  ");
        IM_CHECK_EQ(mApp->planarityThreshold, 0.001);
        // TODO test configuration with non-planar facets
        if (currentConfig == empty || currentConfig == qPipe || currentConfig == profile) {
            IM_CHECK_EQ(0, interfGeom->GetNbSelectedFacets());
        }

        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("###Selection/Select non planar facets");
        ctx->SetRef("Select non planar facets");
        ctx->ItemClick("Planarity larger than");
        ctx->KeyCharsReplace("1e-3");
        ctx->ItemClick("  OK  ");
        IM_CHECK_EQ(mApp->planarityThreshold, 1e-3);
        if (currentConfig == empty || currentConfig == qPipe || currentConfig == profile) {
            IM_CHECK_EQ(0, interfGeom->GetNbSelectedFacets());
        }

        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("###Selection/Select non planar facets");
        ctx->SetRef("Select non planar facets");
        ctx->ItemClick("Planarity larger than");
        ctx->KeyCharsReplace("abc");
        ctx->ItemClick("  OK  ");
        IM_CHECK_EQ(mApp->planarityThreshold, 1e-3);
        ctx->SetRef("Error");
        ctx->ItemClick("  Ok  ");

        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("###Selection/Select non planar facets");
        ctx->SetRef("Select non planar facets");
        ctx->ItemClick("Planarity larger than");
        ctx->KeyCharsReplace("10");
        ctx->ItemClick("  Cancel  ");
        IM_CHECK_EQ(mApp->planarityThreshold, 1e-3);
        };
    t = IM_REGISTER_TEST(engine, "SelectionMenu", "Select non simple");
    t->TestFunc = [this](ImGuiTestContext* ctx) {
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("###Selection/Select non simple facets");
        // TODO test configuration with non-simple facets
        if (currentConfig == empty || currentConfig == qPipe || currentConfig == profile) {
            IM_CHECK_EQ(0, interfGeom->GetNbSelectedFacets());
        }
        };
    t = IM_REGISTER_TEST(engine, "SelectionMenu", "Select Invert");
    t->TestFunc = [this](ImGuiTestContext* ctx) {
        int expected = 0;
        if (currentConfig != empty) {
            SelectFacet(0);
            expected = 6;
        }
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("###Selection/Invert selection");
        IM_CHECK_EQ(expected, interfGeom->GetNbSelectedFacets());
        };
    t = IM_REGISTER_TEST(engine, "SelectionMenu", "Selection Memory");
    t->TestFunc = [this](ImGuiTestContext* ctx) {
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("###Selection/Clear memorized/Clear All");
        ctx->SetRef("Clear all?");
        ctx->KeyDown(ImGuiKey_Enter);
        IM_CHECK_EQ(mApp->selections.size(), 0);
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("###Selection/Memorize selection to/Add new...");

        ctx->SetRef("Enter selection name");
        ctx->KeyDown(ImGuiKey_Enter);
        IM_CHECK_EQ(mApp->selections.size(), 1);

        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("###Selection/Clear memorized/Clear All");
        ctx->SetRef("Clear all?");
        ctx->KeyDown(ImGuiKey_Enter);
        IM_CHECK_EQ(mApp->selections.size(), 0);
        };
    t = IM_REGISTER_TEST(engine, "SelectionMenu", "Select Desorption");
    t->TestFunc = [this](ImGuiTestContext* ctx) {
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("###Selection/Select Desorption");
        if (currentConfig == empty) {
            IM_CHECK_EQ(0, interfGeom->GetNbSelectedFacets());
        }
        if (currentConfig == qPipe || currentConfig == profile) {
            IM_CHECK_EQ(1, interfGeom->GetNbSelectedFacets());
        }
        };
    t = IM_REGISTER_TEST(engine, "SelectionMenu", "Select Outgassing Map");
    t->TestFunc = [this](ImGuiTestContext* ctx) {
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("###Selection/Select Outgassing Map");
        if (currentConfig == empty || currentConfig == qPipe || currentConfig == profile) {
            IM_CHECK_EQ(0, interfGeom->GetNbSelectedFacets());
        }
        };
    t = IM_REGISTER_TEST(engine, "SelectionMenu", "Select Reflective");
    t->TestFunc = [this](ImGuiTestContext* ctx) {
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("###Selection/Select Reflective");
        if (currentConfig == empty) {
            IM_CHECK_EQ(0, interfGeom->GetNbSelectedFacets());
        }
        if (currentConfig == qPipe || currentConfig == profile) {
            IM_CHECK_EQ(5, interfGeom->GetNbSelectedFacets());
        }
        };
    t = IM_REGISTER_TEST(engine, "ToolsMenu", "Formula editor + Convergence Plotter"); // this test fails in fast mode for unknown reason
    t->TestFunc = [this](ImGuiTestContext* ctx) {
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("###Tools/###Formula editor");
        ctx->SetRef("Formula editor");
        // -----
        ctx->ItemClick("**/##FL/##NE");
        ctx->KeyCharsReplace("10");
        ctx->ItemClick("**/##FL/##NN");
        ctx->KeyCharsReplace("A");
        ctx->ItemClick("**/##FL/ Add ");
        ctx->ItemClick("**/##FL/##NE");
        ctx->KeyCharsReplace("20");
        ctx->ItemClick("**/##FL/##NN");
        ctx->KeyCharsReplace("B");
        ctx->ItemClick("**/##FL/ Add ");
        ctx->ItemClick("Move Up");
        ctx->ItemClick("Move Down");
        ctx->ItemClick("Open convergence plotter >>");
        // ----- CONVERGENCE PLOTTER
        ctx->SetRef("Convergence Plotter");
        ctx->ItemClick("Add curve");
        ctx->ItemClick("Remove curve");
        ctx->SetRef("Error");
        ctx->ItemClick("  Ok  ");
        ctx->SetRef("Convergence Plotter");
        IM_CHECK_EQ(mApp->imWnd->convPlot.data.size(), 0);
        // Export Menu
        ctx->MenuClick("Export/All to clipboard");
        ctx->SetRef("Error");
        ctx->ItemClick("  Ok  ");
        ctx->SetRef("Convergence Plotter");
        ctx->MenuClick("Export/Plotted to clipboard");
        ctx->SetRef("Error");
        ctx->ItemClick("  Ok  ");
        ctx->SetRef("Convergence Plotter");
        ctx->MenuClick("Export/Plotted to file");
        ctx->SetRef("Error");
        ctx->ItemClick("  Ok  ");
        ctx->SetRef("Convergence Plotter");
        // View Menu
        ctx->MenuClick("View/Log Y");
        IM_CHECK_EQ(mApp->imWnd->convPlot.logY, true);
        ctx->MenuClick("View/Log Y");
        IM_CHECK_EQ(mApp->imWnd->convPlot.logY, false);
        ctx->MenuClick("View/Datapoints");
        IM_CHECK_EQ(mApp->imWnd->convPlot.showDatapoints, true);
        ctx->MenuClick("View/Datapoints");
        IM_CHECK_EQ(mApp->imWnd->convPlot.showDatapoints, false);
        ctx->MenuClick("View/##lineWidth");
        ctx->KeyCharsReplaceEnter("abc");
        ctx->MenuClick("View/##lineWidth");
        ctx->KeyCharsReplaceEnter("-1");
        ctx->MenuClick("View/##plotMax");
        ctx->KeyCharsReplaceEnter("abc");
        ctx->MenuClick("View/##plotMax");
        ctx->KeyCharsReplaceEnter("-1");
        ctx->MenuClick("View/Display hovered value");
        ctx->MenuClick("View/Display hovered value");
        ctx->MenuClick("Custom Plot/##expressionInput");
        ctx->KeyCharsReplace("x");
        ctx->MenuClick("Custom Plot/-> Plot expression");
        ctx->MenuClick("Custom Plot/##expressionInput");
        ctx->KeyCharsReplace("");
        ctx->MenuClick("Custom Plot/-> Plot expression");

        ctx->ComboClick("##Formula Picker/[A]10");
        ctx->ItemClick("Add curve");
        ctx->ComboClick("##Formula Picker/[B]20");
        ctx->ItemClick("Add curve");
        ctx->MouseMoveToPos(ImVec2(100, 100));
        ctx->MenuClick("Export/Plotted to clipboard");
        ctx->MouseMoveToPos(ImVec2(100, 100));
        ctx->MenuClick("Export/All to clipboard");
        ctx->ItemClick("Remove curve");
        ctx->ItemClick("Remove all");

        ctx->ItemClick("#CLOSE");
        // -----
        ctx->SetRef("Formula editor");
        if (currentConfig == empty) {
            ctx->ItemClick("**/##FL/1");
            ctx->ItemClick("**/##FL/##changeExp");
            ctx->KeyCharsReplace("");
            ctx->ItemClick("**/##FL/##changeNam");
            ctx->KeyCharsReplaceEnter("");
            ctx->ItemClick("**/##FL/1");
            ctx->ItemClick("**/##FL/##changeExp");
            ctx->KeyCharsReplace("");
            ctx->ItemClick("**/##FL/##changeNam");
            ctx->KeyCharsReplaceEnter("");
        } else
        {
            ctx->ItemClick("**/##FL/2");
            ctx->ItemClick("**/##FL/##changeExp");
            ctx->KeyCharsReplace("");
            ctx->ItemClick("**/##FL/##changeNam");
            ctx->KeyCharsReplaceEnter("");
            ctx->ItemClick("**/##FL/2");
            ctx->ItemClick("**/##FL/##changeExp");
            ctx->KeyCharsReplace("");
            ctx->ItemClick("**/##FL/##changeNam");
            ctx->KeyCharsReplaceEnter("");
        }
        // -----
        ctx->MouseClick(1);
        ctx->ItemClick("/**/Copy table");
        ctx->MouseClick(1);
        ctx->ItemClick("/**/Copy table (for all time moments)");
        // -----
        ctx->ItemClick("Recalculate now");
        ctx->ItemClick("Record values for convergence");
        ctx->ItemClick("Auto-update formulas");
        ctx->ItemClick("Syntax help");
        ctx->SetRef("Formula Editor Syntax Help");
        ctx->ItemClick("Close");
        ctx->SetRef("Formula editor");
        ctx->ItemClick("Record values for convergence");
        ctx->ItemClick("Auto-update formulas");
        ctx->SetRef("Formula editor");
        ctx->ItemClick("#CLOSE");
        };
    t = IM_REGISTER_TEST(engine, "ToolsMenu", "Texture plotter");
    t->TestFunc = [this](ImGuiTestContext* ctx) {
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("###Tools/Texture Plotter ...");
        ctx->SetRef("###TexturePlotter");
        ctx->MenuClick("Export/To clipboard");
        ctx->SetRef("Error");
        ctx->ItemClick("  Ok  ");
        ctx->SetRef("###TexturePlotter");
        ctx->MenuClick("Export/To file");
        ctx->SetRef("Error");
        ctx->ItemClick("  Ok  ");
        ctx->SetRef("###TexturePlotter");
        ctx->MenuClick("View/Resizable columns");
        ctx->MenuClick("View/Fit to window");
        ctx->MenuClick("View/Autosize to window");
        IM_CHECK_EQ(mApp->imWnd->textPlot.resizableColumns, false);
        IM_CHECK_EQ(mApp->imWnd->textPlot.fitToWindow, true);
        ctx->SetRef("###TexturePlotter");
        ctx->MouseMoveToPos(ImVec2(100, 100));
        ctx->MenuClick("View/Autosize to data");
        IM_CHECK_EQ(mApp->imWnd->textPlot.resizableColumns, false);
        IM_CHECK_EQ(mApp->imWnd->textPlot.fitToWindow, false);
        ctx->MenuClick("Data");
        ctx->ItemClick("Find Max");
        for (int i = 0; i < mApp->imWnd->textPlot.comboOpts.size(); i++) {
            ctx->ComboClick(("##View/###" + std::to_string(i)).c_str());
        }
        ctx->ItemClick("#CLOSE");
        // TODO test with a texture selected
        };
    t = IM_REGISTER_TEST(engine, "ToolsMenu", "Profile plotter");
    t->TestFunc = [this](ImGuiTestContext* ctx) {
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("###Tools/Profile Plotter ...");
        ctx->SetRef("Profile Plotter");
        ctx->ItemClick("Remove all");
        ctx->ItemClick("##manualFacetSel");
        ctx->KeyCharsReplaceEnter("");
        ctx->MenuClick("Export/To clipboard");
        ctx->SetRef("Error");
        ctx->ItemClick("  Ok  ");
        ctx->SetRef("Profile Plotter");
        ctx->MenuClick("Export/To file");
        ctx->SetRef("Error");
        ctx->ItemClick("  Ok  ");
        ctx->SetRef("Profile Plotter");

        if (currentConfig == profile) {
            ctx->ComboClick("##ProfilePlotterCombo/Select [v] or type->");
            ctx->ItemClick("##manualFacetSel");
            ctx->KeyCharsReplaceEnter("3");
            ctx->ItemClick("Add Curve");
            ctx->ComboClick("##ProfilePlotterCombo/###profileCombo4");
            ctx->ItemClick("Add Curve");
            ctx->ComboClick("##ProfilePlotterCombo/###profileCombo6");
            ctx->ItemClick("Add Curve");
        }

        ctx->MenuClick("View/Log Y");
        IM_CHECK_EQ(mApp->imWnd->profPlot.setLog, true);
        ctx->MenuClick("View/Log Y");
        IM_CHECK_EQ(mApp->imWnd->profPlot.setLog, false);
        ctx->MenuClick("View/Datapoints");
        IM_CHECK_EQ(mApp->imWnd->profPlot.showDatapoints, true);
        ctx->MenuClick("View/Datapoints");
        IM_CHECK_EQ(mApp->imWnd->profPlot.showDatapoints, false);
        ctx->MenuClick("View/##lineWidth");
        ctx->KeyCharsReplaceEnter("abc");
        ctx->MenuClick("View/##lineWidth");
        ctx->KeyCharsReplaceEnter("-1");
        ctx->MenuClick("View/Display hovered value");
        ctx->MenuClick("View/Display hovered value");
        ctx->MenuClick("View/Identify profiles in geometry");
        ctx->MenuClick("View/Identify profiles in geometry");
        ctx->MenuClick("Custom Plot/##expressionInput");
        ctx->KeyCharsReplace("x");
        ctx->MenuClick("Custom Plot/-> Plot expression");
        ctx->MenuClick("Custom Plot/##expressionInput");
        ctx->KeyCharsReplaceEnter("");
        ctx->MenuClick("Custom Plot/-> Plot expression");
        ctx->ItemClick("Show Facet");
        if (mApp->imWnd->profPlot.manualFacetSel != "3") {
            ctx->SetRef("Error");
            ctx->ItemClick("  Ok  ");
            ctx->SetRef("Profile Plotter");
        }
        ctx->ItemClick("Add Curve");
        ctx->SetRef("Error");
        ctx->ItemClick("  Ok  ");
        ctx->SetRef("Profile Plotter");
        ctx->ComboClickAll("##View");
        ctx->ItemClick("Remove Curve");
        ctx->ItemClick("Remove all");
        ctx->ItemClick("Select plotted facets");
        ctx->ItemClick("#CLOSE");
        };
    t = IM_REGISTER_TEST(engine, "ToolsMenu", "Histogram plotter"); // this test faisl in fast mode for unknown reasons
    t->TestFunc = [this](ImGuiTestContext* ctx) {
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("###Tools/Histogram Plotter...");
        ctx->SetRef("Histogram Plotter");
        ctx->ItemClick("Histogram types/Flight distance before absorption");
        ctx->ItemClick("Histogram types/Flight time before absorption");
        ctx->ItemClick("Histogram types/Bounces before absorption");
        ctx->ItemClick("<- Show Facet");
        ctx->ItemClick("Add");
        ctx->ItemClick("Remove");
        ctx->ItemClick("Remove all");
        ctx->ItemClick("Normalize");
        ctx->ItemClick("Normalize");
        ctx->ItemClick("Log Y");
        ctx->ItemClick("Log Y");
        ctx->ItemClick("Log X");
        ctx->ItemClick("Log X");
        ctx->MenuClick("Export/All to clipboard");
        ctx->MouseMoveToPos(ImVec2(100, 100));
        ctx->MenuClick("Export/Plotted to clipboard");
        ctx->SetRef("Error");
        ctx->ItemClick("  Ok  ");
        ctx->SetRef("Histogram Plotter");
        ctx->ItemClick("<< Hist settings");
        /* // cannot target items because of name conflicts (limitation of ImGui Test Engine)
        ctx->SetRef("Histogram Settings");
        ctx->ItemClick("Global Settings/Record bounces until absorbtion");
        ctx->ItemClick("Apply");
        ctx->SetRef("Histogram Plotter");
        ctx->ComboClick("Global");
        ctx->ItemClick("Add");
        ctx->ItemClick("Remove");
        ctx->ItemClick("Add");
        ctx->SetRef("Histogram Settings");
        ctx->ItemClick("Facet Settings/Record bounces until absorbtion");
        ctx->ItemClick("Apply");
        */
        ctx->SetRef("Histogram Plotter");
        ctx->ItemClick("<< Hist settings");
        ctx->ItemClick("#CLOSE");
        };
    t = IM_REGISTER_TEST(engine, "ToolsMenu", "Texture scaling");
    t->TestFunc = [this](ImGuiTestContext* ctx) {
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("###Tools/Texture scaling...");
        ctx->SetRef("###TextureScaling");

        ctx->ItemClick("**/Autoscale");
        ctx->ItemClick("**/Autoscale");
        ctx->ItemClick("**/Use colors");
        ctx->ItemClick("**/Use colors");
        ctx->ItemClick("**/Logarithmic scale");
        ctx->ItemClick("**/Logarithmic scale");

        ctx->ComboClickAll("##Show");
        ctx->ItemClick("#CLOSE");
        };
    // VIEW
    t = IM_REGISTER_TEST(engine, "ViewMenu", "FullScreen");
    t->TestFunc = [this](ImGuiTestContext* ctx) {
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("View/###Full Screen");
        ctx->MouseMoveToPos(ImVec2(100, 100));
        ctx->MenuClick("View/###Full Screen");
        };
    // geometry altering tests (to be run last)
    t = IM_REGISTER_TEST(engine, "TestMenu", "Quick Pipe");
    t->TestFunc = [this](ImGuiTestContext* ctx) {
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("Test/Quick Pipe");
        if (mApp->changedSinceSave) {
            ctx->SetRef("File not saved");
            ctx->ItemClick("  No  ");
        }
        IM_CHECK_EQ(interfGeom->GetNbFacet(), 7);
        };
    t = IM_REGISTER_TEST(engine, "FileMenu", "New, empty geometry");
    t->TestFunc = [this](ImGuiTestContext* ctx) {
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("###File/###NewGeom");
        if (mApp->changedSinceSave) {
            ctx->SetRef("File not saved");
            ctx->ItemClick("  No  ");
        }
        IM_CHECK_EQ(interfGeom->GetNbFacet(), 0);
        };
}
