#ifndef BAS_UI_AUTOMATION_WIDGET_TRAITS_HPP
#define BAS_UI_AUTOMATION_WIDGET_TRAITS_HPP

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/combobox.h>
#include <wx/listbox.h>
#include <wx/radiobut.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/window.h>

#if wxUSE_CHECKLISTBOX
#include <wx/checklst.h>
#endif

namespace bas::ui::automation {

template <typename T>
inline T* asWidget(wxWindow* w) {
    return w ? dynamic_cast<T*>(w) : nullptr;
}

inline wxButton* asButton(wxWindow* w) { return asWidget<wxButton>(w); }
inline wxTextCtrl* asTextCtrl(wxWindow* w) { return asWidget<wxTextCtrl>(w); }
inline wxCheckBox* asCheckBox(wxWindow* w) { return asWidget<wxCheckBox>(w); }
inline wxRadioButton* asRadioButton(wxWindow* w) { return asWidget<wxRadioButton>(w); }
inline wxChoice* asChoice(wxWindow* w) { return asWidget<wxChoice>(w); }
inline wxComboBox* asComboBox(wxWindow* w) { return asWidget<wxComboBox>(w); }
inline wxListBox* asListBox(wxWindow* w) { return asWidget<wxListBox>(w); }
inline wxStaticText* asStaticText(wxWindow* w) { return asWidget<wxStaticText>(w); }

#if wxUSE_CHECKLISTBOX
inline wxCheckListBox* asCheckListBox(wxWindow* w) { return asWidget<wxCheckListBox>(w); }
#endif

inline bool isTextLike(wxWindow* w) {
    return asTextCtrl(w) || (asComboBox(w) && asComboBox(w)->IsEditable());
}

} // namespace bas::ui::automation

#endif
