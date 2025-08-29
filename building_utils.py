from __future__ import annotations

import dataclasses
import pathlib
import traceback
import typing

import beartype

from . import utils


@beartype.beartype
def bfs(
    srcs: typing.Iterable[object],
    get_adj: typing.Callable[[object], typing.Iterable[object]],
) -> list[object]:
    ret: list[object] = list()
    visited: set[object] = set()

    i = 0

    for src in srcs:
        if src in visited:
            continue

        ret.append(src)
        visited.add(src)

        while i < len(ret):
            n = ret[i]
            i += 1

            for adj in get_adj(n):
                if adj not in visited:
                    ret.append(adj)
                    visited.add(adj)

    return ret


@beartype.beartype
def tp_sort(
    srcs: typing.Iterable[object],
    get_adj: typing.Callable[[object], typing.Iterable[object]],
) -> list[object]:
    q = bfs(srcs, get_adj)

    indegs = {n: 0 for n in q}

    for n in q:
        for adj in get_adj(n):
            indegs[adj] += 1

    q = [n for n, c in indegs.items() if c == 0]

    ret: list[object] = list()

    while 0 < len(q):
        n = q.pop()
        ret.append(n)

        for adj in get_adj(n):
            indegs[adj] -= 1

            if indegs[adj] == 0:
                q.append(adj)

    if len(indegs) != len(ret):
        cycle_nodes = sorted(map(str, set(indegs.keys()) - set(ret)))

        raise RuntimeError(
            "Nodes in cycle: " + ", ".join(cycle_nodes))

    return ret


@beartype.beartype
@dataclasses.dataclass
class BuildNode:
    path: pathlib.Path
    get_deps: typing.Optional[typing.Callable[[], set[pathlib.Path]]]
    build_unit: typing.Optional[typing.Callable[[], None]]


@beartype.beartype
@dataclasses.dataclass
class BuildResult:
    is_success: bool

    skipped_units: set[pathlib.Path]
    finished_units: set[pathlib.Path]
    failed_units: set[pathlib.Path]
    unready_units: set[pathlib.Path]


@beartype.beartype
class Builder:
    def __init__(self):
        self.syms: dict[str, object] = dict()
        self.build_nodes: dict[pathlib.Path, BuildNode] = dict()

    def add_sym(self, name: str, val: object) -> None:
        assert name not in self.syms
        self.syms[name] = val

    def add_build_node(
        self,
        path: utils.PathLike,
        get_deps: typing.Optional[typing.Callable[[], set[pathlib.Path]]],
        build_unit: typing.Optional[typing.Callable[[], None]],
    ):
        path = utils.to_canon_path(path, solve_symlink=True)

        assert path not in self.build_nodes

        self.build_nodes[path] = BuildNode(
            path=path,
            get_deps=get_deps,
            build_unit=build_unit
        )

    def get_build_nodes(self) -> typing.Iterable[BuildNode]:
        return self.build_nodes.values()

    def get_deps(self, target_unit: utils.PathLike) -> list[pathlib.Path]:
        target_unit = utils.to_canon_path(target_unit, solve_symlink=True)

        if target_unit not in self.build_nodes:
            raise RuntimeError(f"Unknown unit {target_unit}.")

        @beartype.beartype
        def get_adjs(unit: pathlib.Path) -> set[pathlib.Path]:
            if unit not in self.build_nodes:
                raise RuntimeError(f"Unknown unit {unit}.")

            build_node = self.build_nodes[unit]

            return set() if build_node.get_deps is None else build_node.get_deps()

        ret = tp_sort([target_unit], get_adjs)

        ret.reverse()

        return ret

    def build(self, target_unit: utils.PathLike, rebuild: bool) -> BuildResult:
        deps = self.get_deps(target_unit)

        mtimes = {
            unit: unit.stat().st_mtime if unit.exists() else 0.0
            for unit in deps
        }

        skipped_build_units: set[pathlib.Path] = set()
        finished_build_units: set[pathlib.Path] = set()

        ready_build_units: set[pathlib.Path] = set()

        failed_build_units: set[pathlib.Path] = set()
        unready_build_units: set[pathlib.Path] = set()

        for cur_path in deps:
            build_node = self.build_nodes[cur_path]

            cur_deps: set[pathlib.Path] = set() if build_node.get_deps is None \
                else build_node.get_deps()

            if len(cur_deps) == 0:
                skipped_build_units.add(cur_path)
                ready_build_units.add(cur_path)
                continue

            if not cur_deps.issubset(ready_build_units):
                unready_build_units.add(cur_path)
                continue

            max_dep_mtimes = max(mtimes[dep] for dep in cur_deps)

            if not rebuild and max_dep_mtimes <= mtimes[cur_path]:
                skipped_build_units.add(cur_path)
                ready_build_units.add(cur_path)

                continue

            if build_node.build_unit is None:
                mtimes[cur_path] = max_dep_mtimes + 1e-3

                skipped_build_units.add(cur_path)
                ready_build_units.add(cur_path)

                continue

            print(
                f"{utils.Color.yellow('Building')} ({round(len(ready_build_units) / len(deps) * 100):3}%): {cur_path}")

            try:
                build_node.build_unit()
            except Exception as e:
                failed_build_units.add(cur_path)
                print(traceback.format_exc())
                continue

            assert cur_path.exists(), \
                f"Unit {cur_path} does not exist after building."

            mtimes[cur_path] = cur_path.stat().st_mtime

            finished_build_units.add(cur_path)
            ready_build_units.add(cur_path)

        return BuildResult(
            is_success=len(deps) == len(ready_build_units),
            skipped_units=skipped_build_units,
            finished_units=finished_build_units,
            failed_units=failed_build_units,
            unready_units=unready_build_units
        )
