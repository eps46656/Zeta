from __future__ import annotations

import dataclasses
import functools
import pathlib
import typing

import beartype
import clang.cindex

from ... import building_utils, llvm_utils, utils

FILE = utils.to_canon_path(__file__, solve_symlink=False)
DIR = FILE.parent


@beartype.beartype
@dataclasses.dataclass
class Config:
    name: str

    verbose: bool

    out_dir: utils.PathLike

    target: utils.Target

    c_standard: str
    cpp_standard: str

    c_include_dirs: list[utils.PathLike]
    cpp_include_dirs: list[utils.PathLike]

    enable_debug: bool
    enable_asan: bool

    opt_type: str
    link_time_opt: bool


@beartype.beartype
def add_deps(builder: building_utils.Builder, config: Config):
    out_dir = utils.to_pathlib_path(config.out_dir)

    # --------------------------------------------------------------------------

    compiler = llvm_utils.LLVMCompiler(llvm_utils.LLVMCompilerConfig(
        verbose=config.verbose,

        target=config.target,

        base_dir=DIR.parent.parent,

        c_standard=config.c_standard,
        cpp_standard=config.cpp_standard,

        c_include_dirs=config.c_include_dirs,
        cpp_include_dirs=config.cpp_include_dirs,

        enable_debug=config.enable_debug,
        enable_asan=config.enable_asan,

        opt_type=config.opt_type,
        link_time_opt=config.link_time_opt,
    ))

    # --------------------------------------------------------------------------

    builder.add_sym("zeta_core_dir", DIR)
    builder.add_sym("zeta_core_out_dir", out_dir)

    # --------------------------------------------------------------------------

    @beartype.beartype
    class CCPPFileNode:
        def __init__(
            self,
            file: utils.PathLike,
            langs: utils.Language | typing.Iterable[utils.Language],
        ):
            self.file = utils.to_pathlib_path(file)
            self.langs: tuple[utils.Language] = \
                (langs,) if isinstance(langs, utils.Language) \
                else tuple(sorted(set(langs)))

            self.cached_parsed_tu: typing.Optional[clang.cindex.TranslationUnit] = None

        @functools.cached_property
        def parsed_asts(self) -> dict[utils.Language, clang.cindex.TranslationUnit]:
            return {
                lang: compiler.parse_ast(self.file, lang)
                for lang in self.langs
            }

        def get_deps(self) -> set[pathlib.Path]:
            base_dir = DIR.parent

            cache_file = out_dir / \
                f"{self.file.name}.file_including_files.json"

            if cache_file.is_file() and \
                    1e-3 <= cache_file.stat().st_mtime - self.file.stat().st_mtime:
                return {
                    utils.to_canon_path(val, solve_symlink=True)
                    for val in utils.read_json(cache_file)
                }

            include_files: set[pathlib.Path] = {FILE}

            for parsed_ast in self.parsed_asts.values():
                for include_file in llvm_utils.get_include_files(parsed_ast):
                    if include_file.is_relative_to(base_dir):
                        include_files.add(include_file)

            utils.write_json(
                cache_file, [val.as_posix() for val in include_files])

            return include_files

        def build(self):
            print(f"Checking {self.file}...")

    file_nodes: dict[pathlib.Path, CCPPFileNode] = dict()

    @beartype.beartype
    def add_c_cpp_file(file: pathlib.Path, lang: utils.Language):
        file_node = None

        if file in file_nodes:
            file_node = file_nodes[file]
        else:
            file_node = file_nodes[file] = CCPPFileNode(file, lang)

        builder.add_build_node(file, file_node.get_deps, file_node.build)

    @beartype.beartype
    def add_c_cpp_module(module: str):
        h_file = DIR / f"{module}.h"
        hpp_file = DIR / f"{module}.hpp"
        ipp_file = DIR / f"{module}.ipp"
        c_file = DIR / f"{module}.c"
        cpp_file = DIR / f"{module}.cpp"
        bc_file = out_dir / f"{module}.bc"

        assert not c_file.exists() or not cpp_file.exists()

        if h_file.exists():
            add_c_cpp_file(h_file, utils.Language.C_HEADER)

        if hpp_file.exists():
            add_c_cpp_file(hpp_file, utils.Language.CPP_HEADER)

        if ipp_file.exists():
            add_c_cpp_file(ipp_file, utils.Language.CPP_HEADER)

        if c_file.exists():
            add_c_cpp_file(c_file, utils.Language.C_SOURCE)

            builder.add_build_node(
                bc_file,
                lambda: {FILE, c_file},
                lambda: compiler.compile_to_bc(
                    bc_file, c_file, utils.Language.CPP_SOURCE),
            )

        if cpp_file.exists():
            add_c_cpp_file(cpp_file, utils.Language.CPP_SOURCE)

            builder.add_build_node(
                bc_file,
                lambda: {FILE, cpp_file},
                lambda: compiler.compile_to_bc(
                    bc_file, cpp_file, utils.Language.CPP_SOURCE),
            )

    # --------------------------------------------------------------------------

    builder.add_build_node(FILE, None, None)

    add_c_cpp_module("allocator_ref")
    add_c_cpp_module("allocator")
    add_c_cpp_module("assoc_cntr_ref")
    add_c_cpp_module("assoc_cntr")
    add_c_cpp_module("basic_bin_tree_node")
    add_c_cpp_module("bin_tree")
    add_c_cpp_module("cascade_allocator")
    add_c_cpp_module("circular_array")
    add_c_cpp_module("compare")
    add_c_cpp_module("datetime")
    add_c_cpp_module("debug_deque")
    add_c_cpp_module("debug_hash_table")
    add_c_cpp_module("debug_utils")
    add_c_cpp_module("define")
    add_c_cpp_module("dynamic_hash_table")
    add_c_cpp_module("fixed_point")
    add_c_cpp_module("function_ref")
    add_c_cpp_module("generic_hash_table")
    add_c_cpp_module("hash")
    add_c_cpp_module("integral")
    add_c_cpp_module("llist_node_tpl")
    add_c_cpp_module("llist")
    add_c_cpp_module("mem_recorder")
    add_c_cpp_module("meta")
    add_c_cpp_module("multi_level_data_table")
    add_c_cpp_module("multi_level_ptr_table")
    add_c_cpp_module("multi_level_table.mpp")
    add_c_cpp_module("percent_prime_table")
    add_c_cpp_module("pool_allocator")
    add_c_cpp_module("ptr_utils")
    add_c_cpp_module("rbtree")
    add_c_cpp_module("seg_utils")
    add_c_cpp_module("seg_vector.mpp")
    add_c_cpp_module("seg_vector")
    add_c_cpp_module("seq_cntr_ref")
    add_c_cpp_module("seq_cntr")
    add_c_cpp_module("staging_seg_vector")
    add_c_cpp_module("tuple")
    add_c_cpp_module("type_list")
    add_c_cpp_module("type_wrapper")
    add_c_cpp_module("utils")
    add_c_cpp_module("value_wrapper")
