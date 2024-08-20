daemon_generated_sources = []

daemon_generated_sources += gnome.gdbus_codegen(
    'plenjos-notifyd-fdo-gbus',
    sources: 'plenjos-notifyd-fdo-dbus.xml',
    interface_prefix: 'org.freedesktop.Notifications',
    namespace: 'Plenjos',
    install_header: false,
)

notifyd_sources = [
    'notifyd.c',
]

#executable(
#    'plenjos-notifyd', notifyd_sources + daemon_generated_sources,
#    build_by_default: true,
#    dependencies: [gtk, gtk_layer_shell, dbus],
#    install: true)