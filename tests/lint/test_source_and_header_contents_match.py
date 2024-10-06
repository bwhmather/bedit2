import itertools
import sys

import clang.cindex
from bedit_lint import (
    PROJECT_ROOT,
    enumerate_source_paths,
    header_paths_for_source_path,
    read_ast_from_path,
    resolve_clang_path,
)


def test():
    passed = True

    for source_path in itertools.chain(enumerate_source_paths()):
        header_paths = header_paths_for_source_path(source_path)
        if not header_paths:
            continue
        header_decls = set()
        for header_path in header_paths:
            header = read_ast_from_path(header_path)

            header_decls.update(
                node.spelling
                for node in header.cursor.get_children()
                if node.kind == clang.cindex.CursorKind.FUNCTION_DECL
                and not node.is_definition()
                and resolve_clang_path(node.location.file.name)
                == resolve_clang_path(header.spelling)
            )

        source = read_ast_from_path(source_path)
        source_defs = {
            node.spelling
            for node in source.cursor.get_children()
            if node.kind == clang.cindex.CursorKind.FUNCTION_DECL
            and node.is_definition()
            and node.storage_class != clang.cindex.StorageClass.STATIC
            and resolve_clang_path(node.location.file.name)
            == resolve_clang_path(source.spelling)
        }

        if header_decls != source_defs:
            msg = "======================================================================\n"
            msg += f"FAIL: test_source_and_header_contents_match: {source_path.relative_to(PROJECT_ROOT)}\n"
            msg += "----------------------------------------------------------------------\n"

            msg += "Source file does not match header.\n\n"

            if header_decls - source_defs:
                msg += f"The following symbols were not defined in {source_path.relative_to(PROJECT_ROOT)} but were declared in {' or '.join(str(header_path.relative_to(PROJECT_ROOT)) for header_path in header_paths)}:\n"
                for symbol in header_decls - source_defs:
                    msg += f"  - {symbol}\n"
                msg += "\n"

            if source_defs - header_decls:
                msg += f"The following symbols were not declared in {' or '.join(str(header_path.relative_to(PROJECT_ROOT)) for header_path in header_paths)} but were defined in {source_path.relative_to(PROJECT_ROOT)}:\n"
                for symbol in source_defs - header_decls:
                    msg += f"  - {symbol}\n"
                msg += "\n"

            print(msg, file=sys.stderr)
            passed = False

    return passed


if __name__ == "__main__":
    sys.exit(0 if test() else 1)
