/**
* Define a container using AoS (array of struct) syntax (both construction and iteration)
* but underneath it is an SoA (struct of array) object which allow iterating in SoA style.
*
* Example usage:
*
* ```c
*
* #include "ContainerSOA.h"
* #include <array>
* 
* //define a layout convertible (AOS/SOA) structure
* struct Point : Layout::Convertable<Point, float, float, float> {
*   // properties
*   float x, y, z;
* 
*   // indices of properties, as they are stored out in memory
*   constexpr static std::size_t xid{}, yid{ 1 }, zid{ 2 };
* 
*   // constructor
*   explicit constexpr Point() : x(0.0f), y(0.0f), z(0.0f) {}
* 
*   // constructor and casting required by Layout
*   explicit constexpr Point(const soa_value_type xi_point) noexcept : x(std::get<xid>(xi_point)), y(std::get<yid>(xi_point)), z(std::get<zid>(xi_point)) {}
*   constexpr operator soa_value_type() const { return { x, y, z }; };
* };
* 
* 
* // SOA container underlying container
* template<typename T> using array200 = std::array<T, 200>;
* 
* // main function
* int main() {
*   // SOA container of 'Point'
*   Layout::Container<array200, Point> vertexBuffer;
* 
*   // in-order-iterate and change only the 'x' property in 'Point' struct (SOA style iteration)
*   int i{};
*   for (auto& x : Layout::array_view<Point::xid, decltype(vertexBuffer)>(vertexBuffer)) {
*       x += static_cast<float>(i);
*       ++i;
*   }
* 
*   // in-order-iterate and print all 'Points' objects (SOA style iteration)
*   i = 0;
*   for (const Point& p : Layout::struct_view(vertexBuffer)) {
*       printf("Point {%d}: x: %f, y: %f, z: %f\n", i, p.x, p.y, p.z);
*       ++i;
*   }
* 
*   // extract a continues array holding only y coordinate
*   auto _y = Layout::array_view<Point::yid, decltype(vertexBuffer)>(vertexBuffer);
*
*   return 0;
* }
*
* ```
*
* Dan Israel Malta
**/
#pragma once
#include <tuple>
#include <functional>
#include <utility>

/**
* tuple handling utilities 
**/
namespace tuple_utils {

    /**
    * implementation detail of the various utilities
    **/
    namespace impl {
        template<typename T, typename F, size_t... Is>
        constexpr auto map(const T& xi_tuple, const F& xi_function, std::index_sequence<Is...>) {
            return std::make_tuple(std::invoke(xi_function, std::get<Is>(xi_tuple))...);
        }

        template<size_t I, typename F, typename T>
        constexpr void foreach(T& xi_tuple, const F& xi_function) {
            std::invoke(xi_function, std::get<I>(xi_tuple));
            if constexpr (I > 0) {
                foreach<I - 1>(xi_tuple, xi_function);
            }
        }
    }

    /**
    * \brief apply a function on each element in a tuple, and return the outcome
    * 
    * @param {in}  tuple holding the elements on which function should be applied
    * @param {in}  function to applt
    * @param {out} tuple after applying the function
    **/
    template<typename T, typename F, typename Is = std::make_index_sequence<std::tuple_size_v<T>>>
    constexpr auto map(const T& xi_tuple, const F& xi_function) {
        return impl::map(xi_tuple, xi_function, Is{});
    }

    /**
    * \brief apply a function on each element in a tuple
    *
    * @param {in}  tuple holding the elements on which function should be applied
    * @param {in}  function to applt
    **/
    template<typename F, typename... Ts> constexpr void foreach(std::tuple<Ts...>& xi_tuple, const F& xi_function) {
        constexpr std::size_t last{ sizeof...(Ts) - 1 };
	static_assert(last > 0, "tuple_utils::foreach can not operate on an empty tuple.");
        impl::foreach<last>(xi_tuple, xi_function);
    }
}

/**
* a set of utilities that allow one to to handle (iterate/extract/interface) a container
* both in 'struct of array' (SOA) and 'array of struct' (AOS) manners.
**/
namespace Layout {

