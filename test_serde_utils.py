import beartype
import random


"""

signedness_table = False / True

the order of
    range_width
    io_width
    oi_width

digit width = 1 ~ 20
variant digit count = False / True


"""


@beartype.beartype
def main1() -> None:
    signedness_table = ["UNSIGNED", "SIGNED"]
    endianness_table = ["LE", "BE"]
    range_width_table = list(range(1, 168+1))
    io_width_table = list(range(1, 168+1))
    oi_width_table = list(range(1, 168+1))
    digit_width_table = list(range(2, 12+1))
    variant_digit_cnt_table = ["FWC", "VWC"]
    digit_cnt_table = list(range(1, 20+1))

    cur_cnt = 10

    while 0 < cur_cnt:
        cur_signedness = random.choice(signedness_table)
        cur_range_width = random.choice(range_width_table)
        cur_endianness = random.choice(endianness_table)
        cur_io_width = random.choice(io_width_table)
        cur_oi_width = random.choice(oi_width_table)
        cur_digit_width = random.choice(digit_width_table)
        cur_variant_digit_cnt = random.choice(variant_digit_cnt_table)
        cur_digit_cnt = random.choice(digit_cnt_table)

        is_ok = True

        is_ok &= cur_range_width <= cur_io_width

        if not is_ok:
            continue

        cur_cnt -= 1

        print(f"F<{cur_signedness}, {cur_range_width}, {cur_endianness}, {cur_io_width}, {cur_oi_width}, {cur_digit_width}, {cur_variant_digit_cnt}, {cur_digit_cnt}>(special_value);")


if __name__ == "__main__":
    main1()
