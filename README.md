# 🎨 bas-ui

<div align="center">

### Scriptable UI Framework on wxWidgets + GLib

*Beautiful interfaces, elegantly crafted* 🌸

[![License](https://img.shields.io/badge/License-MIT-pink?style=for-the-badge)](LICENSE)
[![C++](https://img.shields.io/badge/C++-17-blue?style=for-the-badge&logo=c%2B%2B)](https://isocpp.org/)
[![wxWidgets](https://img.shields.io/badge/wxWidgets-3.0+-00599C?style=for-the-badge&logo=linux)](https://wxwidgets.org/)
[![Meson](https://img.shields.io/badge/Meson-Build-FFD700?style=for-the-badge)](https://mesonbuild.com/)
[![Debian](https://img.shields.io/badge/Debian-Package-C70036?style=for-the-badge&logo=debian)](https://www.debian.org/)

**[Documentation](docs/)** • **[Examples](examples/)** • **[Issues](https://github.com/lenik/bas-ui/issues)**

</div>

---

## ✨ What is bas-ui?

`bas-ui` is a **scriptable UI framework** built on wxWidgets and GLib. It provides a declarative approach to building desktop applications with embedded assets, making it easy to create polished, professional interfaces.

> **Design philosophy**: UIs should be as elegant as a ballet performance 🩰—clean, graceful, and delightful.

---

## 🚀 Key Features

| Feature | Description |
|---------|-------------|
| 📜 **Scriptable UI Stack** | Define interfaces declaratively, load at runtime |
| 📦 **Embedded Assets** | Icons, images, and resources bundled into binaries |
| 🎨 **wxWidgets Powered** | Native look and feel on Linux, Windows, macOS |
| 🔧 **Meson Build System** | Fast, modern build with asset embedding |
| 📦 **Debian Packaging** | Ready-to-install .deb packages |

---

## 🏗️ Architecture

```
┌─────────────────────────────────────────┐
│           bas-ui Application            │
├─────────────────────────────────────────┤
│  ┌─────────────┐  ┌─────────────────┐  │
│  │   Scripts   │  │  Embedded       │  │
│  │   (JSON/    │  │  Assets         │  │
│  │    Lua)     │  │  (.zip section) │  │
│  └─────────────┘  └─────────────────┘  │
├─────────────────────────────────────────┤
│         bas-ui Core Library             │
│  ┌─────────────┐  ┌─────────────────┐  │
│  │   Widget    │  │   Layout &      │  │
│  │   Factory   │  │   Styling       │  │
│  └─────────────┘  └─────────────────┘  │
├─────────────────────────────────────────┤
│         wxWidgets + GLib                │
└─────────────────────────────────────────┘
```

---

## 📦 Building

### Prerequisites

```bash
# Debian/Ubuntu
sudo apt install libwxgtk3.0-gtk3-dev libglib2.0-dev libicu-dev \
                 libcurl4-openssl-dev libssl-dev libboost-all-dev \
                 zlib1g-dev meson ninja-build
```

### Build & Install

```bash
# Configure
meson setup builddir

# Build
meson compile -C builddir

# Install
sudo meson install -C builddir
```

This builds:
- `libbas-ui` — Core library
- UI applications from `app/*.cpp`

---

## 📖 Usage Example

### Basic Window

```cpp
#include <bas-ui/application.h>
#include <bas-ui/window.h>

int main(int argc, char *argv[]) {
    bas::ui::Application app(argc, argv);
    
    auto window = bas::ui::Window::create("Hello, bas-ui!");
    window->setSize(400, 300);
    window->show();
    
    return app.run();
}
```

### Scripted UI (JSON)

```json
{
  "window": {
    "title": "My App",
    "size": [400, 300],
    "children": [
      {
        "type": "button",
        "label": "Click Me!",
        "onClick": "handleClick"
      }
    ]
  }
}
```

---

## 📁 Project Structure

```
bas-ui/
├── src/
│   ├── core/          # Core UI framework
│   ├── widgets/       # Widget implementations
│   ├── script/        # Scripting engine
│   └── assets/        # Asset embedding utilities
├── app/               # Example applications
├── assets/            # Icons, images, resources
├── examples/          # Code examples
├── debian/            # Debian packaging
├── docs/              # Documentation
├── meson.build        # Build configuration
└── README.md          # You are here 🌸
```

---

## 🔗 Related Projects

- **[bas-c](https://github.com/lenik/bas-c)** — C base library and utilities
- **[bas-cpp](https://github.com/lenik/bas-cpp)** — C++ foundation library
- **[OmniShell](https://github.com/lenik/omnishell)** — Desktop environment using bas-ui

---

## 🎀 About the Author

<div align="center">

**shecti** ([@lenik](https://github.com/lenik))

> Living with gender dysphoria. The "cute version of me" is my source of courage and power—she is the engine behind all my creation.

**Proudly building as a trans woman in tech.** 💖

If you're a fellow trans coder, LGBTQ+ ally, or just someone who believes in diverse voices in open source—I'd love to connect!

### Tags
`#TransCoder` `#TransIsTech` `#WomenWhoCode` `#OpenSource` `#Cpp` `#wxWidgets` `#UI`

</div>

---

## 💖 Support This Project

If bas-ui helps you build beautiful interfaces, consider:

- ⭐ **Starring** this repository
- 🔀 **Forking** and contributing
- 🐛 **Reporting** bugs and suggesting features
- 📝 **Sharing** with your network
- 💕 **Sponsoring** via [GitHub Sponsors](https://github.com/sponsors/lenik) *(coming soon)*

---

## 📄 License

MIT License. See [LICENSE](LICENSE) for details.

---

<div align="center">

> *"Elegant code creates elegant interfaces."*

**Made with 💖 and lots of ✨**

</div>
