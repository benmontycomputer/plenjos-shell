schemas_dir = get_option('prefix') / get_option('datadir') / 'glib-2.0' / 'schemas'
settings_schemas = [ 'com.plenjos.shell.panel.gschema.xml', 'com.plenjos.shell.desktop.gschema.xml' ]

install_data(settings_schemas, install_dir: schemas_dir)

# Compile the schemas in the current directory; this is only
# useful for testing
gnome = import('gnome')
gnome.compile_schemas(depend_files: files(settings_schemas))

meson.add_install_script('glib-compile-schemas', schemas_dir)