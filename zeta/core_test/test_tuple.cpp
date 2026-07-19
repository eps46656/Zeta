#include <string>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/tuple.hpp>
#include <zeta/core/type_list.hpp>
#include <zeta/core/utils.ipp>

inline void main1() {
    using type_list = zeta::core::TypeList<int, float, double>;
    using type_list_b = zeta::core::TypeList<char, short>;

    std::cout << "TypeList<int, float, double>: "
              << zeta::core::debug_utils::GetTypeStr<type_list>() << std::endl;

    using appended_type_list = type_list::Append<char>;

    std::cout << "Appended TypeList<int, float, double, char>: "
              << zeta::core::debug_utils::GetTypeStr<appended_type_list>()
              << std::endl;

    using prepended_type_list = type_list::Prepend<char>;

    std::cout << "Prepended TypeList<char, int, float, double>: "
              << zeta::core::debug_utils::GetTypeStr<prepended_type_list>()
              << std::endl;

    using concatenated_type_list = type_list::Concat<type_list_b>;

    std::cout << "Concatenated TypeList<int, float, double, char, short>: "
              << zeta::core::debug_utils::GetTypeStr<concatenated_type_list>()
              << std::endl;

    using popped_front_type_list = type_list::PopFront;

    std::cout << "Popped Front TypeList<float, double>: "
              << zeta::core::debug_utils::GetTypeStr<popped_front_type_list>()
              << std::endl;

    using popped_back_type_list = type_list::PopBack;

    std::cout << "Popped Back TypeList<int, float>: "
              << zeta::core::debug_utils::GetTypeStr<popped_back_type_list>()
              << std::endl;

    using reversed_type_list = type_list::Reverse;

    std::cout << "Reversed TypeList<double, float, int>: "
              << zeta::core::debug_utils::GetTypeStr<reversed_type_list>()
              << std::endl;
}

inline void main2() {
    zeta::core::tuple::Tuple<int, float, double> t1;

    ZETA_Core_PrintVar(sizeof(zeta::core::tuple::Tuple<int, float, double>));

    std::cout << "Tuple<int, float, double>: "
              << zeta::core::debug_utils::GetTypeStr<decltype(t1)>()
              << std::endl;

    std::cout << "t1.Get<0>(): " << t1.Access<0>() << std::endl;
    std::cout << "t1.Get<1>(): " << t1.Access<1>() << std::endl;
    std::cout << "t1.Get<2>(): " << t1.Access<2>() << std::endl;

    zeta::core::tuple::Tuple<char, short, int, long> t2{
        'a',
        static_cast<short>(2),
    };

    ZETA_Core_PrintVar(sizeof(zeta::core::tuple::Tuple<int, int, long>));
    // 12

    ZETA_Core_PrintVar(sizeof(zeta::core::tuple::Tuple<double, int, int>));
    // 16

    ZETA_Core_PrintVar(
        sizeof(zeta::core::tuple::Tuple<double, char, char, char>));
    // 16

    ZETA_Core_PrintVar(
        alignof(zeta::core::tuple::Tuple<double, char, char, char>));

    ZETA_Core_PrintVar(
        sizeof(zeta::core::tuple::Tuple<char, int, double, char, char, char>));
    // 24

    ZETA_Core_PrintVar(
        alignof(zeta::core::tuple::Tuple<char, int, double, char, char, char>));

    std::cout << "Tuple<char, short, int, long>: "
              << zeta::core::debug_utils::GetTypeStr<decltype(t2)>()
              << std::endl;

    std::cout << "t2.Get<0>(): " << t2.Access<0>() << std::endl;
    std::cout << "t2.Get<1>(): " << t2.Access<1>() << std::endl;
    std::cout << "t2.Get<2>(): " << t2.Access<2>() << std::endl;
    std::cout << "t2.Get<3>(): " << t2.Access<3>() << std::endl;
}

int main() {
    main2();
    printf("ok\n");
    return 0;
}
