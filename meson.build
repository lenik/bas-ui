project(
    'bas-ui',
    ['c', 'cpp'],
    version: run_command(
        'sh',
        '-c', '''
        v=$(
            command -v zfr >/dev/null 2>&1 && zfr version 2>/dev/null || true
        )
        if [ -z "$v" ] && [ -f VERSION ]; then
            v=$(head -n1 VERSION | tr -d '\r')
            v=${v#v}
        fi
        if [ -z "$v" ]; then
            v="0.0.0" # FIXED TO 0.0.0, DO NOT MODIFY
        fi
        printf '%s' "$v"
        ''',
        check: false,
    ).stdout().strip(),
    license: 'AGPL-3.0-or-later',
    default_options: [
        #'warning_level=0',
        'cpp_std=c++17',
        'buildtype=debug',
    ],
)
project_author = 'BAS Developers'
project_email = 'debian@example.com'
project_year = 2026

fs = import('fs')
cpp = meson.get_compiler('cpp')

prefix = get_option('prefix')
bindir = prefix / get_option('bindir')
datadir = prefix / get_option('datadir')
mandir = prefix / get_option('mandir')
localedir = prefix / get_option('localedir')
pkgdatadir = datadir / meson.project_name()
pkgdocdir = datadir / 'doc' / meson.project_name()

config_h = configuration_data()
config_h.set_quoted('PROJECT_VERSION', meson.project_version())
config_h.set_quoted('PROJECT_AUTHOR', project_author)
config_h.set_quoted('PROJECT_EMAIL', project_email)
config_h.set('PROJECT_YEAR', project_year)
config_h.set_quoted('PREFIX', prefix)
config_h.set_quoted('DATADIR', datadir)
config_h.set_quoted('MANDIR', mandir)
config_h.set_quoted('PKGDATADIR', pkgdatadir)
config_h.set_quoted('LOCALEDIR', localedir)

configure_file(
    output: 'config.h',
    configuration: config_h,
)

if fs.is_dir('subprojects/bas-c')
    subproject('bas-c')
endif
if fs.is_dir('subprojects/bas-cpp')
    subproject('bas-cpp')
endif

bas_c_dep = dependency('bas-c', required: true)
bas_cpp_dep = dependency('bas-cpp', required: true)
bas_args = ['-DLOGGER_NAME=' + meson.project_name(), '-DTEXT_DOMAIN=' + meson.project_name()]
cpp_args_bas = bas_args


bas_c_static_dep = dependency('bas-c-static', required: true)
bas_cpp_static_dep = dependency('bas-cpp-static', required: true)

if fs.is_dir('subprojects/boost')
    boost_sub = subproject('boost')
    boost_json_dep = boost_sub.get_variable('boost_json_dep')
else
    boost_json_dep = dependency('boost', modules: ['json'], required: true)
endif

if fs.is_dir('subprojects/nanosvg')
    nanosvg_dep = subproject('nanosvg').get_variable('nanosvg_dep')
else
    nanosvg_dep = dependency('nanosvg', required: true)
endif

date_dep = subproject('date_tz').get_variable('date_dep')

wx_sub = subproject('wx_logic')
wx_dep = wx_sub.get_variable('wx_dep')
wx_lib_ver = wx_sub.get_variable('wx_lib_ver')
wx_aui_dep = cpp.find_library('wx_gtk3u_aui-' + wx_lib_ver, required: true)

intl_dep = dependency('intl', required: false)
if not intl_dep.found()
    intl_dep = cpp.find_library('intl', required: true)
endif

# Disable all warnings (wx 3.0 and deps are noisy)
cpp_nowarn = cpp.get_supported_arguments(['-w', '/w'])

cpp_args = cpp_nowarn + [
    '-gdwarf-4',
    '-DLOGGER_NAME=ui',
    '-DASSETS_NAME=bas_ui',
]

cpp_pch = 'src/wx_pch.h'

