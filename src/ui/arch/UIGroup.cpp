#include "UIGroup.hpp"

#include "UIAction.hpp"
#include "UIState.hpp"
#include "UIWidgetsContext.hpp"

#include <bas/log/uselog.h>

#include <wx/bitmap.h>
#include <wx/menu.h>
#include <wx/string.h>
#include <wx/toolbar.h>

#include <iostream>
#include <memory>
#include <string>
#include <vector>

// --- InstallRecords implementation ---

InstallRecord::~InstallRecord() {
    switch (kind) {
        case MENU:
            if (menuBar && menuPos >= 0) {
                wxMenu* _menu = menuBar->Remove(menuPos);
                assert(_menu && _menu == menu);
                delete _menu;
                menu = nullptr;
                menuPos = -1;
            }
            break;

        case SUBMENU:
            if (menu && subMenuId >= 0) {
                wxMenuItem* subMenuItem = menu->Remove(subMenuId);
                assert(subMenuItem && subMenuItem == menuItem);
                // wxMenu *subMenu = subMenuItem->GetSubMenu();
                delete subMenuItem; // auto delete submenu
                menuItem = nullptr;
                subMenuId = -1;
            }
            break;

        case MENU_ITEM:
            if (menu && menuItem) {
                menu->Remove(menuItem);
                delete menuItem;
                menuItem = nullptr;
            }
            break;

        case TOOLBAR_TOOL:
            if (toolbar && toolId >= 0) {
                toolbar->RemoveTool(toolId);
                toolId = -1;
            }
            break;
        default:
            break;
    }
}

void InstallRecords::dump(std::ostream& output) const {
    output << "InstallRecords (" << size() << " items):\n";
    for (size_t i = 0; i < size(); ++i) {
        const InstallRecord* rec = (*this)[i].get();
        output << "  [" << i << "] ";
        
        switch (rec->kind) {
            case InstallRecord::MENU:
                output << "MENU";
                if (rec->menuBar && rec->menuPos >= 0) {
                    output << " (menuBar=" << rec->menuBar << ", pos=" << rec->menuPos << ")";
                }
                break;
            case InstallRecord::SUBMENU:
                output << "SUBMENU";
                if (rec->menu && rec->subMenuId >= 0) {
                    output << " (menu=" << rec->menu << ", id=" << rec->subMenuId << ")";
                }
                break;
            case InstallRecord::MENU_ITEM:
                output << "MENU_ITEM";
                if (rec->menu && rec->menuItem) {
                    output << " (menu=" << rec->menu << ", item=" << rec->menuItem << ")";
                }
                break;
            case InstallRecord::TOOLBAR_TOOL:
                output << "TOOLBAR_TOOL";
                if (rec->toolbar && rec->toolId >= 0) {
                    output << " (toolbar=" << rec->toolbar << ", id=" << rec->toolId << ")";
                }
                break;
            default:
                output << "UNKNOWN";
                break;
        }
        
        if (rec->group) {
            output << " -> group=" << rec->group;
        }
        
        output << "\n";
    }
}

UIGroup* UIGroup::resolveGroup(const Path& path) {
    return resolveGroup(path.str());
}

int UIGroup::s_next_internal_id = 10000;

UIGroup* UIGroup::resolveGroup(std::string_view path) {
    while (!path.empty() && path.front() == '/')
        path.remove_prefix(1);
    if (path.empty()) return this;

    int slash = path.find('/');
    std::string_view head = slash == std::string::npos ? path
        : path.substr(0, slash);

    UIElement* child = getChild(head);
    if (!child) {
        std::string dir = this->path ? this->path->str() : std::string();
        auto& owned = internals.emplace_back(
            std::make_unique<UIGroup>(
                s_next_internal_id++,
                dir,
                std::string(head),
                0,
                "<internal>",
                "",
                "",
                ImageSet(),
                true
            )
        );
        owned->internal = true;
        child = owned.get();
        child->attach(this);
    }
    
    if (!child->isGroup())
        return nullptr;

    UIGroup* child_group = dynamic_cast<UIGroup*>(child);
    if (slash == std::string::npos)
        return child_group;

    std::string_view tail = path.substr(slash + 1);
    return child_group->resolveGroup(tail);
}

