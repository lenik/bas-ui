exe_inc = include_directories('../src')
cpp_pch = '../src/wx_pch.h'

app_cpp_list = [
    'notepad.cpp',
    'guitanks.cpp',
    'aui.cpp',
    'assets.cpp',
]
foreach app_cpp : app_cpp_list
    app_name = app_cpp.split('/')[-1].replace('.cpp', '')


    # Use 'includes' utility to get implementation dependencies
    # 'includes -e -c test.cpp' returns source files, one per line (not -I include paths)
    includes_result = run_command('includes', '-e', '-c', app_cpp, check: false)

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

    if app_name == 'guitanks'
        impl_deps += ['tanks_rbac.cpp', 'tanks_sim.cpp']
    endif

    if app_name == 'notepad'
        install_dir = get_option('bindir')
    else
        install_dir = pkgdatadir / 'app'
    endif
    
    executable(
        app_name,
        app_cpp,
        impl_deps,
        include_directories: exe_inc,
        dependencies: exe_deps + [bas_ui_dep],
        cpp_args: cpp_nowarn
        + [
            # '-gdwarf-4',
            '-DLOGGER_NAME='
            + app_name,
        ],
        cpp_pch: cpp_pch,
        link_args: ['-rdynamic', '-no-pie'],
        install: false,
        install_dir: install_dir,
    )
endforeach

man_list = [
    'notepad.1',
]
foreach man : man_list
    install_man(
        man,
    )
endforeach

bash_list = [
    'notepad.bash',
]
foreach bash : bash_list
    completion_name = fs.replace_suffix(bash.split('/')[-1], '')
    message(completion_name)
    install_data(
        bash,
        rename: completion_name,
        install_dir: get_option('datadir') / 'bash-completion' / 'completions',
        install_mode: 'rw-r--r--',
    )
endforeach