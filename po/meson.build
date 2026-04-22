i18n = import('i18n')

i18n.gettext(
    'bas-ui',
    args: [
        '--from-code=UTF-8',
        '--keyword=_:1',
        '--keyword=N_:1',
        '--keyword=Name:1',
    ],
)

add_project_arguments(
    '-DBAS_UI_LOCALEDIR="@0@"'.format(meson.current_build_dir()),
    language: 'cpp',
)
