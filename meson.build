project('AetherTheme', 'c',
  version: '0.1.0',
  meson_version: '>= 0.50.0',
  default_options: [ 'warning_level=2', 'werror=false', 'c_std=gnu11' ],
)

cc = meson.get_compiler('c')

gtk_dep = dependency('gtk+-3.0', version: '>= 3.22.0')
glib_dep = dependency('glib-2.0', version: '>= 2.50.0')
gio_dep = dependency('gio-2.0')
gio_unix_dep = dependency('gio-unix-2.0')
json_glib_dep = dependency('json-glib-1.0')

add_project_arguments('-DVERSION="@0@"'.format(meson.project_version()), language: 'c')

# Installation directories
datadir = get_option('datadir')
bindir = get_option('bindir')

aethertheme_sources = [
  'src/main.c',
  'src/tools.c',
  'src/uicomponents.c',
  'src/stylepak.c'
]

executable('aethertheme', aethertheme_sources,
  dependencies: [gtk_dep, glib_dep, gio_dep, gio_unix_dep, json_glib_dep],
  install: true
)

# Install data
install_data('stuff/main.glade', install_dir: join_paths(datadir, 'AetherTheme'))
install_data('stuff/AetherTheme.desktop', install_dir: join_paths(datadir, 'applications'))
install_data('stuff/AetherTheme.svg', install_dir: join_paths(datadir, 'pixmaps'))

install_subdir('langs', install_dir: join_paths(datadir, 'AetherTheme'))
