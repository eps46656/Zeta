from __future__ import annotations

import argparse
import dataclasses
import os
import sys

import beartype

from . import building_utils, utils
from .zeta import core as zeta_core
from .zeta import core_test as zeta_core_test

FILE = utils.to_resolved_path(__file__)
DIR = FILE.parent

zeta_dev_dir = DIR

zeta_dir = zeta_dev_dir / "zeta"

zeta_core_dir = zeta_dir / "core"
zeta_core_test_dir = zeta_dir / "core_test"

zeta_out_dir = zeta_dev_dir / "build"

match os.name:
    case "nt":
        target = utils.Target(
            arch=utils.ArchEnum.INTEL64,
            vendor=utils.VendorEnum.PC,
            sys=utils.SysEnum.WINDOWS,
            env=utils.EnvEnum.MSVC,
        )
    case "posix":
        target = utils.Target(
            arch=utils.ArchEnum.INTEL64,
            vendor=utils.VendorEnum.PC,
            sys=utils.SysEnum.LINUX,
            env=utils.EnvEnum.ELF,
        )

    case _:
        raise NotImplementedError()

# ------------------------------------------------------------------------------
# ------------------------------------------------------------------------------
# ------------------------------------------------------------------------------

zeta_core_debug_config = zeta_core.Config(
    parent_module_chain=building_utils.ModuleChain((
        building_utils.ModulePair(
            name="zeta",
            config="debug",
        ),
    )),

    name="debug",

    out_dir=zeta_out_dir / "core" / "debug",

    target=target,

    c_standard="c2x",
    cpp_standard="c++23",

    c_include_dirs=[zeta_dev_dir],
    cpp_include_dirs=[zeta_dev_dir],

    debug_enable=True,
    asan_enable=True,

    core_debug_utils_sanity_enable=True,

    opt_type="0",
    link_time_opt=False,
)

zeta_core_test_debug_config = zeta_core_test.Config(
    parent_module_chain=building_utils.ModuleChain((
        building_utils.ModulePair(
            name="zeta",
            config="debug",
        ),
    )),

    name="debug",

    out_dir=zeta_out_dir / "core_test" / "debug",

    target=target,

    c_standard="c2x",
    cpp_standard="c++23",

    c_include_dirs=[zeta_dev_dir],
    cpp_include_dirs=[zeta_dev_dir],

    debug_enable=True,
    asan_enable=True,

    core_debug_utils_sanity_enable=True,

    opt_type="0",
    link_time_opt=False,
)

zeta_core_release_config = zeta_core.Config(
    parent_module_chain=building_utils.ModuleChain((
        building_utils.ModulePair(
            name="zeta",
            config="release",
        ),
    )),

    name="release",

    out_dir=zeta_out_dir / "core" / "release",

    target=target,

    c_standard="c2x",
    cpp_standard="c++23",

    c_include_dirs=[zeta_dev_dir],
    cpp_include_dirs=[zeta_dev_dir],

    debug_enable=False,
    asan_enable=False,

    core_debug_utils_sanity_enable=False,

    opt_type="3",
    link_time_opt=True,
)

zeta_core_test_release_config = zeta_core_test.Config(
    parent_module_chain=building_utils.ModuleChain((
        building_utils.ModulePair(
            name="zeta",
            config="release",
        ),
    )),

    name="release",

    out_dir=zeta_out_dir / "core_test" / "release",

    target=target,

    c_standard="c2x",
    cpp_standard="c++23",

    c_include_dirs=[zeta_dev_dir],
    cpp_include_dirs=[zeta_dev_dir],

    debug_enable=False,
    asan_enable=False,

    core_debug_utils_sanity_enable=False,

    opt_type="3",
    link_time_opt=True,
)

zeta_core_raw_config = zeta_core.Config(
    parent_module_chain=building_utils.ModuleChain((
        building_utils.ModulePair(
            name="zeta",
            config="raw",
        ),
    )),

    name="raw",

    out_dir=zeta_out_dir / "core" / "raw",

    target=target,

    c_standard="c2x",
    cpp_standard="c++23",

    c_include_dirs=[zeta_dev_dir],
    cpp_include_dirs=[zeta_dev_dir],

    debug_enable=True,
    asan_enable=False,

    core_debug_utils_sanity_enable=True,

    opt_type="2",
    link_time_opt=False,
)

