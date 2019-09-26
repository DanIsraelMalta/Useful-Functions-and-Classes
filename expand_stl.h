/**
* this namespace extend STL algorithms so that they would:
* 1) operate on a variadic amount of collections where each collection can be of a different type but must hold the same underlying type.
* 2) operate on homogeneous parameter packs.
*
* Dan Israel Malta
**/

#pragma once
#include <type_traits>
#include <numeric>
#include <algorithm>
#include <tuple>
#include <array>

// type traits
namespace {
    // test if an object is iterate-able
    template<typename T, typename = void> struct is_iterate_able : std::false_type {};
    template<typename T>                  struct is_iterate_able<T, std::void_t<decltype(std::begin(std::declval<T>())), decltype(std::end(std::declval<T>()))>> : std::true_type {};
    template<typename T> inline constexpr bool is_iterate_able_v = is_iterate_able<T>::value;

    // test if a pack of objects are iterate-able
    template<typename ...Ts> struct are_iterate_able { static constexpr bool value{ (is_iterate_able_v<Ts> && ...) }; };
    template<typename ...Ts> inline constexpr bool are_iterate_able_v = are_iterate_able<Ts...>::value;

    // test if all elements in parameter pack are of identical type
    template<typename...Args> constexpr bool all_same_v = sizeof...(Args) ? (std::is_same_v<std::tuple_element_t<0, std::tuple<Args...>>, Args> && ...) : false;

    // test if all elements are of identical types and are not iterate-able collections
    template<typename...Args> constexpr bool are_homogeneous_pack = !are_iterate_able_v<Args...> && all_same_v<Args...>;
}

/**
* \brief perform a binary reduction operation on all elements in a variadic number of different type of collections, reduction operation is performed left to right.
*        i.e. - generalized 'std::reduce' to a variadic number of different type of collections.
*
* @param {ExecutionPolicy, in}  execution policy
* @param {T,               in}  initial reductionvalue
* @param {BinaryOp,        in}  binary operation
* @param {collections,     in}  collections of elements (collection should be iterate-able and with underlying type 'T')
*                               or
*                               parameter pack of T objects
* @param {double,          out} reduced output
**/
template<class ExecutionPolicy, class T, class BinaryOp, class...Containers, typename std::enable_if<are_iterate_able_v<Containers...>>::type* = nullptr>
constexpr inline T reduce(ExecutionPolicy&& xi_policy, const T xi_init, BinaryOp&& xi_operation, const Containers&... xi_containers) {
    const auto reduced_containers = std::initializer_list<T>{
        std::reduce(std::forward<ExecutionPolicy>(xi_policy), xi_containers.begin(), xi_containers.end(), xi_init, std::forward<BinaryOp>(xi_operation))...
    };
    return std::reduce(std::forward<ExecutionPolicy>(xi_policy), reduced_containers.begin(), reduced_containers.end(), xi_init, std::forward<BinaryOp>(xi_operation));
}

template<class T, class BinaryOp, class...Containers, typename std::enable_if<are_homogeneous_pack<Containers...>>::type* = nullptr>
constexpr inline T reduce(const T xi_init, BinaryOp&& xi_operation, const Containers&... xi_containers) {
    return reduce(std::execution::seq, xi_init, std::forward<BinaryOp>(xi_operation), std::initializer_list<T>{ xi_containers... });
}

/**
* \brief check if a unary predicate returns 'true' for all elements in a variadic number of different type of collections.
*        i.e. - generalized 'std::all_of' to a variadic number of different type of collections.
* 
* @param {ExecutionPolicy, in}  execution policy
* @param {UnaryPredicate , in}  unary predicate
* @param {collections,     in}  collections whose elements (collection should be iterate-able and with underlying type 'T')
*                               or
*                               parameter pack of T objects
* @param {bool,            out} true if predicate is 'true' for all elements, false otherwise
**/
template<class ExecutionPolicy, class UnaryPredicate, class...Containers, typename std::enable_if<are_iterate_able_v<Containers...>>::type* = nullptr>
constexpr inline bool all_of(ExecutionPolicy&& xi_policy, UnaryPredicate&& xi_predicate, const Containers&... xi_containers) {
    return (std::all_of(std::forward<ExecutionPolicy>(xi_policy), xi_containers.begin(), xi_containers.end(), std::forward<UnaryPredicate>(xi_predicate)) && ...);
}

