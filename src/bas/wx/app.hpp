#ifndef WX_APP_HPP
#define WX_APP_HPP

#include <wx/app.h>
#include <wx/frame.h>
#include <wx/string.h>
#include <bas/locale/i18n.h>
#include <bas/proc/env.h>

#include <config.h>


class uiApp : public wxApp {
  public:
    uiApp() : wxApp() {}
    virtual ~uiApp() {}

    bool OnInit() override;

    virtual bool OnUserInit() =0;

    virtual void OnAssertFailure(const wxChar* file, int line, const wxChar* func, const wxChar* cond,
                         const wxChar* msg);

    int main(int argc, char** argv) {
        const char *exe = self_exe();
        (void)exe;
        init_i18n(LOCALEDIR);
        return run(argc, argv);
    }

  protected:
    int run(int argc, char** argv);
};

#endif // WX_APP_HPP
