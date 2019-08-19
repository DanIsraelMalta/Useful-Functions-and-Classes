/**
* FRP (Functional Reactive Programing) is all about (asynchronously & lazy) processing data 
* by building basic functions which can be extended and combined into more complex operations,
* in a functional manner.
* This header only library allows the user to do just that in a simple yet efficient manner.
* 
* ```c
*    
* int result{}; // value which is the result of a reactive chain output.
* 
* // reactive components
* auto square    = React::map(    [](int v)          { return v * v; }                    );
* auto keep_even = React::filter( [](int v)          { return (v % 2) == 0; }             );
* auto sum       = React::fold(0, [](int acu, int v) { return (acu + v); }                );
* auto output    = React::map(    [&result](int v)   { std::cout << v << "\n"; result = v; return 0; });
* 
* // collection to be processed
* std::vector<int> values = { 1, 3, 4, 2, 7, 6, 19, -7 };
* 
* // sum of squares of even integers
* auto sum_of_even_squares = square | keep_even | sum | output;
* values >> sum_of_even_squares;
* 
* // sum of of all integers
* auto sum_of_all = sum | output;
* values >> sum_of_all;	
* 
* // write all the even integers to console
* auto cout_even = keep_even | output;
* values >> cout_even;
* ```
* 
* Dan Israel Malta
**/
#pragma once
#include<type_traits>

/**
* Reactive procssing library.
**/
namespace React {

    /**
    * \brief reactive component interface (CRTP inhertiance style)
    **/
    template<class T> struct ReactiveComponent {
        void onNext() { static_cast<T*>(this).onNext(); } // process data
        void onEnd()  { static_cast<T*>(this).onEnd();  } // handle data termination
    };

    /**
    * type traits to see if the methods 'onNext' or 'onEnd' are included in a class
    **/
    namespace {
        template<typename T, typename = void> struct has_onNext : std::false_type { };
        template<typename T> struct has_onNext<T, decltype(std::declval<T>().onNext, void())> : std::true_type { };

        template<typename T, typename = void> struct has_onEnd : std::false_type { };
        template<typename T> struct has_onEnd<T, decltype(std::declval<T>().onEnd, void())> : std::true_type { };
    };

    /**
    * \brief terminal element in reactive chain
    *
    * @param {in} processed value
    **/
    template<typename T> struct Last : public ReactiveComponent<Last<T>> {
        void onNext(const T& v) {}
        void onEnd()            {}
    };

    /**
    * \brief filter data stream using a given predicate
    *
    * @param {in} predicate (a function)
    * @param {in} next element in stream
    **/
    template<typename P, typename N> struct Filter : public ReactiveComponent<Filter<P, N>> {
        // properties
        P m_predicate;
        N m_next;

        // constructor
        explicit constexpr Filter(const P& p, N n) : m_predicate(p), m_next(n) {
            static_assert(!std::is_function<decltype(p)>::value, "Filter<P,N> - P is not a function.");
        }

        // reactive interface
        template<typename T> constexpr void onNext(const T& v) {
            if (m_predicate(v)) m_next.onNext(v);
        }

        constexpr void onEnd() {
            m_next.onEnd();
        }
    };

    // apply filter
    template<typename F>             constexpr auto filter(const F& f)      { static_assert(!std::is_function<decltype(f)>::value, "filter: F is not a function."); return Filter<F, Last<int>>(f, Last<int>()); }
    template<typename F, typename N> constexpr auto filter(const F& f, N n) { static_assert(!std::is_function<decltype(f)>::value, "filter: F is not a function."); return Filter<F, N>(f, n); }

    // concatenate componentes
    template<typename P, typename N>             constexpr auto operator | (Filter<P, Last<int>> f, N n) { return filter(f.m_predicate, n); }
    template<typename P, typename X, typename N> constexpr auto operator | (Filter<P, X> f,         N n) { return filter(f.m_predicate, f.m_next | n); }

