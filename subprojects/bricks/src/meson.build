libbricks_header_subdir = package_subdir / package_api_name
libbricks_header_dir = get_option('includedir') / libbricks_header_subdir

libbricks_public_headers = files([
  'bricks.h',
  'brk-tab-bar.h',
  'brk-tab-view.h',
  'brk-animation-target.h',
  'brk-bin.h',
  'brk-timed-animation.h',
  'brk-animation.h',
])
libbricks_private_headers = files([
  'brk-animation-private.h',
  'brk-animation-util.h',
  'brk-bidi-private.h',
  'brk-easing.h',
  'brk-fading-label-private.h',
  'brk-gizmo-private.h',
  'brk-tab-bar-private.h',
  'brk-tab-box-private.h',
  'brk-tab-button.h',
  'brk-tab-private.h',
  'brk-tab-view-private.h',
  'brk-animation-target-private.h',
  'brk-indicator-bin-private.h',
  'brk-widget-utils-private.h',
])

libbricks_public_sources = files([
  'brk-tab-bar.c',
  'brk-tab-view.c',
  'brk-animation-target.c',
  'brk-bin.c',
  'brk-timed-animation.c',
])
libbricks_private_sources = files([
  'brk-animation.c',
  'brk-animation-util.c',
  'brk-bidi.c',
  'brk-easing.c',
  'brk-fading-label.c',
  'brk-gizmo.c',
  'brk-tab-box.c',
  'brk-tab-button.c',
  'brk-tab.c',
  'brk-indicator-bin.c',
  'brk-widget-utils.c',
])

libbricks_generated_headers = []
libbricks_generated_sources = []

libbricks_resources = gnome.compile_resources(
  'brk-resources',
  './resources/bricks.gresources.xml',
  source_dir: 'resources',
  gresource_bundle: false,
  c_name: 'brk',
)
libbricks_generated_headers += [libbricks_resources[1]]
libbricks_generated_sources += [libbricks_resources[0]]

libbricks_public_enum_headers = [
  'brk-animation.h',
  'brk-easing.h',
  'brk-tab-view.h',
]
libbricks_public_enums = gnome.mkenums_simple(
  'brk-enums',
  sources: libbricks_public_enum_headers,
  body_prefix: '#include "config.h"',
  header_prefix: '''
#if !defined(_BRICKS_INSIDE) && !defined(BRICKS_COMPILATION)
#error "Only <bricks.h> can be included directly."
#endif
''',
  install_dir: libbricks_header_dir,
)
libbricks_generated_headers += [libbricks_public_enums[1]]
libbricks_generated_sources += [libbricks_public_enums[0]]

libbricks_marshalers = gnome.genmarshal(
  'brk-marshalers',
  sources: 'brk-marshalers.list',
  prefix: 'brk_marshal',
  valist_marshallers: true,
)
libbricks_generated_headers += [libbricks_marshalers[1]]
libbricks_generated_sources += [libbricks_marshalers[0]]

version_data = configuration_data()
version_data.set('BRK_MAJOR_VERSION', bricks_version_major)
version_data.set('BRK_MINOR_VERSION', bricks_version_minor)
version_data.set('BRK_MICRO_VERSION', bricks_version_micro)
version_data.set('BRK_VERSION', meson.project_version())
libbricks_version_h = configure_file(
  input: 'brk-version.h.in',
  output: 'brk-version.h',
  install_dir: libbricks_header_dir,
  configuration: version_data,
)
libbricks_generated_headers += [libbricks_version_h]

config_data = configuration_data()
config_data.set_quoted('G_LOG_DOMAIN', 'Bricks')
config_data.set_quoted('GETTEXT_PACKAGE', 'libbricks')
config_data.set_quoted('LOCALEDIR', get_option('prefix') / get_option('localedir'))
libbricks_config_h = configure_file(output: 'config.h', configuration: config_data)
libbricks_generated_headers += [libbricks_config_h]

install_headers(libbricks_public_headers, subdir: libbricks_header_subdir)

libbricks_link_args =[]
libbricks_symbols_file = 'libbricks.syms'

libbricks_deps = [
  fribidi_dep,
  glib_dep,
  gtk_dep,
  math_dep,
]
libbricks_public_deps = [
  gio_dep,
  gtk_dep,
]

libbricks = library(
  'bricks-' + apiversion,
  libbricks_private_sources + libbricks_public_sources + libbricks_generated_headers + libbricks_generated_sources,
  soversion: soversion,
  dependencies: libbricks_deps,
  include_directories: include_directories('.'),
  install: true,
  install_dir: libdir,
)
libbricks_dep = declare_dependency(
  sources: libbricks_public_headers,
  dependencies: libbricks_public_deps,
  link_with: libbricks,
  include_directories: include_directories('.'),
)


