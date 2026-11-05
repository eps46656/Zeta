from __future__ import annotations

import dataclasses
import functools
import pathlib
import traceback
import typing
import os
import time

import beartype

from .. import utils
from .utils import *


@beartype.beartype
def get_virtual_dir_() -> pathlib.Path:
    match os.name:
        case "posix":
            return pathlib.Path("/VIRTUAL")

        case "nt":
            return pathlib.Path("V:VIRTUAL")

        case _:
            raise NotImplementedError(f"Unknown os.name: {os.name}")


virtual_dir = get_virtual_dir_()


@beartype.beartype
def is_virtual_path(path: utils.PathLike) -> bool:
    return utils.to_pathlib_path(path).is_relative_to(virtual_dir)


@beartype.beartype
def bfs(
    srcs: typing.Iterable[typing.Hashable],
    get_adj: typing.Callable[[typing.Hashable], typing.Iterable[typing.Hashable]],
) -> list[typing.Hashable]:
    ret: list[typing.Hashable] = list()
    visited: set[typing.Hashable] = set()

    i = 0

    for src in srcs:
        if src in visited:
            continue

        ret.append(src)
        visited.add(src)

        while i < len(ret):
            n = ret[i]
            i += 1

            for adj in set(get_adj(n)):
                if adj not in visited:
                    ret.append(adj)
                    visited.add(adj)

    return ret


@beartype.beartype
def tp_sort(
    srcs: typing.Iterable[typing.Hashable],
    get_adj: typing.Callable[[typing.Hashable], typing.Iterable[typing.Hashable]],
) -> list[typing.Hashable]:
    q = bfs(srcs, get_adj)

    indegs = {n: 0 for n in q}

    for n in q:
        for adj in set(get_adj(n)):
            indegs[adj] += 1

    q = [n for n, c in indegs.items() if c == 0]

    ret: list[typing.Hashable] = list()

    while 0 < len(q):
        n = q.pop()
        ret.append(n)

        for adj in set(get_adj(n)):
            indegs[adj] -= 1

            if indegs[adj] == 0:
                q.append(adj)

    if len(indegs) != len(ret):
        cycle_nodes = set(indegs.keys()) - set(ret)

        raise RuntimeError(
            "Cycle detected: " + ", ".join(cycle_nodes))

    return ret


@typing.runtime_checkable
class BuildAction(typing.Protocol):
    name: typing.Hashable
    module_chain: ModuleChain

    def get_der_arts(self) -> typing.Iterable[utils.PathLike]:
        ...

    def get_dep_arts(self) -> typing.Iterable[utils.PathLike]:
        ...

    def run(self) -> None:
        ...


@dataclasses.dataclass
class SimpleBuildAction:
    name: utils.PathLike
    module_chain: ModuleChain
    get_der_arts: typing.Callable[[], typing.Iterable[utils.PathLike]]
    get_dep_arts: typing.Callable[[], typing.Iterable[utils.PathLike]]
    run: typing.Callable[[], None]


@beartype.beartype
@dataclasses.dataclass
class BuildResult:
    is_success: bool

    skipped_arts: set[utils.PathLike]
    skipped_acts: set[typing.Hashable]

    finished_arts: set[utils.PathLike]
    finished_acts: set[typing.Hashable]

    failed_arts: set[utils.PathLike]
    failed_acts: set[typing.Hashable]

    unready_arts: set[utils.PathLike]
    unready_acts: set[typing.Hashable]