zeta_core_test_raw_config = zeta_core_test.Config(
    parent_module_chain=building_utils.ModuleChain((
        building_utils.ModulePair(
            name="zeta",
            config="raw",
        ),
    )),

    name="raw",

    out_dir=zeta_out_dir / "core_test" / "raw",

    target=target,

    c_standard="c2x",
    cpp_standard="c++23",

    c_include_dirs=[zeta_dev_dir],
    cpp_include_dirs=[zeta_dev_dir],

    debug_enable=True,
    asan_enable=False,

    core_debug_utils_sanity_enable=True,

    opt_type="2",
    link_time_opt=False,
)

# ------------------------------------------------------------------------------
# ------------------------------------------------------------------------------
# ------------------------------------------------------------------------------


@beartype.beartype
@dataclasses.dataclass
class ConfigSet:
    zeta_core_config: zeta_core.Config
    zeta_core_test_config: zeta_core_test.Config


configs = {
    "debug": ConfigSet(
        zeta_core_config=zeta_core_debug_config,
        zeta_core_test_config=zeta_core_test_debug_config,
    ),

    "release": ConfigSet(
        zeta_core_config=zeta_core_release_config,
        zeta_core_test_config=zeta_core_test_release_config,
    ),

    "raw": ConfigSet(
        zeta_core_config=zeta_core_raw_config,
        zeta_core_test_config=zeta_core_test_raw_config,
    ),
}


def main():
    parser = argparse.ArgumentParser()

    parser.add_argument("--target",
                        dest="target",
                        action="store",
                        required=True)

    parser.add_argument("--config",
                        dest="config",
                        action="store",
                        choices=list(configs.keys()),
                        required=True)

    parser.add_argument("--run",
                        dest="run",
                        action="store_true")

    parser.add_argument("--rebuild",
                        dest="rebuild",
                        action="store_true")

    args = parser.parse_args(sys.argv[1:])

    print(f"{utils.ANSIColorCode.cyan}args: {args}{utils.ANSIColorCode.reset}")

    config = configs[args.config]

    builder = building_utils.BuildGraph()

    builder.add_act(building_utils.SimpleBuildAction(
        name=FILE,
        module_chain=building_utils.ModuleChain((
            building_utils.ModulePair(
                name="building_script",
                config=None,
            ),
        )),
        get_der_arts=lambda: [FILE],
        get_dep_arts=lambda: tuple(),
        run=lambda: None,
    ))

    zeta_core.add_deps(builder, config.zeta_core_config)
    zeta_core_test.add_deps(builder, config.zeta_core_test_config)

    target_replace_table = {
        "{zeta_core_out_dir}": utils.to_resolved_path(
            config.zeta_core_config.out_dir).as_posix(),
        "{zeta_core_test_out_dir}": utils.to_resolved_path(
            config.zeta_core_test_config.out_dir).as_posix(),
    }

    target = args.target

    for k, l in target_replace_table.items():
        target = target.replace(k, l)

    build_result = builder.build(target, args.rebuild)

    total_build_state_str = f"{utils.ANSIColorCode.green}Success{utils.ANSIColorCode.reset}" \
        if build_result.is_success else f"{utils.ANSIColorCode.red}Failed{utils.ANSIColorCode.reset}"

    print(total_build_state_str)

    for unit_build_state, units in [
        ("Skipped Artifacts", build_result.skipped_arts),
        ("Success Artifacts", build_result.finished_arts),
        (" Failed Artifacts", build_result.failed_arts),
        ("Unready Artifacts", build_result.unready_arts),
    ]:
        print(
            f"{utils.ANSIColorCode.yellow}{unit_build_state}{utils.ANSIColorCode.reset}:")

        for i in sorted(units):
            print(f"\t{i}")

    print(f"{utils.ANSIColorCode.cyan}Target Artifact:{utils.ANSIColorCode.reset} {target}", sep="")

    print(total_build_state_str)

    if not build_result.is_success:
        return

    if args.run:
        print(
            f"{utils.ANSIColorCode.yellow}Running{utils.ANSIColorCode.reset}: {target}")
        os.system(str(target))


if __name__ == "__main__":
    main()
