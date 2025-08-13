def SplitBasic(keys, children, child_idx, key, l, r, max_degree):
    d = max_degree // 2

    new_keys = keys[:]
    new_keys.insert(child_idx, key)

    new_children = children[:]
    new_children.insert(child_idx, l)
    new_children[child_idx + 1] = r

    print(new_keys)
    print(new_children)

    new_key = None
    # new_l =

    left_split = child_idx < max_degree - child_idx

    if left_split:
        nl_keys = new_keys[:d]
        nl_children = new_children[:d+1]

        nr_keys = new_keys[d+1:]
        nr_children = new_children[d+1:]
    else:
        nl_keys = new_keys[:d+1]
        nl_children = new_children[:d+2]

        nr_keys = new_keys[d+2:]
        nr_children = new_children[d+2:]

def Split(keys, children, child_idx, key, l, r, max_degree):
    pass

def main1():
    keys = [0, 1, 2, 3, 4]
    children = [10, 11, 12, 13, 14, 15]

    SplitBasic(keys, children, 4, 123, 456, 789, 5)

if __name__ == "__main__":
    main1()