cpp_sources = files(
    './src/bas/security/wxLoginUi.cpp',
    './src/bas/ui/arch/BuildViewContext.cpp',
    './src/bas/ui/arch/BuildViewLog.cpp',
    './src/bas/ui/arch/CreateViewContext.cpp',
    './src/bas/ui/arch/EditHistory.cpp',
    './src/bas/ui/arch/ImageSet.cpp',
    './src/bas/ui/arch/IdManager.cpp',
    './src/bas/ui/arch/UIAction.cpp',
    './src/bas/ui/arch/UIElement.cpp',
    './src/bas/ui/arch/UIFragment.cpp',
    './src/bas/ui/arch/UIGroup-build.cpp',
    './src/bas/ui/arch/UIGroup-remove.cpp',
    './src/bas/ui/arch/UIGroup.cpp',
    './src/bas/ui/arch/UIState.cpp',
    './src/bas/ui/arch/b-action.cpp',
    './src/bas/ui/arch/b-group.cpp',
    './src/bas/ui/arch/b-state.cpp',
    './src/bas/ui/automation/AutomationMap.cpp',
    './src/bas/ui/automation/AutomationQuery.cpp',
    './src/bas/ui/automation/DefaultAutomation.cpp',
    './src/bas/ui/automation/EventRegistry.cpp',
    './src/bas/ui/automation/ScriptRunner.cpp',
    './src/bas/util/mem.cpp',
    './src/bas/wx/app.cpp',
    './src/bas/wx/artprovs.cpp',
    './src/bas/wx/images.cpp',
    './src/bas/wx/menus.cpp',
    './src/bas/wx/toolbar.cpp',
    './src/bas/wx/uiframe.cpp',
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
    input: 'src/assets.s',
    output: 'assets.s',
    copy: true,
)

# Compile the assembly file (assets.zip will be in the same directory)
as = find_program('as', required: true)
assets_o = custom_target(
    'assets_o',
    output: 'assets.o',
    input: [assets_s, assets_zip],
    command: [
        'sh',
        '-c', 'cd "$1" && "$2" -o assets.o assets.s',
        'sh',
        build_dir,
        as,
    ],
    depends: assets_zip,
)

# Executable (optional wx media/webview only when found)
exe_deps = [
    bas_c_dep,
    bas_cpp_dep,
    boost_json_dep,
    date_dep,
    intl_dep,
    nanosvg_dep,
    wx_dep,
    wx_aui_dep,
]

exe_static_deps = [
    bas_c_static_dep,
    bas_cpp_static_dep,
    boost_json_dep,
    date_dep,
    intl_dep,
    nanosvg_dep,
    wx_dep,
    wx_aui_dep,
]

exe_inc = [
    include_directories('.'),
    include_directories('src'),
]

override_options = ['b_lundef=false'] # Allows undefined symbols at link time

subdir('po')

lib_bas_ui = shared_library(
    'bas-ui',
    cpp_sources,
    'src/module.cpp',
    assets_o,
    dependencies: exe_deps,
    include_directories: exe_inc,
    cpp_args: cpp_args,
    cpp_pch: cpp_pch,
    link_args: ['-rdynamic', '-Wl,-E', '-Wl,--export-dynamic'],
    override_options: override_options,
    version: meson.project_version().split('-')[0],
    soversion: meson.project_version().split('.')[0],
    install: true,
)

lib_bas_ui_static = static_library(
    'bas-ui',
    cpp_sources,
    'src/module.cpp',
    assets_o,
    dependencies: exe_static_deps,
    include_directories: exe_inc,
    cpp_args: cpp_args,
    cpp_pch: cpp_pch,
    override_options: override_options,
    install: true,
)

bas_ui_dep = declare_dependency(
    link_with: lib_bas_ui,
    include_directories: exe_inc,
    dependencies: exe_deps,
)
bas_ui_static_dep = declare_dependency(
    link_with: lib_bas_ui_static,
    include_directories: exe_inc,
    dependencies: exe_static_deps,
)

