from __future__ import annotations

import dataclasses
import enum
import io
import typing

import beartype
import leb128

BytesIOLike = typing.BinaryIO | io.BytesIO


UTF8 = "utf8"


@beartype.beartype
class NodeTypeEnum(enum.IntEnum):
    Null = 0
    Integral = 1
    IntegralList = 2
    NodeList = 3
    Terminator = 4


min_region_attr_size = 4
max_region_attr_size = 8


@beartype.beartype
def is_valid_region_attr_size(region_attr_size: int) -> bool:
    return min_region_attr_size <= region_attr_size <= max_region_attr_size


@beartype.beartype
@dataclasses.dataclass
class Version:
    major: int  # 1
    minor: int  # 1
    patch0: int  # 1
    patch1: int  # 1


@beartype.beartype
@dataclasses.dataclass
class Header:
    magic: bytes  # 4
    version: Version  # 4
    region_attr_size: int  # 1
    reserved: bytes  # 32 - 4 - 4 - 1 == 23

    RESERVED_SIZE: typing.ClassVar[int] = 23

    def check(self) -> bool:
        if self.magic != b"OSN\0":
            return False

        if not is_valid_region_attr_size(self.region_attr_size):
            return False

        if any(b != 0 for b in self.reserved):
            return False

        return True


@beartype.beartype
@dataclasses.dataclass
class Config:
    version: Version
    region_attr_size: int

    def check(self) -> None:
        assert is_valid_region_attr_size(self.region_attr_size)

    @staticmethod
    def from_header(header: Header) -> Config:
        header.check()

        config = Config(
            version=header.version,
            region_attr_size=header.region_attr_size,
        )

        config.check()

        return config

    def to_header(self) -> Header:
        header = Header(
            magic=b"OSN\0",
            version=self.version,
            region_attr_size=self.region_attr_size,
            reserved=b"\0" * Header.RESERVED_SIZE,
        )

        header.check()

        return header


@beartype.beartype
@dataclasses.dataclass
class NodeTag:
    node_type: NodeTypeEnum
    has_name: bool
    has_obj_type: bool
    has_region: bool

    NodeTypeMask: typing.ClassVar[int] = 0b0000_1111
    HasNameMask: typing.ClassVar[int] = 0b0001_0000
    HasObjectTypeMask: typing.ClassVar[int] = 0b0010_0000
    HasRegionMask: typing.ClassVar[int] = 0b0100_0000

    @staticmethod
    def from_encoded_value(encoded_value: int) -> NodeTag:
        assert 0 <= encoded_value <= 0xFF

        node_type = NodeTypeEnum(encoded_value & NodeTag.NodeTypeMask)
        has_name = bool(encoded_value & NodeTag.HasNameMask)
        has_obj_type = bool(encoded_value & NodeTag.HasObjectTypeMask)
        has_region = bool(encoded_value & NodeTag.HasRegionMask)

        return NodeTag(
            node_type=node_type,
            has_name=has_name,
            has_obj_type=has_obj_type,
            has_region=has_region,
        )

    def to_encoded_value(self) -> int:
        return (
            (self.node_type.value) +
            (NodeTag.HasNameMask if self.has_name else 0b0) +
            (NodeTag.HasObjectTypeMask if self.has_obj_type else 0b0) +
            (NodeTag.HasRegionMask if self.has_region else 0b0)
        )


@beartype.beartype
@dataclasses.dataclass
class IntegralDescriptor:
    is_signed: bool
    size: int

    def check(self) -> None:
        assert 0 <= self.size

    @staticmethod
    def from_encoded_value(encoded_value: int) -> IntegralDescriptor:
        assert 0 <= encoded_value

        is_signed = bool(encoded_value % 2 == 1)
        size = encoded_value // 2

        return IntegralDescriptor(
            is_signed=is_signed,
            size=size,
        )

    def to_encoded_value(self) -> int:
        return self.size * 2 + int(self.is_signed)


@beartype.beartype
@dataclasses.dataclass
class Node:
    node_type: NodeTypeEnum

    name: None | str

    obj_type: None | str

    region_beg: None | int
    region_size: None | int

    integral_is_signed: None | bool
    # when node_type == NodeTypeEnum.NegativeIntegral or
    #      node_type == NodeTypeEnum.ZeroOrPositiveIntegral

    integral_size: None | int

    list_elem_cnt: None | int
    # when node_type == NodeTypeEnum.IntegralList or
    #      node_type == NodeTypeEnum.NodeList

    content: None | int | list[int] | list[Node]


