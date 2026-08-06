# Automation / e2e demo tests under tests/

test_inc = include_directories('../src', '.')

e2e_automation_test = executable(
    'e2e_automation_test',
    [
        'e2e_automation_test.cpp',
        'demo_dialog.cpp',
        'demo_frame.cpp',
    ],
    include_directories: test_inc,
    dependencies: exe_deps + [bas_ui_dep],
    cpp_args: cpp_nowarn
    + [
        '-DLOGGER_NAME=test',
        '-DASSETS_NAME=bas_ui',
    ],
    cpp_pch: meson.project_source_root() / 'src' / 'wx_pch.h',
    link_args: ['-rdynamic', '-no-pie'],
    install: false,
)

test('e2e_automation_test', e2e_automation_test)
