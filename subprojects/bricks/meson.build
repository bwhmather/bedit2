project(
  'libbricks', 'c',
  version: '1.0.alpha',
  license: 'LGPL-2.1-or-later',
  meson_version: '>= 1.3.0',
)

gnome = import('gnome')
pkgconfig = import('pkgconfig')

cc = meson.get_compiler('c')

version_arr = meson.project_version().split('-')[0].split('.')
bricks_version_major = version_arr[0].to_int()
bricks_version_minor = version_arr[1].to_int()
if version_arr[2].startswith('alpha') or version_arr[2].startswith('beta') or version_arr[2].startswith('rc')
  bricks_version_micro = 0
else
  bricks_version_micro = version_arr[2].to_int()
endif

apiversion = bricks_version_major.to_string()
package_api_name = '@0@-@1@'.format(meson.project_name(), apiversion)

# Before making a release, the libversion string should be modified.
#
#  * Bump the first component if binary compatibility has been broken; or
#  * Bump the second component if new APIs are added; or
#  * Bump the third component otherwise.
#
# When bumping the first component version, set the second and third components
# to 0. When bumping the second version, set the third one to zero.
#
# A lot easier than libtool, right?
libversion = '0.1.0'
# The so major version of the library
soversion = 0

add_project_arguments([
  '-DBRICKS_COMPILATION',
], language: 'c')

# Setup various paths that subdirectory meson.build files need
package_subdir = get_option('package_subdir') # When used as subproject
if package_subdir != ''
  datadir = get_option('datadir') / package_subdir
  libdir = get_option('libdir') / package_subdir
  girdir = get_option('datadir') / package_subdir / 'gir-1.0'
  typelibdir = get_option('libdir') / package_subdir / 'girepository-1.0'
  vapidir = get_option('datadir') / package_subdir / 'vapi'
else
  datadir = get_option('datadir')
  libdir = get_option('libdir')
  girdir = get_option('datadir') / 'gir-1.0'
  typelibdir = get_option('libdir') / 'girepository-1.0'
  vapidir = get_option('datadir') / 'vala' / 'vapi'
endif

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
  '-Wno-overlength-strings',
  '-Wno-pedantic',
  '-Wno-sign-compare',
  '-Wno-unused-parameter',
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


glib_dep = dependency('glib-2.0')
gio_dep = dependency('gio-2.0')
fribidi_dep = dependency('fribidi')
gtk_dep = dependency('gtk4', include_type: 'system')
math_dep = cc.find_library('m')

subdir('src')
