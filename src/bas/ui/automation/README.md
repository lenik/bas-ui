# bas.ui.automation

In-process wxWidgets e2e automation for bas-ui.

Events are synthesized with `wxEvtHandler::ProcessEvent` (not OS-level
`wxUIActionSimulator`), so tests stay deterministic and focus-independent.

Event names are dispatched through a **version-gated `EventRegistry` map**
(not an if/else chain). wx 3.1+ only registers gesture / magnify handlers when
`BAS_WX_MODERN` is set.

## Quick start

```cpp
#include <bas/ui/automation/automation.hpp>

class MyDialog : public wxDialog, public bas::ui::automation::Automatable {
public:
    MyDialog(wxWindow* parent)
        : wxDialog(parent, ...), Automatable(this) {
        automationMap().bind("btn1", ID_BTN1);
        automationMap().bind("user", m_user);
    }
};

dlg.emulate("setValue", "user", {{"text", "admin"}});
dlg.emulate("click", "btn1", {{"x", 20}, {"y", 40}});
```

`uiFrame` already mixes in `Automatable` and calls `bindAutomationArch()` after
`createView()`, so notepad / guitanks can `emulate("perform", "file/save")` etc.

## Components

| Type | Role |
|------|------|
| `IAutomation` | `emulate(event, objId, data)` surface |
| `EventRegistry` | name → handler map (wx-version conditioned) |
| `AutomationMap` | name → wx ID / `wxWindow*` / `UIElement*` |
| `DefaultAutomation` | ProcessEvent composers + arch perform/setstate |
| `Automatable` | mixin for dialogs/frames |
| `AutomationQuery` | widgets + UIState assertions |
| `ScriptRunner` | JSON step arrays (`wait` handled here) |

## Events (registry)

Pointer: `click` (`press`), `dblclick`, `rightclick`, `middleclick`,
`mousedown`/`up`/`move`, `mouseenter`/`leave`, `mousewheel`, `aux1click`/`aux2click`

Text: `type`, `setvalue`, `clear`

Keys: `key`, `keydown`, `keyup`, `char`, `charhook`

Widgets: `select`, `check`/`uncheck`/`toggle`, `focus`/`blur`, `activate`, `scroll`

Arch: `perform` (`action`), `setstate`, `togglestate`

**wx 3.1+ only:** `magnify`, `gesturepan`, `gesturezoom`, `gesturerotate`,
`twofingertap`, `longpress`

Unknown events or missing targets throw `AutomationError`.
Use `tryEmulate` for soft fail.

## UI arch

```cpp
frame.emulate("perform", "file/new");
frame.emulate("setstate", "view/toolbar_show_label", {{"value", true}});
frame.emulate("togglestate", "demo/bold");
```

`AutomationMap::bindTree` registers leaf name, `pathFromRoot()`, relative path,
and `#id` for each `UIElement`.

## Script format

```json
[
  {"do": "setValue", "on": "user", "data": {"text": "admin"}},
  {"do": "perform", "on": "file/save"},
  {"do": "wait", "data": {"ms": 50}}
]
```

Compact: `["click", "ok", {"x": 1}]`.

## Tests

See [`tests/`](../../../tests/): `DemoDialog`, `DemoFrame`, and
`e2e_automation_test`.