template<class UnaryPredicate, class... Containers, typename std::enable_if<are_homogeneous_pack<Containers...>>::type* = nullptr>
constexpr inline bool all_of(UnaryPredicate&& xi_predicate, const Containers&... xi_containers) {
    return (xi_predicate(xi_containers) && ...);
}

/**
* \brief check if a unary predicate returns 'true' for at least elements in a variadic number of different type of collections.
*        i.e. - generalized 'std::any_of' to a variadic number of different type of collections.
*
* @param {ExecutionPolicy, in}  execution policy
* @param {UnaryPredicate , in}  unary predicate
* @param {collections,     in}  collections whose elements (collection should be iterate-able and with underlying type 'T')
*                               or
*                               parameter pack of T objects
* @param {bool,            out} true if predicate is 'true' for all elements, false otherwise
**/
template<class ExecutionPolicy, class UnaryPredicate, class...Containers, typename std::enable_if<are_iterate_able_v<Containers...>>::type* = nullptr>
constexpr inline bool any_of(ExecutionPolicy&& xi_policy, UnaryPredicate&& xi_predicate, const Containers&... xi_containers) {
    return (std::any_of(std::forward<ExecutionPolicy>(xi_policy), xi_containers.begin(), xi_containers.end(), std::forward<UnaryPredicate>(xi_predicate)) || ...);
}

template<class UnaryPredicate, class... Containers, typename std::enable_if<are_homogeneous_pack<Containers...>>::type* = nullptr>
constexpr inline bool any_of(UnaryPredicate&& xi_predicate, const Containers&... xi_containers) {
    return (xi_predicate(xi_containers) || ...);
}

/**
* \brief check if a unary predicate returns 'true' for no elements in a variadic number of different type of collections.
*        i.e. - generalized 'std::none_of' to a variadic number of different type of collections.
*
* @param {ExecutionPolicy, in}  execution policy
* @param {UnaryPredicate , in}  unary predicate
* @param {collections,     in}  collections whose elements (collection should be iterate-able and with underlying type 'T')
*                               or
*                               parameter pack of T objects
* @param {bool,            out} true if predicate is 'true' for all elements, false otherwise
**/
template<class ExecutionPolicy, class UnaryPredicate, class...Containers, typename std::enable_if<are_iterate_able_v<Containers...>>::type* = nullptr>
constexpr inline bool none_of(ExecutionPolicy&& xi_policy, UnaryPredicate&& xi_predicate, const Containers&... xi_containers) {
    return (std::none_of(std::forward<ExecutionPolicy>(xi_policy), xi_containers.begin(), xi_containers.end(), std::forward<UnaryPredicate>(xi_predicate)) && ...);
}

template<class UnaryPredicate, class...Containers, typename std::enable_if<are_homogeneous_pack<Containers...>>::type* = nullptr>
constexpr inline bool none_of(UnaryPredicate&& xi_predicate, const Containers&... xi_containers) {
    using T = std::tuple_element_t<0, std::tuple<Containers...>>;
    constexpr std::size_t len{ sizeof...(Containers) };
    return none_of(std::execution::seq, std::forward<UnaryPredicate>(xi_predicate), std::array<T, len>{ xi_containers... });
}

/**
* \brief return the number of elements which return 'true' for a given predicate in a variadic number of different type of collections, reduction operation is performed left to right.
*        i.e. - generalized 'std::count_if' to a variadic number of different type of collections.
*
* @param {ExecutionPolicy, in}  execution policy
* @param {UnaryPredicate , in}  unary predicate
* @param {collections,     in}  collections whose elements (collection should be iterate-able and with underlying type 'T')
*                               or
*                               parameter pack of T objects
* @param {size_t,          out} reduced output
**/
template<class ExecutionPolicy, class UnaryPredicate, class...Containers, typename std::enable_if<are_iterate_able_v<Containers...>>::type* = nullptr>
constexpr inline std::size_t count_if(ExecutionPolicy&& xi_policy, UnaryPredicate&& xi_predicate, const Containers&... xi_containers) {
    return (std::count_if(std::forward<ExecutionPolicy>(xi_policy), xi_containers.begin(), xi_containers.end(), std::forward<UnaryPredicate>(xi_predicate)) + ...);
}

