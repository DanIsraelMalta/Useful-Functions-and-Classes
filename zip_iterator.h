/**
* ZIP iterator
*
* Dan Israel Malta
**/
#include <utility>
#include <tuple>
#include <iterator>
#include <functional>

namespace ZipIteratorNS {
    // zip iterator traits
    template<typename... Iterators> struct zip_iterator_traits {
        using iterators         = std::tuple<Iterators...>;
        using difference_type   = std::ptrdiff_t;
        using value_type        = std::tuple<typename Iterators::value_type...>;
        using pointer           = std::tuple<typename Iterators::pointer...>;
        using reference         = std::tuple<typename Iterators::reference...>;
        using iterator_category = typename std::iterator_traits<typename std::tuple_element<0, iterators>::type>::iterator_category;
    };
 
    // zip iterator
    template<typename... Iterators> class zip_iterator {
        private:
            using self_type = zip_iterator<Iterators...>;
            using traits    = zip_iterator_traits<Iterators...>;
            using iterators = typename traits::iterators;

        public:
            using difference_type   = typename traits::difference_type;
            using value_type        = typename traits::value_type;
            using pointer           = typename traits::pointer;
            using reference         = typename traits::reference;
            using iterator_category = typename traits::iterator_category;
            
        private:
            template<size_t... Is> struct m_indexSequence;
            
        public:
            template<typename... Iterators2> zip_iterator(const Iterators2&... iterators) : m_content{ iterators... } {}
            
        public:
            reference operator*() {
                return reference_helper(std::make_index_sequence<sizeof...(Iterators)>());
            }
            
            pointer operator->() {
                pointer result;
                set_pointer<0>(result);
                return result;
            }
            
            self_type& operator++() {
                increment<0>();
                return *this;
            }
            
            self_type operator++(int) {
                self_type old{ *this };
                increment<0>();
                return old;
            }
            
            self_type& operator--() {
                decrement<0>();
                return *this;
            }
            
            self_type operator--(int) {
                self_type old{ *this };
                decrement<0>();
                return old;
            }
            
            difference_type operator-(const self_type& aOther) const {
                return std::get<0>(contents()) - std::get<0>(aOther.contents());
            }
            
            self_type operator+(difference_type aAmount) const {
                self_type result{ *this };
                result.add<0>(aAmount);
                return result;
            }
            
            self_type operator-(difference_type aAmount) const {
                self_type result{ *this };
                result.subtract<0>(aAmount);
                return result;
            }
            
            bool operator<(const self_type& aOther) const {
                return std::get<0>(contents()) < std::get<0>(aOther.contents());
            }
            
            bool operator==(const self_type& aOther) const {
                return std::get<0>(contents()) == std::get<0>(aOther.contents());
            }
            
            bool operator!=(const self_type& aOther) const {
                return std::get<0>(contents()) != std::get<0>(aOther.contents());
            }
            
        public:
            const iterators& contents() const {
                return m_content;
            }
            
        private:
            template<std::size_t Index> void increment() {
                ++std::get<Index>(contents());
                if constexpr (Index < std::tuple_size<iterators>::value - 1) {
                    increment<Index + 1>();
                }
            }
            
            template<std::size_t Index> void decrement() {
                --std::get<Index>(contents());
                if constexpr (Index < std::tuple_size<iterators>::value - 1) {
                    decrement<Index + 1>();
                }
            }
            
            template<std::size_t Index> void add(difference_type aAmount) {
                std::get<Index>(contents()) += aAmount;
                if constexpr (Index < std::tuple_size<iterators>::value - 1) {
                    add<Index + 1>(aAmount);
                }
            }
            
            template<std::size_t Index> void subtract(difference_type aAmount) {
                std::get<Index>(contents()) -= aAmount;
                if constexpr (Index < std::tuple_size<iterators>::value - 1) {
                    subtract<Index + 1>(aAmount);
                }
            }
            
            template<std::size_t... Is> reference reference_helper(std::m_indexSequence<Is...>) {
                return reference_helper_2(*std::get<Is>(contents())...);
            }
            
            template<typename... References> reference reference_helper_2(References&... aReferences) {
                return reference{ aReferences... };
            }
            
            template<std::size_t Index> void set_pointer(pointer& aResult) {
                std::get<Index>(aResult) = &std::get<Index>(contents());
                if constexpr (Index < std::tuple_size<iterators>::value - 1) {
                    set_pointer<Index + 1>(aResult);
                }
            }
            
            iterators& contents() {
                return m_content;
            }
            
        private:
            iterators m_content;
    };

    template<typename... Iterators> inline zip_iterator<Iterators...> make_zip_iterator(Iterators&&... iterators) {
        return zip_iterator<Iterators...>(std::forward<Iterators>(iterators)...);
    }
}
 
 // add zip iterator to 'std' namespace
 namespace std {
     template<typename... Iterators> struct iterator_traits<ZipIteratorNS::zip_iterator<Iterators...>> {
         using difference_type   = typename ZipIteratorNS::zip_iterator_traits<Iterators...>::difference_type;
         using value_type        = typename ZipIteratorNS::zip_iterator_traits<Iterators...>::value_type;
         using pointer           = typename ZipIteratorNS::zip_iterator_traits<Iterators...>::pointer;
         using reference         = typename ZipIteratorNS::zip_iterator_traits<Iterators...>::reference;
         using iterator_category = typename ZipIteratorNS::zip_iterator_traits<Iterators...>::iterator_category;
     };
     
     template<typename... Iterators> inline void 
     iter_swap(ZipIteratorNS::zip_iterator<Iterators...> a, ZipIteratorNS::zip_iterator<Iterators...> b) {
         auto temp = *a;
         *a = *b;
         *b = temp;
     }
 }
