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
            cache_file = out_dir / f"{file.name}.file_including_paris.json"
            base_dir = DIR.parent

            if not cache_file.is_file() or 1e-3 <= file.stat().st_mtime - cache_file.stat().st_mtime:
                if lang == utils.Language.C_CPP_HEADER:
                    include_files = set.union(
                        compiler.get_include_files(
                            file, utils.Language.C_HEADER),
                        compiler.get_include_files(
                            file, utils.Language.CPP_HEADER),
                    )

                    include_files = set(sorted(include_files))
                else:
                    include_files = compiler.get_include_files(file, lang)

                utils.write_json(
                    cache_file, [val.as_posix() for val in include_files])
            else:
                include_files = {
                    utils.to_canon_path(val, solve_symlink=True)
                    for val in utils.read_json(cache_file)
                }

            return {
                FILE,
                *(include_file for include_file in include_files if include_file.is_relative_to(base_dir)),
            }

        builder.add_build_node(file, get_deps, None)

    @beartype.beartype
    def add_h_src_bc(name: str, lang: utils.Language):
        h_file = DIR / f"{name}.h"

        match lang:
            case utils.Language.C:
                src_file = DIR / f"{name}.c"
            case utils.Language.CPP:
                src_file = DIR / f"{name}.cpp"
            case _:
                raise NotImplementedError()

        bc_file = out_dir / f"{name}.bc"

        add_c_cpp_file(h_file, utils.Language.C_CPP_HEADER)
        add_c_cpp_file(src_file, lang.source)

        builder.add_build_node(
            bc_file,
            lambda: {FILE, src_file},
            lambda: compiler.compile_to_bc(
                bc_file, src_file, lang.source),
        )

    @beartype.beartype
    def add_hpp_cpp_bc(name: str):
        hpp_file = DIR / f"{name}.hpp"
        cpp_file = DIR / f"{name}.cpp"
        bc_file = out_dir / f"{name}.bc"

        add_c_cpp_file(hpp_file, utils.Language.C_CPP_HEADER)
        add_c_cpp_file(cpp_file, utils.Language.CPP_SOURCE)

        builder.add_build_node(
            bc_file,
            lambda: {FILE, cpp_file},
            lambda: compiler.compile_to_bc(
                bc_file, cpp_file, utils.Language.CPP_SOURCE),
        )

    # --------------------------------------------------------------------------

    builder.add_build_node(FILE, None, None)

    add_c_cpp_file(DIR / "bin_tree_node_temp.hpp", utils.Language.CPP_HEADER)
    add_c_cpp_file(DIR / "bin_tree.hpp", utils.Language.CPP_HEADER)

    add_c_cpp_file(DIR / "define.h", utils.Language.CPP_HEADER)

    add_c_cpp_file(DIR / "integer.hpp", utils.Language.CPP_HEADER)

    add_h_src_bc("mem_check_utils", utils.Language.CPP)

    add_c_cpp_file(DIR / "ptr_utils.hpp", utils.Language.CPP_HEADER)

    add_c_cpp_file(DIR / "rbtree.hpp", utils.Language.CPP_HEADER)

    add_hpp_cpp_bc("utils")
