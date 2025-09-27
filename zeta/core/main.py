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

    C_HEADER = utils.Language.C_HEADER
    C_SOURCE = utils.Language.C_SOURCE
    C = utils.Language.C

    CPP_HEADER = utils.Language.CPP_HEADER
    CPP_SOURCE = utils.Language.CPP_SOURCE
    CPP = utils.Language.CPP

    C_CPP_HEADER = utils.Language.C_CPP_HEADER

    # --------------------------------------------------------------------------

    builder.add_sym("zeta_core_dir", DIR)
    builder.add_sym("zeta_core_out_dir", out_dir)

    # --------------------------------------------------------------------------

    @beartype.beartype
    def add_c_cpp(file: pathlib.Path, lang: utils.Language):
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
    def add_cpp_bc(name: str):
        cpp_file = DIR / f"{name}.cpp"
        bc_file = out_dir / f"{name}.bc"

        add_c_cpp(cpp_file, CPP_SOURCE)

        builder.add_build_node(
            bc_file,
            lambda: {FILE, cpp_file},
            lambda: compiler.compile_to_bc(bc_file, cpp_file, CPP_SOURCE),
        )

    # --------------------------------------------------------------------------

    builder.add_build_node(FILE, None, None)

    add_c_cpp(DIR / "allocator.hpp", CPP_HEADER)
    add_c_cpp(DIR / "allocator.ipp", CPP_HEADER)

    add_c_cpp(DIR / "bin_tree_node_tpl.hpp", CPP_HEADER)
    add_c_cpp(DIR / "bin_tree_node_tpl.ipp", CPP_HEADER)

    add_c_cpp(DIR / "bin_tree.hpp", CPP_HEADER)
    add_c_cpp(DIR / "bin_tree.ipp", CPP_HEADER)

    add_c_cpp(DIR / "circular_array.hpp", CPP_HEADER)
    add_c_cpp(DIR / "circular_array.ipp", CPP_HEADER)

    add_c_cpp(DIR / "datetime.hpp", CPP_HEADER)
    add_cpp_bc("datetime")

    add_c_cpp(DIR / "debug_deque.hpp", CPP_HEADER)
    add_c_cpp(DIR / "debug_deque.ipp", CPP_HEADER)

    add_c_cpp(DIR / "debug_utils.hpp", CPP_HEADER)
    add_c_cpp(DIR / "debug_utils.ipp", CPP_HEADER)

    add_c_cpp(DIR / "define.hpp", CPP_HEADER)

    add_c_cpp(DIR / "integral.hpp", CPP_HEADER)

    add_c_cpp(DIR / "mem_check_utils.hpp", CPP_HEADER)
    add_cpp_bc("mem_check_utils")

    add_c_cpp(DIR / "multi_level_table.hpp", CPP_HEADER)
    add_c_cpp(DIR / "multi_level_table.ipp", CPP_HEADER)

    add_c_cpp(DIR / "multi_level_ptr_data_table.xmacro.hpp", CPP_HEADER)
    add_c_cpp(DIR / "multi_level_ptr_data_table.xmacro.cpp", CPP_HEADER)

    add_c_cpp(DIR / "multi_level_ptr_table.hpp", CPP_HEADER)
    add_cpp_bc("multi_level_ptr_table")

    add_c_cpp(DIR / "multi_level_data_table.hpp", CPP_HEADER)
    add_cpp_bc("multi_level_data_table")

    add_c_cpp(DIR / "ptr_utils.hpp", CPP_HEADER)
    add_c_cpp(DIR / "ptr_utils.ipp", CPP_HEADER)

    add_c_cpp(DIR / "rbtree.hpp", CPP_HEADER)
    add_c_cpp(DIR / "rbtree.ipp", CPP_HEADER)

    add_c_cpp(DIR / "seq_cntr.hpp", CPP_HEADER)
    add_c_cpp(DIR / "seq_cntr.ipp", CPP_HEADER)

    add_c_cpp(DIR / "type_list.hpp", CPP_HEADER)
    add_c_cpp(DIR / "type_traits.hpp", CPP_HEADER)

    add_c_cpp(DIR / "utils.hpp", CPP_HEADER)
    add_c_cpp(DIR / "utils.ipp", CPP_HEADER)

    add_cpp_bc("utils")

    add_c_cpp(DIR / "tuple.hpp", CPP_HEADER)
