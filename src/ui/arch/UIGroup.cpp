#include "UIGroup.hpp"

#include "UIAction.hpp"
#include "UIState.hpp"
#include "UIWidgetsContext.hpp"

#include "wx/menus.hpp"
#include "wx/toolbars.hpp"

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
        if (menuBar && menuPos >= 0 && menu) {
            // Only remove if the menu is still at the expected position
            // Menu might have been already removed by tearDown() or wxWidgets cleanup
            int currentMenuCount = menuBar->GetMenuCount();
            if (menuPos < currentMenuCount) {
                wxMenu* _menu = menuBar->Remove(menuPos);
                // Only delete if we got the expected menu back
                if (_menu == menu) {
                    delete _menu;
                }
            }
            menu = nullptr;
            menuPos = -1;
        }
        break;

    case SUBMENU:
        if (menu && subMenuId >= 0 && menuItem) {
            // Only remove if the menu item still exists
            wxMenuItem* subMenuItem = menu->Remove(subMenuId);
            // Only delete if we got the expected item back
            if (subMenuItem == menuItem) {
                delete subMenuItem; // auto delete submenu
            }
            menuItem = nullptr;
            subMenuId = -1;
        }
        break;

    case MENU_ITEM:
        if (menu && menuItem) {
            // Menu item might have been already removed by tearDown()
            // Check if it still exists in the menu before removing
            bool found = false;
            for (int i = 0; i < menu->GetMenuItemCount(); ++i) {
                if (menu->FindItemByPosition(i) == menuItem) {
                    found = true;
                    break;
                }
            }
            if (found) {
                menu->Remove(menuItem);
                delete menuItem;
            }
            menuItem = nullptr;
        }
        break;

    case TOOLBAR_TOOL:
        if (toolbar && toolId >= 0) {
            // Tool might have been already removed
            // Check if it still exists before removing
            wxToolBarToolBase* tool = toolbar->FindById(toolId);
            if (tool) {
                toolbar->RemoveTool(toolId);
            }
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

int UIGroup::actionCount() const {
    int count = 0;
    for (const UIElement* el : children) {
        if (el->isAction())
            count++;
    }
    return count;
}

int UIGroup::flattenActionCount() const {
    int count = 0;
    for (const UIElement* el : children) {
        if (el->isAction())
            count++;
        else if (el->isGroup())
            count += dynamic_cast<const UIGroup*>(el)->flattenActionCount();
    }
    return count;
}

UIGroup* UIGroup::resolveGroup(const Path& path) { return resolveGroup(path.str()); }

int UIGroup::s_next_internal_id = 10000;

UIGroup* UIGroup::resolveGroup(std::string_view path) {
    while (!path.empty() && path.front() == '/')
        path.remove_prefix(1);
    if (path.empty())
        return this;

    int slash = path.find('/');
    std::string_view head = slash == std::string::npos ? path : path.substr(0, slash);

    UIElement* child = getChild(head);
    if (!child) {
        std::string dir = this->path ? this->path->str() : std::string();
        auto& owned = internals.emplace_back(
            std::make_unique<UIGroup>(s_next_internal_id++, dir, std::string(head), 0, "<internal>",
                                      "", "", ImageSet(), true));
        owned->internal = true;
        child = owned.get();
        child->attach(this);
    }

    if (!child->isGroup())
        return nullptr;

    UIGroup* gchild = dynamic_cast<UIGroup*>(child);
    if (slash == std::string::npos)
        return gchild;

    std::string_view tail = path.substr(slash + 1);
    return gchild->resolveGroup(tail);
}

void UIGroup::buildTree(std::vector<UIElement*>& elements) {
    for (UIElement* el : elements) {
        std::optional<Path> path = el->path;
        if (!path)
            continue;
        std::string dir = path->dir();
        UIGroup* parent = resolveGroup(dir);
        if (parent == nullptr) {
            throw std::runtime_error("dir collision: " + dir);
        }
        el->attach(parent);
        logdebug_fmt("resolved path %s dir %s attached to %s\n", path->str().c_str(), dir.c_str(),
                     parent->str().c_str());
    }
}

static wxString toWx(const std::string& s) { return wxString::FromUTF8(s.c_str()); }

void UIGroup::setUp(UIWidgetsContext* context, InstallRecords* installs) {
    int menuIconSize = context->preferredMenuIconSize();
    int toolIconSize = context->preferredToolIconSize();

    int klast = -1;

    for (UIElement* child : children) {
        if (!child->visible.get())
            continue;
        std::string dir = child->dir();

        wxString label = toWx(child->label.get().empty() ? child->name() : child->label.get());
        wxString help = toWx(child->description.get());

        ImageSet icon = child->icon.get();

        std::vector<wxMenuBar*> parentMenubars = context->getMenubars(dir);
        std::vector<wxMenu*> parentMenus = context->getMenus(dir);
        std::vector<wxToolBar*> parentToolbars = context->getToolbars(dir);

        int priority = child->priority.get();
        int kgroup = priority / 1000;
        if (kgroup != klast) {
            if (klast != -1) {
                // add separator between k-groups
                // if (child->menuWanted())
                //     for (wxMenuBar* parentMenubar : parentMenubars) {
                //         wx::addNecessarySeparator(parentMenubar);
                //     }
                if (child->menuWanted())
                    for (wxMenu* parentMenu : parentMenus) {
                        wx::addNecessarySeparator(parentMenu);
                    }
                if (child->toolWanted())
                    for (wxToolBar* parentToolbar : parentToolbars) {
                        wx::addNecessarySeparator(parentToolbar);
                    }
            }
            klast = kgroup;
        }

        if (child->isGroup()) {
            UIGroup* gchild = dynamic_cast<UIGroup*>(child);
            if (!gchild)
                continue;

            if (gchild->menuWanted())
                for (wxMenuBar* parentMenubar : parentMenubars) {
                    int menuPos = parentMenubar->GetMenuCount();
                    wxMenu* menu = new wxMenu();
                    parentMenubar->Append(menu, label);

                    // menu icon not useful
                    // if (icon.isSet())
                    //     menu->SetBitmap(icon.loadBitmap(24, 24));

                    std::string menuPath = child->path ? child->path->str() : dir;
                    context->registerMenu(menuPath, menu);

                    auto rec = std::make_unique<InstallRecord>();
                    rec->kind = InstallRecord::MENU;
                    rec->menuBar = parentMenubar;
                    rec->menuPos = menuPos;
                    rec->group = gchild;
                    installs->push_back(std::move(rec));
                }

            if (gchild->menuWanted())
                for (wxMenu* parentMenu : parentMenus) {
                    wxMenu* submenu = new wxMenu();
                    wxMenuItem* item = parentMenu->AppendSubMenu(submenu, label, help);

                    if (icon.isSet()) {
                        wxBitmap bmp = icon.loadBitmap(menuIconSize, menuIconSize);
                        if (bmp.IsOk())
                            item->SetBitmap(bmp);
                    }

                    std::string menuPath = child->path ? child->path->str() : dir;
                    context->registerMenu(menuPath, submenu);

                    auto rec = std::make_unique<InstallRecord>();
                    rec->kind = InstallRecord::SUBMENU;
                    rec->menu = parentMenu;
                    rec->subMenuId = item->GetId();
                    rec->menuItem = item;
                    rec->group = gchild;
                    installs->push_back(std::move(rec));
                }

            if (gchild->toolWanted() &&
                gchild->flattenActionCount() > 0) // add necessary separators
                for (wxToolBar* parentToolbar : parentToolbars) {
                    wxToolBarToolBase* sep = wx::addNecessarySeparator(parentToolbar);
                    if (!sep)
                        continue;

                    auto rec = std::make_unique<InstallRecord>();
                    rec->kind = InstallRecord::TOOLBAR_TOOL;
                    rec->toolbar = parentToolbar;
                    rec->toolId = sep->GetId();
                    installs->push_back(std::move(rec));
                }

            // recursively set up the children
            gchild->setUp(context, installs);
        }

        else if (child->isAction()) {
            UIAction* achild = dynamic_cast<UIAction*>(child);
            if (!achild)
                continue;

            if (child->menuWanted())
                for (wxMenu* m : parentMenus) {
                    wxMenuItem* item = m->Append(child->id, label, help);
                    if (icon.isSet()) {
                        wxBitmap bmp = icon.loadBitmap(menuIconSize, menuIconSize);
                        if (bmp.IsOk())
                            item->SetBitmap(bmp);
                    }

                    auto rec = std::make_unique<InstallRecord>();
                    rec->kind = InstallRecord::MENU_ITEM;
                    rec->menu = m;
                    rec->menuItem = item;
                    installs->push_back(std::move(rec));
                }

            if (child->toolWanted())
                for (wxToolBar* tb : parentToolbars) {
                    wxBitmap bmp = icon.loadBitmap(toolIconSize, toolIconSize);
                    wxString toolLabel = label;
                    toolLabel.Replace("&", "");
                    tb->AddTool(child->id, toolLabel, bmp, help, wxITEM_NORMAL);

                    auto rec = std::make_unique<InstallRecord>();
                    rec->kind = InstallRecord::TOOLBAR_TOOL;
                    rec->toolbar = tb;
                    rec->toolId = child->id;
                    installs->push_back(std::move(rec));
                }
        }

        else if (child->isState()) {
            UIState* stchild = dynamic_cast<UIState*>(child);
            if (!stchild)
                continue;

            if (stchild->getType() == UIStateType::BOOL) {
                bool checked = false;
                if (auto* p = std::get_if<bool>(&stchild->value.get()))
                    checked = *p;

                if (child->menuWanted())
                    for (wxMenu* m : parentMenus) {
                        wxMenuItem* item = m->AppendCheckItem(child->id, label, help);
                        if (icon.isSet()) {
                            wxBitmap bmp = icon.loadBitmap(menuIconSize, menuIconSize);
                            if (bmp.IsOk())
                                item->SetBitmap(bmp);
                        }
                        m->Check(child->id, checked);
                        auto rec = std::make_unique<InstallRecord>();
                        rec->kind = InstallRecord::MENU_ITEM;
                        rec->menu = m;
                        rec->menuItem = item;
                        installs->push_back(std::move(rec));
                    }

                if (child->toolWanted())
                    for (wxToolBar* tb : parentToolbars) {
                        wxBitmap bmp;
                        if (icon.isSet()) {
                            bmp = icon.loadBitmap(toolIconSize, toolIconSize);
                        } else {
                            bmp =
                                wxArtProvider::GetBitmap(wxART_MISSING_IMAGE, wxART_TOOLBAR,
                                                         wxSize(context->preferredToolIconSize(),
                                                                context->preferredToolIconSize()));
                        }

                        tb->AddTool(child->id, label, bmp, help, wxITEM_CHECK);
                        auto rec = std::make_unique<InstallRecord>();
                        rec->kind = InstallRecord::TOOLBAR_TOOL;
                        rec->toolbar = tb;
                        rec->toolId = child->id;
                        installs->push_back(std::move(rec));
                    }
            } else if (stchild->getType() == UIStateType::ENUM) {
                const int enumCount = stchild->getEnumCount();

                if (child->menuWanted())
                    for (wxMenu* m : parentMenus) {
                        wxMenu* submenu = new wxMenu();
                        for (int v = 0; v < enumCount; ++v) {
                            UIStateValueDescriptor d = stchild->getValueDescriptor(v);
                            if (d.label.empty())
                                break;
                            submenu->AppendRadioItem(wxID_ANY, toWx(d.label), toWx(d.description));
                        }
                        wxMenuItem* item = m->Append(child->id, label, submenu, help);
                        auto rec = std::make_unique<InstallRecord>();
                        rec->kind = InstallRecord::SUBMENU;
                        rec->menu = m;
                        rec->menuItem = item;
                        installs->push_back(std::move(rec));
                    }

                if (child->toolWanted())
                    for (wxToolBar* tb : parentToolbars) {
                        for (int v = 0; v < enumCount; ++v) {
                            int toolId = child->id * 1000 + v;
                            UIStateValueDescriptor d = stchild->getValueDescriptor(v);
                            if (d.label.empty())
                                break;
                            tb->AddTool(toolId + v, toWx(d.label), wxBitmap(), toWx(d.description),
                                        wxITEM_RADIO);
                            auto rec = std::make_unique<InstallRecord>();
                            rec->kind = InstallRecord::TOOLBAR_TOOL;
                            rec->toolbar = tb;
                            rec->toolId = toolId;
                            installs->push_back(std::move(rec));
                        }
                    }
            }
        }
    }
}

void UIGroup::tearDown(UIWidgetsContext* context) {
    for (UIElement* child : children) {
        if (!child->visible.get())
            continue;
        std::string dir = child->dir();

        wxString label = toWx(child->label.get().empty() ? child->name() : child->label.get());
        wxString help = toWx(child->description.get());

        ImageSet icon = child->icon.get();

        std::vector<wxMenuBar*> menubars = context->getMenubars(dir);
        std::vector<wxMenu*> menus = context->getMenus(dir);
        std::vector<wxToolBar*> toolbars = context->getToolbars(dir);

        if (child->isGroup()) {
            UIGroup* gchild = dynamic_cast<UIGroup*>(child);
            if (!gchild)
                continue;
            gchild->tearDown(context);

            if (gchild->menuWanted())
                for (wxMenuBar* mb : menubars) {
                    int menuPos = mb->FindMenu(label);
                    if (menuPos >= 0) {
                        wxMenu* menu = mb->Remove(menuPos);
                        delete menu;
                    }
                }

            if (gchild->menuWanted())
                for (wxMenu* m : menus) {
                    wxMenuItem* subMenuItem = m->FindChildItem(child->id);
                    if (subMenuItem) {
                        m->Remove(subMenuItem);
                        delete subMenuItem;
                    }
                }

            if (gchild->toolWanted())
                for (wxToolBar* tb : toolbars) {
                    wxToolBarToolBase* tool = tb->FindById(child->id);
                    if (tool) {
                        tb->RemoveTool(tool->GetId());
                        delete tool;
                    }
                }
        }

        else if (child->isAction()) {
            UIAction* action = dynamic_cast<UIAction*>(child);
            if (!action)
                continue;

            if (child->menuWanted())
                for (wxMenu* m : menus) {
                    wxMenuItem* item = m->FindChildItem(child->id);
                    if (item) {
                        m->Remove(item);
                        delete item;
                    }
                }

            if (child->toolWanted())
                for (wxToolBar* tb : toolbars) {
                    wxToolBarToolBase* tool = tb->FindById(child->id);
                    if (tool) {
                        tb->RemoveTool(tool->GetId());
                        delete tool;
                    }
                }
        }

        else if (child->isState()) {
            UIState* state = dynamic_cast<UIState*>(child);
            if (!state)
                continue;

            if (state->getType() == UIStateType::BOOL) {
                if (child->menuWanted())
                    for (wxMenu* m : menus) {
                        // find menuitem matching el
                        wxMenuItem* item;
                        for (int i = 0; i < m->GetMenuItemCount(); ++i) {
                            size_t pos = -1;
                            wxMenuItem* item = m->FindChildItem(child->id, &pos);
                            if (item) {
                                m->Remove(item);
                                delete item;
                                break;
                            }
                        }
                    }

                if (child->toolWanted())
                    for (wxToolBar* tb : toolbars) {
                        wxToolBarToolBase* tool = tb->FindById(child->id);
                        if (tool) {
                            tb->RemoveTool(tool->GetId());
                            delete tool;
                        }
                    }
            } else if (state->getType() == UIStateType::ENUM) {
                if (child->menuWanted())
                    for (wxMenu* m : menus) {
                        size_t pos = -1;
                        wxMenuItem* item = m->FindChildItem(child->id, &pos);
                        if (item) {
                            // wxMenu* submenu = item->GetSubMenu();
                            m->Remove(item);
                            delete item;
                        }
                    }

                if (child->toolWanted()) {
                    const int enumCount = state->getEnumCount();
                    for (wxToolBar* tb : toolbars) {
                        for (int v = 0; v < enumCount; ++v) {
                            int toolId = child->id * 1000 + v;
                            tb->RemoveTool(toolId);
                        }
                    }
                }
            }
        }
    }
}

void UIGroup::tearDown(InstallRecords* installs) {
    for (auto it = installs->rbegin(); it != installs->rend(); ++it) {
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
    installs->clear();
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
