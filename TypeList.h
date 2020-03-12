/**
* compile time 'type list' data structure
*
* Dan Israel Malta
**/
#include <type_traits>

// parameter pack related operations
namespace Pack {

    //
    // implementation
    //
    namespace detail {

        // get parameter pack element at index I
        template<std::size_t I, typename T, typename...Ts>
        struct nth_element_impl {
            static_assert(I <= sizeof...(Ts));
            using type = typename nth_element_impl<I - 1, Ts...>::type;
        };
        template<typename T, typename ...Ts>
        struct nth_element_impl<0, T, Ts...> {
            using type = T;
        };

        // test if type {@T} exists in a pack
        template<typename T, typename...Args>
        struct is_type_in_pack {
            static constexpr bool value{ (std::is_same<T, Args>::value || ...) };
        };
    };

    //
    // API
    //

    // return parameter pack element at index I
    template<std::size_t I, typename ...Ts> using element_at_index = typename detail::nth_element_impl<I, Ts...>::type;

    // test if a given type exists in parameter pack
    template<typename T, typename...Args> constexpr bool is_type_in_pack = detail::is_type_in_pack<T, Args...>::value;

    //
    // tests
    //

    static_assert(std::is_same<element_at_index<0, float, double, char>, float>::value);
    static_assert(std::is_same<element_at_index<1, float, double, char>, double>::value);
    static_assert(std::is_same<element_at_index<2, float, double, char>, char>::value);

    static_assert(is_type_in_pack<float, float, double, char>);
    static_assert(is_type_in_pack<int, float, double, char> == false);
}

// compile time 'type list' data structure
namespace DataStructure {

    //
    // forward decleration
    //
    
    // a NaN/null style type
    struct type_null {};

    // a variadic type list
    template<typename... Ts> struct type_list {};
    
    //
    // implementations
    //
    namespace detail {

        // type_list size
        template<typename SEQ> struct size_impl;
        template<template<typename...> class SEQ, typename... Ts>
        struct size_impl<SEQ<Ts...>> {
            using type = std::integral_constant<std::size_t, sizeof...(Ts)>;
        };
        
        // type_list head element
        template<typename SEQ> struct front_impl;
        template<template<typename...> class SEQ, typename T, typename... Ts>
        struct front_impl<SEQ<T, Ts...>> {
            using type = T;
        };
        template<template<typename...> class SEQ>
        struct front_impl<SEQ<>> {
            using type = type_null;
        };
    
        // remove type_list head element
        template<typename SEQ> struct pop_front_impl;
        template<template<typename...> class SEQ, typename T, typename... Ts>
        struct pop_front_impl<SEQ<T, Ts...>> {
            using type = SEQ<Ts...>;
        };
        template<template<typename...> class SEQ>
        struct pop_front_impl<SEQ<>> {
            using type = type_null;
        };
    
        // insert type_list head
        template<typename SEQ, typename T> struct push_front_impl;
        template<template<typename...> class SEQ, typename T, typename... Ts>
        struct push_front_impl<SEQ<Ts...>, T> {
            using type = SEQ<T, Ts...>;
        };
    
        // insert type_list tail
        template<typename SEQ, typename T> struct push_back_impl;
        template<template<typename...> class SEQ, typename T, typename... Ts>
        struct push_back_impl<SEQ<Ts...>, T> {
            using type = SEQ<Ts..., T>;
        };
    
        // concat two type_list's together (concatenation is performed by order)
        template<typename SEQ1, typename SEQ2> struct concat_impl;
        template<template<typename...> class SEQ1, template<typename...> class SEQ2, typename... Ts, typename... Us>
        struct concat_impl<SEQ1<Ts...>, SEQ2<Us...>> {
            using type = SEQ1<Ts..., Us...>;
        };
    
        // test if two type_list's are identical (by content and order)
        template<typename SEQ1, typename SEQ2> struct identical_impl;
        template<template<typename...> class SEQ1, template<typename...> class SEQ2, typename... Ts, typename... Us>
        struct identical_impl<SEQ1<Ts...>, SEQ2<Us...>> {
            using type = std::false_type;
        };
        template<template<typename...> class SEQ1, template<typename...> class SEQ2, typename... Ts>
        struct identical_impl<SEQ1<Ts...>, SEQ2<Ts...>> {
            using type = std::true_type;
        };
    
        // return type at a given index
        template<std::size_t I, typename SEQ> struct element_at_index_impl;
        template<std::size_t I, template<typename...> class SEQ, typename... Ts>
        struct element_at_index_impl<I, SEQ<Ts...>> {
            static_assert(I < sizeof...(Ts));
            using type = Pack::element_at_index<I, Ts...>;
        };
    
