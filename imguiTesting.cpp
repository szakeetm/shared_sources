#include "imguiTesting.h"
#if defined(MOLFLOW)
#include "../../src/MolFlow.h"
#else
#include "../../src/SynRad.h"
#endif
#include "ImguiWindow.h"

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
    mApp->imWnd->forceDrawNextFrame=true;
    ImGui::StyleColorsDark(); // on light background the log is not readable
    ImGuiTestEngine_ShowTestEngineWindows(engine, &drawn);
    ImGui::StyleColorsLight();
}

void ImTest::PostSwap()
{
    ImGuiTestEngine_PostSwap(engine);
}

void ImTest::RegisterTests()
{
    ImGuiTest* t = NULL;
    t = IM_REGISTER_TEST(engine, "FileMenu", "New, empty geometry");
    t->TestFunc = [this](ImGuiTestContext* ctx) {
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("###File/###NewGeom");
        };
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
        ctx->MouseMoveToPos(ImVec2(100,100));
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
    t = IM_REGISTER_TEST(engine, "TestMenu", "Quick Pipe");
    t->TestFunc = [this](ImGuiTestContext* ctx) {
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("Test/Quick Pipe");
        IM_CHECK_EQ(interfGeom->GetNbFacet(), 7);
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
        // TODO Test Correctness of Input and button behaviour
        ctx->ItemClick("#CLOSE");
        };
    t = IM_REGISTER_TEST(engine, "SelectionMenu", "Select Sticking");
    t->TestFunc = [this](ImGuiTestContext* ctx) {
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("###Selection/Select Sticking");
        // TODO check if selection correct (may need to conditionally add quickpipe if running tests out of order)
        };
    t = IM_REGISTER_TEST(engine, "SelectionMenu", "Select Transparent");
    t->TestFunc = [this](ImGuiTestContext* ctx) {
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("###Selection/Select Transparent");
        // Cannot test more as transparency cannot be changed using ImGui UI yet
        };
    t = IM_REGISTER_TEST(engine, "SelectionMenu", "Select 2 Sided");
    t->TestFunc = [this](ImGuiTestContext* ctx) {
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("###Selection/Select 2 sided");
        // Cannot test more as 2-sidedness cannot be changed using ImGui UI yet
        };
    t = IM_REGISTER_TEST(engine, "SelectionMenu", "Select Texture");
    t->TestFunc = [this](ImGuiTestContext* ctx) {
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("###Selection/Select Texture");
        // Cannot test more as textures cannot be applied using ImGui UI yet
        };
    t = IM_REGISTER_TEST(engine, "SelectionMenu", "Select By Texture Type");
    t->TestFunc = [this](ImGuiTestContext* ctx) {
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("###Selection/Select by Texture type...");
        ctx->SetRef("Select facets by texture properties");
        // TODO test tristate behaviour
        // Cannot test more as textures cannot be applied using ImGui UI yet
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
        ctx->MouseMoveToPos(ImVec2(100,100));
        ctx->SetRef("Error");
        ctx->ItemClick("  Ok  ");
        ctx->SetRef("Select facets by simulation result");
        ctx->ItemClick("#CLOSE");
        };
    t = IM_REGISTER_TEST(engine, "SelectionMenu", "Select Link facets");
    t->TestFunc = [this](ImGuiTestContext* ctx) {
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("###Selection/Select link facets");
        };
    t = IM_REGISTER_TEST(engine, "SelectionMenu", "Select teleport facets");
    t->TestFunc = [this](ImGuiTestContext* ctx) {
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("###Selection/Select teleport facets");
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

        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("###Selection/Select non planar facets");
        ctx->SetRef("Select non planar facets");
        ctx->ItemClick("Planarity larger than");
        ctx->KeyCharsReplace("1e-3");
        ctx->ItemClick("  OK  ");
        IM_CHECK_EQ(mApp->planarityThreshold, 1e-3);

        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("###Selection/Select non planar facets");
        ctx->SetRef("Select non planar facets");
        ctx->ItemClick("Planarity larger than");
        ctx->KeyCharsReplace("abc");
        ctx->ItemClick("  OK  ");
        IM_CHECK_EQ(mApp->planarityThreshold, 1e-3);
        ctx->MouseMoveToPos(ImVec2(100, 100));
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
        };
    t = IM_REGISTER_TEST(engine, "SelectionMenu", "Select Invert");
    t->TestFunc = [this](ImGuiTestContext* ctx) {
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("###Selection/Invert selection");
        };
    t = IM_REGISTER_TEST(engine, "SelectionMenu", "Selection Memory");
    t->TestFunc = [this](ImGuiTestContext* ctx) {
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("###Selection/Clear memorized/Clear All");
        ctx->MouseMoveToPos(ImVec2(100, 100));
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
        ctx->MouseMoveToPos(ImVec2(100, 100));
        ctx->SetRef("Clear all?");
        ctx->KeyDown(ImGuiKey_Enter);
        IM_CHECK_EQ(mApp->selections.size(), 0);
        };
    t = IM_REGISTER_TEST(engine, "SelectionMenu", "Select Desorption");
    t->TestFunc = [this](ImGuiTestContext* ctx) {
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("###Selection/Select Desorption");
        };
    t = IM_REGISTER_TEST(engine, "SelectionMenu", "Select Outgassing Map");
    t->TestFunc = [this](ImGuiTestContext* ctx) {
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("###Selection/Select Outgassing Map");
        };
    t = IM_REGISTER_TEST(engine, "SelectionMenu", "Select Reflective");
    t->TestFunc = [this](ImGuiTestContext* ctx) {
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("###Selection/Select Reflective");
        };
    t = IM_REGISTER_TEST(engine, "ToolsMenu", "Formula editor");
    t->TestFunc = [this](ImGuiTestContext* ctx) {
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("###Tools/###Formula editor");
        // For some reason cannot click on the input inside a table cell
        //ctx->ItemClick(ctx->GetID("/Formula editor/###Formula list/##FL/##NE"));
        //ctx->KeyCharsReplace("10");
        ctx->SetRef("Formula editor");
        ctx->ItemClick("Recalculate now");
        ctx->ItemClick("Record values for convergence");
        ctx->ItemClick("Auto-update formulas");
        ctx->ItemClick("Syntax help");
        ctx->MouseMoveToPos(ImVec2(100, 100));
        ctx->SetRef("Formula Editor Syntax Help");
        ctx->ItemClick("Close");
        ctx->SetRef("Formula editor");
        ctx->ItemClick("Record values for convergence");
        ctx->ItemClick("Auto-update formulas");
        ctx->ItemClick("Open convergence plotter >>");
        ctx->SetRef("Convergence Plotter");
        ctx->ItemClick("#CLOSE");
        ctx->SetRef("Formula editor");
        ctx->ItemClick("#CLOSE");
        // TODO Expand this test beyond just opening and closing the window
        };
    t = IM_REGISTER_TEST(engine, "ToolsMenu", "Convergence plotter");
    t->TestFunc = [this](ImGuiTestContext* ctx) {
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("###Tools/Convergence Plotter ...");
        ctx->SetRef("Convergence Plotter");
        ctx->ItemClick("Add curve");
        ctx->ItemClick("Remove curve");
        ctx->MouseMoveToPos(ImVec2(100, 100));
        ctx->SetRef("Error");
        ctx->ItemClick("  Ok  ");
        ctx->SetRef("Convergence Plotter");
        ctx->ItemClick("Remove all");
        IM_CHECK_EQ(mApp->imWnd->convPlot.data.size(), 0);
        // Export Menu
        ctx->MenuClick("Export/All to clipboard");
        ctx->MouseMoveToPos(ImVec2(100, 100));
        ctx->SetRef("Error");
        ctx->ItemClick("  Ok  ");
        ctx->SetRef("Convergence Plotter");
        ctx->MenuClick("Export/All to file");
        ctx->MouseMoveToPos(ImVec2(100, 100));
        ctx->SetRef("Error");
        ctx->ItemClick("  Ok  ");
        ctx->SetRef("Convergence Plotter");
        ctx->MenuClick("Export/Plotted to clipboard");
        ctx->MouseMoveToPos(ImVec2(100, 100));
        ctx->SetRef("Error");
        ctx->ItemClick("  Ok  ");
        ctx->SetRef("Convergence Plotter");
        ctx->MenuClick("Export/Plotted to file");
        ctx->MouseMoveToPos(ImVec2(100, 100));
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
        ctx->KeyCharsReplace("abc");
        ctx->MenuClick("View/##lineWidth");
        ctx->KeyCharsReplace("-1");
        ctx->MenuClick("View/##plotMax");
        ctx->KeyCharsReplace("abc");
        ctx->MenuClick("View/##plotMax");
        ctx->KeyCharsReplace("-1");
        ctx->MenuClick("View/Display hovered value");
        ctx->MenuClick("View/Display hovered value");
        ctx->MenuClick("Custom Plot/##expressionInput");
        ctx->KeyCharsReplace("x");
        ctx->MenuClick("Custom Plot/-> Plot expression");
        ctx->MenuClick("Custom Plot/##expressionInput");
        ctx->KeyCharsReplace("");
        ctx->MenuClick("Custom Plot/-> Plot expression");
        ctx->ItemClick("#CLOSE");
        };
    // VIEW
    t = IM_REGISTER_TEST(engine, "ViewMenu", "FullScreen");
    t->TestFunc = [this](ImGuiTestContext* ctx) {
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("View/###Full Screen");
        ctx->MouseMoveToPos(ImVec2(100,100));
        ctx->MenuClick("View/###Full Screen");
        };
}
