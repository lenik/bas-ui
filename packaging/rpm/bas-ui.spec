# Version is injected by packaging/rpm/Makefile via `zfr version`.
# RPM Version cannot contain '-'; use `zfr version -r` (hyphens → '_').
# srcversion is the unsanitized Meson/git version and names the tarball.
%{!?version:%global version 0.0.0}
%{!?srcversion:%global srcversion %{version}}

Name:           bas-ui
Version:        %{version}
Release:        1%{?dist}
Summary:        Scriptable UI library for BAS (runtime)

License:        AGPL-3.0-or-later
URL:            https://example.com/bas-ui
Packager:       BAS Developers <debian@example.com>
Source0:        %{name}-%{srcversion}.tar.xz

BuildRequires:  meson
BuildRequires:  ninja-build
BuildRequires:  pkg-config
BuildRequires:  libglib2.0-dev
BuildRequires:  libicu-dev
BuildRequires:  libcurl4-openssl-dev
BuildRequires:  libssl-dev
BuildRequires:  libwxgtk3.2-dev
BuildRequires:  libboost-dev
BuildRequires:  zlib1g-dev
BuildRequires:  asciidoctor
BuildRequires:  libbas-cpp-dev

%description
A C++ library that provides a scriptable UI stack built on
wxWidgets and GLib, with assets embedded via Meson.

%prep
%setup -q -n %{name}-%{srcversion}

%build
meson setup build \
    --prefix=%{_prefix} \
    --bindir=%{_bindir} \
    --datadir=%{_datadir} \
    --mandir=%{_mandir} \
    --sysconfdir=%{_sysconfdir} \
    --localstatedir=%{_localstatedir} \
    --buildtype=plain
meson compile -C build

%install
meson install -C build --destdir=%{buildroot}

%files
%{_datadir}/bash-completion/completions/notepad
%{_mandir}/man1/notepad.1*
%{_datadir}/bas-ui/
%{_includedir}/*
%{_datadir}/locale/*/LC_MESSAGES/bas-ui.mo
%{_mandir}/*/man1/notepad.1*
%{_datadir}/doc/bas-ui/

%changelog
* Thu Aug 20 2026 BAS Developers <debian@example.com>
- Align spec with debian/control (Meson, AGPL-3.0-or-later).
- Version comes from `zfr version`, the same method meson.build uses.
