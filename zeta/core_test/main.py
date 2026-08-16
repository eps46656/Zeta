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
    compiler = llvm_utils.LLVMToolchain(llvm_utils.LLVMCompilerConfig(
        verbose=config.verbose,

        target=config.target,

        base_dir=DIR.parent.parent,

        c_standard=config.c_standard,
        cpp_standard=config.cpp_standard,

        c_include_dirs=config.c_include_dirs,
        cpp_include_dirs=config.cpp_include_dirs,

        enable_debug=config.enable_debug,
        enable_asan=config.enable_asan,

        c_defines={},
        cpp_defines={},

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

    # --------------------------------------------------------------------------

    zeta_core_dir = builder.syms["zeta_core_dir"]
    zeta_core_out_dir = builder.syms["zeta_core_out_dir"]

    out_dir = utils.to_pathlib_path(config.out_dir)

    builder.add_sym("zeta_core_test_dir", DIR)
    builder.add_sym("zeta_core_test_out_dir", out_dir)

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

    c_cpp_file_nodes: dict[pathlib.Path, CCPPFileNode] = dict()

    @beartype.beartype
    def add_c_cpp(
        c_cpp_file: pathlib.Path,
        langs: utils.Language | typing.Iterable[utils.Language],
    ):
        cur_node = CCPPFileNode(c_cpp_file, langs)

        if c_cpp_file in c_cpp_file_nodes:
            assert cur_node.langs == c_cpp_file_nodes[c_cpp_file].langs
            return

        c_cpp_file_nodes[c_cpp_file] = cur_node
        builder.add_build_node(c_cpp_file, cur_node.get_deps, cur_node.build)

    @beartype.beartype
    def add_c_cpp_to_bc(
        bc_file: pathlib.Path,
        c_cpp_file: pathlib.Path,
        lang: utils.Language,
    ) -> None:
        assert lang.base != lang
        assert lang.enmacro != lang

        add_c_cpp(c_cpp_file, lang)

        builder.add_build_node(
            bc_file,
            lambda: {FILE, c_cpp_file},
            lambda: compiler.compile_to_bc(
                bc_file, c_cpp_file, utils.Language.CPP_SOURCE),
        )

    @beartype.beartype
    def add_files_to_exe(exe_file: pathlib.Path, files: set[pathlib.Path]):
        builder.add_build_node(
            exe_file,
            lambda: {FILE, *files},
            lambda: compiler.compile_to_exe(exe_file, files),
        )

    def add_c_cpp_bc_exe(
        name: str,
        lang: utils.Language,
        files: set[pathlib.Path],
    ):
        match lang:
            case utils.Language.C:
                src_file = DIR / f"{name}.c"
                src_lang = utils.Language.C_SOURCE
            case utils.Language.CPP:
                src_file = DIR / f"{name}.cpp"
                src_lang = utils.Language.CPP_SOURCE
            case _:
                raise NotImplementedError()

        src_bc_file = out_dir / f"{name}.bc"
        exe_file = out_dir / f"{name}.exe"

        add_c_cpp_to_bc(src_bc_file, src_file, src_lang)

        add_files_to_exe(exe_file, {src_bc_file, *files})

    @beartype.beartype
    def add_c_cpp_module(module: str, macro: bool = False):
        h_file = DIR / f"{module}.h"
        hpp_file = DIR / f"{module}.hpp"
        ipp_file = DIR / f"{module}.ipp"
        c_file = DIR / f"{module}.c"
        cpp_file = DIR / f"{module}.cpp"
        bc_file = out_dir / f"{module}.bc"

        assert not c_file.exists() or not cpp_file.exists()

        if h_file.exists():
            add_c_cpp(
                h_file,
                utils.Language.MACRO_C_HEADER
                if macro else utils.Language.C_HEADER
            )

        if hpp_file.exists():
            add_c_cpp(
                hpp_file,
                utils.Language.MACRO_CPP_HEADER
                if macro else utils.Language.CPP_HEADER
            )

        if ipp_file.exists():
            add_c_cpp(
                ipp_file,
                utils.Language.MACRO_CPP_HEADER
                if macro else utils.Language.CPP_HEADER
            )

        if c_file.exists():
            add_c_cpp(
                c_file,
                utils.Language.MACRO_C_SOURCE
                if macro else utils.Language.C_SOURCE
            )

            if not macro:
                add_c_cpp_to_bc(bc_file, c_file, utils.Language.C_SOURCE)

        if cpp_file.exists():
            add_c_cpp(
                cpp_file,
                utils.Language.MACRO_CPP_SOURCE
                if macro else utils.Language.CPP_SOURCE
            )

            if not macro:
                add_c_cpp_to_bc(bc_file, cpp_file, utils.Language.CPP_SOURCE)

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
    add_c_cpp_module("multi_level_circular_array_utils")

    add_c_cpp_module("hash_utils")

    add_c_cpp(DIR / "key_value_pair.h", (C_HEADER, CPP_HEADER))
    add_c_cpp(DIR / "lru_cache_manager_utils.h", (C_HEADER, CPP_HEADER))
    add_c_cpp(DIR / "hash.h", (C_HEADER, CPP_HEADER))

    add_c_cpp_module("multi_level_circular_array_utils")

    add_c_cpp(DIR / "naive_search_table.h", (C_HEADER, CPP_HEADER))

    add_c_cpp(DIR / "pod_value.hpp", CPP_HEADER)

    add_c_cpp(DIR / "random.hpp", CPP_HEADER)

    add_c_cpp(DIR / "ptr_iter.hpp", CPP_HEADER)
    add_c_cpp(DIR / "ptr_iter.ipp", CPP_HEADER)

    add_c_cpp(DIR / "staging_seg_vector_utils.hpp", CPP_HEADER)
    add_c_cpp(DIR / "static_search_table.h", (C_HEADER, CPP_HEADER))
    add_c_cpp(DIR / "std_allocator.hpp", CPP_HEADER)
    add_c_cpp(DIR / "test_head.h", (C_HEADER, CPP_HEADER))

    add_c_cpp_module("test_1")
    add_c_cpp_module("test_binheap")

    add_c_cpp(DIR / "seg_vector_utils.hpp", CPP_HEADER)

    add_c_cpp(DIR / "seq_cntr_utils.hpp", CPP_HEADER)

    # --------------------------------------------------------------------------

    add_c_cpp_bc_exe(
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

    add_c_cpp_bc_exe(
        "test_3",
        C,
        set(),
    )

    add_c_cpp_bc_exe(
        "test_4",
        C,
        set(),
    )

    add_c_cpp_bc_exe(
        "test_datetime",
        CPP,
        {
        },
    )

    add_c_cpp_bc_exe(
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
            zeta_core_out_dir / "percent_prime_table.bc",
            zeta_core_out_dir / "mem_recorder.bc",
            zeta_core_out_dir / "utils.bc",
            out_dir / "timer.bc",
        },
    )

    add_c_cpp_bc_exe(
        "test_fixed_point",
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
            zeta_core_out_dir / "mem_recorder.bc",
            zeta_core_out_dir / "utils.bc",
            out_dir / "timer.bc",
        },
    )

    add_c_cpp_bc_exe(
        "test_lin_space_mapper",
        CPP,
        {
            zeta_core_out_dir / "mem_recorder.bc",
            zeta_core_out_dir / "utils.bc",
            out_dir / "timer.bc",
        },
    )

    add_c_cpp_bc_exe(
        "test_random",
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
            zeta_core_out_dir / "utils.bc",
            out_dir / "timer.bc",
        },
    )

    add_c_cpp_bc_exe(
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

    add_c_cpp_bc_exe(
        "test_osn",
        CPP,
        {
            out_dir / "timer.bc",
        }
    )

    add_c_cpp_bc_exe(
        "test_search_table",
        CPP,
        {
            zeta_core_out_dir / "debug_str_pipe.bc",
            zeta_core_out_dir / "debugger.bc",
            out_dir / "timer.bc",
        }
    )

    add_c_cpp_bc_exe(
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

    add_c_cpp_bc_exe(
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

    add_c_cpp_bc_exe(
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

    add_c_cpp_bc_exe(
        "test_segvec_speed2",
        CPP,
        {
            zeta_core_out_dir / "seg_vector.bc",
            zeta_core_out_dir / "circular_array.bc",
            zeta_core_out_dir / "debug_hash_table.bc",
            zeta_core_out_dir / "utils.bc",
        },
    )

    add_c_cpp_bc_exe(
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

    add_c_cpp_bc_exe(
        "test_pool_alctr",
        CPP,
        {
            zeta_core_out_dir / "mem_recorder.bc",
            out_dir / "timer.bc",
        }
    )

    add_c_cpp_bc_exe(
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
            # zeta_core_out_dir / "seg_utils.bc",
            # zeta_core_out_dir / "seg_vector.bc",
            # zeta_core_out_dir / "seq_cntr.bc",
            # zeta_core_out_dir / "staging_vector.bc",
            zeta_core_out_dir / "mem_recorder.bc",
            out_dir / "timer.bc",
        },
    )

    add_c_cpp_bc_exe(
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
            zeta_core_out_dir / "mem_recorder.bc",
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

    add_c_cpp_bc_exe(
        "test_kmp",
        CPP,
        {
            zeta_core_out_dir / "debug_str_pipe.bc",
            zeta_core_out_dir / "debugger.bc",
        },
    )

    add_c_cpp_bc_exe(
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
            zeta_core_out_dir / "mem_recorder.bc",
            zeta_core_out_dir / "memory.bc",
            zeta_core_out_dir / "multi_level_ptr_table.bc",
            zeta_core_out_dir / "seq_cntr.bc",
            zeta_core_out_dir / "utils.bc",
            out_dir / "timer.bc",
        },
    )

    add_c_cpp_bc_exe(
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

    add_c_cpp_bc_exe(
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
            zeta_core_out_dir / "mem_recorder.bc",
            zeta_core_out_dir / "memory.bc",
            zeta_core_out_dir / "utils.bc",
            out_dir / "timer.bc",
        },
    )

    add_c_cpp_bc_exe(
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

    add_c_cpp_bc_exe(
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
            zeta_core_out_dir / "mem_recorder.bc",
            zeta_core_out_dir / "utils.bc",
        },
    )

    add_c_cpp_bc_exe(
        "test_mlv",
        CPP,
        {
            zeta_core_out_dir / "allocator.bc",
            zeta_core_out_dir / "debug_hash_table.bc",
            zeta_core_out_dir / "multi_level_ptr_table.bc",
            zeta_core_out_dir / "utils.bc",
        },
    )

    add_c_cpp_bc_exe(
        "test_mlt",
        CPP,
        {
            zeta_core_out_dir / "mem_recorder.bc",
            zeta_core_out_dir / "utils.bc",
            out_dir / "timer.bc",
        },
    )

    add_c_cpp_bc_exe(
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

    add_c_cpp_bc_exe(
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

    add_c_cpp_bc_exe(
        "test_json_utils",
        CPP,
        {
            out_dir / "timer.bc",
        },
    )

    add_c_cpp_bc_exe(
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

    add_c_cpp_bc_exe(
        "test_serde_utils",
        CPP,
        {
            out_dir / "timer.bc",
        },
    )

    add_c_cpp_bc_exe(
        "test_vlq_utils",
        CPP,
        {
            out_dir / "timer.bc",
        },
    )

    add_c_cpp_bc_exe(
        "test_utf8",
        CPP,
        {
            out_dir / "timer.bc",
        },
    )

    builder.add_build_node(
        DIR / "switch.s",
        lambda: {FILE},
        None
    )

    add_c_cpp_bc_exe(
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
            zeta_core_out_dir / "mem_recorder.bc",
            zeta_core_out_dir / "utils.bc",
            out_dir / "timer.bc",
        },
    )

    add_c_cpp_bc_exe(
        "test_cntrbt",
        CPP,
        {
            zeta_core_out_dir / "mem_recorder.bc",
            zeta_core_out_dir / "utils.bc",
        },
    )

    zeta_core_flow_s = {
        utils.ArchEnum.INTEL64: zeta_core_dir / "flow_intel64.s",
    }[config.target.arch]

    add_c_cpp_bc_exe(
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
            zeta_core_out_dir / "mem_recorder.bc",
            zeta_core_out_dir / "memory.bc",
            zeta_core_out_dir / "utils.bc",
        },
    )

    add_c_cpp_bc_exe(
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

    add_c_cpp_bc_exe(
        "test_tuple",
        CPP,
        {
            zeta_core_out_dir / "utils.bc",
            out_dir / "timer.bc",
        },
    )

    builder.add_build_node(DIR / "cpuid.s", lambda: {FILE}, None)

    add_c_cpp_module("timer")

    builder.add_build_node(
        DIR / "buffered_allocator.h",
        lambda: {
            FILE,
            zeta_core_dir / "allocator.h",
        },
        None
    )
