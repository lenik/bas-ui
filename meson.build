project(
    'bas-ui',
    ['c', 'cpp'],
    version: '1.0.1',
    default_options: [
        #'warning_level=0',
        'cpp_std=c++17',
        'buildtype=debug',
    ],
)

pkgdatadir = get_option('datadir') / meson.project_name()

fs = import('fs')
cpp = meson.get_compiler('cpp')

# libbas
bas_c_dep = dependency('bas-c', required: true)
bas_cpp_dep = dependency('bas-cpp', required: true)

nanosvg_dep = dependency('nanosvg', required: false)

# wxWidgets dependencies (must use a single GTK toolkit to avoid "GTK+ 2.x and GTK+ 3 in the same process" error)
wx_dep = dependency('wxwidgets', method: 'config-tool', required: true)
# Detect toolkit from default wx-config so we link only gtk2 or only gtk3
wx_config = find_program('wx-config', required: true)
wx_libs_out = run_command(wx_config, '--libs', check: true, capture: true).stdout()
wx_use_gtk3 = wx_libs_out.contains('gtk3')
if wx_use_gtk3
    wx_gtk_prefix = 'wx_gtk3u'
    wx_ver = '3.0'
else
    wx_gtk_prefix = 'wx_gtk2u'
    wx_ver = '3.1'
endif
# wxWidgets media (wxMediaCtrl) and extras - use same toolkit as core
# Media/webview optional on GTK2 (often not built for gtk2)
wx_media_lib = cpp.find_library(wx_gtk_prefix + '_media-' + wx_ver, required: false)
wx_richtext_lib = cpp.find_library(wx_gtk_prefix + '_richtext-' + wx_ver, required: true)
wx_stc_lib = cpp.find_library(wx_gtk_prefix + '_stc-' + wx_ver, required: true)
wx_webview_lib = cpp.find_library(wx_gtk_prefix + '_webview-' + wx_ver, required: false)

# wx_dep = dependency('wxwidgets', modules : [
#     'core',
#     'media',
#     'richtext',
#     'stc',
#     'webview',
#     ], required : true)

if wx_dep.version() < '3.1'
    # wx 3.0: media and webview are in separate libs; core setup has wxUSE_*=0, so enable for our build
    wx_defines = ['-DwxUSE_MEDIACTRL=1', '-DwxUSE_WEBVIEW=1']
    add_project_arguments(wx_defines, language: 'cpp')
endif

# Disable all warnings (wx 3.0 and deps are noisy)
cpp_nowarn = cpp.get_supported_arguments(['-w', '/w'])
cpp_nowarn = cpp_nowarn + wx_defines

# Precompiled header for wxWidgets
wx_pch = 'src/wx_pch.h'

cpp_sources = files(
    './src/ui/arch/BuildViewContext.cpp',
    './src/ui/arch/BuildViewLog.cpp',
    './src/ui/arch/CreateViewContext.cpp',
    './src/ui/arch/EditHistory.cpp',
    './src/ui/arch/ImageSet.cpp',
    './src/ui/arch/UIAction.cpp',
    './src/ui/arch/UIElement.cpp',
    './src/ui/arch/UIFragment.cpp',
    './src/ui/arch/UIGroup-build.cpp',
    './src/ui/arch/UIGroup-remove.cpp',
    './src/ui/arch/UIGroup.cpp',
    './src/ui/arch/UIState.cpp',
    './src/util/mem.cpp',
    './src/wx/app.cpp',
    './src/wx/artprovs.cpp',
    './src/wx/images.cpp',
    './src/wx/menus.cpp',
    './src/wx/toolbars.cpp',
    './src/wx/uiframe.cpp',
)

# Assets will be zipped and linked as a section in the executable
all_sources = cpp_sources

# Build assets.zip every time (no file dependency)
assets_dir = meson.current_source_dir() / 'assets'
build_dir = meson.current_build_dir()
assets_zip = custom_target(
    'assets_zip',
    output: 'assets.zip',
    input: [],
    command: [
        'sh',
        '-c', 'dst=$(readlink -f "@1@"); cd "@0@" && zip -r "$dst" .'.format(assets_dir, '@OUTPUT@'),
    ],
    #build_always : true,
    build_by_default: true,
    install: false,
)

# Compile assembly file that embeds assets.zip using .incbin
# Use configure_file to copy assets.s to build directory
assets_s = configure_file(
    input: 'app/assets.s',
    output: 'assets.s',
    copy: true,
)

# Compile the assembly file (assets.zip will be in the same directory)
as = find_program('as', required: true)
assets_o = custom_target(
    'assets_o',
    output: 'assets.o',
    input: [assets_s, assets_zip],
    command: [as, '-o', '@OUTPUT@', '@INPUT0@'],
    depends: assets_zip,
)

# Executable (optional wx media/webview only when found)
exe_deps = [
    bas_c_dep,
    bas_cpp_dep,
    nanosvg_dep,
    wx_dep,
]

# if wx_dep.version() < '3.1'
#     exe_deps += [
#         wx_media_lib,
#         wx_richtext_lib,
#         wx_stc_lib,
#         wx_webview_lib,
#     ]
# endif

exe_inc = [
    include_directories('src'),
]

