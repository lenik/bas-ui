#include "AutomationQuery.hpp"

#include "AutomationError.hpp"
#include "WidgetTraits.hpp"

#include "../arch/UIElement.hpp"
#include "../arch/UIState.hpp"

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/combobox.h>
#include <wx/listbox.h>
#include <wx/radiobut.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

namespace bas::ui::automation {

namespace {

std::string wxToUtf8(const wxString& s) { return std::string(s.utf8_str()); }

} // namespace

AutomationQuery::AutomationQuery(wxWindow* root, const AutomationMap& map)
    : m_root(root), m_map(map) {}

wxWindow* AutomationQuery::find(const std::string& objId) const {
    if (!m_root) {
        return nullptr;
    }
    return m_map.resolve(m_root, objId);
}

UIElement* AutomationQuery::findElement(const std::string& objId) const {
    return m_map.resolveElement(objId);
}

bool AutomationQuery::exists(const std::string& objId) const {
    return find(objId) != nullptr || findElement(objId) != nullptr;
}

bool AutomationQuery::elementExists(const std::string& objId) const {
    return findElement(objId) != nullptr;
}

wxWindow* AutomationQuery::require(const std::string& objId) const {
    wxWindow* w = find(objId);
    if (!w) {
        throw AutomationError("query", objId, "target not found");
    }
    return w;
}

bool AutomationQuery::isEnabled(const std::string& objId) const {
    if (UIElement* el = findElement(objId)) {
        return el->isEnabled();
    }
    return require(objId)->IsEnabled();
}

bool AutomationQuery::isShown(const std::string& objId) const {
    if (UIElement* el = findElement(objId)) {
        return el->isVisible();
    }
    return require(objId)->IsShown();
}

bool AutomationQuery::isChecked(const std::string& objId) const {
    if (UIElement* el = findElement(objId)) {
        if (auto* state = dynamic_cast<UIState*>(el)) {
            if (state->getType() == UIStateType::BOOL &&
                std::holds_alternative<bool>(state->value.get())) {
                return std::get<bool>(state->value.get());
            }
        }
        throw AutomationError("query", objId, "element does not support isChecked");
    }
    wxWindow* w = require(objId);
    if (auto* box = asCheckBox(w)) {
        return box->GetValue();
    }
    if (auto* radio = asRadioButton(w)) {
        return radio->GetValue();
    }
#if wxUSE_CHECKLISTBOX
    if (auto* list = asCheckListBox(w)) {
        const int sel = list->GetSelection();
        return sel != wxNOT_FOUND && list->IsChecked(sel);
    }
#endif
    throw AutomationError("query", objId, "target does not support isChecked");
}

std::string AutomationQuery::getLabel(const std::string& objId) const {
    if (UIElement* el = findElement(objId)) {
        return el->label.get();
    }
    wxWindow* w = require(objId);
    if (auto* btn = asButton(w)) {
        return wxToUtf8(btn->GetLabel());
    }
    if (auto* box = asCheckBox(w)) {
        return wxToUtf8(box->GetLabel());
    }
    if (auto* radio = asRadioButton(w)) {
        return wxToUtf8(radio->GetLabel());
    }
    if (auto* st = asStaticText(w)) {
        return wxToUtf8(st->GetLabel());
    }
    return wxToUtf8(w->GetLabel());
}

std::string AutomationQuery::getText(const std::string& objId) const {
    wxWindow* w = find(objId);
    if (w) {
        if (auto* text = asTextCtrl(w)) {
            return wxToUtf8(text->GetValue());
        }
        if (auto* combo = asComboBox(w)) {
            return wxToUtf8(combo->GetValue());
        }
        if (auto* st = asStaticText(w)) {
            return wxToUtf8(st->GetLabel());
        }
        return getLabel(objId);
    }
    return getLabel(objId);
}

boost::json::value AutomationQuery::getStateValue(const std::string& objId) const {
    UIElement* el = findElement(objId);
    if (!el) {
        throw AutomationError("query", objId, "UI element not found");
    }
    auto* state = dynamic_cast<UIState*>(el);
    if (!state) {
        throw AutomationError("query", objId, "element is not a UIState");
    }
    const auto& v = state->value.get();
    if (std::holds_alternative<bool>(v)) {
        return std::get<bool>(v);
    }
    if (std::holds_alternative<int>(v)) {
        return std::get<int>(v);
    }
    if (std::holds_alternative<double>(v)) {
        return std::get<double>(v);
    }
    if (std::holds_alternative<std::string>(v)) {
        return boost::json::value(std::get<std::string>(v).c_str());
    }
    return nullptr;
}

std::string AutomationQuery::getValue(const std::string& objId) const {
    if (findElement(objId)) {
        try {
            auto v = getStateValue(objId);
            if (v.is_bool()) {
                return v.as_bool() ? "true" : "false";
            }
            if (v.is_int64()) {
                return std::to_string(v.as_int64());
            }
            if (v.is_string()) {
                return std::string(v.as_string().c_str());
            }
        } catch (const AutomationError&) {
            // fall through to widget / label
        }
    }
    wxWindow* w = find(objId);
    if (!w) {
        return getLabel(objId);
    }
    if (asTextCtrl(w) || asComboBox(w) || asStaticText(w)) {
        return getText(objId);
    }
    if (auto* box = asCheckBox(w)) {
        return box->GetValue() ? "true" : "false";
    }
    if (auto* radio = asRadioButton(w)) {
        return radio->GetValue() ? "true" : "false";
    }
    if (auto* choice = asChoice(w)) {
        const int sel = choice->GetSelection();
        return sel == wxNOT_FOUND ? std::string{} : wxToUtf8(choice->GetString(sel));
    }
    if (auto* list = asListBox(w)) {
        const int sel = list->GetSelection();
        return sel == wxNOT_FOUND ? std::string{} : wxToUtf8(list->GetString(sel));
    }
    return getLabel(objId);
}

int AutomationQuery::getSelection(const std::string& objId) const {
    wxWindow* w = require(objId);
    if (auto* choice = asChoice(w)) {
        return choice->GetSelection();
    }
    if (auto* combo = asComboBox(w)) {
        return combo->GetSelection();
    }
    if (auto* list = asListBox(w)) {
        return list->GetSelection();
    }
    throw AutomationError("query", objId, "target does not support getSelection");
}

std::optional<std::string> AutomationQuery::getSelectedString(const std::string& objId) const {
    const int sel = getSelection(objId);
    if (sel == wxNOT_FOUND) {
        return std::nullopt;
    }
    wxWindow* w = require(objId);
    if (auto* choice = asChoice(w)) {
        return wxToUtf8(choice->GetString(sel));
    }
    if (auto* combo = asComboBox(w)) {
        return wxToUtf8(combo->GetString(sel));
    }
    if (auto* list = asListBox(w)) {
        return wxToUtf8(list->GetString(sel));
    }
    return std::nullopt;
}

void AutomationQuery::dumpTreeRecursive(wxWindow* node, boost::json::array& out, int depth) {
    if (!node) {
        return;
    }
    boost::json::object item;
    item["depth"] = depth;
    item["id"] = node->GetId();
    const wxString className =
        node->GetClassInfo() ? wxString(node->GetClassInfo()->GetClassName()) : wxString("unknown");
    item["class"] = wxToUtf8(className);
    item["name"] = wxToUtf8(node->GetName());
    item["label"] = wxToUtf8(node->GetLabel());
    item["shown"] = node->IsShown();
    item["enabled"] = node->IsEnabled();
    out.push_back(std::move(item));

    for (wxWindowList::compatibility_iterator child = node->GetChildren().GetFirst(); child;
         child = child->GetNext()) {
        dumpTreeRecursive(child->GetData(), out, depth + 1);
    }
}

boost::json::array AutomationQuery::dumpTree() const {
    boost::json::array out;
    dumpTreeRecursive(m_root, out, 0);
    return out;
}

} // namespace bas::ui::automation
