#include "bas/proc/MyStackWalker.hpp"
#include "bas/ui/arch/ImageSet.hpp"
#include "wx/aui/auibar.h"
#include "wx/aui/framemanager.h"

#include <wx/artprov.h>
#include <wx/aui/aui.h>
#include <wx/wx.h>

class MyFrame : public wxFrame {
  public:
    MyFrame()
        : wxFrame(NULL, wxID_ANY, "Multiple AUI Toolbars", wxDefaultPosition, wxSize(800, 600)) {
        m_mgr.SetManagedWindow(this);

        long style = wxAUI_TB_DEFAULT_STYLE | wxAUI_TB_TEXT;
        m_tb1 = new wxAuiToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, style);
        m_tb2 = new wxAuiToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, style);

        populate();
        addPanes();
        // updateAuiPaneInfo();
        m_mgr.Update();

        // First update with empty toolbars, then populate after AddPane.
        // m_mgr.Update();
        CallAfter([this]() { populate(); });
    }
    ~MyFrame() { m_mgr.UnInit(); }

  private:
    wxAuiManager m_mgr;
    wxAuiToolBar* m_tb1{nullptr};
    wxAuiToolBar* m_tb2{nullptr};

    void populate();
    void addTool(wxAuiToolBar* tb, int id, const wxString& label, const ImageSet& icon);
    void addPanes();
    void addToPane(wxAuiToolBar* tb, int position, const wxString& name, const wxString& caption);
    void updateAuiPaneInfo();
};

void MyFrame::populate() {
    std::string dir1 = "streamline-vectors/core/pop/interface-essential";

    ImageSet newFile = ImageSet(wxART_NEW, dir1, "new-file.svg");
    ImageSet openFile = ImageSet(wxART_FILE_OPEN, dir1, "open-book.svg");
    ImageSet saveFile = ImageSet(wxART_FILE_SAVE, dir1, "file-add-alternate.svg");
    ImageSet cut = ImageSet(wxART_CUT, dir1, "cut.svg");
    ImageSet copy = ImageSet(wxART_COPY, dir1, "clipboard-add.svg");
    ImageSet paste = ImageSet(wxART_PASTE, dir1, "empty-clipboard.svg");

    addTool(m_tb1, 1, "New", newFile);
    addTool(m_tb1, 2, "Open", openFile);
    m_tb1->AddSeparator();
    addTool(m_tb1, 3, "Save", saveFile);

    addTool(m_tb2, 4, "Cut", cut);
    addTool(m_tb2, 5, "Copy", copy);
    addTool(m_tb2, 6, "Paste", paste);
}

void MyFrame::addTool(wxAuiToolBar* tb, int id, const wxString& label, const ImageSet& icon) {
    int size = 32;
    wxBitmap bmp = icon.toBitmap1(size, size, wxART_TOOLBAR);
    auto tool = tb->AddTool(id, label, bmp);
}

void MyFrame::addPanes() {
    addToPane(m_tb1, 1, "toolbar1", "File Tools");
    addToPane(m_tb2, 2, "toolbar2", "Edit Tools");
}

void MyFrame::addToPane(wxAuiToolBar* tb, int position, const wxString& name,
                        const wxString& caption) {
    m_mgr.AddPane(tb, wxAuiPaneInfo()
                          .Name(name)
                          .Caption(caption)
                          .ToolbarPane()
                          .Layer(10)
                          .Top()
                          .Row(0)
                          .Position(position)
                          .Dockable(true)
                          .Movable(true)
                          .Floatable(true)
                          .Gripper(true)
                          .PaneBorder(false)
                          .CaptionVisible(false));
}

void MyFrame::updateAuiPaneInfo() {
    wxAuiPaneInfoArray& allPanes = m_mgr.GetAllPanes();
    for (size_t i = 0; i < allPanes.GetCount(); ++i) {
        wxAuiPaneInfo& pane = allPanes.Item(i);
        if (pane.IsToolbar() && pane.window) {
            wxAuiToolBar* tb = wxDynamicCast(pane.window, wxAuiToolBar);
            if (tb) {
                tb->Realize();
                wxSize size = tb->GetSize();
                wxSize bestSize = tb->GetBestSize();
                wxSize minSize = tb->GetMinSize();
                wxSize maxSize = tb->GetMaxSize();
                printf("toolbar id: %d, size: %d, %d, bestSize: %d, %d, minSize: %d, %d, maxSize: "
                       "%d, %d\n",
                       tb->GetId(), size.GetWidth(), size.GetHeight(), bestSize.GetWidth(),
                       bestSize.GetHeight(), minSize.GetWidth(), minSize.GetHeight(),
                       maxSize.GetWidth(), maxSize.GetHeight());
                pane.BestSize(bestSize);
                pane.MinSize(minSize);
                pane.MaxSize(maxSize);
            }
        }
    }
    m_mgr.Update();
}

class MyApp : public wxApp {
  public:
    bool OnInit() {
        (new MyFrame())->Show();
        return true;
    }
};
wxIMPLEMENT_APP(MyApp);