template<class UnaryPredicate, class...Containers, typename std::enable_if<are_homogeneous_pack<Containers...>>::type* = nullptr>
constexpr inline std::size_t count_if(UnaryPredicate&& xi_predicate, const Containers&... xi_containers) {
    using T = std::tuple_element_t<0, std::tuple<Containers...>>;
    constexpr std::size_t len{ sizeof...(Containers) };
    return count_if(std::execution::seq, std::forward<UnaryPredicate>(xi_predicate), std::array<T, len>{ xi_containers... });
}

/**
* \brief perform a unary operation on all elements in a variadic number of different type of collections.
*        i.e. - generalized 'std::for_each' to a variadic number of different type of collections.
*
* @param {ExecutionPolicy, in}  execution policy
* @param {UnaryPredicate , in}  unary operation
* @param {collections,     in}  collections whose elements (collection should be iterate-able and with underlying type 'T')
*                               or
*                               parameter pack of T objects
**/
template<class ExecutionPolicy, class UnaryPredicate, class...Containers, typename std::enable_if<are_iterate_able_v<Containers...>>::type* = nullptr>
constexpr inline void for_each(ExecutionPolicy&& xi_policy, UnaryPredicate&& xi_predicate, const Containers&... xi_containers) {
    (std::for_each(std::forward<ExecutionPolicy>(xi_policy), xi_containers.begin(), xi_containers.end(), std::forward<UnaryPredicate>(xi_predicate)), ...);
}

template<class UnaryPredicate, class...Containers, typename std::enable_if<are_homogeneous_pack<Containers...>>::type* = nullptr>
constexpr inline void for_each(UnaryPredicate&& xi_predicate, const Containers&... xi_containers) {
    using T = std::tuple_element_t<0, std::tuple<Containers...>>;
    constexpr std::size_t len{ sizeof...(Containers) };
    for_each(std::execution::seq, std::forward<UnaryPredicate>(xi_predicate), std::array<T, len>{ xi_containers... });
}

/**
* \brief assign a given value to all elements in a variadic number of different type of collections.
*        i.e. - generalized 'std::fill' to a variadic number of different type of collections.
*
* @param {ExecutionPolicy, in}  execution policy
* @param {T ,              in}  filling value
* @param {collections,     in}  collections whose elements (collection should be iterate-able and with underlying type 'T')
**/
template<class ExecutionPolicy, class T, class...Containers, typename std::enable_if<are_iterate_able_v<Containers...>>::type* = nullptr>
constexpr inline void fill(ExecutionPolicy&& xi_policy, const T xi_value, Containers&... xi_containers) {
    (std::fill(std::forward<ExecutionPolicy>(xi_policy), xi_containers.begin(), xi_containers.end(), xi_value), ...);
}

/**
* \brief removes all elements (in a variadic number of different type of collections) satisfying specific criteria.
*        i.e. - generalized 'std::remove_if' to a variadic number of different type of collections.
*
* @param {ExecutionPolicy, in}  execution policy
* @param {UnaryPredicate , in}  unary predicate
* @param {collections,     in}  collections on which the removal operation shall be performed.
*                               collection should be iterate-able and with "same" underlying type.
**/
template<class ExecutionPolicy, class UnaryPredicate, class...Containers, typename std::enable_if<are_iterate_able_v<Containers...>>::type* = nullptr>
constexpr inline void remove_if(ExecutionPolicy&& xi_policy, UnaryPredicate&& xi_predicate, Containers&... xi_containers) {
    ((void)std::remove_if(std::forward<ExecutionPolicy>(xi_policy), xi_containers.begin(), xi_containers.end(), std::forward<UnaryPredicate>(xi_predicate)), ...);
}