void UIGroup::buildTree(std::vector<UIElement*>& elements) {
    for (UIElement* el : elements) {
        std::optional<Path> path = el->path;
        if (!path) continue;
        std::string dir = path->dir();
        UIGroup* parent = resolveGroup(dir);
        if (parent == nullptr) {
            throw std::runtime_error("dir collision: " + dir);
        }
        el->attach(parent);
        logdebug_fmt("resolved path %s dir %s attached to %s\n", 
            path->str().c_str(), dir.c_str(), parent->str().c_str());
    }
}

static wxString toWx(const std::string& s) {
    return wxString::FromUTF8(s.c_str());
}

void UIGroup::setUp(UIWidgetsContext& context, InstallRecords& installs) {
    for (UIElement* el : children) {
        if (!el->visible.get()) continue;
        std::string dir = el->dir();

        wxString label = toWx(el->label.get().empty() ? el->name() : el->label.get());
        wxString help = toWx(el->description.get());

        ImageSet icon = el->icon.get();

        std::vector<wxMenuBar*> menubars = context.getMenubars(dir);
        std::vector<wxMenu*> menus = context.getMenus(dir);
        std::vector<wxToolBar*> toolbars = context.getToolbars(dir);

        if (el->isGroup()) {
            UIGroup* childGroup = dynamic_cast<UIGroup*>(el);
            if (!childGroup) continue;
            
            for (wxMenuBar* mb : menubars) {
                int menuPos = mb->GetMenuCount();
                wxMenu* menu = new wxMenu();
                mb->Append(menu, label);

                // if (icon.isSet())
                //     menu->SetBitmap(icon.loadBitmap(24, 24));

                std::string menuPath = el->path ? el->path->str() : dir;
                context.registerMenu(menuPath, menu);
                
                auto rec = std::make_unique<InstallRecord>();
                rec->kind = InstallRecord::MENU;
                rec->menuBar = mb;
                rec->menuPos = menuPos;
                rec->group = childGroup;
                installs.push_back(std::move(rec));
            }
            
            for (wxMenu* m : menus) {
                wxMenu* submenu = new wxMenu();
                wxMenuItem* item = m->AppendSubMenu(submenu, label, help);

                if (icon.isSet()) {
                    item->SetBitmap(icon.loadBitmap(24, 24));
                }

                std::string menuPath = el->path ? el->path->str() : dir;
                context.registerMenu(menuPath, submenu);

                auto rec = std::make_unique<InstallRecord>();
                rec->kind = InstallRecord::SUBMENU;
                rec->menu = m;
                rec->subMenuId = item->GetId();
                rec->menuItem = item;
                rec->group = childGroup;
                installs.push_back(std::move(rec));
            }

            for (wxToolBar* tb : toolbars) {
                auto a = tb->AddTool(el->id, label,
                    icon.loadBitmap(24, 24), help);
                
                auto rec = std::make_unique<InstallRecord>();
                rec->kind = InstallRecord::TOOLBAR_TOOL;
                rec->toolbar = tb;
                rec->toolId = el->id;
                installs.push_back(std::move(rec));
            }

            childGroup->setUp(context, installs);
        }
        
        else if (el->isAction()) {
            UIAction* action = dynamic_cast<UIAction*>(el);
            if (!action) continue;

            for (wxMenu* m : menus) {
                wxMenuItem* item = m->Append(el->id, label, help);
                if (icon.isSet()) {
                    item->SetBitmap(icon.loadBitmap(24, 24));
                }

                auto rec = std::make_unique<InstallRecord>();
                rec->kind = InstallRecord::MENU_ITEM;
                rec->menu = m;
                rec->menuItem = item;
                installs.push_back(std::move(rec));
            }
            for (wxToolBar* tb : toolbars) {
                tb->AddTool(el->id, label, 
                    icon.loadBitmap(24, 24), help, wxITEM_NORMAL);
                
                    auto rec = std::make_unique<InstallRecord>();
                rec->kind = InstallRecord::TOOLBAR_TOOL;
                rec->toolbar = tb;
                rec->toolId = el->id;
                installs.push_back(std::move(rec));
            }
        }
        
        else if (el->isState()) {
            UIState* state = dynamic_cast<UIState*>(el);
            if (!state) continue;

            if (state->getType() == UIStateType::BOOL) {
                bool checked = false;
                if (auto* p = std::get_if<bool>(&state->value.get()))
                    checked = *p;
                for (wxMenu* m : menus) {
                    wxMenuItem* item = m->AppendCheckItem(el->id, label, help);
                    if (icon.isSet()) {
                        item->SetBitmap(icon.loadBitmap(24, 24));
                    }
                    m->Check(el->id, checked);
                    auto rec = std::make_unique<InstallRecord>();
                    rec->kind = InstallRecord::MENU_ITEM;
                    rec->menu = m;
                    rec->menuItem = item;
                    installs.push_back(std::move(rec));
                }
                for (wxToolBar* tb : toolbars) {
                    tb->AddTool(el->id, label, wxBitmap(), help, wxITEM_CHECK);
                    auto rec = std::make_unique<InstallRecord>();
                    rec->kind = InstallRecord::TOOLBAR_TOOL;
                    rec->toolbar = tb;
                    rec->toolId = el->id;
                    installs.push_back(std::move(rec));
                }
            } else if (state->getType() == UIStateType::ENUM) {
                const int enumCount = state->getEnumCount();
                for (wxMenu* m : menus) {
                    wxMenu* submenu = new wxMenu();
                    for (int v = 0; v < enumCount; ++v) {
                        UIStateValueDescriptor d = state->getValueDescriptor(v);
                        if (d.label.empty()) break;
                        submenu->AppendRadioItem(wxID_ANY, toWx(d.label), toWx(d.description));
                    }
                    wxMenuItem* item = m->Append(el->id, label, submenu, help);
                    auto rec = std::make_unique<InstallRecord>();
                    rec->kind = InstallRecord::SUBMENU;
                    rec->menu = m;
                    rec->menuItem = item;
                    installs.push_back(std::move(rec));
                }
                for (wxToolBar* tb : toolbars) {
                    for (int v = 0; v < enumCount; ++v) {
                        int toolId = el->id * 1000 + v;
                        UIStateValueDescriptor d = state->getValueDescriptor(v);
                        if (d.label.empty()) break;
                        tb->AddTool(toolId + v, toWx(d.label), wxBitmap(), toWx(d.description), wxITEM_RADIO);
                        auto rec = std::make_unique<InstallRecord>();
                        rec->kind = InstallRecord::TOOLBAR_TOOL;
                        rec->toolbar = tb;
                        rec->toolId = toolId;
                        installs.push_back(std::move(rec));
                    }
                }
            }
        }
    }
}

