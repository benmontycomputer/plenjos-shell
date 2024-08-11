notifyd_sources = [
    'notifyd.c',
]

executable(
    'plenjos-notifyd', notifyd_sources,
    build_by_default: true,
    dependencies: [gtk, gtk_layer_shell, dbus],
    install: true)