/**
* \brief replace all elements (in a variadic number of different type of collections) satisfying specific criteria, if so - replace them with a given value.
*        i.e. - generalized 'std::replace_if' to a variadic number of different type of collections.
*
* @param {ExecutionPolicy, in}  execution policy
* @param {UnaryPredicate , in}  unary predicate
* @param {T ,              in}  new value
* @param {collections,     in}  collections on which the replace operation shall be performed.
*                               collection should be iterate-able and with "same" underlying type.
**/
template<class ExecutionPolicy, class T, class UnaryPredicate, class...Containers, typename std::enable_if<are_iterate_able_v<Containers...>>::type* = nullptr>
constexpr inline void replace_if(ExecutionPolicy&& xi_policy, UnaryPredicate&& xi_predicate, const T xi_value, Containers&... xi_containers) {
    (std::replace_if(std::forward<ExecutionPolicy>(xi_policy), xi_containers.begin(), xi_containers.end(), std::forward<UnaryPredicate>(xi_predicate), xi_value), ...);
}

/**
* \brief Eliminates all but the first element from every consecutive group of equivalent elements (in a variadic number of different type of collections) satisfying specific criteria.
*        i.e. - generalized 'std::unique' to a variadic number of different type of collections.
*
* @param {ExecutionPolicy, in}  execution policy
* @param {BinaryPredicate, in}  binary predicate
* @param {collections,     in}  collections on which the 'unique' operation shall be performed.
*                               collection should be iterate-able and with "same" underlying type.
**/
template<class ExecutionPolicy, class BinaryPredicate, class...Containers, typename std::enable_if<are_iterate_able_v<Containers...>>::type* = nullptr>
constexpr inline void unique(ExecutionPolicy&& xi_policy, BinaryPredicate&& xi_predicate, Containers&... xi_containers) {
    ((void)std::unique(std::forward<ExecutionPolicy>(xi_policy), xi_containers.begin(), xi_containers.end(), std::forward<BinaryPredicate>(xi_predicate)), ...);
}

/**
* \brief check if elements (in a variadic number of different type of collections) are sorted given a binary comparison function.
*        i.e. - generalized 'std::is_sorted' to a variadic number of different type of collections.
*
* @param {ExecutionPolicy, in}  execution policy
* @param {BinaryPredicate, in}  binary comparison function
* @param {collections,     in}  collections on which the 'is_sorted' operation shall be performed.
*                               collection should be iterate-able and with "same" underlying type.
**/
template<class ExecutionPolicy, class Comparison, class...Containers, typename std::enable_if<are_iterate_able_v<Containers...>>::type* = nullptr>
constexpr inline bool is_sorted(ExecutionPolicy&& xi_policy, Comparison&& xi_comparison, const Containers&... xi_containers) {
    return (std::is_sorted(std::forward<ExecutionPolicy>(xi_policy), xi_containers.begin(), xi_containers.end(), std::forward<Comparison>(xi_comparison)) && ...);
}

/**
* \brief sort elements (in a variadic number of different type of collections) according to a given a binary comparison function.
*        i.e. - generalized 'std::sorted' to a variadic number of different type of collections.
*        each collection shall be sorted by itself.
*
* @param {ExecutionPolicy, in}  execution policy
* @param {Comparison,      in}  binary comparison function
* @param {collections,     in}  collections on which the 'sort' operation shall be performed.
*                               collection should be iterate-able and with "same" underlying type.
**/
template<class ExecutionPolicy, class Comparison, class...Containers, typename std::enable_if<are_iterate_able_v<Containers...>>::type* = nullptr>
constexpr inline void sort(ExecutionPolicy&& xi_policy, Comparison&& xi_comparison, Containers&... xi_containers) {
    return (std::sort(std::forward<ExecutionPolicy>(xi_policy), xi_containers.begin(), xi_containers.end(), std::forward<Comparison>(xi_comparison)), ...);
}
