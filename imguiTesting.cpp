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

    ExecuteQueue();

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
            double timeEnd = ImGui::GetTime();
            std::cout << fmt::format("Elapsed time: {:.2f}s", timeEnd) << std::endl;
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
    case texture:
    {
        ConfigureGeometry(qPipe);
        TextureType t;
        t.enabled = true;
        t.countDes = true;
        TextureFacet(0, 10, 10, t);
        t = TextureType();
        t.enabled = true;
        t.countAbs = true;
        TextureFacet(1, 10, 10, t);
        t = TextureType();
        t.enabled = true;
        t.countRefl = true;
        TextureFacet(2, 10, 10, t);
        t = TextureType();
        t.enabled = true;
        t.countACD = true;
        TextureFacet(3, 10, 10, t);
        t = TextureType();
        t.enabled = true;
        t.countDirection = true;
        TextureFacet(4, 10, 10, t);
        ExecuteQueue();
        currentConfig = index;
    }
        break;
    default:
        return false;
    }
    return true;
}

void ImTest::ConfigureGeometryMidTest(Configuration index)
{
    std::function<void()> f = [this, index]() {
        ConfigureGeometry(index);
        };
    callQueue.push(f);
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
        if (ImGui::Selectable("Texture", selection == texture)) {
            previewVal = "Texture";
            selection = texture;
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
    if (idx >= interfGeom->GetNbVertex()) return;
    std::function<void()> f = [this, idx, add]() {
        if (!add) interfGeom->EmptySelectedVertexList();
        interfGeom->SelectVertex(static_cast<int>(idx));
        interfGeom->UpdateSelection();
        mApp->imWnd->Refresh();
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
    DeselectAllvertices();
}

void ImTest::DeselectAllvertices()
{
    std::function<void()> f = [this]() {
        interfGeom->UnselectAllVertex();
        };
    callQueue.push(f);
}

// not working for unknown reasons
void ImTest::TextureFacet(size_t idx, int width, int height, TextureType type)
{
    if (idx >= interfGeom->GetNbFacet()) return;
    std::function<void()> func = [this, idx, width, height, type]() {
        InterfaceFacet* f = interfGeom->GetFacet(idx);
        f->sh.isTextured = type.enabled;
        f->sh.countAbs = type.countAbs;
        f->sh.countRefl = type.countRefl;
        f->sh.countTrans = type.countTrans;
        f->sh.countDirection = type.countDirection;
        f->sh.countDes = type.countDes;
        f->sh.countACD = type.countACD;
        interfGeom->SetFacetTexture(idx, width, height, true);
        interfGeom->BuildGLList();
        mApp->UpdateModelParams();
        mApp->UpdateFacetParams(true);
        mApp->worker.MarkToReload();
        if (mApp->imWnd && mApp->imWnd->textPlot.IsVisible()) mApp->imWnd->textPlot.UpdatePlotter();
        };
    callQueue.push(func);
}

void ImTest::DeleteFacet(size_t idx)
{
    if (idx >= interfGeom->GetNbFacet()) return;
    std::function<void()> f = [this, idx]() {
        if (mApp->worker.IsRunning()) mApp->worker.Stop_Public();
        if (idx >= interfGeom->GetNbFacet()) return;
        interfGeom->RemoveFacets({idx});
        mApp->UpdateModelParams();
        mApp->worker.MarkToReload();
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
    return true;
}

void ImTest::ExecuteQueue()
{
    LockWrapper lW(mApp->imguiRenderLock);
    while (callQueue.size() > 0) {
        std::function<void()> f = callQueue.front();
        f();
        callQueue.pop();
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
        if (currentConfig == empty || currentConfig == qPipe || currentConfig == profile) {
            IM_CHECK_EQ(0, interfGeom->GetNbSelectedFacets());
        }
        if (currentConfig == texture) {
            IM_CHECK_EQ(5, interfGeom->GetNbSelectedFacets());
        }
        };
    t = IM_REGISTER_TEST(engine, "SelectionMenu", "Select By Texture Type");
    t->TestFunc = [this](ImGuiTestContext* ctx) {
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("###Selection/Select by Texture type...");
        ctx->SetRef("Select facets by texture properties");
        ctx->ItemClick("  Select  ");
        if (currentConfig == empty || currentConfig == qPipe || currentConfig == profile) {
            IM_CHECK_EQ(0, interfGeom->GetNbSelectedFacets());
        }
        if (currentConfig == texture) {
            IM_CHECK_EQ(5, interfGeom->GetNbSelectedFacets());
            ctx->ItemClick("**/Count desorbtion");
            ctx->ItemClick("  Select  ");
            IM_CHECK_EQ(1, interfGeom->GetNbSelectedFacets());
            ctx->ItemClick("**/Count desorbtion");
            ctx->ItemClick("  Select  ");
            IM_CHECK_EQ(4, interfGeom->GetNbSelectedFacets());
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
        // TODO test sidebar
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
        DeselectAll();
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
        if (currentConfig == texture) {
            SelectFacet(0);
            ctx->Sleep(1);
            SelectFacet(1);
            ctx->Sleep(1);
            SelectFacet(2);
            ctx->Sleep(1);
            SelectFacet(3);
            ctx->Sleep(1);
            SelectFacet(4);
            ctx->Sleep(1);
            ctx->ItemClick("Find Max");
            ctx->ComboClickAll("##View");
        }
        ctx->ItemClick("#CLOSE");
        };
    t = IM_REGISTER_TEST(engine, "ToolsMenu", "Profile plotter");
    t->TestFunc = [this](ImGuiTestContext* ctx) {
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("###Tools/Profile Plotter ...");
        ctx->SetRef("Profile Plotter");
        ctx->MenuClick("Export/To clipboard");
        ctx->SetRef("Error");
        ctx->ItemClick("  Ok  ");
        ctx->SetRef("Profile Plotter");
        ctx->MenuClick("Export/To file");
        ctx->SetRef("Error");
        ctx->ItemClick("  Ok  ");
        ctx->SetRef("Profile Plotter");

        if (currentConfig == profile) {
            // todo test sidebar
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
        ctx->ComboClickAll("##View");
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
        ctx->ItemClick("Histogram settings");
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
        ctx->ItemClick("Histogram settings");
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
    t = IM_REGISTER_TEST(engine, "ToolsMenu", "Particle Logger");
    t->TestFunc = [this](ImGuiTestContext* ctx) {
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("###Tools/Particle logger...");
        ctx->SetRef("Particle Logger");
        // all other elements are inside child windows
        ctx->ItemClick("#CLOSE");
        };
    t = IM_REGISTER_TEST(engine, "ToolsMenu", "Global Settings");
    t->TestFunc = [this](ImGuiTestContext* ctx) {
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("###Tools/Global Settings ...");
        ctx->SetRef("Global settings");

        ctx->ItemClick("split/Autosave frequency (minutes)");
        ctx->KeyCharsReplaceEnter("a");
        ctx->ItemClick("split/Autosave only when simulation is running");
        ctx->ItemClick("split/Autosave only when simulation is running");
        ctx->ItemClick("split/Use .zip as default extension (otherwise .xml)");
        ctx->ItemClick("split/Use .zip as default extension (otherwise .xml)");
        ctx->ItemClick("split/Check for updates at startup");
        ctx->ItemClick("split/Check for updates at startup");
        ctx->ItemClick("split/Anti-Aliasing");
        ctx->ItemClick("split/Anti-Aliasing");
        ctx->ItemClick("split/White Background");
        ctx->ItemClick("split/White Background");
        ctx->ItemClick("split/Left-handed coord. system");
        ctx->ItemClick("split/Left-handed coord. system");
        ctx->ItemClick("split/Highlight non-planar facets");
        ctx->ItemClick("split/Highlight non-planar facets");
        ctx->ItemClick("split/Highlight selected facets");
        ctx->ItemClick("split/Highlight selected facets");
        ctx->ItemClick("split/Use old XML format");
        ctx->ItemClick("split/Use old XML format");

        ctx->ItemClick("##nbProc/-");
        ctx->ItemClick("##nbProc");
        ctx->KeyCharsReplaceEnter("a");
        ctx->ItemClick("Apply and restart processes");

        ctx->ItemClick("split/###GMass");
        ctx->KeyCharsReplaceEnter("a");
        ctx->ItemClick("split/###GMass");
        ctx->KeyCharsReplaceEnter("30");
        ctx->ItemClick("split/Apply above settings");
        ctx->MouseMoveToPos(ImVec2(100, 100));
        ctx->SetRef("You have changed the gas mass.");
        ctx->ItemClick("  Ok  ");
        ctx->SetRef("Global settings");
        ctx->ItemClick("split/###GMass");
        ctx->KeyCharsReplaceEnter("28");
        ctx->ItemClick("split/Apply above settings");
        ctx->MouseMoveToPos(ImVec2(100, 100));
        ctx->SetRef("You have changed the gas mass.");
        ctx->ItemClick("  Ok  ");
        ctx->SetRef("Global settings");
        ctx->ItemClick("split/##EnableDecay");
        ctx->ItemClick("split/Gas half life (s)");
        ctx->KeyCharsReplaceEnter("a");
        ctx->ItemClick("split/Apply above settings");
        ctx->ItemClick("split/##EnableDecay");
        ctx->ItemClick("split/Apply above settings");
        ctx->ItemClick("split/Enable low flux mode");
        ctx->ItemClick("split/Apply above settings");
        ctx->ItemClick("split/Cutoff ratio");
        ctx->KeyCharsReplaceEnter("a");
        ctx->ItemClick("split/Apply above settings");
        ctx->ItemClick("split/Enable low flux mode");
        ctx->ItemClick("split/Apply above settings");

        ctx->ItemClick("split/Recalc. outgassing");

        ctx->SetRef("Global settings");
        ctx->ItemClick("#CLOSE");
        };
    t = IM_REGISTER_TEST(engine, "ToolsMenu", "Screenshot");
    t->TestFunc = [this](ImGuiTestContext* ctx) {
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("###Tools/###Screenshot");
        };
    t = IM_REGISTER_TEST(engine, "ToolsMenu", "Moving Parts");
    t->TestFunc = [this](ImGuiTestContext* ctx) {
        if (currentConfig != empty) {
            DeselectAll();
        }
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("###Tools/Moving parts...");
        ctx->SetRef("Define moving parts");
        ctx->ItemClick("**/No moving parts");
        ctx->ItemClick("Apply");
        ctx->ItemClick("//Define moving parts/**/Fixed (same velocity vector everywhere)");
        ctx->ItemClick("//Define moving parts/**/###MovMartT1/##vx");
        ctx->KeyCharsReplaceEnter("a");
        ctx->ItemClick("Apply");
        ctx->MouseMoveToPos(ImVec2(100, 100));
        ctx->SetRef("Error");
        ctx->ItemClick("  Ok  ");
        ctx->SetRef("Define moving parts");
        ctx->ItemClick("//Define moving parts/**/###MovMartT1/##vx");
        ctx->KeyCharsReplaceEnter("0");
        ctx->ItemClick("Apply");
        ctx->MouseMoveToPos(ImVec2(100, 100));
        ctx->SetRef("Error");
        ctx->ItemClick("  Ok  ");
        ctx->SetRef("Define moving parts");
        ctx->ItemClick("**/Rotation around axis");
        ctx->ItemClick("**/###MovingPartsTable/Use selected vertex");
        ctx->MouseMoveToPos(ImVec2(100, 100));
        ctx->SetRef("Error");
        ctx->ItemClick("  Ok  ");
        ctx->SetRef("Define moving parts");
        ctx->ItemClick("//Define moving parts/**/###MovingPartsTable/Base to sel. vertex");
        ctx->MouseMoveToPos(ImVec2(100, 100));
        ctx->SetRef("Error");
        ctx->ItemClick("  Ok  ");
        ctx->SetRef("Define moving parts");
        if (currentConfig != empty) {
            SelectVertex(1);
            ctx->ItemClick("**/###MovingPartsTable/Use selected vertex");
            ctx->ItemClick("//Define moving parts/**/###MovingPartsTable/Base to sel. vertex");
        }

        ctx->ItemClick("//Define moving parts/**/###MovingPartsTable/##ax");
        ctx->KeyCharsReplaceEnter("a");
        ctx->ItemClick("Apply");
        ctx->MouseMoveToPos(ImVec2(100, 100));
        ctx->SetRef("Error");
        ctx->ItemClick("  Ok  ");
        ctx->SetRef("Define moving parts");
        ctx->ItemClick("//Define moving parts/**/###MovingPartsTable/##ax");
        ctx->KeyCharsReplaceEnter("0");
        ctx->ItemClick("**/No moving parts");
        ctx->ItemClick("Apply");

        ctx->ItemClick("#CLOSE");
        };
    t = IM_REGISTER_TEST(engine, "ToolsMenu", "Measure forces");
    t->TestFunc = [this](ImGuiTestContext* ctx) {
        if (currentConfig != empty) {
            DeselectAll();
        }
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("###Tools/Measure forces...");
        ctx->SetRef("Measure forces");
        if (currentConfig == empty) {
            ctx->ItemClick("**/Selected vertex");
            ctx->SetRef("Error");
            ctx->ItemClick("  Ok  ");
            ctx->SetRef("Measure forces");
            ctx->ItemClick("**/Center of selected facet");
            ctx->SetRef("Error");
            ctx->ItemClick("  Ok  ");
            ctx->SetRef("Measure forces");
            ctx->ItemClick("Enable force measurement (has performance impact)");
            ctx->ItemClick("Apply");
            ctx->ItemClick("Enable force measurement (has performance impact)");
            ctx->ItemClick("Apply");
        }
        else {
            SelectVertex(0);
            ctx->ItemClick("**/Selected vertex");
            SelectFacet(0);
            ctx->ItemClick("**/Center of selected facet");
            ctx->SetRef("Measure forces");
            ctx->ItemClick("Enable force measurement (has performance impact)");
            ctx->ItemClick("Apply");
            ctx->ItemClick("Enable force measurement (has performance impact)");
            ctx->ItemClick("Apply");

            SelectVertex({ 0,1 });
            ctx->ItemClick("**/Selected vertex");
            ctx->SetRef("Error");
            ctx->ItemClick("  Ok  ");
            ctx->SetRef("Measure forces");
            SelectFacet({ 0,1 });
            ctx->ItemClick("**/Center of selected facet");
            ctx->SetRef("Error");
            ctx->ItemClick("  Ok  ");
            ctx->SetRef("Measure forces");
            ctx->ItemClick("Enable force measurement (has performance impact)");
            ctx->ItemClick("Apply");
            ctx->ItemClick("Enable force measurement (has performance impact)");
            ctx->ItemClick("Apply");
        }
        ctx->ItemClick("#CLOSE");
        };
    t = IM_REGISTER_TEST(engine, "FacetMenu", "Facet coordiantes");
    t->TestFunc = [this](ImGuiTestContext* ctx) {
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("Facet/Facet coordinates ...");
        ctx->SetRef("###FCoords");
        if (currentConfig != empty) {
            SelectFacet(0);
            ctx->ItemClick("/**/##VIID");
            ctx->KeyCharsReplaceEnter("");
            ctx->ItemClick("/**/Insert as last vertex");
            ctx->ItemClick("//Error/  Ok  ");
            ctx->ItemClick("/**/Insert before sel. row");
            ctx->ItemClick("//Error/  Ok  ");
            ctx->ItemClick("/**/Remove selected row");
            ctx->ItemClick("//Error/  Ok  ");

            ctx->ItemClick("X");
            ctx->ItemClick("//Set all X coordinates to:/New coordinate");
            ctx->KeyCharsReplace("");
            ctx->ItemClick("//Set all X coordinates to:/  OK  ");
            ctx->ItemClick("//Error/  Ok  ");

            ctx->ItemClick("Z");
            ctx->ItemClick("//Set all Z coordinates to:/New coordinate");
            ctx->KeyCharsReplaceEnter("1");
           
            ctx->ItemClick("Apply");
            ctx->ItemClick("//Question/  Ok  ");

            ctx->ItemClick("Z");
            ctx->ItemClick("//Set all Z coordinates to:/New coordinate");
            ctx->KeyCharsReplace("0");
            ctx->ItemClick("//Set all Z coordinates to:/  OK  ");

            ctx->ItemClick("Apply");
            ctx->ItemClick("//Question/  Ok  ");
        }
        ctx->ItemClick("#CLOSE");
        };
    t = IM_REGISTER_TEST(engine, "FacetMenu", "Facet Move");
    t->TestFunc = [this](ImGuiTestContext* ctx) {
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("Facet/Move ...");
        ctx->SetRef("Move Facet");
        DeselectAll();
        ctx->ItemClick("/**/Facet normal");
        ctx->ItemClick("//Error/  Ok  ");
        
        ctx->ItemClick("/**/Selected Vertex##0");
        ctx->ItemClick("//Error/  Ok  ");
        ctx->ItemClick("/**/Facet center##0");
        ctx->ItemClick("//Error/  Ok  ");

        ctx->ItemClick("Move facets");
        ctx->ItemClick("//Error/  Ok  ");
        ctx->ItemClick("Copy facets");
        ctx->ItemClick("//Error/  Ok  ");
        if (currentConfig != empty) {
            SelectFacet(0);
            ctx->ItemClick("cm##X");
            ctx->KeyCharsReplace("1");
            ctx->ItemClick("cm##Y");
            ctx->KeyCharsReplace("0");
            ctx->ItemClick("cm##Z");
            ctx->KeyCharsReplace("0");
            ctx->ItemClick("Move facets");
            ctx->ItemClick("cm##X");
            ctx->KeyCharsReplace("-1");
            ctx->ItemClick("Move facets");
            ctx->ItemClick("/**/options/Facet center##0");
            SelectFacet(1);
            ctx->ItemClick("/**/options/Facet center##1");
            ctx->ItemClick("Direction and Distance");
            ctx->ItemClick("/**/cm##D");
            ctx->KeyCharsReplace("-1");
            ctx->ItemClick("Copy facets");
            DeselectAll();
            DeleteFacet(7);
        }
        ctx->ItemClick("#CLOSE");
        };
    t = IM_REGISTER_TEST(engine, "FacetMenu", "Facet Scale");
    t->TestFunc = [this](ImGuiTestContext* ctx) {
        DeselectAll();
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("Facet/Scale ...");
        ctx->SetRef("Scale selected facets");
        ctx->ItemClick("Scale facet");
        ctx->ItemClick("//Nothing to scale/  Ok  ");
        ctx->ItemClick("Copy facet");
        ctx->ItemClick("//Nothing to scale/  Ok  ");
        ctx->ItemClick("/**/###SSF");
        ctx->ItemClick("/**/Selected vertex");
        ctx->ItemClick("/**/Center of selected facet #");
        ctx->ItemClick("/**/<-Get selected");
        ctx->ItemClick("//Error/  Ok  ");
        ctx->ItemClick("/**/Uniform");
        ctx->ItemClick("/**/Distorted");
        if (currentConfig != empty) {
            SelectFacet(0);
            ctx->ItemClick("/**/<-Get selected");
            SelectFacet(1);
            ctx->ItemClick("/**/##X:");
            ctx->KeyCharsReplace("1.1");
            ctx->ItemClick("/**/##Y:");
            ctx->KeyCharsReplace("1.1");
            ctx->ItemClick("/**/##Z:");
            ctx->KeyCharsReplaceEnter("1.1");
            ctx->ItemClick("Copy facet");
            ctx->ItemClick("/**/Uniform");
            ctx->ItemClick("/**/###1by");
            ctx->KeyCharsReplaceEnter("1.1");
            ctx->ItemClick("Scale facet");
            ctx->ItemClick("/**/###facetN");
            ctx->KeyCharsReplaceEnter("0");
            ctx->ItemClick("Scale facet");
            ctx->ItemClick("//Error/  Ok  ");
            ctx->ItemClick("/**/###SSF");
            ctx->ItemClick("/**/##X=");
            ctx->KeyCharsReplaceEnter("a");
            DeleteFacet(7);
        }
        ctx->ItemClick("#CLOSE");
        };
    t = IM_REGISTER_TEST(engine, "FacetMenu", "Facet MirrorProject");
    t->TestFunc = [this](ImGuiTestContext* ctx) {
        DeselectAll();
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("Facet/###MPF");
        ctx->SetRef("###MPFW");
        ctx->ItemClick("Mirror facet");
        ctx->ItemClick("//Nothing to mirror/  Ok  ");
        ctx->ItemClick("Copy mirror facet");
        ctx->ItemClick("//Nothing to mirror/  Ok  ");
        ctx->ItemClick("Project facet");
        ctx->ItemClick("//Nothing to mirror/  Ok  ");
        ctx->ItemClick("Copy project facet");
        ctx->ItemClick("//Nothing to mirror/  Ok  ");
        if (currentConfig != empty && mApp->imWnd->mirrProjFacet.mode == ImFacetMirrorProject::PlaneDefinition::none) {
            SelectFacet(2);
            ctx->ItemClick("Mirror facet");
            ctx->ItemClick("//Error/  Ok  ");
            ctx->ItemClick("Copy mirror facet");
            ctx->ItemClick("//Error/  Ok  ");
            ctx->ItemClick("Project facet");
            ctx->ItemClick("//Error/  Ok  ");
            ctx->ItemClick("Copy project facet");
            ctx->ItemClick("//Error/  Ok  ");
        }
        ctx->ItemClick("/**/Define by plane equation");
        ctx->ItemClick("/**/Define by 3 selected vertices");
        ctx->ItemClick("/**/Plane of facet #");
        ctx->ItemClick("/**/XZ plane");
        ctx->ItemClick("/**/YZ plane");
        ctx->ItemClick("/**/XY plane");
        if (currentConfig != empty) {
            SelectFacet(2);
            ctx->ItemClick("/**/YZ plane");
            ctx->ItemClick("Copy mirror facet");
            ctx->Sleep(1);
            DeleteFacet(7);
            SelectFacet(2);
            ctx->ItemClick("Copy project facet");
            ctx->Sleep(1);
            DeleteFacet(7);
        }
        DeselectAll();
        ctx->ItemClick("#CLOSE");
        };
    t = IM_REGISTER_TEST(engine, "FacetMenu", "Facet Rotate");
    t->TestFunc = [this](ImGuiTestContext* ctx) {
        DeselectAll();
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("Facet/Rotate ...");
        ctx->SetRef("Rotate selected facets");
        ctx->ItemClick("##Degrees:");
        ctx->KeyCharsReplaceEnter("180");
        ctx->ItemClick("##Radians:");
        ctx->KeyCharsReplaceEnter("2");
        ctx->ItemClick("/**/X axis");
        ctx->ItemClick("/**/Y axis");
        ctx->ItemClick("/**/Z axis");
        ctx->ItemClick("/**/U vector");
        ctx->ItemClick("/**/V vector");
        ctx->ItemClick("/**/Normal vector");
        ctx->ItemClick("/**/###2V");
        ctx->ItemClick("/**/Define by equation:");
        ctx->ItemClick("Rotate facet");
        ctx->ItemClick("//Nothing to rotate/  Ok  ");
        ctx->ItemClick("Copy facet");
        ctx->ItemClick("//Nothing to rotate/  Ok  ");
        if (currentConfig != empty) {
            ctx->ItemClick("/**/###RSFADMEQ/###FPDMa");
            ctx->KeyCharsReplaceEnter("x");
            SelectFacet(0);
            ctx->ItemClick("Copy facet");
            ctx->ItemClick("//Error/  Ok  ");
            SelectVertex(0);
            ctx->ItemClick("/**/<-Get base");
            DeselectAll();
            SelectVertex(1);
            ctx->ItemClick("/**/<-Calc diff");
            SelectFacet(0);
            ctx->ItemClick("Copy facet");
            ctx->Sleep(1);
            DeleteFacet(7);
        }
        ctx->ItemClick("#CLOSE");
        };
    t = IM_REGISTER_TEST(engine, "FacetMenu", "Facet Align");
    t->TestFunc = [this](ImGuiTestContext* ctx) {
        DeselectAll();
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("Facet/Align to ...");
        ctx->SetRef("Align selected facets to an other");
        if (currentConfig != empty) {
            SelectFacet(1);
            ctx->ItemClick("/**/Update from selection");
            SelectFacet({0,1});
            SelectVertex(3);
            SelectVertex(5);
            SelectVertex(4);
            SelectVertex(6);
            ctx->ItemClick("Align");
            ctx->Sleep(1);
            ctx->ItemClick("Undo");
        }
        ctx->ItemClick("#CLOSE");
        };
    t = IM_REGISTER_TEST(engine, "FacetMenu", "Facet Extrude");
    t->TestFunc = [this](ImGuiTestContext* ctx) {
        DeselectAll();
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("Facet/Extrude ...");
        ctx->SetRef("Extrude Facet");
        ctx->ItemClick("Extrude");
        ctx->ItemClick("//Nothing to move/  Ok  ");
        if (currentConfig != empty && currentConfig != texture) {
            SelectFacet(0);
            if (mApp->imWnd->extrudeFacet.mode == ImFacetExtrude::Mode::none) {
                ctx->ItemClick("Extrude");
                ctx->ItemClick("//Error/  Ok  ");
            }
            ctx->ItemClick("/**/Towards normal");
            ctx->ItemClick("/**/Against normal");
            ctx->ItemClick("/**/##extrusion length:");
            ctx->KeyCharsReplaceEnter("");
            ctx->ItemClick("Extrude");
            ctx->ItemClick("//Error/  Ok  ");
            ctx->ItemClick("/**/##extrusion length:");
            ctx->KeyCharsReplaceEnter("x");
            ctx->ItemClick("Extrude");
            ctx->ItemClick("//Error/  Ok  ");
            ctx->ItemClick("/**/##extrusion length:");
            ctx->KeyCharsReplaceEnter("0");
            ctx->ItemClick("Extrude");
            ctx->ItemClick("//Error/  Ok  ");
            ctx->ItemClick("/**/##extrusion length:");
            ctx->KeyCharsReplaceEnter("1");
            ctx->ItemClick("Extrude");
            DeselectAll();
            SelectVertex(0);
            ctx->ItemClick("/**/Direction vector");
            ctx->ItemClick("/**/Get Base Vertex");
            ctx->MouseMoveToPos(ImVec2(100, 100));
            DeselectAll();
            SelectVertex(1);
            ctx->ItemClick("/**/Get Dir. Vertex");
            ctx->MouseMoveToPos(ImVec2(100, 100));
            DeselectAll();
            SelectFacet(1);
            ctx->ItemClick("Extrude");
            ctx->Sleep(1);
            // 'Along Curve' section
            ctx->ItemClick("/**/Towards normal##C");
            ctx->ItemClick("/**/Against normal##C");
            SelectFacet(2);
            ctx->ItemClick("/**/Facet center");
            ctx->ItemClick("/**/Facet V");
            ctx->ItemClick("/**/###EF3TR");
            ctx->KeyCharsReplaceEnter("5");
            ctx->ItemClick("/**/###EF3TAD");
            ctx->KeyCharsReplaceEnter("90");
            ctx->ItemClick("/**/###FE3TS");
            ctx->KeyCharsReplaceEnter("5");
            ctx->ItemClick("Extrude");
            ctx->Sleep(1);
            ConfigureGeometryMidTest(currentConfig);
        }
        ctx->ItemClick("#CLOSE");
        };
    t = IM_REGISTER_TEST(engine, "FacetMenu", "Facet Split");
    t->TestFunc = [this](ImGuiTestContext* ctx) {
        DeselectAll();
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("Facet/Split ...");
        ctx->SetRef("Split Facet");
        ctx->ItemClick("**/XY plane");
        ctx->ItemClick("**/XZ plane");
        ctx->ItemClick("**/YZ plane");
        if (currentConfig != empty) {
            SelectFacet(3);
            ctx->ItemClick("Split");
            ctx->Sleep(1);
            ctx->ItemClick("Undo");
            ctx->Sleep(1);
        }
        ctx->ItemClick("#CLOSE");
        };
    t = IM_REGISTER_TEST(engine, "FacetMenu", "Create Shape");
    t->TestFunc = [this](ImGuiTestContext* ctx) {
        DeselectAll();
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("Facet/Create shape...");
        ctx->SetRef("Create shape");
        ctx->ItemClick("Shape selection/###SR");
        ctx->ItemClick("Shape selection/###CE");
        ctx->ItemClick("Shape selection/###RT");
        ctx->ItemClick("**/###centerXIn");
        ctx->KeyCharsReplaceEnter("0");
        ctx->ItemClick("**/###centerYIn");
        ctx->KeyCharsReplaceEnter("0");
        ctx->ItemClick("**/###centerZIn");
        ctx->KeyCharsReplaceEnter("a");
        ctx->ItemClick("**/###axis1XIn");
        ctx->KeyCharsReplaceEnter("1");
        ctx->ItemClick("**/###axis1YIn");
        ctx->KeyCharsReplaceEnter("0");
        ctx->ItemClick("**/###axis1ZIn");
        ctx->KeyCharsReplaceEnter("0");
        ctx->ItemClick("**/###normalXIn");
        ctx->KeyCharsReplaceEnter("0");
        ctx->ItemClick("**/###normalYIn");
        ctx->KeyCharsReplaceEnter("0");
        ctx->ItemClick("**/###normalZIn");
        ctx->KeyCharsReplaceEnter("1");
        ctx->ItemClick("**/###axis1Len");
        ctx->KeyCharsReplaceEnter("2");
        ctx->ItemClick("**/###axis2Len");
        ctx->KeyCharsReplaceEnter("1");
        ctx->ItemClick("**/###trackTopLenIn");
        ctx->KeyCharsReplaceEnter("1.5");
        ctx->ItemClick("**/###arcStepsIn");
        ctx->KeyCharsReplaceEnter("8");
        ctx->ItemClick("Create facet");
        ctx->SetRef("Error");
        ctx->ItemClick("  Ok  ");
        ctx->SetRef("Create shape");
        ctx->ItemClick("**/###centerZIn");
        ctx->KeyCharsReplaceEnter("0");
        ctx->ItemClick("Create facet");
        ctx->Sleep(1);
        DeleteFacet(interfGeom->GetNbFacet() - 1); // delete the created facet (it will always be the last one in the facet vector
        ctx->ItemClick("Shape selection/###CE");
        ctx->ItemClick("Create facet");
        ctx->Sleep(1);
        DeleteFacet(interfGeom->GetNbFacet() - 1);
        ctx->ItemClick("Shape selection/###SR");
        ctx->ItemClick("Create facet");
        ctx->Sleep(1);
        DeleteFacet(interfGeom->GetNbFacet() - 1);
        ctx->ItemClick("#CLOSE");
        };
    t = IM_REGISTER_TEST(engine, "VertexMenu", "Vertex Coordinates");
    t->TestFunc = [this](ImGuiTestContext* ctx) {
        DeselectAll();
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("Vertex/Vertex coordinates...");
        ctx->SetRef("Vertex coordinates");
        ctx->ItemClick("X");
        ctx->SetRef("Set all X coordinates to:");
        ctx->ItemClick("New coordinate");
        ctx->KeyCharsReplaceEnter("5");
        ctx->SetRef("Vertex coordinates");
        if (currentConfig != empty) {
            SelectVertex({ 0,2,4,6,8 });
            ctx->MouseMoveToPos(ImVec2(100, 100));
            ctx->ItemClick("/**/###VCT/###0-Z");
            ctx->KeyCharsReplaceEnter("1");
            ctx->ItemClick("Apply");
            ctx->SetRef("Apply?");
            ctx->ItemClick("  Ok  ");
            ctx->SetRef("Vertex coordinates");
            ctx->ItemClick("Z");
            ctx->SetRef("Set all Z coordinates to:");
            ctx->ItemClick("New coordinate");
            ctx->KeyCharsReplaceEnter("1");
            ctx->SetRef("Vertex coordinates");
            ctx->ItemClick("Apply");
            ctx->SetRef("Apply?");
            ctx->ItemClick("  Ok  ");
            ctx->SetRef("Vertex coordinates");
        }
        ctx->ItemClick("#CLOSE");
        };
    t = IM_REGISTER_TEST(engine, "VertexMenu", "Move Vertex");
    t->TestFunc = [this](ImGuiTestContext* ctx) {
        DeselectAll();
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("Vertex/Move...");
        ctx->SetRef("Move Vertex");
        if (currentConfig != empty) {
            SelectVertex({ 0,2,4,6,8 });
            ctx->ItemClick("Absolute offset");
            ctx->ItemClick("###xIn");
            ctx->KeyCharsReplaceEnter("0");
            ctx->ItemClick("###yIn");
            ctx->KeyCharsReplaceEnter("0");
            ctx->ItemClick("###zIn");
            ctx->KeyCharsReplaceEnter("-1");
            ctx->ItemClick("Move vertices");

            ctx->ItemClick("Direction and distance");
            SelectFacet(0);
            ctx->ItemClick("**/Facet normal");
            ctx->ItemClick("**/###dIn");
            ctx->KeyCharsReplaceEnter("1");
            ctx->ItemClick("Move vertices");
        }
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
    t = IM_REGISTER_TEST(engine, "FacetMenu", "Outgassing Map");
    t->TestFunc = [this](ImGuiTestContext* ctx) {
        DeselectAll();
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("Facet/Convert to outgassing map...");
        ctx->SetRef("###OgM");
        if (currentConfig != empty) {
            TextureType t;
            t.enabled = true;
            t.countDes = true;
            TextureFacet(0, 10, 10, t);
            SelectFacet(0);
            // edit map, explode
        }
        ctx->ComboClickAll("###OMDT");
        ctx->ItemClick("#CLOSE");
        };
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
    t = IM_REGISTER_TEST(engine, "ToolsMenu", "Convergence Plotter formula list");
    t->TestFunc = [this](ImGuiTestContext* ctx) {
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("###Tools/Convergence Plotter ...");
        ctx->SetRef("Convergence Plotter");
        ctx->ItemClick("/**/All");
        ctx->SetRef("##MainMenuBar");
        if (currentConfig != empty) {
            IM_CHECK_EQ(mApp->imWnd->convPlot.formulaDrawToggle.size(), mApp->appFormulas->formulas.size());
        }
        ctx->MenuClick("###File/###NewGeom");
        if (mApp->changedSinceSave) {
            ctx->SetRef("File not saved");
            ctx->ItemClick("  No  ");
            ctx->MouseMoveToPos(ImVec2(100, 100));
            IM_CHECK_EQ(mApp->imWnd->convPlot.formulaDrawToggle.size(), 0);
            IM_CHECK_EQ(mApp->imWnd->convPlot.data.size(), 0);
        }
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("Test/Quick Pipe");
        if (mApp->changedSinceSave) {
            ctx->SetRef("File not saved");
            ctx->ItemClick("  No  ");
            ctx->MouseMoveToPos(ImVec2(100, 100));
            ctx->ItemClick("/**/All");
            IM_CHECK_GT(mApp->imWnd->convPlot.formulaDrawToggle.size(), 0);
        }
        //ConfigureGeometryMidTest(currentConfig);
        ctx->SetRef("Convergence Plotter");
        ctx->ItemClick("#CLOSE");
        };
}
