from __future__ import annotations

import argparse
import dataclasses
import os
import sys

import beartype

from . import building_utils, utils
from .zeta import core as zeta_core
from .zeta import core_test as zeta_core_test

FILE = utils.to_canon_path(__file__, solve_symlink=False)
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
    name="debug",

    verbose=True,

    out_dir=zeta_out_dir / "core" / "debug",

    target=target,

    c_standard="c2x",
    cpp_standard="c++20",

    c_include_dirs=[zeta_dev_dir],
    cpp_include_dirs=[zeta_dev_dir],

    enable_debug=True,
    enable_asan=True,

    opt_type="2",
    link_time_opt=False,
)

zeta_core_test_debug_config = zeta_core_test.Config(
    name="debug",

    verbose=True,

    out_dir=zeta_out_dir / "core_test" / "debug",

    target=target,

    c_standard="c2x",
    cpp_standard="c++20",

    c_include_dirs=[zeta_dev_dir],
    cpp_include_dirs=[zeta_dev_dir],

    enable_debug=True,
    enable_asan=True,

    opt_type="2",
    link_time_opt=False,
)

zeta_core_release_config = zeta_core.Config(
    name="release",

    verbose=True,

    out_dir=zeta_out_dir / "core" / "release",

    target=target,

    c_standard="c2x",
    cpp_standard="c++20",

    c_include_dirs=[zeta_dev_dir],
    cpp_include_dirs=[zeta_dev_dir],

    enable_debug=False,
    enable_asan=False,

    opt_type="3",
    link_time_opt=True,
)

zeta_core_test_release_config = zeta_core_test.Config(
    name="release",

    verbose=True,

    out_dir=zeta_out_dir / "core_test" / "release",

    target=target,

    c_standard="c2x",
    cpp_standard="c++20",

    c_include_dirs=[zeta_dev_dir],
    cpp_include_dirs=[zeta_dev_dir],

    enable_debug=False,
    enable_asan=False,

    opt_type="3",
    link_time_opt=True,
)

zeta_core_raw_config = zeta_core.Config(
    name="raw",

    verbose=True,

    out_dir=zeta_out_dir / "core" / "raw",

    target=target,

    c_standard="c2x",
    cpp_standard="c++20",

    c_include_dirs=[zeta_dev_dir],
    cpp_include_dirs=[zeta_dev_dir],

    enable_debug=True,
    enable_asan=False,

    opt_type="2",
    link_time_opt=False,
)

zeta_core_test_raw_config = zeta_core_test.Config(
    name="raw",

    verbose=True,

    out_dir=zeta_out_dir / "core_test" / "raw",

    target=target,

    c_standard="c2x",
    cpp_standard="c++20",

    c_include_dirs=[zeta_dev_dir],
    cpp_include_dirs=[zeta_dev_dir],

    enable_debug=True,
    enable_asan=False,

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

    print(f"{sys.argv[1:]}")

    args = parser.parse_args(sys.argv[1:])

    print(f"args: {args}")

    config = configs[args.config]

    builder = building_utils.Builder()

    zeta_core.add_deps(builder, config.zeta_core_config)
    zeta_core_test.add_deps(builder, config.zeta_core_test_config)

    builder.add_build_node(FILE, None, None)

    target_replace_table = {
        "{zeta_core_out_dir}": str(config.zeta_core_config.out_dir),
        "{zeta_core_test_out_dir}": str(config.zeta_core_test_config.out_dir),
    }

    target = args.target

    for k, l in target_replace_table.items():
        target = target.replace(k, l)

    target = utils.to_pathlib_path(target)

    build_result = builder.build(target, args.rebuild)

    total_build_state_str = "success" if build_result.is_success else "failed"

    print(utils.Color.yellow(total_build_state_str))

    for unit_build_state, units in [
        ("skipped_units", build_result.skipped_units),
        ("finished_units", build_result.finished_units),
        ("failed_units", build_result.failed_units),
        ("unready_units", build_result.unready_units),
    ]:
        print(f"{utils.Color.yellow(f'{unit_build_state} build units')}:")

        for i in sorted(units):
            print(f"\t{i}")

    print(f"{utils.Color.yellow('target:')} {target}", sep="")

    print(utils.Color.yellow(total_build_state_str))

    if not build_result.is_success:
        return

    if args.run:
        print(f"{utils.Color.yellow(f'Running:')} {target}")
        os.system(str(target))


if __name__ == "__main__":
    main()
