/**
* Lazy string splitting and iteration.
*
* example usage:
*
*   std::string sentence{"Expressive C++ rocks"},
*               delimiter{" "};
*   std::size_t i{};
*   for (auto& word : split(sentence, delimiter)) {
*       std::cout << "word #" << std::to_string(i) << " = " <<word << "\n";
*       ++i;
*   }
*
* Dan Israel Malts
**/

#include<assert.h>
#include<iterator>
#include<string>
#include<string_view>
#include<vector>

/**
 * \brief allow lazy splinting and iteration over a the components of given string/string_view
 *
 * @param {STRING,    in} splitted/iterated object underlying type
 * @param {DELIMITER, in} set of characters (or continues STRING elements) which are used to split the string/string_view
 */
template<typename STRING, typename DELIMITER> class SplitView {
    // aliases
    using character = typename STRING::value_type;
        
    // properties
    private:
        const STRING& m_source;
        DELIMITER     m_delimiter;
    
    // methods
    public:     

        /**
        * (owning) Iterator over the split-ed component
        */
        class iterator {
            // aliases
            using difference_type   = int;
            using value_type        = std::basic_string_view<character>;
            using pointer_type      = value_type*;
            using reference         = value_type&;
            using iterator_category = std::input_iterator_tag;

            // properties
            private:
                const STRING*              m_source;
                typename STRING::size_type m_position;
                value_type                 m_curent;
                DELIMITER                  m_delimiter;
                
            // methods
            public:
        
                constexpr iterator() noexcept : m_source(nullptr), m_position(STRING::npos), m_delimiter(DELIMITER()) {}
                explicit iterator(const STRING& src, DELIMITER delimiter) : m_source(&src), m_position(0), m_delimiter(delimiter) {
                    ++*this;
                }
        
                reference operator*() noexcept {
                    assert(m_source != nullptr);
                    return m_curent;
                }
                
                pointer_type operator->() noexcept {
                    assert(m_source != nullptr);
                    return &m_curent;
                }
                
                iterator& operator++() {
                    assert(m_source != nullptr);
                    if (m_position == STRING::npos) {
                        m_source = nullptr;
                        return *this;
                    }

                    auto last_pos = m_position;
                    m_position = m_source->find(m_delimiter, m_position);
                    if (m_position != STRING::npos) {
                        m_curent = std::basic_string_view<character>(m_source->data() + last_pos, m_position - last_pos);
        
                        if constexpr (sizeof(DELIMITER) == sizeof(character)) {
                            ++m_position;
                        } else {
                            m_position += m_delimiter.size();
                        }
                        
                        return *this;
                    }
                    
                    m_curent = std::basic_string_view<character>(m_source->data() + last_pos, m_source->size() - last_pos);             
                    return *this;
                }
                
                iterator operator++(int) {
                    iterator temp(*this);
                    ++*this;
                    return temp;
                }
        
                bool operator==(const iterator& rhs) const noexcept {
                    return (m_source == rhs.m_source) && (m_position == rhs.m_position);
                }
                bool operator!=(const iterator& rhs) const noexcept {
                    return !operator==(rhs);
                }
        };

        /**
        * Constructor.
        *
        * @param src        the source input to be split
        * @param delimiter  delimiter used to split \a src; its type should be
        *                   the same as that of \a src, or its character type
        */
        constexpr explicit SplitView(const STRING& src, DELIMITER delimiter) noexcept : m_source(src), m_delimiter(delimiter) {}
    
        iterator begin() const {
            return iterator(m_source, m_delimiter);
        }
        
        constexpr iterator end() const noexcept {
            return iterator();
        }
    
        /** Converts the view to a string vector. */
        std::vector<std::basic_string<character>> to_vector() const {
            std::vector<std::basic_string<character>> result;
            for (const auto& sv : *this) result.emplace_back(sv);
            return result;
        }
        
        /** Converts the view to a string_view vector. **/
        std::vector<std::basic_string_view<character>> to_vector_sv() const {
            std::vector<std::basic_string_view<character>> result;
            for (const auto& sv : *this) result.push_back(sv);
            return result;
        }
};

/**
 * Splits a string (or string_view) into lazy views.  The source input shall
 * remain unchanged when the generated SplitView is used in anyway.
 *
 * @param src        the source input to be split
 * @param delimiter  delimiter used to split \a src; its type should be
 *                   the same as that of \a src, or its character type
 */
template<typename STRING, typename DELIMITER> constexpr SplitView<STRING, DELIMITER> split(const STRING& src, DELIMITER delimiter) noexcept {
    return SplitView<STRING, DELIMITER>(src, delimiter);
}