void UIGroup::tearDown(UIWidgetsContext& context) {
    for (UIElement* el : children) {
        if (!el->visible.get()) continue;
        std::string dir = el->dir();

        wxString label = toWx(el->label.get().empty() ? el->name() : el->label.get());
        wxString help = toWx(el->description.get());

        ImageSet icon = el->icon.get();

        std::vector<wxMenuBar*> menubars = context.getMenubars(dir);
        std::vector<wxMenu*> menus = context.getMenus(dir);
        std::vector<wxToolBar*> toolbars = context.getToolbars(dir);

        if (el->isGroup()) {
            UIGroup* childGroup = dynamic_cast<UIGroup*>(el);
            if (!childGroup) continue;
            childGroup->tearDown(context);

            for (wxMenuBar* mb : menubars) {
                int menuPos = mb->FindMenu(label);
                if (menuPos >= 0) {
                    wxMenu* menu = mb->Remove(menuPos);
                    delete menu;
                }
            }
            for (wxMenu* m : menus) {
                wxMenuItem* subMenuItem = m->FindChildItem(el->id);
                if (subMenuItem) {
                    m->Remove(subMenuItem);
                    delete subMenuItem;
                }
            }
            for (wxToolBar* tb : toolbars) {
                wxToolBarToolBase* tool = tb->FindById(el->id);
                if (tool) {
                    tb->RemoveTool(tool->GetId());
                    delete tool;
                }
            }
        }

        else if (el->isAction()) {
            UIAction* action = dynamic_cast<UIAction*>(el);
            if (!action) continue;

            for (wxMenu* m : menus) {
                wxMenuItem* item = m->FindChildItem(el->id);
                if (item) {
                    m->Remove(item);
                    delete item;
                }
            }
            for (wxToolBar* tb : toolbars) {
                wxToolBarToolBase* tool = tb->FindById(el->id);
                if (tool) {
                    tb->RemoveTool(tool->GetId());
                    delete tool;
                }
            }
        }

        else if (el->isState()) {
            UIState* state = dynamic_cast<UIState*>(el);
            if (!state) continue;
            
            if (state->getType() == UIStateType::BOOL) {
                for (wxMenu* m : menus) {
                    // find menuitem matching el
                    wxMenuItem* item;
                    for (int i = 0; i < m->GetMenuItemCount(); ++i) {
                        size_t pos = -1;
                        wxMenuItem* item = m->FindChildItem(el->id, &pos);
                        if (item) {
                            m->Remove(item);
                            delete item;
                            break;
                        }
                    }
                }
                for (wxToolBar* tb : toolbars) {
                    wxToolBarToolBase* tool = tb->FindById(el->id);
                    if (tool) {
                        tb->RemoveTool(tool->GetId());
                        delete tool;
                    }
                }
            } else if (state->getType() == UIStateType::ENUM) {
                for (wxMenu* m : menus) {
                    size_t pos = -1;
                    wxMenuItem* item = m->FindChildItem(el->id, &pos);
                    if (item) {
                        // wxMenu* submenu = item->GetSubMenu();
                        m->Remove(item);
                        delete item;
                    }   
                }
                const int enumCount = state->getEnumCount();
                for (wxToolBar* tb : toolbars) {
                    for (int v = 0; v < enumCount; ++v) {
                        int toolId = el->id * 1000 + v;
                        tb->RemoveTool(toolId);
                    }
                }
            }
        }
    }
}

