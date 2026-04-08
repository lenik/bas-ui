i18n = import('i18n')

i18n.gettext(
  'bas-ui',
  languages: [
    'en',
    'zh_CN',
    'zh_TW',
    'ko',
    'ja',
  ],
  args: [
    '--from-code=UTF-8',
    '--keyword=basUiTr:1',
    '--keyword=basUiMsg:1',
  ],
)

add_project_arguments(
  '-DBAS_UI_LOCALEDIR="@0@"'.format(meson.current_build_dir()),
  language: 'cpp',
)
