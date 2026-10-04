from __future__ import annotations

import dataclasses
import functools
import os
import pathlib
import typing

import beartype

from ... import building_utils, llvm_utils, utils

FILE = utils.to_absolute_path(__file__)
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
@dataclasses.dataclass(frozen=True)
class SymbolTable:
    config: Config

    dir: pathlib.Path

    module_chain: building_utils.ModuleChain
    configed_module_chain: building_utils.ModuleChain


@beartype.beartype
def add_deps(build_graph: building_utils.BuildGraph, config: Config):
    module_chain = building_utils.ModuleChain((
        *config.parent_module_chain.module_pairs,
        building_utils.ModulePair(
            name="core",
            config=None,
        ),
    ))

    configed_module_chain = building_utils.ModuleChain((
        *config.parent_module_chain.module_pairs,
        building_utils.ModulePair(
            name="core",
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

        c_defines={
            "ZetaDir": f"\"{DIR.as_posix()}\"",
        },
        cpp_defines={
            "ZetaDir": f"\"{DIR.as_posix()}\"",
        },

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
        file = utils.to_pathlib_path(file)

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

    # --------------------------------------------------------------------------

    build_graph.add_sym(
        "zeta_core_sym_table",
        SymbolTable(
            config=config,
            dir=DIR,
            module_chain=module_chain,
            configed_module_chain=configed_module_chain,
        )
    )

    build_graph.add_act(building_utils.SimpleBuildAction(
        name=FILE,
        module_chain=module_chain,
        get_dep_arts=lambda: list(),
        get_der_arts=lambda: [FILE],
        run=lambda: os.utime(FILE, None),
    ))

    add_c_cpp_module("allocator")
    add_c_cpp_module("array")
    add_c_cpp_module("ascii")
    add_c_cpp_module("assoc_cntr_ref")
    add_c_cpp_module("assoc_cntr")
    add_c_cpp_module("basic_bin_tree_node")
    add_c_cpp_module("basic_llist_node")
    add_c_cpp_module("bin_tree")
    add_c_cpp_module("cascade_allocator")
    add_c_cpp_module("circular_array")
    add_c_cpp_module("comparison_ref")
    add_c_cpp_module("comparison_utils")
    add_c_cpp_module("comparison")
    add_c_cpp_module("datetime")
    add_c_cpp_module("debug_deque")
    add_c_cpp_module("debug_hash_table")
    add_c_cpp_module("debug_utils/diag")
    add_c_cpp_module("debug_utils/identity_graph")
    add_c_cpp_module("debug_utils/lifecycle_sanity")
    add_c_cpp_module("debug_utils/logging")
    add_c_cpp_module("debug_utils/memory")
    add_c_cpp_module("debug_utils/recording_allocator")
    add_c_cpp_module("debug_utils/sanity")
    add_c_cpp_module("define")
    add_c_cpp_module("dynamic_hash_table")
    add_c_cpp_module("error")
    add_c_cpp_module("fixed_point")
    add_c_cpp_module("function_ref")
    add_c_cpp_module("generic_hash_table")
    add_c_cpp_module("hash_ref")
    add_c_cpp_module("hash_utils")
    add_c_cpp_module("hash")
    add_c_cpp_module("integral_bit")
    add_c_cpp_module("integral_endec")
    add_c_cpp_module("integral_math")
    add_c_cpp_module("integral_utils")
    add_c_cpp_module("integral")
    add_c_cpp_module("json_utils")
    add_c_cpp_module("lcg_random_engine")
    add_c_cpp_module("lifecycle")
    add_c_cpp_module("lin_seq_endpoint")
    add_c_cpp_module("lin_seq_utils")
    add_c_cpp_module("lin_space_mapper")
    add_c_cpp_module("llist")
    add_c_cpp_module("mem_recorder")
    add_c_cpp_module("meta")
    add_c_cpp_module("multi_level_circular_array")
    add_c_cpp_module("multi_level_data_table")
    add_c_cpp_module("multi_level_ptr_table")
    add_c_cpp_module("multi_level_table.mpp")
    add_c_cpp_module("object_state_notation")
    add_c_cpp_module("pair")
    add_c_cpp_module("percent_prime_table")
    add_c_cpp_module("poly_allocator")
    add_c_cpp_module("poly_seq_cntr")
    add_c_cpp_module("poly_seq_endpoint")
    add_c_cpp_module("pool_allocator")
    add_c_cpp_module("ptr_utils")
    add_c_cpp_module("random")
    add_c_cpp_module("rbtree")
    add_c_cpp_module("reduce")
    add_c_cpp_module("seg_utils")
    add_c_cpp_module("seg_vector.mpp")
    add_c_cpp_module("seg_vector")
    add_c_cpp_module("seq_cntr")
    add_c_cpp_module("seq_endpoint")
    add_c_cpp_module("sha256")
    add_c_cpp_module("staging_seg_vector")
    add_c_cpp_module("static_seq")
    add_c_cpp_module("string")
    add_c_cpp_module("tuple")
    add_c_cpp_module("type_identity")
    add_c_cpp_module("type_list")
    add_c_cpp_module("unicode")
    add_c_cpp_module("utf8")
    add_c_cpp_module("utils")
    add_c_cpp_module("value_wrapper")
    add_c_cpp_module("vlq_utils")
