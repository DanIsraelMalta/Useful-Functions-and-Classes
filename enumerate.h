/**
* Dan Israel Malta
**/
#include <tuple>

/**
* \brief allow enumeration of a list of things
* \usage example #1:
*        std::vector<std::tuple<ThingA, ThingB>> things;
*        for (auto [a, b] : things) {
*           // a gets the ThingA and b gets the ThingB from each tuple
*        }
*
*        example #2:
*        std::vector<Thing> things;
*        for (auto [i, thing] : enumerate(things)) {
*           // i gets the index and thing gets the Thing in each iteration
*        }
*
* @param {T, in} enumeratable object
**/
template<typename T, typename TIter = decltype(std::begin(std::declval<T>())), typename = decltype(std::end(std::declval<T>()))>
constexpr auto enumerate(T && iterable) {
    // iterator
    struct iterator {
        std::size_t i;
        TIter iter;

        bool operator != (const iterator & other) const { return iter != other.iter; }
        void operator ++ ()                             { ++i; ++iter; }
        auto operator *  ()                       const { return std::tie(i, *iter); }
    };

    // wrapper over enumerable object
    struct iterable_wrapper {
        T iterable;

        auto begin() { return iterator{ 0, std::begin(iterable) }; }
        auto end()   { return iterator{ 0, std::end(iterable) };   }
    };

    // output
    return iterable_wrapper{ std::forward<T>(iterable) };
}
