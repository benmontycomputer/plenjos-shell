project('plenjos-session', 'c',
  version : '0.1',
  default_options : ['warning_level=3'])

exe = executable('plenjos_session', 'plenjos_session.c',
  install : true)

gtk = dependency('gtk4', version: '>=4.8.0')
gtk_layer_shell = dependency('gtk4-layer-shell-0', version: '>=1.0.2')

wayland_client = dependency('wayland-client', version: '>=1.10.0')

wayland_scanner = dependency('wayland-scanner', version: '>=1.10.0', native: true)

wayland_protocols = dependency('wayland-protocols', version: '>=1.16')

cc = meson.get_compiler('c')

m = cc.find_library('m', required : true)

playerctl = dependency('playerctl', version: '>=2.4.1')

alsa = dependency('alsa', version: '>=1.2.8')

test('basic', exe)

subdir('protocol')

subdir('src')