@beartype.beartype
def integral_range(signed: bool, width: int) -> tuple[int, int]:
    assert 1 <= width
    k = 2**(width - 1)
    return (-k + 1, +k - 1) if signed else (0, k * 2 - 1)


@beartype.beartype
def max_integral_size(integral_descriptor_size: int) -> int:
    assert 1 <= integral_descriptor_size
    return 2**(integral_descriptor_size * 8 - 1) - 1


@beartype.beartype
def serialize_unsigned_integral(
    ostream: BytesIOLike,
    size: int,
    value: int,
) -> None:
    assert 1 <= size

    assert 0 <= value <= 2**(size * 8) - 1

    ostream.write(value.to_bytes(size, byteorder="little", signed=False))


@beartype.beartype
def deserialize_unsigned_integral(
    istream: BytesIOLike,
    size: int,
) -> int:
    return int.from_bytes(istream.read(size), byteorder="little", signed=False)


@beartype.beartype
def serialize_signed_integral(
    ostream: BytesIOLike,
    size: int,
    value: int,
) -> None:
    assert 1 <= size

    k = 2**(size * 8 - 1)

    range_min, range_max = -k + 1, +k - 1

    assert range_min <= value <= range_max

    if value < 0:
        value += k + k

    ostream.write(value.to_bytes(size, byteorder="little", signed=False))


@beartype.beartype
def deserialize_signed_integral(
    istream: BytesIOLike,
    size: int,
) -> int:
    k = 2**(size * 8 - 1)

    value = int.from_bytes(istream.read(
        size), byteorder="little", signed=False)

    if k <= value:
        value -= k + k

    return value


@beartype.beartype
def serialize_unsigned_leb128(
    ostream: BytesIOLike,
    value: int,
) -> None:
    assert 0 <= value

    ostream.write(leb128.u.encode(value))


@beartype.beartype
def deserialize_unsigned_leb128(
    istream: BytesIOLike,
) -> int:
    return leb128.u.decode_reader(istream)[0]


@beartype.beartype
def serialize_signed_leb128(
    ostream: BytesIOLike,
    value: int,
) -> None:
    ostream.write(leb128.i.encode(value))


@beartype.beartype
def deserialize_signed_leb128(
    istream: BytesIOLike,
) -> int:
    return leb128.i.decode_reader(istream)[0]


@beartype.beartype
def serialize_string(
    ostream: BytesIOLike,
    value: str,
) -> None:
    ostream.write(value.encode(UTF8))
    ostream.write(b"\0")


@beartype.beartype
def deserialize_string(istream: BytesIOLike) -> str:
    ba = bytearray()

    while True:
        c = istream.read(1)

        if c == b"" or c == b"\0":
            break

        ba.extend(c)

    return ba.decode(UTF8)


@beartype.beartype
def serialize_header_to_octets(
    ostream: BytesIOLike,
    header: Header,
) -> None:
    header.check()

    ostream.write(header.magic)

    serialize_unsigned_integral(ostream, 1, header.version.major)
    serialize_unsigned_integral(ostream, 1, header.version.minor)
    serialize_unsigned_integral(ostream, 1, header.version.patch0)
    serialize_unsigned_integral(ostream, 1, header.version.patch1)

    serialize_unsigned_integral(ostream, 1, header.region_attr_size)

    ostream.write(header.reserved)


@beartype.beartype
def deserialize_header_from_octets(
    istream: BytesIOLike,
) -> Header:
    magic = istream.read(4)

    version_major = deserialize_unsigned_integral(istream, 1)
    version_minor = deserialize_unsigned_integral(istream, 1)
    version_patch0 = deserialize_unsigned_integral(istream, 1)
    version_patch1 = deserialize_unsigned_integral(istream, 1)

    region_attr_size = deserialize_unsigned_integral(istream, 1)

    reserved = istream.read(Header.RESERVED_SIZE)

    header = Header(
        magic=magic,
        version=Version(
            major=version_major,
            minor=version_minor,
            patch0=version_patch0,
            patch1=version_patch1,
        ),
        region_attr_size=region_attr_size,
        reserved=reserved,
    )

    header.check()

    return header


