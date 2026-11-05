from __future__ import annotations

import dataclasses
import typing

import beartype
import functools


@beartype.beartype
@dataclasses.dataclass(frozen=True)
class ModulePair:
    name: str
    config: typing.Optional[str]

    @functools.cache
    def __str__(self) -> str:
        return self.name if self.config is None \
            else f"{self.name}({self.config})"


@beartype.beartype
@dataclasses.dataclass(frozen=True)
class ModuleChain:
    module_pairs: tuple[ModulePair, ...]

    @functools.cache
    def __str__(self) -> str:
        return "/".join(str(m) for m in self.module_pairs)
