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
    def add_header_source_object(name: str, lang: utils.Language):
        h_file = DIR / f"{name}.h"

        match lang:
            case utils.Language.C:
                src_file = DIR / f"{name}.c"
            case utils.Language.CPP:
                src_file = DIR / f"{name}.cpp"
            case _:
                raise NotImplementedError()

        obj_file = out_dir / f"{name}.o"

        add_c_cpp_file(h_file, utils.Language.C_CPP_HEADER)
        add_c_cpp_file(src_file, lang.source)

        builder.add_build_node(
            obj_file,
            lambda: {FILE, src_file},
            lambda: compiler.compile_to_obj(obj_file, src_file, lang.source),
        )

    # --------------------------------------------------------------------------

    builder.add_build_node(FILE, None, None)

    add_header_source_object("algorithm", utils.Language.C)
    add_header_source_object("allocator", utils.Language.C)
    add_header_source_object("assoc_cntr", utils.Language.C)
    add_header_source_object("bin_heap", utils.Language.C)

    add_c_cpp_file(DIR / "bin_tree_node_temp.h", utils.Language.C_CPP_HEADER)
    add_c_cpp_file(DIR / "bin_tree_node_temp.c", utils.Language.C_SOURCE)

    add_c_cpp_file(DIR / "bin_tree_temp.h", utils.Language.C_CPP_HEADER)
    add_c_cpp_file(DIR / "bin_tree_temp.c", utils.Language.C_SOURCE)

    add_header_source_object("bin_tree_node", utils.Language.C)
    add_header_source_object("cache_manager", utils.Language.C)
    add_header_source_object("caching_array", utils.Language.C)
    add_header_source_object("cascade_allocator", utils.Language.C)
    add_header_source_object("circular_array", utils.Language.C)
    add_header_source_object("crc", utils.Language.C)
    add_header_source_object("datetime", utils.Language.C)
    add_header_source_object("debug_deque", utils.Language.CPP)
    add_header_source_object("debug_hash_table", utils.Language.CPP)
    add_header_source_object("debug_str_pipe", utils.Language.CPP)

    add_header_source_object("debugger", utils.Language.C)
    add_header_source_object("disk_info", utils.Language.C)
    add_header_source_object("disk_part_gpt", utils.Language.C)
    add_header_source_object("disk_part_mbr", utils.Language.C)
    add_header_source_object("dummy_cache_manager", utils.Language.C)

    add_c_cpp_file(DIR / "define.h", utils.Language.C_CPP_HEADER)

    add_header_source_object("dynamic_hash_table", utils.Language.C)
    add_header_source_object("dynamic_vector", utils.Language.C)
    add_header_source_object("elf_utils", utils.Language.C)

    flow_s = {
        utils.ArchEnum.INTEL64: DIR / "flow_intel64.s",
    }[config.target.arch]

    builder.add_build_node(flow_s, lambda: {FILE}, None)

    add_c_cpp_file(DIR / "flow.h", utils.Language.C_CPP_HEADER)

    add_header_source_object("generic_hash_table", utils.Language.C)
    add_header_source_object("io", utils.Language.C)

    add_c_cpp_file(DIR / "jump.h", utils.Language.C_CPP_HEADER)

    builder.add_build_node(DIR / "Jump_x86_64.s", lambda: {FILE}, None)

    builder.add_build_node(
        out_dir / "jump.o",
        lambda: {
            FILE,
            DIR / "jump.h",
            DIR / "Jump_x86_64.s",
        },
        lambda: compiler.asm_to_obj(
            out_dir / "jump.o",
            DIR / "Jump_x86_64.s",
        )
    )

    add_header_source_object("lin_space_allocator", utils.Language.C)

    add_c_cpp_file(DIR / "llist_node_temp.h", utils.Language.C_CPP_HEADER)
    add_c_cpp_file(DIR / "llist_node_temp.c", utils.Language.C_SOURCE)

    add_header_source_object("llist_node", utils.Language.C)

    add_c_cpp_file(DIR / "llist_temp.h", utils.Language.C_CPP_HEADER)
    add_c_cpp_file(DIR / "llist_temp.c", utils.Language.C_SOURCE)

    add_header_source_object("logger", utils.Language.C)
    add_header_source_object("lru_cache_manager", utils.Language.C)
    add_header_source_object("mem_check_utils", utils.Language.CPP)
    add_header_source_object("memory", utils.Language.C)
    add_header_source_object("multi_level_circular_array", utils.Language.C)

    add_c_cpp_file(DIR / "multi_level_table_temp.h",
                   utils.Language.C_CPP_HEADER)
    add_c_cpp_file(DIR / "multi_level_table_temp.c", utils.Language.C_SOURCE)

    add_header_source_object("multi_level_data_table", utils.Language.C)
    add_header_source_object("multi_level_ptr_table", utils.Language.C)

    add_c_cpp_file(DIR / "pipe.h", utils.Language.C_CPP_HEADER)
    add_header_source_object("pool_allocator", utils.Language.C)
    add_header_source_object("random", utils.Language.C)

    add_c_cpp_file(DIR / "rbtree_temp.h", utils.Language.C_CPP_HEADER)
    add_c_cpp_file(DIR / "rbtree_temp.c", utils.Language.C_SOURCE)

    add_c_cpp_file(DIR / "ptr_utils.h", utils.Language.C_CPP_HEADER)

    add_c_cpp_file(DIR / "seg_staging_vector_temp.h",
                   utils.Language.C_CPP_HEADER)
    add_c_cpp_file(DIR / "seg_staging_vector_temp.c", utils.Language.C_SOURCE)

    add_header_source_object("seg_vector", utils.Language.C)
    add_header_source_object("seg_tree", utils.Language.C)
    add_header_source_object("seg_utils", utils.Language.C)
    add_header_source_object("seq_cntr", utils.Language.C)
    add_header_source_object("sha256", utils.Language.C)
    add_header_source_object("slab_allocator", utils.Language.C)
    add_header_source_object("staging_vector", utils.Language.C)
    add_header_source_object("tree_allocator", utils.Language.C)
    add_header_source_object("utf8", utils.Language.C)
    add_header_source_object("utf16", utils.Language.C)
    add_header_source_object("utils", utils.Language.C)
