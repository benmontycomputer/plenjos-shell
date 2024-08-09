project('plenjos-session', 'c',
  version : '0.1',
  default_options : ['warning_level=3'])

exe = executable('plenjos_session', 'plenjos_session.c',
  install : true)

gtk = dependency('gtk+-3.0', version: '>=3.24.0')
gtk_layer_shell = dependency('gtk-layer-shell-0', version: '>=0.7.0')

wayland_client = dependency('wayland-client', version: '>=1.10.0')

wayland_scanner = dependency('wayland-scanner', version: '>=1.10.0', native: true)

wayland_protocols = dependency('wayland-protocols', version: '>=1.16')

test('basic', exe)

subdir('protocol')

subdir('src')
