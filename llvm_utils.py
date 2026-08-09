from __future__ import annotations

import copy
import dataclasses
import functools
import pathlib
import subprocess
import traceback
import typing

import beartype
import clang.cindex

from . import building_utils, utils


@beartype.beartype
@functools.cache
def clang_index() -> clang.cindex.Index:
    clang.cindex.Config.set_library_file("libclang-13.dll")
    return clang.cindex.Index.create()


clang_triple_arch_table = {
    utils.ArchEnum.INTEL64: "x86_64",
    utils.ArchEnum.AMD64: "x86_64",
    utils.ArchEnum.ARM32: "arm32",
    utils.ArchEnum.ARM64: "arm64",
    utils.ArchEnum.RISCV32: "riscv32",
    utils.ArchEnum.RISCV64: "riscv64",
}

clang_triple_vendor_table = {
    utils.VendorEnum.PC: "pc",
}

clang_triple_sys_table = {
    utils.SysEnum.LINUX: "linux",
    utils.SysEnum.WINDOWS: "windows",
}

clang_triple_env_table = {
    utils.EnvEnum.GNU: "gun",
    utils.EnvEnum.ELF: "elf",
    utils.EnvEnum.MSVC: "msvc",
}


@beartype.beartype
def get_clang_triple(target: utils.Target):
    arch = clang_triple_arch_table[target.arch]
    vendor = clang_triple_vendor_table[target.vendor]
    sys = clang_triple_sys_table[target.sys]
    env = clang_triple_env_table[target.env]

    return f"{arch}-{vendor}-{sys}-{env}"


llc_arch_table = {
    utils.ArchEnum.INTEL64: "x86-64",
    utils.ArchEnum.AMD64: "x86-64",
    utils.ArchEnum.ARM32: "arm32",
    utils.ArchEnum.ARM64: "arm64",
    utils.ArchEnum.RISCV32: "riscv32",
    utils.ArchEnum.RISCV64: "riscv64",
}


@beartype.beartype
def get_llc_arch(target: utils.Target):
    arch = llc_arch_table[target.arch]
    return arch


get_clang_lang_table = {
    utils.Language.C_HEADER: "c-header",
    utils.Language.C_SOURCE: "c",
    utils.Language.MACRO_C_HEADER: "c-header",
    utils.Language.MACRO_C_SOURCE: "c",
    utils.Language.CPP_HEADER: "c++-header",
    utils.Language.CPP_SOURCE: "c++",
    utils.Language.MACRO_CPP_HEADER: "c++-header",
    utils.Language.MACRO_CPP_SOURCE: "c++",
}


@beartype.beartype
def get_clang_lang(lang: utils.Language):
    return get_clang_lang_table[lang]


@beartype.beartype
def print_cmd(cmd: typing.Iterable[str]):
    cmd = " ".join(cmd)
    print(utils.Color.cyan(f"{cmd=}"))


@beartype.beartype
def parse_ast(
    src: utils.PathLike,
    args: list[str],
) -> clang.cindex.TranslationUnit:
    src = utils.to_canon_path(src, solve_symlink=True)

    assert src.is_file(), src.as_posix()

    try:
        tu = clang_index().parse(
            src,
            args=args,
            options=clang.cindex.TranslationUnit.PARSE_DETAILED_PROCESSING_RECORD,
        )
    except Exception as e:
        print(traceback.format_exc())
        print(e)
        print(f"{args=}")
        print(" ".join(args))
        print(f"{src=}")
        raise e

    return tu


@beartype.beartype
def get_including_pairs(
    tu: clang.cindex.TranslationUnit
) -> set[tuple[pathlib.Path, pathlib.Path]]:
    return {
        (
            utils.to_canon_path(
                include.location.file.name, solve_symlink=True),
            utils.to_canon_path(
                include.include.name, solve_symlink=True),
        )
        for include in tu.get_includes()
    }