    /**
    * \brief apply a function on stream
    *
    * @param {in} function
    * @param {in} next element in stream
    **/
    template<typename F, typename N> struct Map : public ReactiveComponent<Map<F, N>> {
        // properties
        F m_function;
        N m_next;

        // constructor
        explicit constexpr Map(F f, N n) : m_function(f), m_next(n) {
            static_assert(!std::is_function<decltype(f)>::value, "Map<F,N> - F is not a function.");
        }

        // reactive interface
        template<typename T> constexpr void onNext(const T& v) {
            m_next.onNext(m_function(v));
        }

        constexpr void onEnd() {
            m_next.onEnd();
        }
    };

    // apply map
    template<typename F>             constexpr auto map(F f)      { static_assert(!std::is_function<decltype(f)>::value, "map: F is not a function."); return Map<F, Last<int>>(f, Last<int>()); }
    template<typename F, typename N> constexpr auto map(F f, N n) { static_assert(!std::is_function<decltype(f)>::value, "map: F is not a function."); return Map<F, N>(f, n); }

    // concatenate components
    template<typename F, typename N>             constexpr auto operator | (Map<F, Last<int>> m, N n) { return map(m.m_function, n); }
    template<typename F, typename X, typename N> constexpr auto operator | (Map<F, X> m,         N n) { return map(m.m_function, m.m_next | n); }

    /**
    * \brief (left) fold a stream to one value
    *
    * @param {in} value
    * @param {in} folding function
    * @param {in} next element in stream
    **/
    template<typename T, typename F, typename N> struct Fold : public ReactiveComponent<Fold<T, F, N>> {
        // properties
        F m_function;
        N m_next;
        T m_accumulate;

        // constructor
        explicit constexpr Fold(T xi_accumulate, const F& f, N n) : m_accumulate(xi_accumulate), m_function(f), m_next(n) {
            static_assert(!std::is_function<decltype(f)>::value, "Fold<T,F,N> - F is not a function.");
        }

        // reactive interfacce
        constexpr void onNext(const T& v) {
            m_accumulate = m_function(m_accumulate, v);
        }

        constexpr void onEnd() {
            m_next.onNext(m_accumulate);
            m_next.onEnd();
        }
    };

    // apply reduction
    template<typename T, typename F>             constexpr auto fold(T xi_accumulate, const F& f)      { static_assert(!std::is_function<decltype(f)>::value, "fold: F is not a function."); return Fold<T, F, Last<int>>(xi_accumulate, f, Last<int>()); }
    template<typename T, typename F, typename N> constexpr auto fold(T xi_accumulate, const F& f, N n) { static_assert(!std::is_function<decltype(f)>::value, "fold: F is not a function."); return Fold<T, F, N>(xi_accumulate, f, n); }

    // concatenate componentes
    template<typename T, typename F, typename N>             constexpr auto operator | (Fold<T, F, Last<int>> r, N n) { return fold(r.m_accumulate, r.m_function, n); }
    template<typename T, typename F, typename X, typename N> constexpr auto operator | (Fold<T, F, X> r,         N n) { return fold(r.m_accumulate, r.m_function, r.m_next | n); }
}

/**
* \brief stream values from a collection to a concatenated chain of reactive components
*
* @param {in} collection
* @param {in} reactive components
**/
template<typename COLLECTION, typename REACTIVE> constexpr void operator >> (COLLECTION& xi_collection, REACTIVE& xi_reactive) {
    static_assert(std::is_class<REACTIVE>::value, "Collection >> Reacitve component: given component is not a class.");
    static_assert(!React::has_onNext<REACTIVE>::value, "Collection >> Reacitve component: given component does not implement 'onNext' method.");
    static_assert(!React::has_onEnd<REACTIVE>::value, "Collection >> Reacitve component: given component does not implement 'onEnd' method.");

    for (auto& c : xi_collection) xi_reactive.onNext(c);
    xi_reactive.onEnd();
};
