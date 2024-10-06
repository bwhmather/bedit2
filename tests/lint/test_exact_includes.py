"""
Checks that all source and header files directly import the headers that they
need.
"""
import itertools
import pathlib
import sys

from bedit_lint import (
    INCLUDE_ROOT,
    PROJECT_ROOT,
    SOURCE_ROOT,
    derive_include_from_path,
    enumerate_header_paths,
    enumerate_source_paths,
    header_paths_for_source_path,
    read_ast_from_path,
    read_includes_from_path,
    resolve_clang_path,
    is_source_path,
    walk_file_preorder,
)

INCLUDE_ALIASES = {
    "json_types.h": "json.h",
    "json_tokener.h": "json.h",
    "json_object.h": "json.h",
    "glibconfig.h": "glib.h",
    "bits/stdint-uintn.h": "stdint.h",
    "bits/stdint-intn.h": "stdint.h",
    "bits/types/struct_timespec.h": "time.h",
    "bits/mathcalls.h": "math.h",
    "asm-generic/errno-base.h": "errno.h",
    "asm-generic/ioctls.h": "sys/ioctl.h",
    "bits/fcntl-linux.h": "fcntl.h",
    "bits/getopt_core.h": "getopt.h",
    "bits/getopt_ext.h": "getopt.h",
    "bits/resource.h": "sys/resource.h",
    "bits/sigaction.h": "signal.h",
    "bits/signum-generic.h": "signal.h",
    "bits/socket.h": "sys/socket.h",
    "bits/socket_type.h": "sys/socket.h",
    "bits/struct_stat.h": "sys/socket.h",
    "bits/time.h": "time.h",
    "bits/types/FILE.h": "stdio.h",
    "bits/types/clockid_t.h": "sys/types.h",
    "bits/types/sigset_t.h": "sys/types.h",
}

BUILTIN_SYMBOLS = {
    "ssize_t": "sys/types.h",
    "pid_t": "sys/types.h",
}

IGNORED_SYMBOLS = {"NULL", "__u32", "width", "height", "x", "y"}


def _canonicalize(ref_include):
    if ref_include.startswith("pango/"):
        return "pango/pango.h"
    if ref_include.startswith("gdk/") and ref_include not in ("gdk/gdkwayland.h", "gdk/gdkx.h"):
        return "gdk/gdk.h"
    if ref_include.startswith("gio/"):
        return "gio/gio.h"
    if ref_include.startswith("gsk/"):
        return "gsk/gsk.h"
    if ref_include.startswith("gtk/"):
        return "gtk/gtk.h"
    if ref_include.startswith("glib/") and ref_include not in ("glib/gi18n-lib.h", "glib/gprintf.h"):
        return "glib.h"
    if ref_include.startswith("gobject/"):
        return "glib-object.h"
    if ref_include.startswith("graphene-"):
        return "graphene.h"
    if ref_include.startswith("fribidi-"):
        return "fribidi.h"
    if ref_include.startswith("gtksourceview/"):
        return "gtksourceview/gtksource.h"
    if ref_include.startswith("brk-"):
        return "bricks.h"
    return  INCLUDE_ALIASES.get(ref_include, ref_include)

def test():
    passed = True

    for source_path in itertools.chain(
        enumerate_header_paths(),
        enumerate_source_paths(),
    ):
        includes = set(read_includes_from_path(source_path))

        unused = set(includes)
        indirect = dict()

        source = read_ast_from_path(source_path)

        for node in walk_file_preorder(source):
            if not node.referenced:
                continue
            if node.referenced == node:
                continue
            ref = node.referenced
            ref = ref.canonical
            if ref.get_definition() is not None:
                ref = ref.get_definition()
            if ref.spelling in BUILTIN_SYMBOLS:
                ref_include = BUILTIN_SYMBOLS[ref.spelling]
            else:
                if ref.location.file is None:
                    continue
                ref_path = resolve_clang_path(ref.location.file.name)

                if ref.spelling in IGNORED_SYMBOLS:
                    continue

                if ref_path == resolve_clang_path(source.spelling):
                    continue

                ref_include = derive_include_from_path(ref_path)

            ref_include = _canonicalize(ref_include)
            if ref_include in includes:
                unused.discard(ref_include)
                continue

            indirect.setdefault(ref_include, set()).add(ref.spelling)

        missing_primary = None
        missing_config = None
        if is_source_path(source_path):
            for header_path in header_paths_for_source_path(source_path):
                ref_include = derive_include_from_path(header_path)
                unused.discard(ref_include)
                if ref_include not in includes:
                    missing_primary = ref_include

            unused.discard("config.h")
            if "config.h" not in includes:
                missing_config = "config.h"
        else:
            indirect.pop("config.h", None)

        if indirect or unused or missing_primary:
            msg = "======================================================================\n"
            msg += f"FAIL: test_exact_includes: {source_path.relative_to(PROJECT_ROOT)}\n"
            msg += "----------------------------------------------------------------------\n"

            msg += "Includes do not match requirements.\n\n"

            if missing_config:
                msg += "The following mandatory includes where missing:\n"
                msg += f"  -  {missing_config}\n"
                msg += "\n"

            if missing_primary:
                msg += "The following primary include was missing:\n"
                msg += f"  - {missing_primary}\n"
                msg += "\n"

            if indirect:
                msg += "The following files were depended on indirectly:\n"
                for indirect_path, indirect_refs in sorted(indirect.items()):
                    msg += f"  - {indirect_path} ({', '.join(indirect_refs)})\n"
                msg += "\n"

            if unused:
                msg += "The following includes were unused:\n"
                for unused_path in sorted(unused):
                    msg += f"  - {unused_path}\n"
                msg += "\n"

            print(msg, file=sys.stderr)
            passed = False

    return passed


if __name__ == "__main__":
    sys.exit(0 if test() else 1)