@beartype.beartype
def serialize_node_to_octets(
    ostream: BytesIOLike,
    config: Config,
    node: Node,
) -> None:
    @beartype.beartype
    def serialize_integral(
        ostream: BytesIOLike,
        is_signed: bool,
        size: int,
        value: int
    ) -> None:
        if size == 0:
            if is_signed:
                serialize_signed_leb128(ostream, value)
            else:
                serialize_unsigned_leb128(ostream, value)
        else:
            if is_signed:
                serialize_signed_integral(ostream, size, value)
            else:
                serialize_unsigned_integral(ostream, size, value)

    config.check()

    has_name = node.name is not None

    has_obj_type = node.obj_type is not None

    assert (node.region_beg is None) == (node.region_size is None)
    has_region = node.region_beg is not None

    serialize_unsigned_integral(ostream, 1, NodeTag(
        node_type=node.node_type,
        has_name=has_name,
        has_obj_type=has_obj_type,
        has_region=has_region,
    ).to_encoded_value())

    if has_name:
        serialize_string(ostream, node.name)

    if has_obj_type:
        serialize_string(ostream, node.obj_type)

    if has_region:
        serialize_unsigned_integral(
            ostream, config.region_attr_size, node.region_beg)

        serialize_unsigned_integral(
            ostream, config.region_attr_size, node.region_size)

    if node.node_type == NodeTypeEnum.Integral or node.node_type == NodeTypeEnum.IntegralList:
        assert isinstance(node.integral_is_signed, bool)
        assert isinstance(node.integral_size, int)

        serialize_unsigned_leb128(
            ostream,
            IntegralDescriptor(
                is_signed=node.integral_is_signed,
                size=node.integral_size,
            ).to_encoded_value())

    if node.node_type == NodeTypeEnum.IntegralList or node.node_type == NodeTypeEnum.NodeList:
        list_elem_cnt = node.list_elem_cnt
        assert isinstance(list_elem_cnt, int)

        assert -1 <= list_elem_cnt

        serialize_unsigned_leb128(ostream, list_elem_cnt + 1)

    if node.node_type == NodeTypeEnum.Integral:
        content = node.content
        assert isinstance(content, int)

        if 0 < node.integral_size:
            range_min, range_max = integral_range(
                node.integral_is_signed, node.integral_size * 8)

            assert range_min <= content <= range_max

        serialize_integral(
            ostream, node.integral_is_signed, node.integral_size, content)

    if node.node_type == NodeTypeEnum.IntegralList:
        content = node.content
        assert isinstance(content, list)

        if node.integral_size == 0:
            range_min, range_max = 0, 0
        else:
            range_min, range_max = integral_range(
                node.integral_is_signed, node.integral_size * 8)

            assert all(isinstance(elem, int) and range_min <= elem <= range_max
                       for elem in content)

        if 0 <= node.list_elem_cnt:
            assert node.list_elem_cnt == len(content)

        if node.list_elem_cnt == -1:
            buffer = io.BytesIO()
            cur_buffer = io.BytesIO()
            buffer_capacity = 255

            cur_chunk_elem_cnt = 0

            for integral in content:
                serialize_integral(
                    cur_buffer, node.integral_is_signed, node.integral_size, integral)

                cur_chunk_elem_cnt += 1

                if cur_chunk_elem_cnt == 255 or buffer_capacity < buffer.getbuffer().nbytes + cur_buffer.getbuffer().nbytes:
                    serialize_unsigned_integral(ostream, 1, cur_chunk_elem_cnt)
                    ostream.write(buffer.getbuffer())
                    ostream.write(cur_buffer.getbuffer())

                    buffer.seek(0)
                    buffer.truncate(0)

                    cur_buffer.seek(0)
                    cur_buffer.truncate(0)

                    cur_chunk_elem_cnt = 0
                else:
                    buffer.write(cur_buffer.getbuffer())
                    cur_buffer.seek(0)
                    cur_buffer.truncate(0)

            if 0 < cur_chunk_elem_cnt:
                serialize_unsigned_integral(ostream, 1, cur_chunk_elem_cnt)
                ostream.write(buffer.getbuffer())
                ostream.write(cur_buffer.getbuffer())

            serialize_unsigned_integral(ostream, 1, 0)
        else:
            for elem in content:
                serialize_integral(
                    ostream, node.integral_is_signed, node.integral_size, elem)

    if node.node_type == NodeTypeEnum.NodeList:
        child_nodes = node.content
        assert isinstance(child_nodes, list)

        if 0 <= node.list_elem_cnt:
            assert node.list_elem_cnt == len(child_nodes)

        for child_node in child_nodes:
            assert child_node.node_type != NodeTypeEnum.Terminator
            serialize_node_to_octets(ostream, config, child_node)

        if node.list_elem_cnt == -1:
            serialize_unsigned_integral(ostream, 1, NodeTag(
                node_type=NodeTypeEnum.Terminator,
                has_name=False,
                has_obj_type=False,
                has_region=False,
            ).to_encoded_value())


