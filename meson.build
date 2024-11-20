project(
  'bedit2',
  'c', 'vala',
  version: '0.1.0',
  meson_version: '>=1.3',
)
add_project_arguments(
  '-Wno-overlength-strings',
  '-Wno-deprecated-declarations',
  '-Wno-pedantic',
  language: 'c'
)

add_project_arguments(
  '--gresourcesdir', 'resources',
  '--gresources', meson.current_source_dir() / 'bedit/resources/bedit.gresources.xml',
  '--vapidir=' + meson.project_build_root() / 'subprojects/libbricks-1/src/',
  '--pkg', 'libbricks-1',
  language: 'vala'
)

gnome = import('gnome')

cc = meson.get_compiler('c')

prefix = get_option('prefix')
bindir = get_option('bindir')
datadir = get_option('datadir')

bricks_proj = subproject('libbricks-1')

bricks_dep = bricks_proj.get_variable('libbricks_dep')
gtk_dep = dependency('gtk4', include_type: 'system')
gtksourceview_dep = dependency('gtksourceview-5', version: '>= 5.10.0', include_type: 'system')

bedit_deps = [
  bricks_dep,
  gtk_dep,
  gtksourceview_dep,
]

config_data = configuration_data()
config_data.set_quoted('BEDIT_NAME', 'Bedit')
config_data.set_quoted('BEDIT_APP_ID', 'bedit')
config_data.set_quoted('BEDIT_VERSION', meson.project_version())
config_data.set_quoted('G_LOG_USE_STRUCTURED', meson.project_version())
config_data.set_quoted('G_LOG_DOMAIN', 'bedit')
config_data.set_quoted('GETTEXT_PACKAGE', 'bedit2')
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
  '-Werror=incompatible-pointer-types',
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
  '-Wno-missing-field-initializers',
  '-Wno-sign-compare',
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

bedit_headers = [
  'bedit/bedit-application.h',
  'bedit/bedit-window.h',
]

bedit_enums = gnome.mkenums_simple(
  'bedit-enums',
  sources: bedit_headers,
  body_prefix: '#include <config.h>\n',
)

bedit_vala_sources = [
  'bedit-searchbar'
]

bedit_sources = [
  'bedit/main.c',
  'bedit/bedit-application-actions.c',
  'bedit/bedit-application.c',
  'bedit/bedit-application-shortcuts.c',
  'bedit/bedit-document.c',
  'bedit/bedit-window-actions.c',
  'bedit/bedit-window.c',
  bedit_enums,
  bedit_resources.get(0),
  bedit_resources.get(1),
]

bedit_vala_libs = []
foreach vala_source : bedit_vala_sources
    bedit_vala_libs += static_library(
      vala_source,
      ['bedit/' + vala_source + '.vala'],
      install: false,
      vala_header: vala_source + '.h',
      dependencies: bedit_deps,
    )
endforeach

bedit_exe = executable(
  'bedit2',
  bedit_sources,
  install: true,
  dependencies: bedit_deps,
  link_with: bedit_vala_libs,
)

configure_file(
  input: 'com.bwhmather.Bedit.desktop.in',
  output: 'com.bwhmather.Bedit.desktop',
  install_dir: join_paths(datadir, 'applications'),
  configuration: {'bindir': join_paths(prefix, bindir)},
)

install_data(
  'com.bwhmather.Bedit2.gschema.xml',
  install_dir: join_paths(datadir, 'glib-2.0/schemas'),
)
gnome.compile_schemas()  # Only for testing.
gnome.post_install(glib_compile_schemas: true, update_desktop_database: true)

test_suites = {
  'lint': [
    'exact-includes',
    'include-guards',
    'no-circular-includes',
    'source-and-header-contents-match',
    'source-and-header-orders-match',
    'source-heading',
    'unused-symbols',
  ],
}

pymod = import('python')
python = pymod.find_installation('python3')

foreach suite, tests : test_suites
  foreach test_name : tests
    test(
      suite + '-' + test_name,
      python,
      args: [
        meson.current_source_dir() +
        '/tests/' + suite +
        '/test_' + test_name.replace('-', '_') + '.py',
      ],
      env: {
        'MESON_BUILD_ROOT': meson.project_build_root(),
        'MESON_SOURCE_ROOT': meson.project_source_root(),
      },
      timeout: 1000,
    )
  endforeach
endforeach
