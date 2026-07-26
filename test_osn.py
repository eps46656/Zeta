import io
import pathlib
import random
import string
import os

import beartype

from . import object_state_notation, utils


@beartype.beartype
def gen_random_str(length: None | int = None) -> str:
    if length is None:
        length = random.randint(0, 32)

    assert 0 <= length

    return "".join(random.choices(
        string.ascii_letters + string.digits, k=length))


@beartype.beartype
def gen_uniform_random_unsigned_integral(zero_exclued: bool, size: None | int) -> tuple[
    int,  # size
    int  # value
]:
    if size is None:
        size = random.randint(1, 16)

    value = random.randint(int(zero_exclued), 2**(size * 8 - 1) - 1)

    return size, value


@beartype.beartype
def gen_exp_random_unsigned_integral(zero_exclued: bool, size: None | int) -> tuple[
    int,  # size
    int  # value
]:
    if size is None:
        size = random.randint(1, 16)

    exp = random.uniform(0, size * 8)

    value = round(2**exp)

    if zero_exclued:
        value += 1

    value = min(value, 2**(size * 8 - 1) - 1)

    return size, value


@beartype.beartype
def gen_exp_random_signed_integral(size: None | int) -> tuple[
    int,  # size
    int  # value
]:
    size, value = gen_exp_random_unsigned_integral(False, size)

    if random.choice((False, True)):
        value = -value // 2

    return size, value


@beartype.beartype
def random_partition(n: int, k: int) -> list[int]:
    assert 0 < k <= n

    if k == 1:
        return [n]

    cuts = sorted(random.sample(range(1, n), k - 1))

    return [
        cuts[0],
        *(cuts[i] - cuts[i - 1] for i in range(1, len(cuts))),
        n - cuts[-1],
    ]


@beartype.beartype
def gen_random_object_state_notation_json_struct(
    config: object_state_notation.Config,
    energy: int,
) -> dict[str, object]:
    assert 0 < energy

    node_type = random.choice((
        object_state_notation.NodeTypeEnum.Null,
        object_state_notation.NodeTypeEnum.Integral,
        object_state_notation.NodeTypeEnum.IntegralList,
        object_state_notation.NodeTypeEnum.NodeList,
    )) if energy == 1 else object_state_notation.NodeTypeEnum.NodeList

    has_name = random.choice((False, True))
    name = gen_random_str() if has_name else None

    has_obj_type = random.choice((False, True))
    obj_type = gen_random_str() if has_obj_type else None

    has_region = random.choice((False, True))

    region_attr_max = 2**(config.region_attr_size * 8) - 1

    region_beg = random.randint(0, region_attr_max) if has_region else None
    region_size = random.randint(0, region_attr_max) if has_region else None

    node: dict[str, object] = dict()

    node["node_type"] = node_type.name

    if has_name:
        node["name"] = name

    if has_obj_type:
        node["obj_type"] = obj_type

    if has_region:
        node["region_beg"] = region_beg
        node["region_size"] = region_size

    if node_type == object_state_notation.NodeTypeEnum.Integral or node_type == object_state_notation.NodeTypeEnum.IntegralList:
        integral_signedness = random.choice((False, True))
        integral_size = random.randint(1, 16)

        node["integral_signedness"] = integral_signedness
        node["integral_size"] = integral_size

    @beartype.beartype
    def gen_random_integral_() -> int:
        if integral_signedness:
            value = gen_exp_random_signed_integral(integral_size)[1]
        else:
            value = gen_exp_random_unsigned_integral(False, integral_size)[1]

        return value

    if node_type == object_state_notation.NodeTypeEnum.IntegralList or node_type == object_state_notation.NodeTypeEnum.NodeList:
        is_varying_list = random.choice((False, True))

        if node_type == object_state_notation.NodeTypeEnum.NodeList:
            actual_list_elem_cnt = random.randint(1, min(8, energy))
        else:
            actual_list_elem_cnt = random.randint(0, 128)

        node["list_elem_cnt"] = -1 if is_varying_list else actual_list_elem_cnt

    if node_type == object_state_notation.NodeTypeEnum.Integral:
        value = gen_random_integral_()
        node["content"] = value

    if node_type == object_state_notation.NodeTypeEnum.IntegralList:
        content = [gen_random_integral_() for _ in range(actual_list_elem_cnt)]
        node["content"] = content

    if node_type == object_state_notation.NodeTypeEnum.NodeList:
        child_energies = random_partition(energy, actual_list_elem_cnt)

        child_nodes = [
            gen_random_object_state_notation_json_struct(config, child_energy)
            for child_energy in child_energies
        ]

        node["content"] = child_nodes

    return node