@beartype.beartype
def get_include_files(
    tu: clang.cindex.TranslationUnit
) -> set[pathlib.Path]:
    src = utils.to_canon_path(tu.spelling, solve_symlink=True)

    print(f"{src=}")

    include_files: set[pathlib.Path] = set()

    for from_file, to_file in get_including_pairs(tu):
        from_file = utils.to_canon_path(from_file, solve_symlink=True)
        to_file = utils.to_canon_path(to_file, solve_symlink=True)

        if from_file == src:
            include_files.add(to_file)

    return set(sorted(include_files))


@beartype.beartype
def find_unqualified_calls(
    tu: clang.cindex.TranslationUnit,
) -> list[clang.cindex.Cursor]:
    ret: list[clang.cindex.Cursor] = list()

    q: list[clang.cindex.Cursor] = [tu.cursor]

    while 0 < len(q):
        n = q.pop()

        children = list(n.get_children())
        q.extend(children)

        if n.kind != clang.cindex.CursorKind.CALL_EXPR or len(children) == 0:
            continue

        callee = children[0]

        if callee.kind == clang.cindex.CursorKind.DECL_REF_EXPR:
            ret.append(n)
            continue

        if callee.kind == clang.cindex.CursorKind.UNEXPOSED_EXPR:
            if any(ch.kind in (clang.cindex.CursorKind.FUNCTION_TEMPLATE,
                               clang.cindex.CursorKind.DECL_REF_EXPR)
                   for ch in callee.get_children()):
                ret.append(n)
                continue

    return ret


@beartype.beartype
@dataclasses.dataclass
class LLVMCompilerConfig:
    verbose: bool

    target: utils.Target

    base_dir: typing.Optional[utils.PathLike]

    c_standard: str
    cpp_standard: str

    c_include_dirs: typing.Iterable[utils.PathLike]
    cpp_include_dirs: typing.Iterable[utils.PathLike]

    enable_debug: bool
    enable_asan: bool

    c_defines: dict[str, str]
    cpp_defines: dict[str, str]

    opt_type: str
    link_time_opt: bool


