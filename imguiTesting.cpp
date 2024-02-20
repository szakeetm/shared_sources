#include "imguiTesting.h"
#if defined(MOLFLOW)
#include "../../src/MolFlow.h"
#else
#include "../../src/SynRad.h"
#endif

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
    if (!showTestEngine) return;
    ImGui::StyleColorsDark(); // on light background the log is not readable
    ImGuiTestEngine_ShowTestEngineWindows(engine, &showTestEngine);
    ImGui::StyleColorsLight();
}

void ImTest::PostSwap()
{
    ImGuiTestEngine_PostSwap(engine);
}

void ImTest::RegisterTests()
{
    ImGuiTest* t = NULL;
    t = IM_REGISTER_TEST(engine, "Group", "Title");
    t->TestFunc = [this](ImGuiTestContext* ctx) {

        };
}
