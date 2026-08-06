#include "demo_dialog.hpp"

#include <wx/sizer.h>
#include <wx/stattext.h>

DemoDialog::DemoDialog(wxWindow* parent)
    : wxDialog(parent, wxID_ANY, "Automation Demo Dialog", wxDefaultPosition, wxDefaultSize,
               wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
      Automatable(this) {
    auto* root = new wxBoxSizer(wxVERTICAL);

    auto* grid = new wxFlexGridSizer(2, wxSize(8, 6));
    grid->AddGrowableCol(1, 1);

    grid->Add(new wxStaticText(this, wxID_ANY, "Name:"), 0, wxALIGN_CENTER_VERTICAL);
    m_name = new wxTextCtrl(this, ID_NAME);
    m_name->SetName("name");
    grid->Add(m_name, 1, wxEXPAND);

    grid->Add(new wxStaticText(this, wxID_ANY, "Role:"), 0, wxALIGN_CENTER_VERTICAL);
    m_role = new wxChoice(this, ID_ROLE);
    m_role->Append("guest");
    m_role->Append("admin");
    m_role->Append("operator");
    m_role->SetSelection(0);
    m_role->SetName("role");
    grid->Add(m_role, 1, wxEXPAND);

    root->Add(grid, 0, wxALL | wxEXPAND, 10);

    m_agree = new wxCheckBox(this, ID_AGREE, "I agree");
    m_agree->SetName("agree");
    root->Add(m_agree, 0, wxLEFT | wxRIGHT | wxBOTTOM, 10);

    m_go = new wxButton(this, ID_GO, "Go");
    m_go->SetName("go");
    root->Add(m_go, 0, wxALL, 10);

    SetSizerAndFit(root);

    automationMap().bind("name", ID_NAME);
    automationMap().bind("role", ID_ROLE);
    automationMap().bind("agree", ID_AGREE);
    automationMap().bind("go", ID_GO);

    m_go->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { m_goClicked = true; });
}
