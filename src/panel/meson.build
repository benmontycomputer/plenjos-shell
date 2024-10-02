panel_sources = [
    'panel.c',
]

subdir('taskbar')
subdir('applications-menu')
subdir('blur')
subdir('tray')
subdir('notifyd')
subdir('hyprland')

gnome = import('gnome')

panel_sources += gnome.compile_resources('panel-resources',
  'panel.gresource.xml',
  c_name: 'panel'
)

panel_sources += panel_protocol_srcs

executable(
    'plenjos-panel', panel_sources,
    build_by_default: true,
    dependencies: [gtk, gtk_layer_shell, wayland_client, wayland_protocols, m, playerctl, alsa, dbus, jansson],
    install: true)