@beartype.beartype
def deserialize_node_from_octets(
    istream: BytesIOLike,
    config: Config,
) -> Node:
    @beartype.beartype
    def deserialize_integral(
        istream: BytesIOLike,
        integral_is_signed: bool,
        integral_size: int,
    ) -> int:
        if integral_size == 0:
            if integral_is_signed:
                value = deserialize_signed_leb128(istream)
            else:
                value = deserialize_unsigned_leb128(istream)
        else:
            if integral_is_signed:
                value = deserialize_signed_integral(istream, integral_size)
            else:
                value = deserialize_unsigned_integral(istream, integral_size)

        return value

    config.check()

    node_tag_encoded_value = deserialize_unsigned_integral(istream, 1)

    node_tag = NodeTag.from_encoded_value(
        node_tag_encoded_value)

    node_type = node_tag.node_type
    has_name = node_tag.has_name
    has_obj_type = node_tag.has_obj_type
    has_region = node_tag.has_region

    name = deserialize_string(istream) \
        if has_name else None

    obj_type = deserialize_string(istream) \
        if has_obj_type else None

    if has_region:
        region_beg = deserialize_unsigned_integral(
            istream, config.region_attr_size)

        region_size = deserialize_unsigned_integral(
            istream, config.region_attr_size)
    else:
        region_beg = None
        region_size = None

    integral_is_signed = None
    integral_size = None
    list_elem_cnt = None
    content = None

    if node_type == NodeTypeEnum.Integral or node_type == NodeTypeEnum.IntegralList:
        integral_descriptor = IntegralDescriptor.from_encoded_value(
            deserialize_unsigned_leb128(istream))

        integral_is_signed = integral_descriptor.is_signed
        integral_size = integral_descriptor.size

    if node_type == NodeTypeEnum.IntegralList or node_type == NodeTypeEnum.NodeList:
        list_elem_cnt = deserialize_unsigned_leb128(istream) - 1

    if node_type == NodeTypeEnum.Integral:
        content = deserialize_integral(
            istream, integral_is_signed, integral_size)

    if node_type == NodeTypeEnum.IntegralList:
        if list_elem_cnt == -1:
            content: list[int] = list()

            while True:
                cur_elem_cnt = deserialize_unsigned_integral(istream, 1)

                if cur_elem_cnt == 0:
                    break

                content.extend(
                    deserialize_integral(
                        istream, integral_is_signed, integral_size)
                    for _ in range(cur_elem_cnt))
        else:
            content = [
                deserialize_integral(
                    istream, integral_is_signed, integral_size)
                for _ in range(list_elem_cnt)
            ]

    if node_type == NodeTypeEnum.NodeList:
        content: list[Node] = list()

        if list_elem_cnt == -1:
            while True:
                child_node = deserialize_node_from_octets(istream, config)

                if child_node.node_type == NodeTypeEnum.Terminator:
                    break

                content.append(child_node)
        else:
            for _ in range(list_elem_cnt):
                child_node = deserialize_node_from_octets(istream, config)
                assert child_node.node_type != NodeTypeEnum.Terminator

                content.append(child_node)

    return Node(
        node_type=node_type,
        name=name,
        obj_type=obj_type,
        region_beg=region_beg,
        region_size=region_size,
        integral_is_signed=integral_is_signed,
        integral_size=integral_size,
        list_elem_cnt=list_elem_cnt,
        content=content,
    )