@beartype.beartype
class LLVMToolchain:
    def __init__(self, config: LLVMCompilerConfig):
        self.verbose = config.verbose

        self.target = copy.copy(config.target)

        self.base_dir = config.base_dir

        self.executables = {
            "clang": "clang",
            "clang++": "clang++",
            "llvm-link": "llvm-link",
            "include-what-you-use": "include-what-you-use",
        }

        self.llvm_link_executable = "llvm-link"

        self.asm_to_obj_command = "llvm-as"
        self.link_lls_command = "llvm-link"
        self.opt_ll_command = "opt"
        self.to_exe_command = "clang"

        self.standard = {
            utils.Language.C: config.c_standard,
            utils.Language.CPP: config.cpp_standard,
        }

        self.c_include_dirs = list(map(
            utils.to_pathlib_path, config.c_include_dirs))

        self.cpp_include_dirs = list(map(
            utils.to_pathlib_path, config.cpp_include_dirs))

        self.enable_debug = config.enable_debug
        self.enable_asan = config.enable_asan

        self.opt_type = config.opt_type
        self.link_time_opt = config.link_time_opt

        self.randomize_layout_seed = 13493037705

        self.clang_triple = get_clang_triple(self.target)

        self.error_limit = 24

        # ----------------------------------------------------------------------

        self.compile_args: dict[utils.Language, list[str]] = dict()

        # ----------------------------------------------------------------------

        # Set common compile arguments

        compile_args = [
            f"-v" if self.verbose else "",
            f"--target={self.clang_triple}",
            f"-m64",

            f"-ferror-limit={self.error_limit}",
            # f"-frandomize-layout-seed={self.randomize_layout_seed}",

            f"" if self.base_dir is None else
            f"-ffile-prefix-map={self.base_dir}=.",

            "-Wall",
            "-Wextra",

            "-Wimplicit-fallthrough",
            "-Wmissing-prototypes",

            "-Werror",
            f"-O{self.opt_type}",
        ]

        if self.enable_debug:
            compile_args.extend([
                "-g",
                "-fno-omit-frame-pointer",
                "-fno-optimize-sibling-calls",

                "-DZETA_Core_EnableDebug=1",
            ])

        if self.enable_asan:
            compile_args.extend([
                # "-fprofile-instr-generate",
                # "-fcoverage-mapping",

                "-fsanitize=address",
                # "-fsanitize=undefined",
                # "-fsanitize=memory",
                # "-fsanitize-memory-track-origins",
            ])

        # ----------------------------------------------------------------------

        # Set C specific compile arguments

        c_compile_args = [
            *compile_args,

            *(
                f"-D{key}={value}"
                for key, value in config.c_defines.items()
            ),

            f"--std={self.standard[utils.Language.C]}",

            *(f"--include-directory={include_dir.as_posix()}"
              for include_dir in self.c_include_dirs),
        ]

        self.compile_args[utils.Language.C] = c_compile_args

        # ----------------------------------------------------------------------

        # Set C++ specific compile arguments

        cpp_compile_args = [
            *compile_args,

            *(
                f"-D{key}={value}"
                for key, value in config.cpp_defines.items()
            ),

            f"--std={self.standard[utils.Language.CPP]}",

            *(f"--include-directory={include_dir}"
              for include_dir in self.cpp_include_dirs),

            "-Wold-style-cast",
            "-Wconversion",
            "-Wsign-conversion",

            # "-fno-exceptions",
            # "-fno-rtti",
        ]

        self.compile_args[utils.Language.CPP] = cpp_compile_args

        # ----------------------------------------------------------------------

        self.ll_to_asm_args = [
            f"-march={get_llc_arch(self.target)}",
        ]

        self.ll_to_obj_args = [
            f"-march={get_llc_arch(self.target)}",
        ]

        # ----------------------------------------------------------------------

        self.to_exe_args = [
            "--verbose" if self.verbose else "",
            f"--target={self.clang_triple}",
            "-m64",
            *(f"--include-directory={include_dir}"
              for include_dir in self.c_include_dirs),
            # "-lstdc++",
            # "-lm",

            # "-lC:/Program Files/clang+llvm-19.1.4-x86_64-pc-windows-msvc/lib/clang/19/lib/windows/clang_rt.builtins-x86_64",

            "-lC:/Users/yu46656/anaconda3/envs/ZetaDevelop_2025_0806/Library/lib/clang/20/lib/windows/clang_rt.builtins-x86_64.lib",

            # "-lclang_rt.builtins-x86_64",

            # f"-frandomize-layout-seed={self.randomize_layout_seed}",
        ]

        self.to_exe_args.append(f"-O{self.opt_type}")

        if self.enable_debug:
            self.to_exe_args.extend([
                "-g",

                "-fno-omit-frame-pointer",
                "-fno-optimize-sibling-calls",

                "-DZETA_Core_EnableDebug=1",
            ])

        if self.enable_asan:
            self.to_exe_args.extend([
                # "-fprofile-instr-generate",
                # "-fcoverage-mapping",

                "-fsanitize=address",
                # "-fsanitize=undefined",
                # "-fsanitize=memory",
                # "-fsanitize-memory-track-origins",
            ])

        if self.link_time_opt:
            self.to_exe_args.append("-flto")

    def run_command_(
        self,
        *cmd: object,
        check: bool = True,
        capture_output: bool = False,
    ):
        list_cmd = utils.to_list_command(cmd)
        print_cmd(list_cmd)
        return subprocess.run(
            list_cmd,
            check=check,
            capture_output=capture_output,
            text=True,
        )

    def get_compile_args(
        self,
        lang: utils.Language,
    ) -> list[str]:
        return [
            *self.compile_args[lang.base],
            "--language", get_clang_lang(lang),
        ]

    def parse_ast(
        self,
        src: utils.PathLike,
        lang: utils.Language,
    ) -> clang.cindex.TranslationUnit:
        return parse_ast(
            src,
            [
                *self.compile_args[lang.base],
                f"-fsyntax-only",
                "--language", get_clang_lang(lang),
            ],
        )

    def run_iwyu(
        self,
        src: utils.PathLike,
        lang: utils.Language,
    ) -> None:
        result = self.run_command_(
            self.executables["include-what-you-use"],
            "-Xiwyu",
            "--no_fwd_decls",

            "--compile",
            *self.compile_args[lang.base],
            "--language", get_clang_lang(lang),
            src,

            check=False,
            capture_output=True,
        )

        print(result.stdout)
        print(result.stderr)

        assert "has correct" in result.stderr

    def get_including_pairs(
        self,
        src: utils.PathLike,
        lang: utils.Language,
    ) -> set[pathlib.Path]:
        return get_including_pairs(parse_ast(
            src,
            [
                *self.compile_args[lang.base],
                f"-fsyntax-only",
                "--language", get_clang_lang(lang),
            ],
        ))

    def get_including_files_(
        self,
        src: utils.PathLike,
        lang: utils.Language,
    ) -> set[pathlib.Path]:
        return get_include_files(parse_ast(
            src,
            [
                *self.compile_args[lang.base],
                f"-fsyntax-only",
                "--language", get_clang_lang(lang),
            ],
        ))

    def get_including_files(
        self,
        src: utils.PathLike,
        lang: utils.Language,
        cache_file: typing.Optional[utils.PathLike] = None,
    ) -> set[pathlib.Path]:
        if cache_file is None:
            use_cache = False
        else:
            cache_file = utils.to_pathlib_path(cache_file)

            use_cache = cache_file.is_file() and \
                1e-3 <= cache_file.stat().st_mtime - src.stat().st_mtime

        if use_cache:
            return {
                utils.to_canon_path(val, solve_symlink=True)
                for val in utils.read_json(cache_file)
            }

        if lang == utils.Language.C_CPP_HEADER:
            include_files = set(sorted(set.union(
                self.get_including_files_(src, utils.Language.C_HEADER),
                self.get_including_files_(src, utils.Language.CPP_HEADER),
            )))
        else:
            include_files = self.get_including_files_(src, lang)

        if cache_file is not None:
            utils.write_json(
                cache_file, [val.as_posix() for val in include_files])

        return include_files

    def compile_to_obj(
        self,
        dst: utils.PathLike,
        src: utils.PathLike,
        lang: utils.Language,
    ) -> None:
        dst = utils.to_pathlib_path(dst)
        src = utils.to_pathlib_path(src)

        dst.parent.mkdir(parents=True, exist_ok=True)

        self.run_command_(
            self.executables["clang"],
            "--compile",
            "--output", dst,
            *self.compile_args[lang.base],
            "--language", get_clang_lang(lang),
            src,
        )

    def compile_to_bc(
        self,
        dst: utils.PathLike,
        src: utils.PathLike,
        lang: utils.Language,
    ) -> None:
        dst = utils.to_pathlib_path(dst)
        src = utils.to_pathlib_path(src)

        dst.parent.mkdir(parents=True, exist_ok=True)

        self.run_command_(
            self.executables["clang"],
            "-emit-llvm",
            "--compile",
            "--output", dst,
            *self.compile_args[lang.base],
            "--language", get_clang_lang(lang),
            src,
        )

    def link_irs(
        self,
        dst: utils.PathLike,
        srcs: typing.Iterable[utils.PathLike],
    ) -> None:
        dst = utils.to_pathlib_path(dst)
        srcs = map(utils.to_pathlib_path, srcs)

        dst.parent.mkdir(parents=True, exist_ok=True)

        self.run_command_(
            self.executables["llvm-link"],
            "--output", dst,
            *(src for src in srcs if src is not None),
        )

    def compile_to_exe(
        self,
        dst: utils.PathLike,
        srcs: typing.Iterable[utils.PathLike]
    ) -> None:
        dst = utils.to_pathlib_path(dst)
        srcs = map(utils.to_pathlib_path, srcs)

        self.run_command_(
            self.executables["clang"],
            "--output", dst,
            self.to_exe_args,
            *(src for src in srcs if src is not None),
        )


