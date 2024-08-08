project('plenjos-session', 'c',
  version : '0.1',
  default_options : ['warning_level=3'])

exe = executable('plenjos_session', 'plenjos_session.c',
  install : true)

gtk = dependency('gtk+-3.0', version: '>=3.24.0')
gtk_layer_shell = dependency('gtk-layer-shell-0', version: '>=0.7.0')

test('basic', exe)

subdir('src')