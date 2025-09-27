from __future__ import annotations

import dataclasses
import enum
import functools
import json
import os
import pathlib
import pickle
import typing

import beartype
import termcolor

BYTE = 1
KiBYTE = 1024 * BYTE
MiBYTE = 1024 * KiBYTE
GiBYTE = 1024 * MiBYTE


class ArchEnum(enum.Enum):
    INTEL64 = enum.auto(),
    AMD64 = enum.auto(),
    ARM32 = enum.auto(),
    ARM64 = enum.auto(),
    RISCV32 = enum.auto(),
    RISCV64 = enum.auto(),


class VendorEnum(enum.Enum):
    PC = enum.auto(),


class SysEnum(enum.Enum):
    LINUX = enum.auto(),
    WINDOWS = enum.auto(),


class EnvEnum(enum.Enum):
    GNU = enum.auto(),
    ELF = enum.auto(),
    MSVC = enum.auto(),


@beartype.beartype
@dataclasses.dataclass
class Target:
    arch: ArchEnum
    vendor: VendorEnum
    sys: SysEnum
    env: EnvEnum


class Language(enum.Enum):
    C = enum.auto()
    C_HEADER = enum.auto()
    C_SOURCE = enum.auto()

    CPP = enum.auto()
    CPP_HEADER = enum.auto()
    CPP_SOURCE = enum.auto()

    C_CPP_HEADER = enum.auto()

    @functools.cached_property
    def base(self) -> Language:
        match self:
            case Language.C | Language.C_HEADER | Language.C_SOURCE:
                return Language.C
            case Language.CPP | Language.CPP_HEADER | Language.CPP_SOURCE:
                return Language.CPP
            case _:
                raise NotImplementedError()

    @functools.cached_property
    def header(self) -> Language:
        match self:
            case Language.C | Language.C_HEADER | Language.C_SOURCE:
                return Language.C_HEADER
            case Language.CPP | Language.CPP_HEADER | Language.CPP_SOURCE:
                return Language.CPP_HEADER
            case _:
                raise NotImplementedError()

    @functools.cached_property
    def source(self) -> Language:
        match self:
            case Language.C | Language.C_HEADER | Language.C_SOURCE:
                return Language.C_SOURCE
            case Language.CPP | Language.CPP_HEADER | Language.CPP_SOURCE:
                return Language.CPP_SOURCE
            case _:
                raise NotImplementedError()


PathLike = bytes | str | os.PathLike | pathlib.Path


@beartype.beartype
def pause() -> None:
    input("Press Enter to continue...")


@beartype.beartype
def to_pathlib_path(path: PathLike) -> pathlib.Path:
    return path if isinstance(path, pathlib.Path) else pathlib.Path(path)


@beartype.beartype
def to_canon_path(
    path: PathLike,
    *,
    solve_symlink: bool,
) -> pathlib.Path:
    path = to_pathlib_path(path)

    if solve_symlink:
        return path.resolve()
    else:
        return path.absolute()


@beartype.beartype
def is_subpath(path: PathLike, possible_parent: PathLike) -> bool:
    try:
        to_pathlib_path(path).relative_to(to_pathlib_path(possible_parent))
        return True
    except:
        return False


@beartype.beartype
def create_file(path: PathLike, mode="w", *args, **kwargs) -> typing.IO:
    path = to_pathlib_path(path)

    path.parent.mkdir(parents=True, exist_ok=True)

    return open(path, mode=mode, *args, **kwargs)


@beartype.beartype
def read_json(path: PathLike) -> typing.Any:
    with open(path, "r", encoding="utf-8") as f:
        return json.load(f)


@beartype.beartype
def write_json(path: PathLike, data: typing.Any) -> None:
    with create_file(path, "w", encoding="utf-8") as f:
        json.dump(data, f, ensure_ascii=False, indent=4)
        f.write("\n")

    print(f"Write json to \"{path.as_posix()}\".")


