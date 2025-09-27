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

from . import utils


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
    utils.Language.CPP_HEADER: "c++-header",
    utils.Language.CPP_SOURCE: "c++",
}


@beartype.beartype
def get_clang_lang(lang: utils.Language):
    return get_clang_lang_table[lang]


@beartype.beartype
def print_cmd(cmd: typing.Iterable[str]):
    cmd = " ".join(cmd)
    print(utils.Color.cyan(f"{cmd=}"))


@beartype.beartype
def get_including_pairs(
    src: utils.PathLike,
    args: list[str],
) -> set[tuple[pathlib.Path, pathlib.Path]]:
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
    src: utils.PathLike,
    args: list[str],
) -> set[pathlib.Path]:
    include_files: set[pathlib.Path] = set()

    for from_file, to_file in get_including_pairs(src, args):
        from_file = utils.to_canon_path(from_file, solve_symlink=True)
        to_file = utils.to_canon_path(to_file, solve_symlink=True)

        if from_file == src:
            include_files.add(to_file)

    return set(sorted(include_files))


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

    opt_type: str
    link_time_opt: bool


@beartype.beartype
class LLVMCompiler:
    def __init__(self, config: LLVMCompilerConfig):
        self.verbose = config.verbose

        self.target = copy.copy(config.target)

        self.base_dir = config.base_dir

        self.executables = {
            "clang": "clang",
            "clang++": "clang++",
            "llvm-link": "llvm-link",
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

        self.error_limit = 8

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

            "--std", self.standard[utils.Language.C],

            *(f"--include-directory={include_dir.as_posix()}"
              for include_dir in self.c_include_dirs),
        ]

        self.compile_args[utils.Language.C] = c_compile_args

        # ----------------------------------------------------------------------

        # Set C++ specific compile arguments

        cpp_compile_args = [
            *compile_args,

            f"-std={self.standard[utils.Language.CPP]}",

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
    ) -> None:
        list_cmd = utils.to_list_command(cmd)
        print_cmd(list_cmd)
        subprocess.run(list_cmd, check=True)

    def get_compile_args(
        self,
        lang: utils.Language,
    ) -> list[str]:
        return [
            *self.compile_args[lang.base],
            "--language", get_clang_lang(lang),
        ]

    def get_including_pairs(
        self,
        src: utils.PathLike,
        lang: utils.Language,
        *,
        cache_file: typing.Optional[pathlib.Path] = None,
    ) -> list[tuple[pathlib.Path, pathlib.Path]]:
        return get_including_pairs(
            src,
            self.get_compile_args(lang),
            cache_file=cache_file,
        )

    def get_including_pairs(
        self,
        src: utils.PathLike,
        lang: utils.Language,
    ) -> set[pathlib.Path]:
        return get_including_pairs(
            src,
            [
                *self.compile_args[lang.base],
                f"-fsyntax-only",
                "--language", get_clang_lang(lang),
            ],
        )

    def get_including_files_(
        self,
        src: utils.PathLike,
        lang: utils.Language,
    ) -> set[pathlib.Path]:
        return get_include_files(
            src,
            [
                *self.compile_args[lang.base],
                f"-fsyntax-only",
                "--language", get_clang_lang(lang),
            ],
        )

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
        llvm_compiler: LLVMCompiler,
        records: dict[str, typing.Any],
    ):
        self.llvm_compiler = llvm_compiler
        self.records = records

    @staticmethod
    def from_json_file(
        llvm_compiler: LLVMCompiler,
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