@beartype.beartype
class DirectIncludeFileRepo:
    def __init__(
        self,
        llvm_compiler: LLVMToolchain,
        records: dict[str, typing.Any],
    ):
        self.llvm_compiler = llvm_compiler
        self.records = records

    @staticmethod
    def from_json_file(
        llvm_compiler: LLVMToolchain,
        path: utils.PathLike,
    ) -> DirectIncludeFileRepo:
        path = utils.to_pathlib_path(path)
        records = utils.read_json(path) if path.is_file() else dict()
        return DirectIncludeFileRepo(llvm_compiler, records)

    def save_to_json_file(self, path: utils.PathLike) -> None:
        utils.write_json(path, self.records)

    def get_include_files(
        self,
        path: utils.PathLike,
        lang: utils.Language,
    ) -> list[pathlib.Path]:
        path = utils.to_canon_path(path, solve_symlink=True)
        path_str = path.as_posix()

        assert path.exists(), path

        cur_mtime = path.stat().st_mtime

        if path_str not in self.records or 1e-3 <= cur_mtime - float(self.records[path_str]["mtime"]):
            direct_include_files = self.llvm_compiler.get_include_files(
                path, lang).direct_include_files

            self.records[path_str] = {
                "mtime": f"{cur_mtime:.12f}",
                "include_files": [
                    utils.to_canon_path(
                        direct_include_file, solve_symlink=True).as_posix()
                    for direct_include_file in direct_include_files
                ],
            }

        return [
            utils.to_pathlib_path(_)
            for _ in self.records[path_str]["include_files"]
        ]


