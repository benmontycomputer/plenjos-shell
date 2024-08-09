executable(
    'plenjos-panel', panel_protocol_srcs,
    files('panel.c', 'panel-interface.c', 'panel-interface-toplevel-button.c'),
    build_by_default: true,
    dependencies: [gtk, gtk_layer_shell, wayland_client, wayland_protocols],
    install: true)