/**
 * E2E tests for bas::ui::automation: widget dialog + UI arch frame.
 */

#include "demo_dialog.hpp"
#include "demo_frame.hpp"

#include <bas/ui/automation/AutomationError.hpp>
#include <bas/ui/automation/EventRegistry.hpp>
#include <bas/ui/automation/ScriptRunner.hpp>

#include <wx/app.h>

#include <cstdlib>
#include <iostream>
#include <string>

namespace {

class TestApp : public wxApp {
  public:
    bool OnInit() override { return true; }
};

void expect(bool cond, const char* msg) {
    if (!cond) {
        std::cerr << "FAIL: " << msg << "\n";
        std::exit(1);
    }
}

void testEventRegistry() {
    using namespace bas::ui::automation;
    const auto& reg = EventRegistry::instance();
    expect(reg.find("click") != nullptr, "registry has click");
    expect(reg.find("press") != nullptr, "alias press -> click");
    expect(reg.find("perform") != nullptr, "registry has perform");
    expect(reg.find("setstate") != nullptr, "registry has setstate");
    expect(reg.find("wait") == nullptr, "wait not in DefaultAutomation registry");
#if BAS_WX_MODERN
    expect(reg.find("magnify") != nullptr, "modern: magnify registered");
    expect(reg.find("gesturepan") != nullptr, "modern: gesturepan registered");
#else
    expect(reg.find("magnify") == nullptr, "legacy: magnify absent");
#endif
    expect(!reg.names().empty(), "registry names non-empty");
}

void testDemoDialog() {
    using namespace bas::ui::automation;

    DemoDialog dlg;
    expect(dlg.query().exists("name"), "name exists");

    dlg.emulate("setValue", "name", {{"text", "alice"}});
    expect(dlg.query().getValue("name") == "alice", "setValue name");

    dlg.emulate("select", "role", {{"value", "admin"}});
    expect(dlg.query().getSelection("role") == 1, "select role");

    dlg.emulate("check", "agree");
    expect(dlg.query().isChecked("agree"), "check agree");

    dlg.emulate("click", "go", {{"x", 3}, {"y", 3}});
    expect(dlg.goClicked(), "go clicked");

    ScriptRunner runner(dlg);
    runner.runJson(R"([
      {"do": "setValue", "on": "name", "data": {"text": "bob"}},
      {"do": "uncheck", "on": "agree"},
      {"do": "wait", "data": {"ms": 1}},
      ["click", "go"]
    ])");
    expect(dlg.query().getValue("name") == "bob", "script name");
    expect(!dlg.query().isChecked("agree"), "script uncheck");

    expect(!dlg.tryEmulate("click", "nope"), "soft fail missing");
}

void testDemoFrameArch() {
    using namespace bas::ui::automation;

    DemoFrame frame;
    expect(frame.query().elementExists("hello") || frame.query().elementExists("/demo/hello") ||
               frame.query().elementExists("demo/hello"),
           "arch hello bound");
    expect(frame.query().exists("editor"), "editor bound");

    frame.emulate("setValue", "editor", {{"text", "start\n"}});
    expect(frame.query().getValue("editor") == "start\n", "editor setValue");

    // perform via leaf name or path
    const char* helloId = frame.query().elementExists("hello")            ? "hello"
                          : frame.query().elementExists("demo/hello")     ? "demo/hello"
                          : frame.query().elementExists("/demo/hello")    ? "/demo/hello"
                                                                          : "hello";
    frame.emulate("perform", helloId);
    expect(frame.body().helloCount() == 1, "perform hello");
    expect(frame.query().getValue("editor").find("hello") != std::string::npos, "hello appended");

    const char* boldId = frame.query().elementExists("bold")         ? "bold"
                         : frame.query().elementExists("demo/bold")  ? "demo/bold"
                         : frame.query().elementExists("/demo/bold") ? "/demo/bold"
                                                                     : "bold";
    expect(!frame.query().isChecked(boldId), "bold initially false");
    frame.emulate("setstate", boldId, {{"value", true}});
    expect(frame.query().isChecked(boldId), "setstate bold");
    frame.emulate("togglestate", boldId);
    expect(!frame.query().isChecked(boldId), "togglestate bold");

    frame.emulate("perform", frame.query().elementExists("clear")         ? "clear"
                             : frame.query().elementExists("demo/clear")  ? "demo/clear"
                             : frame.query().elementExists("/demo/clear") ? "/demo/clear"
                                                                          : "clear");
    expect(frame.query().getValue("editor").empty(), "clear editor via perform");
}

int runAll() {
    testEventRegistry();
    testDemoDialog();
    testDemoFrameArch();
    std::cout << "e2e_automation_test: OK\n";
    return 0;
}

} // namespace

wxIMPLEMENT_APP_NO_MAIN(TestApp);

int main(int argc, char** argv) {
    wxApp::SetInstance(new TestApp());
    if (!wxEntryStart(argc, argv)) {
        std::cerr << "wxEntryStart failed\n";
        return 2;
    }
    if (!wxTheApp->CallOnInit()) {
        wxEntryCleanup();
        return 2;
    }
    const int rc = runAll();
    wxTheApp->OnExit();
    wxEntryCleanup();
    return rc;
}
