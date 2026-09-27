from __future__ import annotations

import dataclasses
import functools
import os
import pathlib
import typing

import beartype

from ... import building_utils, llvm_utils, utils

FILE = utils.to_resolved_path(__file__)
DIR = FILE.parent


@beartype.beartype
@dataclasses.dataclass
class Config:
    parent_module_chain: building_utils.ModuleChain

    name: str

    out_dir: utils.PathLike

    target: utils.Target

    c_standard: str
    cpp_standard: str

    c_include_dirs: list[utils.PathLike]
    cpp_include_dirs: list[utils.PathLike]

    debug_enable: bool
    asan_enable: bool

    core_debug_utils_sanity_enable: bool

    opt_type: str
    link_time_opt: bool


@beartype.beartype
def add_deps(build_graph: building_utils.BuildGraph, config: Config):
    module_chain = building_utils.ModuleChain((
        *config.parent_module_chain.module_pairs,
        building_utils.ModulePair(
            name="core_test",
            config=None,
        ),
    ))

    configed_module_chain = building_utils.ModuleChain((
        *config.parent_module_chain.module_pairs,
        building_utils.ModulePair(
            name="core_test",
            config=config.name,
        ),
    ))

    base_deps = [FILE]

    out_dir = utils.to_pathlib_path(config.out_dir)

    # --------------------------------------------------------------------------

    llvm_toolchain_config = llvm_utils.LLVMToolchainConfig(
        target=config.target,

        base_dir=DIR.parent.parent,

        c_standard=config.c_standard,
        cpp_standard=config.cpp_standard,

        c_include_dirs=config.c_include_dirs,
        cpp_include_dirs=config.cpp_include_dirs,

        enable_debug=config.debug_enable,
        enable_asan=config.asan_enable,

        c_defines=dict(),
        cpp_defines=dict(),

        opt_type=config.opt_type,
        link_time_opt=config.link_time_opt,
    )

    if config.debug_enable:
        llvm_toolchain_config.c_defines["ZETA_Core_DebugEnable"] = "1"
        llvm_toolchain_config.cpp_defines["ZETA_Core_DebugEnable"] = "1"

    if config.core_debug_utils_sanity_enable:
        llvm_toolchain_config.c_defines["ZETA_Core_DebugUtils_Sanity_Enable"] = "1"
        llvm_toolchain_config.cpp_defines["ZETA_Core_DebugUtils_Sanity_Enable"] = "1"

    llvm_toolchain = llvm_utils.LLVMToolchain(llvm_toolchain_config)

    # --------------------------------------------------------------------------

    zeta_core_sym_table = build_graph.get_sym("zeta_core_sym_table")

    zeta_core_dir = zeta_core_sym_table.dir
    zeta_core_out_dir = zeta_core_sym_table.config.out_dir

    # --------------------------------------------------------------------------

    @beartype.beartype
    class CPPBuildAction:
        def __init__(self, file_cpp: utils.PathLike):
            self.name = utils.to_pathlib_path(file_cpp)
            self.module_chain = module_chain

        @functools.cache
        def get_der_arts(self) -> list[pathlib.Path]:
            return [self.name]

        @functools.cache
        def get_dep_arts(self) -> tuple[pathlib.Path, ...]:
            cpp_file = llvm_utils.ClangFile(
                path=self.name,
                lang=utils.Language.CPP_SOURCE,
                toolchain=llvm_toolchain,
                include_files_cache=out_dir /
                f"{self.name.name}.file_including_files.json",
            )

            return tuple(
                include_file
                for include_file in cpp_file.get_include_files()
                if include_file.is_relative_to(DIR.parent.parent)
            )

        def run(self) -> None:
            os.utime(self.name, None)

    @beartype.beartype
    def add_simple(file: pathlib.Path) -> None:
        file = utils.to_absolute_path(file)

        build_graph.add_act(building_utils.SimpleBuildAction(
            name=file,
            module_chain=module_chain,
            get_der_arts=lambda: [file],
            get_dep_arts=lambda: base_deps,
            run=lambda: os.utime(file, None),
        ))

    @beartype.beartype
    def add_c_cpp_module(module: str):
        file_h = DIR / f"{module}.h"
        file_hpp = DIR / f"{module}.hpp"
        file_ipp = DIR / f"{module}.ipp"
        file_c = DIR / f"{module}.c"
        file_cpp = DIR / f"{module}.cpp"
        file_bc = out_dir / f"{module}.bc"

        assert not file_c.exists() or not file_cpp.exists()

        if file_h.exists():
            add_simple(file_h)

        if file_hpp.exists():
            add_simple(file_hpp)

        if file_ipp.exists():
            add_simple(file_ipp)

        if file_cpp.exists():
            build_graph.add_act(CPPBuildAction(file_cpp))

            build_graph.add_act(llvm_utils.BCBuildAction(
                module_chain=configed_module_chain,
                file_bc=file_bc,
                file_src=file_cpp,
                lang=utils.Language.CPP_SOURCE,
                base_deps=base_deps,
                toolchain=llvm_toolchain,
            ))

    @beartype.beartype
    def add_k_build_node(
        name: str,
        deps: typing.Iterable[utils.PathLike],
    ) -> None:
        file_cpp = DIR / f"{name}.cpp"
        file_bc = out_dir / f"{name}.bc"
        file_exe = out_dir / f"{name}.exe"

        build_graph.add_act(CPPBuildAction(file_cpp))

        build_graph.add_act(
            llvm_utils.BCBuildAction(
                module_chain=configed_module_chain,
                file_bc=file_bc,
                file_src=file_cpp,
                lang=utils.Language.CPP_SOURCE,
                base_deps=base_deps,
                toolchain=llvm_toolchain,
            )
        )

        build_graph.add_act(building_utils.SimpleBuildAction(
            module_chain=configed_module_chain,
            name=out_dir / f"{name}.exe",
            get_der_arts=lambda: [file_exe],
            get_dep_arts=lambda: [*base_deps, file_bc, *deps],
            run=lambda: llvm_toolchain.compile_to_exe(
                file_exe, [file_bc, *deps]),
        ))

    # --------------------------------------------------------------------------

    build_graph.add_act(building_utils.SimpleBuildAction(
        name=FILE,
        module_chain=module_chain,
        get_der_arts=lambda: [FILE],
        get_dep_arts=lambda: list(),
        run=lambda: os.utime(FILE, None),
    ))

    add_c_cpp_module("assoc_cntr_utils")
    add_c_cpp_module("buffer")
    add_c_cpp_module("cache_manager_utils")
    add_c_cpp_module("cpp_std_allocator")

    build_graph.add_act(building_utils.SimpleBuildAction(
        name=DIR / "cor.s",
        module_chain=module_chain,
        get_der_arts=lambda: [DIR / "cor.s"],
        get_dep_arts=lambda: list(),
        run=lambda: None,
    ))

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

    add_c_cpp_module("key_value_pair")
    add_c_cpp_module("lru_cache_manager_utils")
    add_c_cpp_module("hash")

    add_c_cpp_module("naive_search_table")

    add_c_cpp_module("pod_value")

    add_c_cpp_module("random")

    add_c_cpp_module("ptr_iter")

    add_c_cpp_module("staging_seg_vector_utils")
    add_c_cpp_module("static_search_table")
    add_c_cpp_module("std_allocator")
    add_c_cpp_module("test_head")

    add_c_cpp_module("test_1")
    add_c_cpp_module("test_binheap")

    add_c_cpp_module("seg_vector_utils")

    add_c_cpp_module("seq_cntr_utils")

    # --------------------------------------------------------------------------

    add_k_build_node("test_datetime", list())

    add_k_build_node("test_debug_utils_new", list())

    add_k_build_node(
        "test_dht",
        [
            zeta_core_out_dir / "percent_prime_table.bc",
            zeta_core_out_dir / "mem_recorder.bc",
            zeta_core_out_dir / "utils.bc",
            out_dir / "timer.bc",
        ],
    )

    add_k_build_node(
        "test_fixed_point",
        [
            zeta_core_out_dir / "mem_recorder.bc",
            out_dir / "timer.bc",
        ],
    )

    add_k_build_node(
        "test_lin_space_mapper",
        [
            zeta_core_out_dir / "mem_recorder.bc",
            out_dir / "timer.bc",
        ],
    )

    add_k_build_node(
        "test_random",
        [
            out_dir / "timer.bc",
        ],
    )

    add_k_build_node("test_utf16", list())

    add_k_build_node(
        "test_osn",
        [
            out_dir / "timer.bc",
        ],
    )

    add_k_build_node(
        "test_search_table",
        [
            out_dir / "timer.bc",
        ],
    )

    add_k_build_node("test_segvec", list())

    add_k_build_node("test_segvec2", list())

    add_k_build_node("test_segvec_speed", list())

    add_k_build_node("test_segvec_speed2", list())

    add_k_build_node("test_seqcntr", list())

    add_k_build_node(
        "test_pool_alctr",
        [
            zeta_core_out_dir / "mem_recorder.bc",
            out_dir / "timer.bc",
        ],
    )

    add_k_build_node(
        "test_stagevec",
        [
            zeta_core_out_dir / "mem_recorder.bc",
            out_dir / "timer.bc",
        ],
    )

    add_k_build_node(
        "test_stagevec_speed",
        [
            zeta_core_out_dir / "mem_recorder.bc",
            out_dir / "timer.bc",
        ],
    )

    add_k_build_node("test_kmp", list())

    add_k_build_node(
        "test_lrucm",
        [
            zeta_core_out_dir / "mem_recorder.bc",
            out_dir / "timer.bc",
        ],
    )

    add_k_build_node("test_seg_tree", list())

    add_k_build_node(
        "test_sort",
        [
            zeta_core_out_dir / "mem_recorder.bc",
            out_dir / "timer.bc",
        ],
    )

    add_k_build_node("test_slaballoc", list())

    add_k_build_node(
        "test_lin_space_allocator",
        [
            zeta_core_out_dir / "mem_recorder.bc",
        ],
    )

    add_k_build_node("test_mlv", list())

    add_k_build_node(
        "test_mlt",
        [
            zeta_core_out_dir / "mem_recorder.bc",
            out_dir / "timer.bc",
        ],
    )

    add_k_build_node("test_pipe", list())

    add_k_build_node("test_qsort", list())

    add_k_build_node(
        "test_json_utils",
        [
            out_dir / "timer.bc",
        ],
    )

    add_k_build_node("test_scheduler", list())

    add_k_build_node(
        "test_integral_endec",
        [
            out_dir / "timer.bc",
        ],
    )

    add_k_build_node(
        "test_vlq_utils",
        [
            out_dir / "timer.bc",
        ],
    )

    add_k_build_node(
        "test_utf8",
        [
            out_dir / "timer.bc",
        ],
    )

    add_simple(DIR / "switch.s")

    add_k_build_node(
        "test_cascade_alloc",
        [
            zeta_core_out_dir / "mem_recorder.bc",
            out_dir / "timer.bc",
        ],
    )

    add_k_build_node(
        "test_cntrbt",
        [
            zeta_core_out_dir / "mem_recorder.bc",
        ],
    )

    zeta_core_flow_s = {
        utils.ArchEnum.INTEL64: zeta_core_dir / "flow_intel64.s",
    }[config.target.arch]

    add_k_build_node(
        "test_exception",
        [
            zeta_core_flow_s,
            zeta_core_out_dir / "mem_recorder.bc",
        ],
    )

    add_k_build_node(
        "test_flow",
        [
            zeta_core_flow_s,
        ],
    )

    add_k_build_node(
        "test_tuple",
        [
            out_dir / "timer.bc",
        ],
    )

    add_simple(DIR / "cpuid.s")

    add_c_cpp_module("timer")