        // check if type_list contains a given type
        template<typename T, typename SEQ> struct contains_type_impl;
        template<typename T, template<typename...> class SEQ, typename... Ts>
        struct contains_type_impl<T, SEQ<Ts...>> {
            using type = std::integral_constant<bool, Pack::is_type_in_pack<T, Ts...>>;
        };

        // return index of a given type (if the type has multiple occurences, return the first one from the tail)
        template<std::size_t I, typename T, typename SEQ> struct index_of_type_impl;
        template<std::size_t I, typename T, template<typename...> class SEQ, typename... Ts>
        struct index_of_type_impl<I, T, SEQ<Ts...>> {
            static_assert(I <= sizeof...(Ts));
            using type = std::conditional_t<std::is_same<T, Pack::element_at_index<I, Ts...>>::value,
                                            std::integral_constant<std::size_t, I>, 
                                            typename index_of_type_impl<I - 1, T, SEQ<Ts...>>::type>;
        };
        template<typename T, template<typename...> class SEQ, typename... Ts>
        struct index_of_type_impl<0, T, SEQ<Ts...>> {
            using type = std::integral_constant<std::size_t, 0>;
        };
    }
    
    //
    // API
    //
    
    // return type_list size
    template<typename SEQ> using size = typename detail::size_impl<SEQ>::type;
    
    // return type_list head element
    template<typename SEQ> using front = typename detail::front_impl<SEQ>::type;
    
    // pop type_list head element
    template<typename SEQ> using pop_front = typename detail::pop_front_impl<SEQ>::type;
    
    // push element to type_list head
    template<typename SEQ, typename T> using push_front = typename detail::push_front_impl<SEQ, T>::type;
    
    // push element to type_list tail
    template<typename SEQ, typename T> using push_back = typename detail::push_back_impl<SEQ, T>::type;
    
    // concat two type_list's
    template<typename SEQ1, typename SEQ2> using concat = typename detail::concat_impl<SEQ1, SEQ2>::type;
    
    // test if two type_list's are identical (by content and order)
    template<typename SEQ1, typename SEQ2> using are_identical = typename detail::identical_impl<SEQ1, SEQ2>::type;
    
    // return type at a given index
    template<std::size_t I, typename SEQ> using element_at_index = typename detail::element_at_index_impl<I, SEQ>::type;

    // test if a given type exists in a type_list
    template<typename T, typename SEQ> using contains_type = typename detail::contains_type_impl<T, SEQ>::type;

    // return index of type T in a type_list (T must be inside type_list)
    // (if the type has multiple occurences, return the first one from the tail)
    template<typename T, typename SEQ> using index_of = typename detail::index_of_type_impl<size<SEQ>::value - 1, T, SEQ>::type;

    //
    // tests
    //
    static_assert(size<type_list<double, char, bool, double>>::value == 4);

    static_assert(std::is_same<front<type_list<double, char, bool, double>>, double>::value);
    static_assert(std::is_same<front<type_list<>>, type_null>::value);

    static_assert(std::is_same<pop_front<type_list<double, char, bool, double>>,
                               type_list<char, bool, double>>::value);
    static_assert(std::is_same<pop_front<type_list<>>, type_null>::value);

    static_assert(std::is_same<push_front<type_list<double, char, bool, double>, char>,
                               type_list<char, double, char, bool, double>>::value);

    static_assert(std::is_same<push_back<type_list<double, char, bool, double>, char>,
                               type_list<double, char, bool, double, char>>::value);

    static_assert(std::is_same<concat<type_list<double, char>, type_list<bool, double>>,
                               type_list<double, char, bool, double>>::value);

    static_assert(are_identical<type_list<double, char>, type_list<double, char>>::value);
    static_assert(are_identical<type_list<double, char>, type_list<char, double>>::value == false);

    static_assert(std::is_same<element_at_index<0, type_list<double, char, bool, double>>, double>::value);
    static_assert(std::is_same<element_at_index<1, type_list<double, char, bool, double>>, char>::value);
    static_assert(std::is_same<element_at_index<2, type_list<double, char, bool, double>>, bool>::value);
    static_assert(std::is_same<element_at_index<3, type_list<double, char, bool, double>>, double>::value);
    static_assert(std::is_same<element_at_index<3, type_list<double, char, bool, double>>, bool>::value == false);

    static_assert(contains_type<int, type_list<double, char, bool, double>>::value == false);
    static_assert(contains_type<char, type_list<double, char, bool, double>>::value);

    static_assert(index_of<char, type_list<double, char, bool, double>>::value == 1);
    static_assert(index_of<bool, type_list<double, char, bool, double, char>>::value == 2);
    static_assert(index_of<double, type_list<double, char, bool, char>>::value == 0);
    static_assert(index_of<double, type_list<double, char, bool, char, double>>::value == 4);   // returns first one from the tail
};