@beartype.beartype
def main1() -> None:
    config = object_state_notation.Config(
        version=object_state_notation.Version(
            major=random.randint(0, 255),
            minor=random.randint(0, 255),
            patch0=random.randint(0, 255),
            patch1=random.randint(0, 255),
        ),
        region_attr_size=4,
    )

    node_json_struct = gen_random_object_state_notation_json_struct(
        config, 1000)
    node = object_state_notation.deserialize_node_from_json_struct(
        node_json_struct)

    re_node_json_struct = object_state_notation.serialize_node_to_json_struct(
        node)

    cur_path = pathlib.Path(__file__).resolve()
    cur_dir = cur_path.parent

    utils.write_json(cur_dir / "test1.json", node_json_struct)

    f = io.BytesIO()

    # object_state_notation.serialize_header_to_octets(f, config.to_header())
    object_state_notation.serialize_node_to_octets(f, config, node)

    result = f.getvalue()

    re_node = object_state_notation.deserialize_node_from_octets(
        io.BytesIO(result), config)

    assert node == re_node

    re_node_json_struct = object_state_notation.serialize_node_to_json_struct(
        re_node)

    utils.write_json(cur_dir / "test1_re.json", re_node_json_struct)

    print(node_json_struct == re_node_json_struct)


@beartype.beartype
def main2() -> None:
    cur_path = pathlib.Path(__file__).resolve()
    cur_dir = cur_path.parent

    os.makedirs((cur_dir / "osn").as_posix(), exist_ok=True)

    for iter in range(100):
        config = object_state_notation.Config(
            version=object_state_notation.Version(
                major=random.randint(0, 255),
                minor=random.randint(0, 255),
                patch0=random.randint(0, 255),
                patch1=random.randint(0, 255),
            ),
            region_attr_size=random.randint(
                object_state_notation.min_region_attr_size,
                object_state_notation.max_region_attr_size,
            ),
        )

        node_json_struct = gen_random_object_state_notation_json_struct(
            config, 1000)
        node = object_state_notation.deserialize_node_from_json_struct(
            node_json_struct)

        re_node_json_struct = object_state_notation.serialize_node_to_json_struct(
            node)

        utils.write_json(cur_dir / "osn" /
                         f"test_osn_{iter}.json", node_json_struct)

        f = io.BytesIO()

        object_state_notation.serialize_header_to_octets(f, config.to_header())
        object_state_notation.serialize_node_to_octets(f, config, node)

        result = f.getvalue()

        with utils.create_file(cur_dir / "osn" / f"test_osn_{iter}.bin", "wb") as bin_file:
            bin_file.write(result)

        g = io.BytesIO(result)

        re_config = object_state_notation.Config.from_header(
            object_state_notation.deserialize_header_from_octets(g))

        assert config == re_config

        re_node = object_state_notation.deserialize_node_from_octets(
            g, re_config)

        assert node == re_node

        re_node_json_struct = object_state_notation.serialize_node_to_json_struct(
            re_node)

        utils.write_json(
            cur_dir / "osn" /
            f"test_osn_{iter}_re.json", re_node_json_struct)

        print(node_json_struct == re_node_json_struct)


if __name__ == "__main__":
    main1()
