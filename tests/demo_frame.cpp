#include "demo_frame.hpp"

DemoBody::DemoBody() {
    defineActions();
    defineStates();
}

void DemoBody::defineActions() {
    int seq = 0;
    action(ID_SAY_HELLO, "demo", "hello", seq++, "&Hello", "Append hello")
        .shortcut("Ctrl+H")
        .performFn([this](PerformContext* ctx) { doHello(ctx); })
        .install();
    action(ID_CLEAR, "demo", "clear", seq++, "&Clear", "Clear editor")
        .shortcut("Ctrl+K")
        .performFn([this](PerformContext* ctx) { doClear(ctx); })
        .install();
}

void DemoBody::defineStates() {
    state(ID_BOLD, "demo", "bold", 0, "Bold", "Toggle bold flag")
        .stateType(UIStateType::BOOL)
        .initValue(false)
        .install();
}

wxWindow* DemoBody::createFragmentView(CreateViewContext* ctx) {
    m_editor = new wxTextCtrl(ctx->getParent(), wxID_ANY, "", wxDefaultPosition, wxDefaultSize,
                              wxTE_MULTILINE);
    m_editor->SetName("editor");
    return m_editor;
}

void DemoBody::doHello(PerformContext*) {
    ++m_helloCount;
    if (m_editor) {
        m_editor->AppendText("hello\n");
    }
}

void DemoBody::doClear(PerformContext*) {
    if (m_editor) {
        m_editor->Clear();
    }
}

DemoFrame::DemoFrame(const wxString& title) : uiFrame(title) {
    group(10, "", "demo", 10, "&Demo").install();
    addFragment(&m_body);
    createView();
    if (wxWindow* editor = FindWindowByName("editor")) {
        automationMap().bind("editor", editor);
    }
}
