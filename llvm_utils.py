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
    print(f"{utils.ANSIColorCode.purple}cmd: \"{cmd}{utils.ANSIColorCode.reset}\"")


@beartype.beartype
def parse_ast(
    src: utils.PathLike,
    args: list[str],
) -> clang.cindex.TranslationUnit:
    src = utils.to_resolved_path(src)

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
            utils.to_resolved_path(include.location.file.name),
            utils.to_resolved_path(include.include.name),
        )
        for include in tu.get_includes()
    }


@beartype.beartype
def get_include_files(
    tu: clang.cindex.TranslationUnit
) -> set[pathlib.Path]:
    src = utils.to_resolved_path(tu.spelling)

    print(f"{src=}")

    include_files: set[pathlib.Path] = set()

    for from_file, to_file in get_including_pairs(tu):
        from_file = utils.to_resolved_path(from_file)
        to_file = utils.to_resolved_path(to_file)

        # if from_file == src:
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
class LLVMToolchainConfig:
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
    def __init__(self, config: LLVMToolchainConfig):
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

        self.c_include_dirs = [
            utils.to_resolved_path(c_include_dir)
            for c_include_dir in config.c_include_dirs
        ]

        self.cpp_include_dirs = [
            utils.to_resolved_path(cpp_include_dir)
            for cpp_include_dir in config.cpp_include_dirs
        ]

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
            f"--verbose",
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

            "-fconstexpr-depth=4096",
            "-ftemplate-depth=4096",
        ]

        if self.enable_debug:
            compile_args.extend([
                "-g",

                "-fno-omit-frame-pointer",
                "-fno-optimize-sibling-calls",
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

            *(f"--include-directory={utils.to_resolved_path(include_dir)}"
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

            *(f"--include-directory={utils.to_resolved_path(include_dir)}"
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

        clang_rt_dir = utils.to_resolved_path(self.run_command_(
            "clang", "--print-resource-dir", capture_output=True
        ).stdout.strip())

        print(f"{utils.to_resolved_path(clang_rt_dir).as_posix()=}")

        self.to_exe_args = [
            "--verbose",
            f"--target={self.clang_triple}",
            "-m64",
            *(f"--include-directory={utils.to_resolved_path(include_dir)}"
              for include_dir in self.c_include_dirs),
            # "-lstdc++",
            # "-lm",

            # "-lC:/Program Files/clang+llvm-19.1.4-x86_64-pc-windows-msvc/lib/clang/19/lib/windows/clang_rt.builtins-x86_64",

            f"-lclang_rt.builtins-x86_64",

            # f"-frandomize-layout-seed={self.randomize_layout_seed}",
        ]

        self.to_exe_args.append(f"-O{self.opt_type}")

        if self.enable_debug:
            self.to_exe_args.extend([
                "-g",

                "-fno-omit-frame-pointer",
                "-fno-optimize-sibling-calls",
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

    def compile_to_obj(
        self,
        dst: utils.PathLike,
        src: utils.PathLike,
        lang: utils.Language,
    ) -> None:
        dst = utils.to_resolved_path(dst)
        src = utils.to_resolved_path(src)

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
        dst = utils.to_resolved_path(dst)
        src = utils.to_resolved_path(src)

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
        dst = utils.to_resolved_path(dst)
        srcs = (utils.to_resolved_path(src) for src in srcs)

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
        dst = utils.to_resolved_path(dst)
        srcs = (utils.to_resolved_path(src) for src in srcs)

        self.run_command_(
            self.executables["clang"],
            "--output", dst,
            self.to_exe_args,
            *(src for src in srcs if src is not None),
        )


@beartype.beartype
class ClangFile:
    def __init__(
        self,
        path: utils.PathLike,
        lang: utils.Language,
        toolchain: LLVMToolchain,
        include_files_cache: typing.Optional[utils.PathLike] = None,
    ):
        self.path = utils.to_resolved_path(path)
        self.lang = lang
        self.toolchain = toolchain
        self.include_files_cache = None if include_files_cache is None \
            else utils.to_resolved_path(include_files_cache)

    @functools.cached_property
    def ast(self) -> clang.cindex.TranslationUnit:
        return self.toolchain.parse_ast(self.path, self.lang)

    @functools.cache
    def get_include_files_from_ast_(self) -> set[pathlib.Path]:
        include_files = set(
            utils.to_resolved_path(include_file)
            for include_file in get_include_files(self.ast)
        )

        if self.include_files_cache is not None:
            utils.write_json(self.include_files_cache, [
                include_file.as_posix() for include_file in include_files
            ])

        return include_files

    @functools.cache
    def get_include_files_(self) -> set[pathlib.Path]:
        path_mtime = utils.get_file_mtime(self.path)

        include_files_cache_mtime = utils.get_file_mtime(
            self.include_files_cache)

        if not utils.is_causally_ordered(path_mtime, include_files_cache_mtime):
            return self.get_include_files_from_ast_()

        include_files = {
            utils.to_resolved_path(include_file)
            for include_file in utils.read_json(self.include_files_cache)
        }

        max_include_file_mtime = max((
            utils.get_file_mtime(include_file) for include_file in include_files
        ), default=float("-inf"))

        return include_files if utils.is_causally_ordered(
            max_include_file_mtime, include_files_cache_mtime) \
            else self.get_include_files_from_ast_()

    def get_include_files(self) -> set[pathlib.Path]:
        return set(self.get_include_files_())


@beartype.beartype
class BCBuildAction:
    def __init__(
        self,
        module_chain: building_utils.ModuleChain,
        file_bc: utils.PathLike,
        file_src: utils.PathLike,
        lang: utils.Language,
        base_deps: typing.Iterable[utils.PathLike],
        toolchain: LLVMToolchain,
    ) -> None:
        self.module_chain = module_chain

        self.name = utils.to_resolved_path(file_bc)

        self.file_src = utils.to_resolved_path(file_src)

        self.lang = lang

        assert lang == utils.Language.C_SOURCE or lang == utils.Language.CPP_SOURCE

        self.base_deps = set(base_deps)

        self.toolchain = toolchain

    def get_der_arts(self) -> list[pathlib.Path]:
        return [self.name]

    def get_dep_arts(self) -> list[pathlib.Path]:
        return [*self.base_deps, self.file_src]

    def run(self) -> None:
        print(f"Compiling {self.name} from {self.file_src}...")

        self.toolchain.compile_to_bc(self.name, self.file_src, self.lang)