void UIGroup::tearDown(InstallRecords& installs) {
    for (auto it = installs.rbegin(); it != installs.rend(); ++it) {
        InstallRecord* rec = it->get();

        switch (rec->kind) {
            case InstallRecord::MENU:
                if (rec->menuBar && rec->menuPos >= 0) {
                    wxMenu* _menu = rec->menuBar->Remove(rec->menuPos);
                    assert(_menu && _menu == rec->menu);
                    delete _menu;
                    rec->menu = nullptr;
                    rec->menuPos = -1;
                }
                break;
            case InstallRecord::SUBMENU:
                if (rec->menu && rec->subMenuId >= 0) {
                    wxMenuItem* subMenuItem = rec->menu->Remove(rec->subMenuId);
                    assert(subMenuItem && subMenuItem == rec->menuItem);
                    delete subMenuItem; // auto delete submenu
                    rec->menuItem = nullptr;
                    rec->subMenuId = -1;
                }
                break;
            case InstallRecord::MENU_ITEM:
                if (rec->menu && rec->menuItem) {
                    rec->menu->Remove(rec->menuItem);
                    delete rec->menuItem;
                    rec->menuItem = nullptr;
                }
                break;
            case InstallRecord::TOOLBAR_TOOL:
                if (rec->toolbar && rec->toolId >= 0) {
                    rec->toolbar->RemoveTool(rec->toolId);
                    rec->toolId = -1;
                }
                break;
            default:
                break;
        }
    }
    installs.clear();
}

void UIGroup::dump(std::ostream& output) const {
    // output << "Tree: \n";
    dumpRecursive(output, "");
    output << "\n";
    // output << "---- above ----\n";
}

void UIGroup::dumpRecursive(std::ostream& output, std::string_view indent) const {
    output << str();
    
    // Print children recursively
    int child_count = children.size();
    if (child_count) {
        output << "/ [" << child_count << "]";
    } else {
        output << "/";
    }

    for (int i = 0; i < child_count; i++) {
        const UIElement* child = children[i];
        
        output << "\n";
        output << indent;
        // Print non-group children (actions, states) with same connector as groups
        if (i != child_count - 1)
            output << " |- ";
        else
            output << " `- ";
        
        if (child->isGroup()) {
            const UIGroup* childGroup = dynamic_cast<const UIGroup*>(child);
            assert(childGroup);

            std::string child_indent;
            if (i != child_count - 1)
                child_indent = std::string(indent) + " |  ";
            else
                child_indent = std::string(indent) + "    ";
    
            childGroup->dumpRecursive(output, child_indent);
        } else {
            output << child->str();
        }
    }
}