lib_bas_ui = shared_library(
    'bas-ui',
    cpp_sources,
    'src/module.cpp',
    dependencies: exe_deps,
    include_directories: exe_inc,
    cpp_args: cpp_nowarn + ['-DLOGGER_NAME=ui', '-gdwarf-4'],
    cpp_pch: wx_pch,
    link_args: ['-rdynamic', '-Wl,-E', '-Wl,--export-dynamic'],
    override_options: ['b_lundef=false'], # Allows undefined symbols at link time
    version: meson.project_version(),
    soversion: meson.project_version().split('.')[0],
    install: true,
)

bas_ui_dep = declare_dependency(link_with: lib_bas_ui, include_directories: exe_inc)

# ---- pkg-config ----
pkgconfig = import('pkgconfig')
pkgconfig.generate(
    lib_bas_ui,
    name: 'bas-ui',
    description: 'bas-ui',
    version: meson.project_version(),
    subdirs: 'bas',
    requires: ['bas-c', 'bas-cpp'],
)

# ---- Headers for libbas-c package (includedir/bas-c) ----
find_other_cmd = run_command(
    'find',
    'src',
    '-name', '*.h',
    '-o',
    '-name', '*.hpp',
    '-o',
    '-type', 'd',
    '-o',
    '-print',
    check: true,
)
other_files = find_other_cmd.stdout().strip().split('\n')

other_list = []
foreach f : other_files
    other_list += f.substring(4)
endforeach

install_subdir(
    'src',
    install_dir: get_option('includedir') / 'bas',
    exclude_files: other_list,
    strip_directory: true,
)

# Unit tests generator
# Find all *_test.cpp files and build them automatically using the 'includes' utility
# Similar to Makefile pattern rule: %_test: %_test.cpp
#   sources: %_test.cpp $(includes -e -c %_test.cpp)
#   cmdline: $(CXX) $(CXXFLAGS) -o $@ $*_test.cpp `includes -c $*_test.cpp` -licuuc -licui18n

# List of all test source files (manually enumerated since Meson lacks fs module)
# This list is automatically generated from: find src -name '*_test.cpp' | sort
all_test_files = []

# Build each test using the 'includes' utility to get source dependencies
foreach test_rel_path : all_test_files
    test_cpp = 'src/' + test_rel_path

    # Extract test name (e.g., ByteBuffer_test from src/io/ByteBuffer_test.cpp)
    path_parts = test_rel_path.split('/')
    test_file = path_parts[path_parts.length() - 1]
    test_name = test_file.replace('.cpp', '')

    # Use 'includes' utility to get implementation dependencies
    # 'includes -e -c test.cpp' returns source files, one per line (not -I include paths)
    includes_result = run_command('includes', '-e', '-c', test_cpp, check: false)

    impl_deps = []
    if includes_result.returncode() == 0
        # Parse the output: each line is a .c/.cpp file path
        output = includes_result.stdout()
        lines = output.split('\n')
        foreach line : lines
            line = line.strip()
            # Keep non-empty lines that are .c/.cpp files
            if line.endswith('.cpp')
                impl_deps += line
            endif
        endforeach
    endif

    # Combine test file with implementation dependencies from 'includes'
    all_sources = [test_cpp] + impl_deps

    # Create the executable (LOGGER_NAME so program.h and logger.c work in tests)
    test_exe = executable(
        test_name,
        all_sources,
        assets_o,
        'src/module.cpp',
        include_directories: exe_inc,
        dependencies: exe_deps + [bas_ui_dep],
        cpp_args: cpp_nowarn + ['-DLOGGER_NAME=test'],
        cpp_pch: wx_pch,
        link_args: ['-rdynamic', '-no-pie'],
        install: false,
    )

    # Register the test with meson
    test(test_name, test_exe)
endforeach

# --- Applications in app/ (one .cpp = one program), install to bindir + manpages to mandir ---
find_app = run_command('find', 'app', '-maxdepth', '1', '-name', '*.cpp', check: false)
app_cpp_list = []
if find_app.returncode() == 0
    foreach line : find_app.stdout().strip().split('\n')
        if line != ''
            app_cpp_list += line
        endif
    endforeach
endif

foreach app_cpp : app_cpp_list
    app_name = app_cpp.split('/')[-1].replace('.cpp', '')
    if app_name == 'notepad'
        install_dir = get_option('bindir')
    else
        install_dir = pkgdatadir / 'app'
    endif
    executable(
        app_name,
        app_cpp,
        assets_o,
        'src/module.cpp',
        include_directories: exe_inc,
        dependencies: exe_deps + [bas_ui_dep],
        cpp_args: cpp_nowarn + ['-DLOGGER_NAME=' + app_name],
        cpp_pch: wx_pch,
        link_args: ['-rdynamic', '-no-pie'],
        install: true,
        install_dir: install_dir,
    )
endforeach

# Manpages: app/*.1 -> install to mandir
find_man = run_command('find', 'app', '-maxdepth', '1', '-name', '*.1', check: false)
if find_man.returncode() == 0
    foreach line : find_man.stdout().strip().split('\n')
        if line == ''
            continue
        endif
        install_man(line)
    endforeach
endif

# ---- Bash completion (install to datadir/bash-completion/completions) ----
find_bash = run_command('find', 'app', '-maxdepth', '1', '-name', '*.bash', check: false)
if find_bash.returncode() == 0
    foreach line : find_bash.stdout().strip().split('\n')
        if line == ''
            continue
        endif
        completion_name = fs.replace_suffix(line.split('/')[-1], '')
        message(completion_name)
        install_data(
            line,
            rename: completion_name,
            install_dir: get_option('datadir') / 'bash-completion' / 'completions',
            install_mode: 'rw-r--r--',
        )
    endforeach
endif
