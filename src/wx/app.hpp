#include "ui/arch/UIFragment.hpp"

#include <wx/frame.h>
#include <wx/app.h>

#include <vector>

class uiApp : public wxApp {

    wxFrame* m_frame;
    std::vector<UIFragment*> fragments;

  public:
    uiApp(wxFrame* frame, std::vector<UIFragment*> fragments)
        : m_frame(frame), fragments(fragments) {}

    bool OnInit() override;

    void OnAssertFailure(const wxChar* file, int line, const wxChar* func, const wxChar* cond,
                         const wxChar* msg);

    static int main(int argc, char** argv, UIFragment* fragment, ...);
};