@beartype.beartype
@dataclasses.dataclass
class ModuleContext:
    builder: building_utils.Builder
    llvm_toolchain: LLVMToolchain
    base_dir: pathlib.Path
    build_dir: pathlib.Path
    config_name: str
    commom_deps: typing.Sequence[object]

    build_nodes: dict[typing.Hashable, building_utils.BuildNode]


@beartype.beartype
class CCPPFileBuildNode(building_utils.BuildNode):
    def __init__(
        self,
        module_context: ModuleContext,
        file: utils.PathLike,
        langs: utils.Language | typing.Iterable[utils.Language],
        additional_deps: set[object],
    ) -> None:
        self.module_context = module_context

        self.file = utils.to_canon_path(file, solve_symlink=True)

        self.langs: tuple[utils.Language] = \
            (langs,) if isinstance(langs, utils.Language) \
            else tuple(sorted(set(langs)))

        self.additional_deps: set[object] = set(additional_deps)

        self.cached_parsed_tu: typing.Optional[clang.cindex.TranslationUnit] = None

    @functools.cached_property
    def get_identity(self) -> tuple[str, str]:
        return (self.file.as_posix(), self.module_context.config_name)

    @functools.cached_property
    def get_time_source(self) -> pathlib.Path:
        return self.file

    @functools.cached_property
    def get_deps(self) -> set[pathlib.Path]:
        cache_file = self.module_context.build_dir / \
            self.module_context.config_name / \
            f"{self.file.name}.file_including_files.json"

        if cache_file.is_file() and \
                1e-3 <= cache_file.stat().st_mtime - self.file.stat().st_mtime:
            include_files = [
                utils.to_canon_path(val, solve_symlink=True)
                for val in utils.read_json(cache_file)
            ]
        else:
            include_files: set[pathlib.Path] = set()

            for lang in self.langs:
                for include_file in get_include_files(
                        self.module_context.llvm_toolchain.parse_ast(
                            self.file, lang)):
                    if include_file.is_relative_to(self.module_context.base_dir):
                        include_files.add(include_file)

            include_files = sorted(include_files)

            utils.write_json(cache_file, [
                val.as_posix() for val in include_files
            ])

        return {
            *self.module_context.commom_deps,

            *(
                (val.as_posix(), self.module_context.config_name)
                for val in include_files
            ),

            *self.additional_deps,
        }

    def build(self) -> None:
        print(f"Checking {self.file}...")


T = typing.TypeVar("T", bound=building_utils.BuildNode)


@beartype.beartype
def try_add_build_node_into_module(
    module_context: ModuleContext,
    build_node: T,
) -> tuple[T, bool]:
    if build_node.get_identity() in module_context.build_nodes:
        old_build_node = module_context.build_nodes[build_node.get_identity()]
        assert build_node == old_build_node
        return (old_build_node, False)

    module_context.build_nodes[build_node.get_identity()] = build_node
    module_context.builder.add_build_node(build_node)
    return (build_node, True)


