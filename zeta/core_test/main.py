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

    zeta_core_dir = builder.syms["zeta_core_dir"]
    zeta_core_out_dir = builder.syms["zeta_core_out_dir"]

    out_dir = utils.to_pathlib_path(config.out_dir)

    builder.add_sym("zeta_core_test_dir", DIR)
    builder.add_sym("zeta_core_test_out_dir", out_dir)

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
    def add_src_to_bc(
        bc_file: pathlib.Path,
        src_file: pathlib.Path,
        lang: utils.Language,
    ) -> None:
        builder.add_build_node(
            bc_file,
            lambda: {FILE, src_file},
            lambda: compiler.compile_to_bc(bc_file, src_file, lang),
        )

    @beartype.beartype
    def add_files_to_exe(exe_file: pathlib.Path, files: set[pathlib.Path]):
        builder.add_build_node(
            exe_file,
            lambda: {FILE, *files},
            lambda: compiler.compile_to_exe(exe_file, files),
        )

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

    def add_src_files_exe(
        name: str,
        lang: utils.Language,
        files: set[pathlib.Path],
    ):
        match lang:
            case utils.Language.C:
                src_file = DIR / f"{name}.c"
            case utils.Language.CPP:
                src_file = DIR / f"{name}.cpp"
            case _:
                raise NotImplementedError()

        src_bc_file = out_dir / f"{name}.bc"
        exe_file = out_dir / f"{name}.exe"

        add_c_cpp_file(src_file, lang.source)

        add_src_to_bc(src_bc_file, src_file, lang.source)

        add_files_to_exe(exe_file, {src_bc_file, *files})

    # --------------------------------------------------------------------------

    builder.add_build_node(FILE, None, None)

    add_c_cpp_file(DIR / "assoc_cntr_utils.h", utils.Language.C_CPP_HEADER)
    add_c_cpp_file(DIR / "buffer.h", utils.Language.C_CPP_HEADER)
    add_c_cpp_file(DIR / "cache_manager_utils.h", utils.Language.C_CPP_HEADER)
    add_c_cpp_file(DIR / "cpp_std_allocator.h", utils.Language.C_CPP_HEADER)

    builder.add_build_node(DIR / "cor.s", None, None)

    add_c_cpp_file(DIR / "cascade_alloc_utils.h", utils.Language.C_CPP_HEADER)
    add_c_cpp_file(DIR / "cmp_utils.h", utils.Language.C_CPP_HEADER)
    add_c_cpp_file(DIR / "caching_array_utils.h", utils.Language.C_CPP_HEADER)
    add_c_cpp_file(DIR / "circular_array_utils.h", utils.Language.C_CPP_HEADER)
    add_c_cpp_file(DIR / "debug_deque_utils.h", utils.Language.C_CPP_HEADER)
    add_c_cpp_file(DIR / "debug_hash_table_utils.h",
                   utils.Language.C_CPP_HEADER)
    add_c_cpp_file(DIR / "dynamic_hash_table_utils.h",
                   utils.Language.C_CPP_HEADER)
    add_c_cpp_file(DIR / "dynamic_search_table.h", utils.Language.C_CPP_HEADER)
    add_c_cpp_file(DIR / "dynamic_vector_utils.h", utils.Language.C_CPP_HEADER)
    add_c_cpp_file(DIR / "hash_utils.h", utils.Language.C_CPP_HEADER)
    add_c_cpp_file(DIR / "key_value_pair.h", utils.Language.C_CPP_HEADER)
    add_c_cpp_file(DIR / "lru_cache_manager_utils.h",
                   utils.Language.C_CPP_HEADER)
    add_c_cpp_file(DIR / "hash.h", utils.Language.C_CPP_HEADER)

    add_h_src_bc("multi_level_circular_array_utils", utils.Language.CPP)

    add_c_cpp_file(DIR / "naive_search_table.h", utils.Language.C_CPP_HEADER)
    add_c_cpp_file(DIR / "pair.h", utils.Language.C_CPP_HEADER)
    add_c_cpp_file(DIR / "pod_value.h", utils.Language.C_CPP_HEADER)
    add_c_cpp_file(DIR / "random.hpp", utils.Language.CPP_HEADER)
    add_c_cpp_file(DIR / "ptr_iter.h", utils.Language.C_CPP_HEADER)
    add_c_cpp_file(DIR / "staging_vector_utils.h", utils.Language.C_CPP_HEADER)
    add_c_cpp_file(DIR / "static_search_table.h", utils.Language.C_CPP_HEADER)
    add_c_cpp_file(DIR / "std_allocator.h", utils.Language.C_CPP_HEADER)
    add_c_cpp_file(DIR / "test_head.h", utils.Language.C_CPP_HEADER)

    add_h_src_bc("test_1", utils.Language.C)
    add_h_src_bc("test_binheap", utils.Language.CPP)

    add_c_cpp_file(DIR / "seg_vector_utils.h", utils.Language.C_CPP_HEADER)
    add_c_cpp_file(DIR / "seq_cntr_utils.h", utils.Language.C_CPP_HEADER)

    # --------------------------------------------------------------------------

    add_src_files_exe(
        "test_2",
        utils.Language.C,
        {
            zeta_core_out_dir / "debug_str_pipe.bc",
            zeta_core_out_dir / "debugger.bc",
            zeta_core_out_dir / "logger.bc",
            zeta_core_out_dir / "io.bc",
            zeta_core_out_dir / "utils.bc",
        },
    )

    add_src_files_exe(
        "test_3",
        utils.Language.C,
        set(),
    )

    add_src_files_exe(
        "test_4",
        utils.Language.C,
        set(),
    )

    add_src_files_exe(
        "test_datetime",
        utils.Language.CPP,
        {
            zeta_core_out_dir / "datetime.bc",
            zeta_core_out_dir / "debug_str_pipe.bc",
            zeta_core_out_dir / "debugger.bc",
        },
    )

    add_src_files_exe(
        "test_dht",
        utils.Language.CPP,
        {
            zeta_core_out_dir / "allocator.bc",
            zeta_core_out_dir / "assoc_cntr.bc",
            zeta_core_out_dir / "bin_tree_node.bc",
            zeta_core_out_dir / "debug_hash_table.bc",
            zeta_core_out_dir / "debug_str_pipe.bc",
            zeta_core_out_dir / "debugger.bc",
            zeta_core_out_dir / "dynamic_hash_table.bc",
            zeta_core_out_dir / "generic_hash_table.bc",
            zeta_core_out_dir / "io.bc",
            zeta_core_out_dir / "llist_node.bc",
            zeta_core_out_dir / "logger.bc",
            zeta_core_out_dir / "mem_check_utils.bc",
            zeta_core_out_dir / "multi_level_ptr_table.bc",
            zeta_core_out_dir / "utils.bc",
            out_dir / "timer.bc",
        }
    )

    add_src_files_exe(
        "test_utf8",
        utils.Language.C,
        {
            zeta_core_out_dir / "debugger.bc",
            zeta_core_out_dir / "debug_str_pipe.bc",
            zeta_core_out_dir / "logger.bc",
            zeta_core_out_dir / "io.bc",
            zeta_core_out_dir / "utf8.bc",
            zeta_core_out_dir / "utils.bc",
        }
    )

    add_src_files_exe(
        "test_utf16",
        utils.Language.C,
        {
            zeta_core_out_dir / "debugger.bc",
            zeta_core_out_dir / "debug_str_pipe.bc",
            zeta_core_out_dir / "logger.bc",
            zeta_core_out_dir / "io.bc",
            zeta_core_out_dir / "utf16.bc",
            zeta_core_out_dir / "utils.bc",
        }
    )

    add_src_files_exe(
        "test_search_table",
        utils.Language.CPP,
        {
            zeta_core_out_dir / "debug_str_pipe.bc",
            zeta_core_out_dir / "debugger.bc",
            out_dir / "timer.bc",
        }
    )

    add_src_files_exe(
        "test_segvec",
        utils.Language.CPP,
        {
            zeta_core_out_dir / "seg_vector.bc",
            zeta_core_out_dir / "circular_array.bc",
            zeta_core_out_dir / "debug_deque.bc",
            zeta_core_out_dir / "debug_hash_table.bc",
            zeta_core_out_dir / "utils.bc",
        },
    )

    add_src_files_exe(
        "test_segvec2",
        utils.Language.CPP,
        {
            zeta_core_out_dir / "circular_array.bc",
            zeta_core_out_dir / "debug_deque.bc",
            zeta_core_out_dir / "debug_hash_table.bc",
            zeta_core_out_dir / "seg_vector.bc",
            zeta_core_out_dir / "utils.bc",
        },
    )

    add_src_files_exe(
        "test_segvec_speed",
        utils.Language.CPP,
        {
            zeta_core_out_dir / "seg_vector.bc",
            zeta_core_out_dir / "circular_array.bc",
            zeta_core_out_dir / "debug_deque.bc",
            zeta_core_out_dir / "debug_hash_table.bc",
            zeta_core_out_dir / "utils.bc",
        },
    )

    add_src_files_exe(
        "test_segvec_speed2",
        utils.Language.CPP,
        {
            zeta_core_out_dir / "seg_vector.bc",
            zeta_core_out_dir / "circular_array.bc",
            zeta_core_out_dir / "debug_hash_table.bc",
            zeta_core_out_dir / "utils.bc",
        },
    )

    add_src_files_exe(
        "test_seqcntr",
        utils.Language.CPP,
        {
            zeta_core_out_dir / "circular_array.bc",
            zeta_core_out_dir / "debug_deque.bc",
            zeta_core_out_dir / "debug_hash_table.bc",
            zeta_core_out_dir / "seg_vector.bc",
            zeta_core_out_dir / "utils.bc",
        },
    )

    add_src_files_exe(
        "test_stagevec",
        utils.Language.CPP,
        {
            zeta_core_out_dir / "algorithm.bc",
            zeta_core_out_dir / "allocator.bc",
            zeta_core_out_dir / "assoc_cntr.bc",
            zeta_core_out_dir / "bin_tree_node.bc",
            zeta_core_out_dir / "cascade_allocator.bc",
            zeta_core_out_dir / "circular_array.bc",
            zeta_core_out_dir / "debug_deque.bc",
            zeta_core_out_dir / "debug_hash_table.bc",
            zeta_core_out_dir / "debug_str_pipe.bc",
            zeta_core_out_dir / "debugger.bc",
            zeta_core_out_dir / "dynamic_hash_table.bc",
            # zeta_out_dir / "dynamic_vector.bc",
            zeta_core_out_dir / "generic_hash_table.bc",
            zeta_core_out_dir / "io.bc",
            zeta_core_out_dir / "llist_node.bc",
            zeta_core_out_dir / "logger.bc",
            zeta_core_out_dir / "mem_check_utils.bc",
            zeta_core_out_dir / "memory.bc",
            zeta_core_out_dir / "multi_level_circular_array.bc",
            zeta_core_out_dir / "multi_level_ptr_table.bc",
            zeta_core_out_dir / "pool_allocator.bc",
            zeta_core_out_dir / "seg_utils.bc",
            zeta_core_out_dir / "seg_vector.bc",
            zeta_core_out_dir / "seq_cntr.bc",
            zeta_core_out_dir / "staging_vector.bc",
            zeta_core_out_dir / "utils.bc",
            out_dir / "timer.bc",
        },
    )

    add_src_files_exe(
        "test_stagevec_speed",
        utils.Language.CPP,
        {
            zeta_core_out_dir / "algorithm.bc",
            zeta_core_out_dir / "allocator.bc",
            zeta_core_out_dir / "assoc_cntr.bc",
            zeta_core_out_dir / "bin_tree_node.bc",
            zeta_core_out_dir / "cascade_allocator.bc",
            zeta_core_out_dir / "circular_array.bc",
            zeta_core_out_dir / "debug_deque.bc",
            zeta_core_out_dir / "debug_hash_table.bc",
            zeta_core_out_dir / "debug_str_pipe.bc",
            zeta_core_out_dir / "debugger.bc",
            zeta_core_out_dir / "dynamic_hash_table.bc",
            # zeta_out_dir / "dynamic_vector.bc",
            zeta_core_out_dir / "generic_hash_table.bc",
            zeta_core_out_dir / "io.bc",
            zeta_core_out_dir / "llist_node.bc",
            zeta_core_out_dir / "logger.bc",
            zeta_core_out_dir / "mem_check_utils.bc",
            zeta_core_out_dir / "memory.bc",
            zeta_core_out_dir / "multi_level_circular_array.bc",
            zeta_core_out_dir / "multi_level_ptr_table.bc",
            zeta_core_out_dir / "pool_allocator.bc",
            zeta_core_out_dir / "seg_utils.bc",
            zeta_core_out_dir / "seg_vector.bc",
            zeta_core_out_dir / "seq_cntr.bc",
            zeta_core_out_dir / "staging_vector.bc",
            zeta_core_out_dir / "utils.bc",
            out_dir / "timer.bc",
        },
    )

    add_src_files_exe(
        "test_kmp",
        utils.Language.CPP,
        {
            zeta_core_out_dir / "debug_str_pipe.bc",
            zeta_core_out_dir / "debugger.bc",
        },
    )

    add_src_files_exe(
        "test_lrucm",
        utils.Language.CPP,
        {
            zeta_core_out_dir / "allocator.bc",
            zeta_core_out_dir / "assoc_cntr.bc",
            zeta_core_out_dir / "cache_manager.bc",
            zeta_core_out_dir / "caching_array.bc",
            zeta_core_out_dir / "cascade_allocator.bc",
            zeta_core_out_dir / "debug_deque.bc",
            zeta_core_out_dir / "debug_hash_table.bc",
            zeta_core_out_dir / "debug_str_pipe.bc",
            zeta_core_out_dir / "debugger.bc",
            zeta_core_out_dir / "dummy_cache_manager.bc",
            zeta_core_out_dir / "generic_hash_table.bc",
            zeta_core_out_dir / "llist_node.bc",
            zeta_core_out_dir / "lru_cache_manager.bc",
            zeta_core_out_dir / "mem_check_utils.bc",
            zeta_core_out_dir / "memory.bc",
            zeta_core_out_dir / "multi_level_ptr_table.bc",
            zeta_core_out_dir / "seq_cntr.bc",
            zeta_core_out_dir / "utils.bc",
            out_dir / "timer.bc",
        },
    )

    add_src_files_exe(
        "test_seg_tree",
        utils.Language.CPP,
        {
            zeta_core_out_dir / "debug_str_pipe.bc",
            zeta_core_out_dir / "debugger.bc",
            zeta_core_out_dir / "io.bc",
            zeta_core_out_dir / "seg_tree.bc",
            zeta_core_out_dir / "utils.bc",
        },
    )

    add_src_files_exe(
        "test_sort",
        utils.Language.CPP,
        {
            zeta_core_out_dir / "algorithm.bc",
            zeta_core_out_dir / "allocator.bc",
            zeta_core_out_dir / "cascade_allocator.bc",
            zeta_core_out_dir / "debug_str_pipe.bc",
            zeta_core_out_dir / "debugger.bc",
            zeta_core_out_dir / "io.bc",
            zeta_core_out_dir / "llist_node.bc",
            zeta_core_out_dir / "mem_check_utils.bc",
            zeta_core_out_dir / "memory.bc",
            zeta_core_out_dir / "utils.bc",
            out_dir / "timer.bc",
        },
    )

    add_src_files_exe(
        "test_slaballoc",
        utils.Language.CPP,
        {
            zeta_core_out_dir / "allocator.bc",
            zeta_core_out_dir / "debug_hash_table.bc",
            zeta_core_out_dir / "llist_node.bc",
            zeta_core_out_dir / "slab_allocator.bc",
            zeta_core_out_dir / "utils.bc",
        },
    )

    add_src_files_exe(
        "test_lin_space_allocator",
        utils.Language.CPP,
        {
            zeta_core_out_dir / "allocator.bc",
            zeta_core_out_dir / "bin_tree_node.bc",
            zeta_core_out_dir / "debug_str_pipe.bc",
            zeta_core_out_dir / "debugger.bc",
            zeta_core_out_dir / "io.bc",
            zeta_core_out_dir / "lin_space_allocator.bc",
            zeta_core_out_dir / "logger.bc",
            zeta_core_out_dir / "mem_check_utils.bc",
            zeta_core_out_dir / "utils.bc",
        },
    )

    add_src_files_exe(
        "test_mlv",
        utils.Language.CPP,
        {
            zeta_core_out_dir / "allocator.bc",
            zeta_core_out_dir / "debug_hash_table.bc",
            zeta_core_out_dir / "multi_level_ptr_table.bc",
            zeta_core_out_dir / "utils.bc",
        },
    )

    add_src_files_exe(
        "test_mlt",
        utils.Language.CPP,
        {
            zeta_core_out_dir / "allocator.bc",
            zeta_core_out_dir / "debug_str_pipe.bc",
            zeta_core_out_dir / "debugger.bc",
            zeta_core_out_dir / "io.bc",
            zeta_core_out_dir / "llist_node.bc",
            zeta_core_out_dir / "logger.bc",
            zeta_core_out_dir / "mem_check_utils.bc",
            zeta_core_out_dir / "multi_level_data_table.bc",
            zeta_core_out_dir / "multi_level_ptr_table.bc",
            zeta_core_out_dir / "utils.bc",
            out_dir / "timer.bc",
        },
    )

    add_src_files_exe(
        "test_pipe",
        utils.Language.C,
        {
            zeta_core_out_dir / "debug_str_pipe.bc",
            zeta_core_out_dir / "debugger.bc",
            zeta_core_out_dir / "io.bc",
            zeta_core_out_dir / "logger.bc",
            zeta_core_out_dir / "utils.bc",
        },
    )

    add_src_files_exe(
        "test_qsort",
        utils.Language.CPP,
        {
            zeta_core_out_dir / "algorithm.bc",
            zeta_core_out_dir / "debugger.bc",
            zeta_core_out_dir / "debug_str_pipe.bc",
            zeta_core_out_dir / "io.bc",
            zeta_core_out_dir / "utils.bc",
        },
    )

    add_src_files_exe(
        "test_scheduler",
        utils.Language.CPP,
        {
            zeta_core_out_dir / "allocator.bc",
            zeta_core_out_dir / "debug_hash_table.bc",
            zeta_core_out_dir / "llist_node.bc",
            zeta_core_out_dir / "multi_level_ptr_table.bc",
            zeta_core_out_dir / "utils.bc",
        },
    )

    builder.add_build_node(
        DIR / "switch.s",
        lambda: {FILE},
        None
    )

    add_src_files_exe(
        "test_cascade_alloc",
        utils.Language.CPP,
        {
            zeta_core_out_dir / "allocator.bc",
            zeta_core_out_dir / "cascade_allocator.bc",
            zeta_core_out_dir / "debug_str_pipe.bc",
            zeta_core_out_dir / "debugger.bc",
            zeta_core_out_dir / "io.bc",
            zeta_core_out_dir / "llist_node.bc",
            zeta_core_out_dir / "logger.bc",
            zeta_core_out_dir / "mem_check_utils.bc",
            zeta_core_out_dir / "utils.bc",
        },
    )

    add_src_files_exe(
        "test_cntrbt",
        utils.Language.CPP,
        {
            zeta_core_out_dir / "mem_check_utils.bc",
            zeta_core_out_dir / "utils.bc",
        },
    )

    zeta_core_flow_s = {
        utils.ArchEnum.INTEL64: zeta_core_dir / "flow_intel64.s",
    }[config.target.arch]

    add_src_files_exe(
        "test_exception",
        utils.Language.CPP,
        {
            zeta_core_flow_s,
            zeta_core_out_dir / "allocator.bc",
            zeta_core_out_dir / "cascade_allocator.bc",
            zeta_core_out_dir / "debug_str_pipe.bc",
            zeta_core_out_dir / "debugger.bc",
            zeta_core_out_dir / "io.bc",
            zeta_core_out_dir / "llist_node.bc",
            zeta_core_out_dir / "logger.bc",
            zeta_core_out_dir / "mem_check_utils.bc",
            zeta_core_out_dir / "memory.bc",
            zeta_core_out_dir / "utils.bc",
        },
    )

    add_src_files_exe(
        "test_flow",
        utils.Language.CPP,
        {
            zeta_core_flow_s,
            zeta_core_out_dir / "debug_str_pipe.bc",
            zeta_core_out_dir / "debugger.bc",
            zeta_core_out_dir / "io.bc",
            zeta_core_out_dir / "logger.bc",
        }
    )

    builder.add_build_node(DIR / "cpuid.s", lambda: {FILE}, None)

    add_h_src_bc("timer", utils.Language.CPP)

    builder.add_build_node(
        DIR / "buffered_allocator.h",
        lambda: {
            FILE,
            zeta_core_dir / "allocator.h",
        },
        None
    )
