project(
  'bedit',
  'c', 'vala',
  version: '0.1.0',
  meson_version: '>=1.3',
)

gnome = import('gnome')

cc = meson.get_compiler('c')

prefix = get_option('prefix')
bindir = get_option('bindir')
datadir = get_option('datadir')

m_dep = cc.find_library('m', required: false)
gtk_dep = dependency(
  'gtk4',
  default_options: [
    'introspection=enabled',
    'media-gstreamer=disabled',
    'build-demos=false',
    'build-examples=false',
    'build-testsuite=false',
    'build-tests=false',
    'werror=false',
  ],
)

bricks_proj = subproject('libbricks-1')
bricks_dep = bricks_proj.get_variable('libbricks_dep')

gtksourceview_dep = dependency(
  'gtksourceview-5', version: '>= 5.10.0',
  include_type: 'system',
  default_options: [
    'introspection=enabled',
    'vapi=true',
    'build-tests=false',
    'build-testsuite=false',
    'werror=false',
  ],
)

bedit_deps = [
  m_dep,
  bricks_dep,
  gtk_dep,
  gtksourceview_dep,
]

add_project_arguments(
  '-include', 'config.h',
  '-Wno-overlength-strings',
  '-Wno-deprecated-declarations',
  '-Wno-pedantic',
  language: 'c'
)

add_project_arguments(
  '--define=GETTEXT_PACKAGE=bedit',
  '--gresourcesdir', 'resources',
  '--gresources', meson.current_source_dir() / 'bedit/resources/bedit.gresources.xml',
  '--vapidir=' + meson.source_root() + '/vapi/',
  '--pkg', 'config',
  '--pkg', 'gtk4',
  '--pkg', 'gtksourceview-5',
  '--enable-deprecated',
  language: 'vala'
)

config_data = configuration_data()
config_data.set_quoted('BEDIT_NAME', 'Bedit')
config_data.set_quoted('BEDIT_APP_ID', 'bedit')
config_data.set_quoted('BEDIT_VERSION', meson.project_version())
config_data.set_quoted('G_LOG_USE_STRUCTURED', meson.project_version())
config_data.set_quoted('G_LOG_DOMAIN', 'bedit')
config_data.set_quoted('GETTEXT_PACKAGE', 'bedit')
config_data.set_quoted('LOCALEDIR', get_option('prefix') / get_option('localedir'))
configure_file(output: 'config.h', configuration: config_data)

test_c_args = [
  '-fno-strict-aliasing',
  '-Wcast-align',
  '-Wdeclaration-after-statement',
  '-Werror=address',
  '-Werror=array-bounds',
  '-Werror=empty-body',
  ['-Werror=format-security', '-Werror=format=2'],
  '-Werror=implicit',
  '-Werror=implicit-function-declaration',
  '-Werror=init-self',
  '-Werror=int-conversion',
  '-Werror=int-to-pointer-cast',
  '-Werror=main',
  '-Werror=misleading-indentation',
  '-Werror=missing-braces',
  '-Werror=missing-include-dirs',
  '-Werror=nonnull',
  '-Werror=overflow',
  '-Werror=parenthesis',
  '-Werror=pointer-arith',
  '-Werror=pointer-to-int-cast',
  '-Werror=return-type',
  '-Werror=sequence-point',
  '-Werror=shadow',
  '-Werror=strict-prototypes',
  '-Werror=trigraphs',
  '-Werror=undef',
  '-Werror=write-strings',
  '-Wformat-nonliteral',
  '-Wignored-qualifiers',
  '-Wimplicit-function-declaration',
  '-Wlogical-op',
  '-Wmissing-declarations',
  '-Wmissing-format-attribute',
  '-Wmissing-include-dirs',
  '-Wmissing-noreturn',
  '-Wnested-externs',
  '-Wno-cast-function-type',
  '-Wno-discarded-qualifiers',
  '-Wno-incompatible-pointer-types',
  '-Wno-missing-field-initializers',
  '-Wno-sign-compare',
  '-Wno-unused-but-set-variable',
  '-Wno-unused-function',
  '-Wno-unused-parameter',
  '-Wno-unused-variable',
  '-Wold-style-definition',
  '-Wpointer-arith',
  '-Wstrict-prototypes',
  '-Wswitch-default',
  '-Wswitch-enum',
  '-Wundef',
  '-Wuninitialized',
  '-Wunused',
]

if get_option('buildtype') != 'plain'
  test_c_args += '-fstack-protector-strong'
endif

global_c_args = []
foreach arg: test_c_args
  if cc.has_multi_arguments(arg)
    global_c_args += arg
  endif
endforeach
add_project_arguments(global_c_args, language: 'c')

bedit_resources = gnome.compile_resources(
  'bedit-resources',
  'bedit/resources/bedit.gresources.xml',
  source_dir: 'bedit/resources',
)

bedit_sources = [
  'bedit/bedit-application.vala',
  'bedit/bedit-close-confirmation-dialog.vala',
  'bedit/bedit-document.vala',
  'bedit/bedit-fonts.vala',
  'bedit/bedit-preferences-window.vala',
  'bedit/bedit-window.vala',
  bedit_resources.get(0),
  bedit_resources.get(1),
]

bedit_exe = executable(
  'bedit',
  bedit_sources,
  install: true,
  dependencies: bedit_deps,
)

configure_file(
  input: 'com.bwhmather.Bedit.desktop.in',
  output: 'com.bwhmather.Bedit.desktop',
  install_dir: join_paths(datadir, 'applications'),
  configuration: {'bindir': join_paths(prefix, bindir)},
)

install_data(
  'com.bwhmather.Bedit.gschema.xml',
  install_dir: join_paths(datadir, 'glib-2.0/schemas'),
)
gnome.compile_schemas()  # Only for testing.
gnome.post_install(glib_compile_schemas: true, update_desktop_database: true)