@beartype.beartype
def add_c_cpp_file_build_node(
    module_context: ModuleContext,
    c_cpp_file: utils.PathLike,
    langs: utils.Language | typing.Iterable[utils.Language],
    additional_deps: typing.Sequence[typing.Hashable],
) -> tuple[CCPPFileBuildNode, bool]:
    c_cpp_file = utils.to_canon_path(c_cpp_file, solve_symlink=True)

    build_node = CCPPFileBuildNode(
        module_context,
        c_cpp_file,
        langs,
        additional_deps,
    )

    return typing.cast(
        tuple[CCPPFileBuildNode, bool],
        try_add_build_node_into_module(module_context, build_node),
    )


@beartype.beartype
def add_c_cpp_file_to_bc_file_build_node(
    module_context: ModuleContext,
    bc_file: utils.PathLike,
    c_cpp_file: utils.PathLike,
    lang: utils.Language,
    additional_deps: typing.Sequence[typing.Hashable],
) -> tuple[building_utils.BuildNode, bool]:
    bc_file = utils.to_canon_path(bc_file, solve_symlink=True)

    assert lang.base != lang
    assert lang.enmacro != lang

    c_cpp_file_build_node = add_c_cpp_file_build_node(
        module_context, c_cpp_file, lang)[0]

    identity = (bc_file.as_posix(), module_context.config_name)

    time_source = bc_file

    deps = {
        *module_context.commom_deps,
        c_cpp_file_build_node.get_identity(),
        *additional_deps,
    }

    build_node = building_utils.BasicBuildNode(
        get_identity=lambda: identity,
        get_time_source=lambda: time_source,
        get_deps=lambda: deps,
        build=lambda: module_context.llvm_toolchain.compile_to_bc(
            bc_file, c_cpp_file, lang),
    )

    return typing.cast(
        tuple[building_utils.BuildNode, bool],
        try_add_build_node_into_module(module_context, build_node),
    )


@beartype.beartype
def add_c_cpp_module(
    module_context: ModuleContext,
    dir: utils.PathLike,
    module_name: str,
    build_dir: utils.PathLike,
):
    dir = utils.to_canon_path(dir, solve_symlink=True)
    build_dir = utils.to_canon_path(build_dir, solve_symlink=True)

    h_file = dir / f"{module_name}.h"
    hpp_file = dir / f"{module_name}.hpp"
    ipp_file = dir / f"{module_name}.ipp"
    c_file = dir / f"{module_name}.c"
    cpp_file = dir / f"{module_name}.cpp"
    bc_file = build_dir / module_context.config_name / f"{module_name}.bc"

    assert not c_file.exists() or not cpp_file.exists()

    is_macro = module_name.endswith(".mpp")

    if h_file.exists():
        add_c_cpp_file_build_node(
            h_file,
            utils.Language.MACRO_C_HEADER
            if is_macro else utils.Language.C_HEADER
        )

    if hpp_file.exists():
        add_c_cpp_file_build_node(
            hpp_file,
            utils.Language.MACRO_CPP_HEADER
            if is_macro else utils.Language.CPP_HEADER
        )

    if ipp_file.exists():
        add_c_cpp_file_build_node(
            ipp_file,
            utils.Language.MACRO_CPP_HEADER
            if is_macro else utils.Language.CPP_HEADER
        )

    if c_file.exists():
        add_c_cpp_file_build_node(
            c_file,
            utils.Language.MACRO_C_SOURCE
            if is_macro else utils.Language.C_SOURCE
        )

        if not is_macro:
            add_c_cpp_file_to_bc_file_build_node(
                bc_file, c_file, utils.Language.C_SOURCE)

    if cpp_file.exists():
        add_c_cpp_file_build_node(
            cpp_file,
            utils.Language.MACRO_CPP_SOURCE
            if is_macro else utils.Language.CPP_SOURCE
        )

        if not is_macro:
            add_c_cpp_to_bc(bc_file, cpp_file, utils.Language.CPP_SOURCE)
