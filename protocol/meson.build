if wayland_scanner.found()
    prog_wayland_scanner = find_program(wayland_scanner.get_pkgconfig_variable('wayland_scanner'))
else
    prog_wayland_scanner = find_program('wayland-scanner')
endif

protocols = [
    
]

if wayland_protocols.found()
    protocols += join_paths(
        wayland_protocols.get_pkgconfig_variable('pkgdatadir'),
        'stable/xdg-shell/xdg-shell.xml')
else
    # use bundled xdg-shell.xml
    protocols += 'xdg-shell.xml'
endif

gen_panel_header = generator(prog_wayland_scanner,
    output: ['@BASENAME@-panel.h'],
    arguments: ['-c', 'panel-header', '@INPUT@', '@BUILD_DIR@/@BASENAME@-panel.h'])

gen_private_code = generator(prog_wayland_scanner,
    output: ['@BASENAME@.c'],
    arguments: ['-c', 'code', '@INPUT@', '@BUILD_DIR@/@BASENAME@.c'])
# 'code' is deprecated, and can be replaced with 'private-code' when all platforms have a new enough wayland-scanner

panel_protocol_srcs = []

foreach protocol : protocols
    panel_header = gen_panel_header.process(protocol)
    code = gen_private_code.process(protocol)
    panel_protocol_srcs += [panel_header, code]
endforeach