@beartype.beartype
def serialize_node_to_json_struct(node: Node) -> dict[str, object]:
    ret: dict[str, object] = dict()

    ret["node_type"] = node.node_type.name

    if node.name is not None:
        ret["name"] = node.name

    if node.obj_type is not None:
        ret["obj_type"] = node.obj_type

    if node.region_beg is not None:
        ret["region_beg"] = node.region_beg

    if node.region_size is not None:
        ret["region_size"] = node.region_size

    if node.node_type == NodeTypeEnum.Integral or node.node_type == NodeTypeEnum.IntegralList:
        integral_signedness = node.integral_is_signed
        assert isinstance(integral_signedness, bool)

        integral_size = node.integral_size
        assert isinstance(integral_size, int)

        assert 0 <= node.integral_size

        ret["integral_signedness"] = node.integral_is_signed
        ret["integral_size"] = node.integral_size

    if node.node_type == NodeTypeEnum.IntegralList or node.node_type == NodeTypeEnum.NodeList:
        list_elem_cnt = node.list_elem_cnt
        assert isinstance(list_elem_cnt, int)
        assert -1 <= list_elem_cnt

        ret["list_elem_cnt"] = list_elem_cnt

    if node.node_type == NodeTypeEnum.Integral:
        content = node.content
        assert isinstance(content, int)

        if 0 < node.integral_size:
            range_min, range_max = integral_range(
                node.integral_is_signed, node.integral_size * 8)

            assert range_min <= content <= range_max

        ret["content"] = content

    if node.node_type == NodeTypeEnum.IntegralList:
        content = node.content
        assert isinstance(content, list)

        if 0 < node.integral_size:
            range_min, range_max = integral_range(
                node.integral_is_signed, node.integral_size * 8)

            assert all(isinstance(elem, int) and range_min <= elem <= range_max
                       for elem in content)

        if node.list_elem_cnt != -1:
            assert node.list_elem_cnt == len(content)

        ret["content"] = content

    if node.node_type == NodeTypeEnum.NodeList:
        child_nodes = node.content
        assert isinstance(child_nodes, list)

        ret["content"] = [
            serialize_node_to_json_struct(child_node)
            for child_node in child_nodes
        ]

    return ret


@beartype.beartype
def deserialize_node_from_json_struct(dict_struct: dict[str, object]) -> Node:
    node_type = NodeTypeEnum[dict_struct["node_type"]]

    name = dict_struct.get("name", None)

    obj_type = dict_struct.get("obj_type", None)

    region_beg = dict_struct.get("region_beg", None)
    region_size = dict_struct.get("region_size", None)

    integral_signedness = None
    integral_size = None
    list_elem_cnt = None
    content = None

    if node_type == NodeTypeEnum.Integral or node_type == NodeTypeEnum.IntegralList:
        integral_signedness = dict_struct["integral_signedness"]
        assert isinstance(integral_signedness, bool)

        integral_size = dict_struct["integral_size"]
        assert isinstance(integral_size, int)
        assert 0 <= integral_size

    if node_type == NodeTypeEnum.IntegralList or node_type == NodeTypeEnum.NodeList:
        list_elem_cnt = dict_struct["list_elem_cnt"]
        assert isinstance(list_elem_cnt, int)
        assert -1 <= list_elem_cnt

    if node_type == NodeTypeEnum.Integral:
        content = dict_struct["content"]
        assert isinstance(content, int)

        if 0 < integral_size:
            range_min, range_max = integral_range(
                integral_signedness, integral_size * 8)

            assert range_min <= content <= range_max

    if node_type == NodeTypeEnum.IntegralList:
        content = dict_struct["content"]
        assert isinstance(content, list)

        if 0 < integral_size:
            range_min, range_max = integral_range(
                integral_signedness, integral_size * 8)

            assert all(isinstance(elem, int) and range_min <= elem <= range_max
                       for elem in content)

        if list_elem_cnt != -1:
            assert list_elem_cnt == len(content)

    if node_type == NodeTypeEnum.NodeList:
        child_node_dict_structs = dict_struct["content"]
        assert isinstance(child_node_dict_structs, list)

        content = [
            deserialize_node_from_json_struct(child_node_dict_struct)
            for child_node_dict_struct in child_node_dict_structs
        ]

        if list_elem_cnt != -1:
            assert list_elem_cnt == len(content)

    return Node(
        node_type=node_type,
        obj_type=obj_type,
        name=name,
        region_beg=region_beg,
        region_size=region_size,
        integral_is_signed=integral_signedness,
        integral_size=integral_size,
        list_elem_cnt=list_elem_cnt,
        content=content,
    )
