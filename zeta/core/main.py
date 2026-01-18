from __future__ import annotations

import dataclasses
import pathlib

import beartype

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
    def add_c_cpp_file(file: pathlib.Path, lang: utils.Language):
        def get_deps() -> set[pathlib.Path]:
            base_dir = DIR.parent
            cache_file = out_dir / f"{file.name}.file_including_files.json"

            include_files = compiler.get_including_files(
                file, lang, cache_file)

            return {
                FILE,
                *(include_file for include_file in include_files if include_file.is_relative_to(base_dir)),
            }

        builder.add_build_node(file, get_deps, None)

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

    add_c_cpp_module("allocator")
    add_c_cpp_module("assoc_cntr")
    add_c_cpp_module("bin_tree_node_tpl")
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
    add_c_cpp_module("function_ref")
    add_c_cpp_module("generic_hash_table")
    add_c_cpp_module("integral")
    add_c_cpp_module("llist_node_tpl")
    add_c_cpp_module("llist")
    add_c_cpp_module("mem_check_utils")
    add_c_cpp_module("multi_level_table.mpp")
    add_c_cpp_module("multi_level_data_table")
    add_c_cpp_module("multi_level_ptr_table")
    add_c_cpp_module("pool_allocator")
    add_c_cpp_module("ptr_utils")
    add_c_cpp_module("rbtree")
    add_c_cpp_module("staging_seg_vector")
    add_c_cpp_module("seg_utils")
    add_c_cpp_module("seg_vector")
    add_c_cpp_module("seg_vector.mpp")
    add_c_cpp_module("seq_cntr")
    add_c_cpp_module("tuple")
    add_c_cpp_module("type_list")
    add_c_cpp_module("type_traits")
    add_c_cpp_module("type_wrapper")
    add_c_cpp_module("hash")
    add_c_cpp_module("utils")
    add_c_cpp_module("value_wrapper")