meson.override_dependency('bas-ui', bas_ui_dep)
meson.override_dependency('bas-ui-static', bas_ui_static_dep)

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

# Static .pc: explicit .a path; chain bas-c-static and bas-cpp-static.
pkgconfig.generate(
    name: 'bas-ui-static',
    filebase: 'bas-ui-static',
    description: 'bas-ui',
    version: meson.project_version(),
    subdirs: 'bas',
    install_dir: get_option('libdir') / 'pkgconfig',
    requires: ['bas-c-static', 'bas-cpp-static'],
    libraries: ['${bas_ui_static_lib}'],
    variables: ['bas_ui_static_lib=${libdir}/libbas-ui.a'],
    uninstalled_variables: ['bas_ui_static_lib=${prefix}/libbas-ui.a'],
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

other_list = [
    'bulk.h',
    'wx_pch.h',
]
foreach f : other_files
    other_list += f.substring(4)
endforeach

install_subdir(
    'src',
    install_dir: get_option('includedir'),
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
all_test_files = [
    'bas/ui/automation/automation_test.cpp',
]

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
        include_directories: exe_inc,
        dependencies: exe_deps + [bas_ui_dep],
        cpp_args: cpp_nowarn
        + [
            # '-gdwarf-4',
            '-DLOGGER_NAME=test',
            '-DASSETS_NAME=bas_ui',
        ],
        cpp_pch: cpp_pch,
        link_args: ['-rdynamic', '-no-pie'],
        install: false,
    )

    # Register the test with meson
    test(test_name, test_exe)
endforeach

subdir('app')

subdir('tests')

asciidoctor = find_program('asciidoctor', required: true)

run_target(
    'look',
    command: [
        'bash',
        '-euc', '''
            tmpdir=$(mktemp -d)
            DESTDIR=$tmpdir meson install -C "@BUILD_ROOT@"
            tree "$tmpdir"
            rm -fr "$tmpdir"
        ''',
    ],
)

install_data(
    [
        'LICENSE',
        'README.md',
        'README-zh_CN.md',
    ],
    install_dir: pkgdocdir,
)



bash_files = [
    'completions/notepad.bash',
]

foreach file : bash_files
name = fs.stem(file)
install_data(
    file,
    install_dir: datadir / 'bash-completion' / 'completions',
    rename: name,
)
endforeach

puffs = [
    'notepad',
]
man_i18n_langs = []
foreach lang : ['en', 'zh_CN', 'ja', 'ko']
  if fs.is_dir('man' / lang)
    man_i18n_langs += [lang]
  endif
endforeach

foreach puff : puffs
  custom_target(
    puff + '-man',
    input: 'man' / (puff + '.adoc'),
    output: puff + '.1',
    command: [
      asciidoctor,
      '-b', 'manpage',
      '-a', 'project-version=' + meson.project_version(),
      '-a', 'project-year=@0@'.format(project_year),
      '-a', 'project-author=' + project_author,
      '-a', 'project-email=' + project_email,
      '-o', '@OUTPUT@',
      '@INPUT@',
    ],
    build_by_default: true,
    install: true,
    install_dir: mandir / 'man1',
  )
  foreach lang : man_i18n_langs
    _man_i18n = custom_target(
      puff + '-' + lang + '-man',
      input: 'man' / lang / (puff + '.adoc'),
      output: puff + '-' + lang + '.1',
      command: [
        asciidoctor,
        '-b', 'manpage',
        '-a', 'project-version=' + meson.project_version(),
        '-a', 'project-year=@0@'.format(project_year),
        '-a', 'project-author=' + project_author,
        '-a', 'project-email=' + project_email,
        '-o', '@OUTPUT@',
        '@INPUT@',
      ],
      build_by_default: true,
    )
    meson.add_install_script(
      'sh',
      '-c',
      'mkdir -p "${DESTDIR:-}$1" && install -m644 "$2" "${DESTDIR:-}$1/$3.1"',
      '_',
      mandir / lang / 'man1',
      _man_i18n,
      puff,
    )
  endforeach
endforeach