    /**
    * \brief define an AOS<->SOA convertible struct layout
    * 
    * @param {in} struct whose layout shall be convertible
    * @param {in} variadic list of types, as they are defined in the struct  above
    **/
    template<typename S, typename... Ts> class Convertable {
        public:
            using soa_reference_type   = std::tuple<Ts&...>;
            using soa_value_type       = std::tuple<Ts...>;
            using soa_convertable_type = S;

            template<template<class...> class raw_container_type> using soa_container_type      = std::tuple<raw_container_type<Ts>...>;
            template<template<class...> class raw_container_type> using soa_iterator_type       = std::tuple<typename raw_container_type<Ts>::iterator...>;
            template<template<class...> class raw_container_type> using soa_const_iterator_type = std::tuple<typename raw_container_type<Ts>::const_iterator...>;
    };

    /**
    * \brief iterate a SOA container struct-by-struct style (AOS)
    *
    * @param {in} struct which is held in a given container, and according to it we iterate
    **/
    template<typename T> class struct_iterator {
        public:
            using iterator_type = typename T::const_array_iterator_type;
            using value_type    = typename T::base_type;

            explicit constexpr struct_iterator(const iterator_type& iterators) : iterators(iterators) {}

            constexpr value_type operator*() const {
                return value_type(tuple_utils::map(iterators, [](auto& t) { return *t; }));
            }

            constexpr struct_iterator& operator++() {
                tuple_utils::foreach(iterators, [](auto& t) { ++t; });
                return *this;
            }

            constexpr struct_iterator& operator--() {
                tuple_utils::foreach(iterators, [](auto& t) { --t; });
                return *this;
            }

            constexpr size_t operator-(const struct_iterator& other) const {
                return std::distance(std::get<0>(other.iterators), std::get<0>(iterators));
            }

            constexpr bool operator==(const struct_iterator& other) const {
                return iterators == other.iterators;
            }

            constexpr bool operator!=(const struct_iterator& other) const {
                return iterators != other.iterators;
            }

        private:
            iterator_type iterators;
    };

    /**
    * \brief view a SOA container, in AOS ("array of struct") style
    *
    * @param {in} container which holds the struct which we view
    **/
    template<typename T> class struct_view {
        public:
            using multi_container_type = typename T::multi_container_type;
            using iterator_type        = typename T::struct_iterator_type;

            const multi_container_type& parent_containers;

            explicit constexpr struct_view(const T& t) : parent_containers(t.containers) {}

            iterator_type begin() const {
                return iterator_type(
                    tuple_utils::map(
                        parent_containers,
                        [](const auto& t) {
                            return std::begin(t);
                        }
                    )
                );
            }

            iterator_type end() const {
                return iterator_type(
                    tuple_utils::map(
                        parent_containers,
                        [](const auto& t) {
                            return std::end(t);
                        }
                    )
                );
            }
    };

    /**
    * \brief view a SOA container, in SOA ("struct of array") style
    * 
    * @param {in} index of element to view in the structure (i.e - as it was defined Convertable<structr, ...{here}...>
    * @param {in} type of container which holds the struct which we view
    **/
    template<size_t I, typename T> class array_view {
        public:
            using container_type = typename std::tuple_element_t<I, typename T::multi_container_type>;

            explicit constexpr array_view(T& t) : container(std::get<I>(t.containers)) {}

            auto begin() const { return std::begin(container); }
            auto end()   const { return std::end(container);   }
            auto begin()       { return std::begin(container); }
            auto end()         { return std::end(container);   }

        private:
            container_type& container;
    };

    /**
    * \brief SOA ("struct of array") container which allow iterating in AOS ("array of struct") manner
    * 
    * @param {in} underlying container
    * @param {in} struct held by container
    **/
    template<template<class...> class raw_container_type, typename T > class Container {
        public:
            using multi_container_type      = typename T::template soa_container_type<raw_container_type>;
            using array_iterator_type       = typename T::template soa_iterator_type<raw_container_type>;
            using const_array_iterator_type = typename T::template soa_const_iterator_type<raw_container_type>;
            using struct_iterator_type      = struct_iterator<Container>;
            using value_type                = typename T::soa_value_type;
            using base_type                 = typename T::soa_convertable_type;

            explicit constexpr Container() : containers() {}

            size_t size() const { return std::get<0>(containers).size(); }

            template<size_t I> auto& iterate_array() { return std::get<I>(containers); }

            template<size_t col> auto& access(const size_t& row) { return iterate_array<col>()[row]; }

            friend class struct_view<Container>;
            template<size_t, typename> friend class array_view;

        protected:
            multi_container_type containers;
    };
}
