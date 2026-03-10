# bas-ui

`bas-ui` is a C++ library and set of tools that provide a scriptable UI stack on top of wxWidgets and GLib. Assets are embedded into the binaries using a Meson-based build that zips the contents of the `assets/` directory and links them as a separate section.

## Building

This project uses Meson and Ninja.

```bash
meson setup builddir
meson compile -C builddir
```

The library `libbas-ui` and the UI applications from `app/*.cpp` will be built. Binaries are installed by running:

```bash
meson install -C builddir
```

## Dependencies

Build-time dependencies (see `meson.build` and `debian/control`):

- C++17-capable compiler
- Meson and Ninja
- GLib 2.0
- ICU (libicu-uc)
- libcurl
- OpenSSL
- wxWidgets (GTK)
- Boost (core/system/json/locale)
- zlib

## Debian packaging

A minimal Debian packaging is provided in the `debian/` directory and is intended to be used with `debhelper` and the Meson build system.

