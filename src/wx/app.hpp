#include <wx/app.h>
#include <wx/frame.h>
#include <wx/string.h>

class uiApp : public wxApp {
  public:
    uiApp() : wxApp() {}
    virtual ~uiApp() {}

    bool OnInit() override = 0;

    void OnAssertFailure(const wxChar* file, int line, const wxChar* func, const wxChar* cond,
                         const wxChar* msg);

    int main(int argc, char** argv);
};