@beartype.beartype
class BuildGraph:
    def __init__(self):
        self.syms: dict[typing.Hashable, object] = dict()
        self.acts: dict[typing.Hashable, BuildAction] = dict()
        self.der_art_to_act: dict[utils.PathLike, BuildAction] = dict()

    def add_sym(self, name: typing.Hashable, value: object) -> None:
        self.syms[name] = value

    def get_sym(self, name: typing.Hashable) -> object:
        return self.syms[name]

    def remove_sym(
        self,
        name: typing.Hashable,
        default_object: object,
    ) -> object:
        return self.syms.pop(name, default_object)

    def add_act(self, act: BuildAction) -> None:
        if act.name in self.acts:
            return

        der_arts = act.get_der_arts()

        for der_art in der_arts:
            assert der_art not in self.der_art_to_act

        self.acts[act.name] = act

        for der_art in der_arts:
            self.der_art_to_act[der_art] = act

    def get_act_run_list(
        self,
        target_art: utils.PathLike,
    ) -> list[typing.Hashable]:
        @beartype.beartype
        @functools.cache
        def get_adjs(
            act_name: typing.Hashable
        ) -> tuple[typing.Hashable, ...]:
            try:
                return tuple(
                    self.der_art_to_act[utils.to_resolved_path(dep_art)].name
                    for dep_art in self.acts[act_name].get_dep_arts()
                )
            except KeyError:
                raise RuntimeError(f"Unknown artifact {act_name}.")

        ret = tp_sort(
            [self.der_art_to_act[utils.to_resolved_path(target_art)].name],
            get_adjs,
        )

        ret.reverse()

        return ret

    def build(
        self,
        target_art: utils.PathLike,
        rebuild: bool,
    ) -> BuildResult:
        target_art = utils.to_resolved_path(target_art)

        virt_art_time: dict[pathlib.Path, float] = dict()

        @beartype.beartype
        def get_art_time(art: pathlib.Path) -> float:
            return virt_art_time.get(art, float("-inf")) \
                if is_virtual_path(art) else utils.get_file_mtime(art)

        @beartype.beartype
        def percent_format(percent: float) -> str:
            return f"{round(percent * 100):3}%"

        act_run_list = self.get_act_run_list(target_art)

        ret = BuildResult(
            is_success=False,

            skipped_arts=set(),
            skipped_acts=set(),

            finished_arts=set(),
            finished_acts=set(),

            failed_arts=set(),
            failed_acts=set(),

            unready_arts=set(),
            unready_acts=set(),
        )

        ready_arts: set[pathlib.Path] = set()

        for act_i in range(len(act_run_list)):
            act_name = act_run_list[act_i]
            act = self.acts[act_name]

            cur_dep_arts = set(utils.to_resolved_path(cur_dep_art)
                               for cur_dep_art in act.get_dep_arts())

            cur_der_arts = set(utils.to_resolved_path(cur_der_art)
                               for cur_der_art in act.get_der_arts())

            progress_percent_before = act_i / len(act_run_list)
            progress_percent_after = (act_i + 1) / len(act_run_list)

            if not cur_dep_arts.issubset(ready_arts):
                print(
                    f"({percent_format(progress_percent_after)}) \033[38;5;208mSkip Unready\033[0m: {act.name}")

                ret.unready_arts.update(cur_der_arts)
                ret.unready_acts.add(act_name)

                continue

            max_dep_art_mtime = max(
                (get_art_time(cur_dep_art) for cur_dep_art in cur_dep_arts),
                default=float("-inf"),
            )

            min_der_art_mtime = min(
                (get_art_time(cur_der_art)for cur_der_art in cur_der_arts),
                default=float("inf"),
            )

            """

            -inf    -inf
                no dep art and virt der arts -> run
            -inf    normal
                no dep art and at least one real der art -> skip
            -inf    inf
                no dep art and no der art -> skip
            normal  -inf
                at least one real dep art and virt der arts -> run
            normal  normal
                real dep arts and real der arts -> depends on time
            normal  inf
                real dep arts and no der art -> skip

            """

            if not rebuild and utils.is_causally_ordered(
                    max_dep_art_mtime, min_der_art_mtime):
                print(
                    f"({percent_format(progress_percent_after)}) \033[38;5;43mSkip Existed\033[0m: {act.name}")

                ret.skipped_arts.update(cur_der_arts)
                ret.skipped_acts.add(act_name)

                ready_arts.update(cur_der_arts)

                continue

            print(
                f"({percent_format(progress_percent_before)}) {utils.ANSIColorCode.cyan}Running     \033[0m: {act.name}")

            try:
                act.run()

            except Exception:
                ret.failed_arts.update(cur_der_arts)
                ret.failed_acts.add(act_name)

                print(traceback.format_exc())
                print(
                    f"({percent_format(progress_percent_after)}) {utils.ANSIColorCode.red}Failed      \033[0m: {act.name}")

                continue

            print(
                f"({percent_format(progress_percent_after)}) {utils.ANSIColorCode.green}Success     \033[0m: {act.name}")

            time.sleep(utils.causal_order_eps * 5)

            cur_time = time.time()

            for cur_der_art in cur_der_arts:
                if is_virtual_path(cur_der_art):
                    virt_art_time[cur_der_art] = cur_time

            ret.finished_arts.update(cur_der_arts)
            ret.finished_acts.add(act_name)

            ready_arts.update(cur_der_arts)

        ret.is_success = target_art in ready_arts

        return ret
