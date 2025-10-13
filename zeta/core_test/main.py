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

    # --------------------------------------------------------------------------

    C_HEADER = utils.Language.C_HEADER
    C_SOURCE = utils.Language.C_SOURCE
    C = utils.Language.C

    CPP_HEADER = utils.Language.CPP_HEADER
    CPP_SOURCE = utils.Language.CPP_SOURCE
    CPP = utils.Language.CPP

    C_CPP_HEADER = utils.Language.C_CPP_HEADER

    # --------------------------------------------------------------------------

    zeta_core_dir = builder.syms["zeta_core_dir"]
    zeta_core_out_dir = builder.syms["zeta_core_out_dir"]

    out_dir = utils.to_pathlib_path(config.out_dir)

    builder.add_sym("zeta_core_test_dir", DIR)
    builder.add_sym("zeta_core_test_out_dir", out_dir)

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

        add_c_cpp_file(h_file, C_CPP_HEADER)
        add_c_cpp_file(src_file, lang.source)

        builder.add_build_node(
            bc_file,
            lambda: {FILE, src_file},
            lambda: compiler.compile_to_bc(bc_file, src_file, lang.source),
        )

    @beartype.beartype
    def add_cpp_bc(name: str):
        cpp_file = DIR / f"{name}.cpp"
        bc_file = out_dir / f"{name}.bc"

        add_c_cpp_file(cpp_file, CPP_SOURCE)

        builder.add_build_node(
            bc_file,
            lambda: {FILE, cpp_file},
            lambda: compiler.compile_to_bc(bc_file, cpp_file, CPP_SOURCE),
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

    add_c_cpp_module("assoc_cntr_utils")
    add_c_cpp_module("buffer")
    add_c_cpp_module("cache_manager_utils")
    add_c_cpp_module("cpp_std_allocator")

    builder.add_build_node(DIR / "cor.s", None, None)

    add_c_cpp_module("cascade_alloc_utils")
    add_c_cpp_module("cmp_utils")
    add_c_cpp_module("caching_array_utils")

    add_c_cpp_module("circular_array_utils")
    add_c_cpp_module("debug_deque_utils")
    add_c_cpp_module("debug_hash_table_utils")
    add_c_cpp_module("dynamic_hash_table_utils")
    add_c_cpp_module("dynamic_search_table")
    add_c_cpp_module("dynamic_vector_utils")

    add_c_cpp_module("hash_utils")

    add_c_cpp_file(DIR / "key_value_pair.h", C_CPP_HEADER)
    add_c_cpp_file(DIR / "lru_cache_manager_utils.h", C_CPP_HEADER)
    add_c_cpp_file(DIR / "hash.h", C_CPP_HEADER)

    add_h_src_bc("multi_level_circular_array_utils", CPP)

    add_c_cpp_file(DIR / "naive_search_table.h", C_CPP_HEADER)

    add_c_cpp_file(DIR / "pod_value.hpp", C_CPP_HEADER)

    add_c_cpp_file(DIR / "random.hpp", CPP_HEADER)

    add_c_cpp_file(DIR / "ptr_iter.hpp", C_CPP_HEADER)
    add_c_cpp_file(DIR / "ptr_iter.ipp", C_CPP_HEADER)

    add_c_cpp_file(DIR / "staging_vector_utils.h", C_CPP_HEADER)
    add_c_cpp_file(DIR / "static_search_table.h", C_CPP_HEADER)
    add_c_cpp_file(DIR / "std_allocator.hpp", C_CPP_HEADER)
    add_c_cpp_file(DIR / "test_head.h", C_CPP_HEADER)

    add_h_src_bc("test_1", C)
    add_h_src_bc("test_binheap", CPP)

    add_c_cpp_file(DIR / "seg_vector_utils.hpp", C_CPP_HEADER)

    add_c_cpp_file(DIR / "seq_cntr_utils.hpp", C_CPP_HEADER)

    # --------------------------------------------------------------------------

    add_src_files_exe(
        "test_2",
        C,
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
        C,
        set(),
    )

    add_src_files_exe(
        "test_4",
        C,
        set(),
    )

    add_src_files_exe(
        "test_datetime",
        CPP,
        {
            zeta_core_out_dir / "datetime.bc",
        },
    )

    add_src_files_exe(
        "test_dht",
        CPP,
        {
            # zeta_core_out_dir / "cascade_allocator.bc",
            # zeta_core_out_dir / "debug_hash_table.bc",
            # zeta_core_out_dir / "dynamic_hash_table.bc",
            # zeta_out_dir / "dynamic_vector.bc",
            # zeta_core_out_dir / "generic_hash_table.bc",
            # zeta_core_out_dir / "logger.bc",
            # zeta_core_out_dir / "memory.bc",
            # zeta_core_out_dir / "multi_level_circular_array.bc",
            # zeta_core_out_dir / "multi_level_ptr_table.bc",
            # zeta_core_out_dir / "seg_utils.bc",
            # zeta_core_out_dir / "seg_vector.bc",
            # zeta_core_out_dir / "seq_cntr.bc",
            # zeta_core_out_dir / "staging_vector.bc",
            zeta_core_out_dir / "mem_check_utils.bc",
            zeta_core_out_dir / "utils.bc",
            out_dir / "timer.bc",
        },
    )

    add_src_files_exe(
        "test_utf8",
        C,
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
        C,
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
        CPP,
        {
            zeta_core_out_dir / "debug_str_pipe.bc",
            zeta_core_out_dir / "debugger.bc",
            out_dir / "timer.bc",
        }
    )

    add_src_files_exe(
        "test_segvec",
        CPP,
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
        CPP,
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
        CPP,
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
        CPP,
        {
            zeta_core_out_dir / "seg_vector.bc",
            zeta_core_out_dir / "circular_array.bc",
            zeta_core_out_dir / "debug_hash_table.bc",
            zeta_core_out_dir / "utils.bc",
        },
    )

    add_src_files_exe(
        "test_seqcntr",
        CPP,
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
        CPP,
        {
            # zeta_core_out_dir / "cascade_allocator.bc",
            # zeta_core_out_dir / "debug_hash_table.bc",
            # zeta_core_out_dir / "dynamic_hash_table.bc",
            # zeta_out_dir / "dynamic_vector.bc",
            # zeta_core_out_dir / "generic_hash_table.bc",
            # zeta_core_out_dir / "logger.bc",
            # zeta_core_out_dir / "memory.bc",
            # zeta_core_out_dir / "multi_level_circular_array.bc",
            # zeta_core_out_dir / "multi_level_ptr_table.bc",
            zeta_core_out_dir / "pool_allocator.bc",
            # zeta_core_out_dir / "seg_utils.bc",
            # zeta_core_out_dir / "seg_vector.bc",
            # zeta_core_out_dir / "seq_cntr.bc",
            # zeta_core_out_dir / "staging_vector.bc",
            zeta_core_out_dir / "mem_check_utils.bc",
            zeta_core_out_dir / "utils.bc",
            out_dir / "timer.bc",
        },
    )

    add_src_files_exe(
        "test_stagevec_speed",
        CPP,
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
        CPP,
        {
            zeta_core_out_dir / "debug_str_pipe.bc",
            zeta_core_out_dir / "debugger.bc",
        },
    )

    add_src_files_exe(
        "test_lrucm",
        CPP,
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
        CPP,
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
        CPP,
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
        CPP,
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
        CPP,
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
        CPP,
        {
            zeta_core_out_dir / "allocator.bc",
            zeta_core_out_dir / "debug_hash_table.bc",
            zeta_core_out_dir / "multi_level_ptr_table.bc",
            zeta_core_out_dir / "utils.bc",
        },
    )

    add_src_files_exe(
        "test_mlt",
        CPP,
        {
            zeta_core_out_dir / "mem_check_utils.bc",
            zeta_core_out_dir / "utils.bc",
            out_dir / "timer.bc",
        },
    )

    add_src_files_exe(
        "test_pipe",
        C,
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
        CPP,
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
        CPP,
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
        CPP,
        {
            # zeta_core_out_dir / "cascade_allocator.bc",
            # zeta_core_out_dir / "debug_hash_table.bc",
            # zeta_core_out_dir / "dynamic_hash_table.bc",
            # zeta_out_dir / "dynamic_vector.bc",
            # zeta_core_out_dir / "generic_hash_table.bc",
            # zeta_core_out_dir / "logger.bc",
            # zeta_core_out_dir / "memory.bc",
            # zeta_core_out_dir / "multi_level_circular_array.bc",
            # zeta_core_out_dir / "multi_level_ptr_table.bc",
            zeta_core_out_dir / "pool_allocator.bc",
            # zeta_core_out_dir / "seg_utils.bc",
            # zeta_core_out_dir / "seg_vector.bc",
            # zeta_core_out_dir / "seq_cntr.bc",
            # zeta_core_out_dir / "staging_vector.bc",
            zeta_core_out_dir / "mem_check_utils.bc",
            zeta_core_out_dir / "utils.bc",
            out_dir / "timer.bc",
        },
    )

    add_src_files_exe(
        "test_cntrbt",
        CPP,
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
        CPP,
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
        CPP,
        {
            zeta_core_flow_s,
            zeta_core_out_dir / "debug_str_pipe.bc",
            zeta_core_out_dir / "debugger.bc",
            zeta_core_out_dir / "io.bc",
            zeta_core_out_dir / "logger.bc",
        }
    )

    add_src_files_exe(
        "test_tuple",
        CPP,
        {
            zeta_core_out_dir / "utils.bc",
            out_dir / "timer.bc",
        },
    )

    builder.add_build_node(DIR / "cpuid.s", lambda: {FILE}, None)

    builder.add_build_node(DIR / "timer.hpp", lambda: {FILE}, None)
    add_cpp_bc("timer")

    builder.add_build_node(
        DIR / "buffered_allocator.h",
        lambda: {
            FILE,
            zeta_core_dir / "allocator.h",
        },
        None
    )