@beartype.beartype
class PickleReader:
    def __init__(
        self,
        path: PathLike,
    ):
        self.f = open(path, mode="rb", buffering=128 * MiBYTE)

    def __del__(self) -> None:
        self.close()

    def __enter__(self) -> PickleReader:
        return self

    def __exit__(self, type, value, traceback) -> None:
        self.close()

    @property
    def is_opened(self) -> bool:
        return not self.f.closed

    @property
    def mode(self) -> str:
        return self.f.mode

    def read(self) -> typing.Any:
        return pickle.load(self.f, encoding="latin1") \
            if self.is_opened else None

    def close(self) -> None:
        self.f.close()


@beartype.beartype
class PickleWriter:
    def __init__(
        self,
        path: PathLike,
        mode: str = "wb+",
    ):
        self.path = to_pathlib_path(path)
        self.f = create_file(self.path, mode=mode, buffering=128 * MiBYTE)
        assert "b" in self.f.mode

    def __del__(self) -> None:
        self.close()

    def __enter__(self) -> PickleWriter:
        return self

    def __exit__(self, type, value, traceback) -> None:
        self.close()

    @property
    def is_opened(self) -> bool:
        return not self.f.closed

    @property
    def mode(self) -> str:
        return self.f.mode

    def write(self, data: typing.Any) -> None:
        assert self.is_opened
        return pickle.dump(data, self.f)

    def close(self) -> None:
        self.f.close()
        print(f"Write pickle to \"{self.path.as_posix()}\".")


@beartype.beartype
def read_pickle(path: PathLike) -> typing.Any:
    with PickleReader(path) as reader:
        return reader.read()


@beartype.beartype
def write_pickle(
    path: PathLike,
    data: object,
    *,
    mode: str = "wb+",
) -> None:
    with PickleWriter(path, mode) as writer:
        writer.write(data)


@beartype.beartype
def to_list_command(
    *cmd: object,
) -> list[str]:
    ret: list[str] = list()

    q: list[object] = list(cmd)

    while 0 < len(q):
        x = q.pop()

        if x is None:
            continue

        if not isinstance(x, str) and isinstance(x, typing.Iterable):
            q.extend(x)
        else:
            ret.append(str(x))

    ret.reverse()

    return ret


@beartype.beartype
def three_way_compare(x: object, y: object) -> int:
    if x < y:
        return -1

    if y < x:
        return 1

    return 0


@beartype.beartype
class CompareWrapper:
    def __init__(
        self,
        obj: object,
        cmp: typing.Callable[[object, object], int],
    ):
        self.obj = obj
        self.cmp = cmp

    def __eq__(self, d: typing.Self) -> bool:
        return self.obj == d.obj

    def __ne__(self, d: typing.Self) -> bool:
        return self.obj != d.obj

    def __lt__(self, d: typing.Self) -> bool:
        assert self.cmp is d.cmp
        return self.cmp(self.obj, d.obj) < 0

    def __gt__(self, d: typing.Self) -> bool:
        assert self.cmp is d.cmp
        return self.cmp(self.obj, d.obj) > 0

    def __le__(self, d: typing.Self) -> bool:
        assert self.cmp is d.cmp
        return self.cmp(self.obj, d.obj) <= 0

    def __ge__(self, d: typing.Self) -> bool:
        assert self.cmp is d.cmp
        return self.cmp(self.obj, d.obj) >= 0


@beartype.beartype
def compare_to_key(cmp: typing.Callable[[object, object], int]):
    return lambda obj: CompareWrapper(obj, cmp)


class Color(enum.StrEnum):
    red = "red"
    orange = "orange"
    yellow = "yellow"
    cyan = "cyan"

    def __call__(self, x: object) -> str:
        return termcolor.colored(str(x), self.value)


@beartype.beartype
def to_color(x: object, color: Color) -> str:
    return termcolor.colored(str(x), Color.cyan.value)


@beartype.beartype
def to_red(x: object) -> str:
    return termcolor.colored(str(x), Color.red.value)


@beartype.beartype
def to_yellow(x: object) -> str:
    termcolor.colored(str(x), Color.yellow.value)


@beartype.beartype
def to_cyan(x: object) -> str:
    return termcolor.colored(str(x), Color.cyan.value)
