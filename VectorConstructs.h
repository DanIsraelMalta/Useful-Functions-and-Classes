/**
* Various (explicit) vectorized construct 
* requires SSE4.1 (or above).
* only tested with MSVC-x64
*
* currently has:
* > vec4x32f - 4x32bit floating point vector.
* > vec16x8i - 16x8bit integer vector.
* > vec128i  - 128 booleans vector
*
* Dan I. Malta
**/
#pragma once

#include<type_traits>
#include<cstdint>
#include<array>
#include<iostream>
#include<optional>
#include<algorithm>
#include<numeric>
#include<assert.h>

#include<intrin.h>
#include<xmmintrin.h>
#include<emmintrin.h>
#include<smmintrin.h>

namespace Vector {

    //
    // forward deceleration
    //

    // vector holding 4x32bit floating numbers
    struct alignas(16) vec4x32f;

    // vector holding 16x8bit integer numbers
    struct alignas(16) vec16x8i;

    // vector holding 128 booleans
    struct alignas(16) vec128i;

    //
    // concepts
    //
    namespace {

        // test if an object is iterate-able
        template<typename T, typename = void> struct is_iterate_able : std::false_type {};
        template<typename T>                  struct is_iterate_able<T, std::void_t<decltype(std::begin(std::declval<T>())),
                                                                                    decltype(std::end(std::declval<T>()))>> : std::true_type  {};
        template<typename T> inline constexpr bool is_iterate_able_v = is_iterate_able<T>::value;

        // concept to check if an object is a vec4x32f
        template<typename> struct is_vec4x32f           : public std::false_type {};
        template<>         struct is_vec4x32f<vec4x32f> : public std::true_type  {};
        template<typename T> inline constexpr bool is_vec4x32f_v = is_vec4x32f<T>::value;

        // concept to check if an object is a vec16x8i
        template<typename> struct is_vec16x8i            : public std::false_type {};
        template<>         struct is_vec16x8i<vec16x8i> : public std::true_type  {};
        template<typename T> inline constexpr bool is_vec16x8i_v = is_vec16x8i<T>::value;

        // concept to check if an object is a vec128i
        template<typename> struct is_vec128i          : public std::false_type {};
        template<>         struct is_vec128i<vec128i> : public std::true_type  {};
        template<typename T> inline constexpr bool is_vec128i_v = is_vec128i<T>::value;

        // concept to check if an object is a vector
        template<typename T> inline constexpr bool is_vec_v = is_vec128i_v<T> || is_vec16x8i_v<T> || is_vec4x32f<T>;

        // how many elements does an object hold
        template<typename> struct Length           { static const std::size_t value{};      };
        template<>         struct Length<vec4x32f> { static const std::size_t value{ 4 };   };
        template<>         struct Length<vec16x8i> { static const std::size_t value{ 16 };  };
        template<>         struct Length<vec128i>  { static const std::size_t value{ 128 }; };
        template<typename T> static inline constexpr std::size_t Length_v = Length<T>::value;

        template<typename T, typename std::enable_if<is_vec_v<T>>::type* = nullptr> 
        constexpr inline std::size_t getVectorLength(const T& vector) { 
            return Length_v<T>; 
        }

        // type of underlying element which a vector object holds
        template<typename> struct UnderlyingType           { using type = void;         };
        template<>         struct UnderlyingType<vec4x32f> { using type = float;        };
        template<>         struct UnderlyingType<vec16x8i> { using type = std::uint8_t; };
        template<>         struct UnderlyingType<vec128i>  { using type = bool;         };

        // size of each element in a given vector object [bits]
        template<typename> struct ElementSize           { const std::size_t value{};                                         };
        template<>         struct ElementSize<vec4x32f> { const std::size_t value{ sizeof(UnderlyingType<vec4x32f>::type) }; };
        template<>         struct ElementSize<vec16x8i> { const std::size_t value{ sizeof(UnderlyingType<vec16x8i>::type) }; };
        template<>         struct ElementSize<vec128i>  { const std::size_t value{ sizeof(UnderlyingType<vec128i>::type)  }; };

        template<typename T, typename std::enable_if<is_vec_v<T>>::type * = nullptr>
        constexpr inline std::size_t getVectorElementSize(const T& vector) {
            return ElementSize<T>::value;
        }

        // test that all integral template parameters are smaller then a given value
        template<std::size_t MAX, std::size_t... Is> struct are_in_range {
            static constexpr bool value{ ((Is < MAX) && ...) };
        };
        template<std::size_t MAX, std::size_t... Is> static inline constexpr bool are_in_range_v = are_in_range<MAX, Is...>::value;
    };

    /**
    * vec4x32f implementation
    **/
    struct alignas(16) vec4x32f {
        // value constructor
        template<typename U, typename std::enable_if<std::is_arithmetic_v<U>>::type* = nullptr>
        vec4x32f(const U x, const U y, const U z, const U w) : m_vec(_mm_set_ps(static_cast<float>(w), static_cast<float>(z),
                                                                                static_cast<float>(y), static_cast<float>(x))) {}
        template<typename U, typename std::enable_if<std::is_arithmetic_v<U>>::type* = nullptr>
        vec4x32f(const U value)                              : m_vec(_mm_set_ps1(static_cast<float>(value)))                   {}
        vec4x32f(const __m128& v)                            : m_vec(v)                                                        {}
        vec4x32f(__m128&& v)                                 : m_vec(std::move(v))                                             {}
        vec4x32f()                                           : vec4x32f(0.0f)                                                  {}

        // "collection" constructor
        template<typename Collection, typename std::enable_if<is_iterate_able_v<Collection>>::type* = nullptr>
        vec4x32f(const Collection& col) {
            alignas(16) std::array<float, 4> arr;
            std::for_each(std::begin(col), std::end(col), [&arr, i = 0](const auto& c) mutable { arr[i] = static_cast<float>(c); ++i; });
            m_vec = _mm_load_ps(arr.data());
        }

        template<typename Collection, typename std::enable_if<is_iterate_able_v<Collection>>::type* = nullptr>
        vec4x32f(Collection&& col) {
            alignas(16) std::array<float, 4> arr;
            std::for_each(std::begin(col), std::end(col), [&arr, i = 0](auto&& c) mutable { arr[i] = static_cast<float>(std::move(c)); ++i; });
            m_vec = _mm_load_ps(arr.data());
        }

        // "load" constructor
        vec4x32f(const void* pointer) : m_vec(_mm_load_ps((const float*)pointer)) {}

        // copy semantics
        vec4x32f(const vec4x32f& other) : m_vec(other.m_vec) {}
        vec4x32f& operator=(const vec4x32f& other) {
            if (this != &other) {
                m_vec = other.m_vec;
            }
            return *this;
        }

        // move semantics
        vec4x32f(vec4x32f&& other) noexcept : m_vec(std::exchange(other.m_vec, _mm_set_ps1(0.0f))) {}
        vec4x32f& operator=(vec4x32f&& other) noexcept {
            if (this != &other) {
                m_vec = std::exchange(other.m_vec, _mm_set_ps1(0.0f));
            }
            return *this;
        }

        // value assignment
        template<typename U, typename std::enable_if<std::is_arithmetic_v<U>>::type* = nullptr>
        vec4x32f& operator=(const U other) {
            m_vec = _mm_set_ps1(static_cast<float>(other));
            return *this;
        }

        // "collection" assignment
        template<typename Collection, typename std::enable_if<is_iterate_able_v<Collection>>::type* = nullptr>
        vec4x32f& operator=(Collection&& other) {
            vec4x32f temp(other);
            m_vec = std::exchange(temp.m_vec, _mm_set_ps1(0.0f));
            return *this;
        }

        template<typename Collection, typename std::enable_if<is_iterate_able_v<Collection>>::type* = nullptr>
        vec4x32f& operator=(const Collection& other) {
            if (this != &other) {
                vec4x32f temp(other);
                m_vec = temp;
            }
            return *this;
        }

        // casting to standard 128bit register
        operator __m128() const { return m_vec; }
        operator __m128()       { return m_vec; }
        operator __m128i() const { return _mm_cvtps_epi32(m_vec); }
        operator __m128i()       { return _mm_cvtps_epi32(m_vec); }

        // set elements
        template<typename U, typename std::enable_if<std::is_arithmetic_v<U>>::type* = nullptr>
        inline void set(const U x, const U y, const U z, const U w) { 
            m_vec = _mm_set_ps(static_cast<float>(w), static_cast<float>(z),
                               static_cast<float>(y), static_cast<float>(x)); 
        }

        template<typename U, typename std::enable_if<std::is_arithmetic_v<U>>::type* = nullptr>
        inline void setX(const U x) { m_vec = _mm_blend_ps(m_vec, _mm_set_ps1(static_cast<float>(x)), 1); }

        template<typename U, typename std::enable_if<std::is_arithmetic_v<U>>::type* = nullptr>
        inline void setY(const U y) { m_vec = _mm_blend_ps(m_vec, _mm_set_ps1(static_cast<float>(y)), 2); }

        template<typename U, typename std::enable_if<std::is_arithmetic_v<U>>::type* = nullptr>
        inline void setZ(const U z) { m_vec = _mm_blend_ps(m_vec, _mm_set_ps1(static_cast<float>(z)), 4); }

        template<typename U, typename std::enable_if<std::is_arithmetic_v<U>>::type* = nullptr>
        inline void setW(const U w) { m_vec = _mm_blend_ps(m_vec, _mm_set_ps1(static_cast<float>(w)), 8); }

        // get specific element
        inline float x()       { __m128 vec = _mm_shuffle_ps(m_vec, m_vec, _MM_SHUFFLE(0, 0, 0, 0)); return _mm_cvtss_f32(vec); }
        inline float y()       { __m128 vec = _mm_shuffle_ps(m_vec, m_vec, _MM_SHUFFLE(1, 1, 1, 1)); return _mm_cvtss_f32(vec); }
        inline float z()       { __m128 vec = _mm_shuffle_ps(m_vec, m_vec, _MM_SHUFFLE(2, 2, 2, 2)); return _mm_cvtss_f32(vec); }
        inline float w()       { __m128 vec = _mm_shuffle_ps(m_vec, m_vec, _MM_SHUFFLE(3, 3, 3, 3)); return _mm_cvtss_f32(vec); }
        inline float x() const { __m128 vec = _mm_shuffle_ps(m_vec, m_vec, _MM_SHUFFLE(0, 0, 0, 0)); return _mm_cvtss_f32(vec); }
        inline float y() const { __m128 vec = _mm_shuffle_ps(m_vec, m_vec, _MM_SHUFFLE(1, 1, 1, 1)); return _mm_cvtss_f32(vec); }
        inline float z() const { __m128 vec = _mm_shuffle_ps(m_vec, m_vec, _MM_SHUFFLE(2, 2, 2, 2)); return _mm_cvtss_f32(vec); }
        inline float w() const { __m128 vec = _mm_shuffle_ps(m_vec, m_vec, _MM_SHUFFLE(3, 3, 3, 3)); return _mm_cvtss_f32(vec); }

        // compound numerical operator overloading
        inline vec4x32f& operator+=(const vec4x32f& b) { m_vec = _mm_add_ps(m_vec, b); return *this; }
        inline vec4x32f& operator-=(const vec4x32f& b) { m_vec = _mm_sub_ps(m_vec, b); return *this; }
        inline vec4x32f& operator*=(const vec4x32f& b) { m_vec = _mm_mul_ps(m_vec, b); return *this; }
        inline vec4x32f& operator/=(const vec4x32f& b) { m_vec = _mm_div_ps(m_vec, b); return *this; }

        template<typename U, typename std::enable_if<std::is_arithmetic_v<U>>::type* = nullptr>
        inline vec4x32f& operator+=(U b) { m_vec = _mm_add_ps(m_vec, _mm_set1_ps(static_cast<float>(b))); return *this; }

        template<typename U, typename std::enable_if<std::is_arithmetic_v<U>>::type* = nullptr>
        inline vec4x32f& operator-=(U b) { m_vec = _mm_sub_ps(m_vec, _mm_set1_ps(static_cast<float>(b))); return *this; }

        template<typename U, typename std::enable_if<std::is_arithmetic_v<U>>::type* = nullptr>
        inline vec4x32f& operator*=(U b) { m_vec = _mm_mul_ps(m_vec, _mm_set1_ps(static_cast<float>(b))); return *this; }

        template<typename U, typename std::enable_if<std::is_arithmetic_v<U>>::type* = nullptr>
        inline vec4x32f& operator/=(U b) { m_vec = _mm_div_ps(m_vec, _mm_set1_ps(static_cast<float>(b))); return *this; }

        // relational operator overloading
        inline bool operator==(const vec4x32f& b) const { return (((_mm_movemask_ps(_mm_cmpeq_ps(m_vec, b))) & 0x7) == 0x7); }
        inline bool operator!=(const vec4x32f& b) const { return !(*this == b); }
        inline bool operator> (const vec4x32f& b) const { return (((_mm_movemask_ps(_mm_cmpgt_ps(m_vec, b))) & 0x7) == 0x7); }
        inline bool operator>=(const vec4x32f& b) const { return (((_mm_movemask_ps(_mm_cmpge_ps(m_vec, b))) & 0x7) == 0x7); }
        inline bool operator< (const vec4x32f& b) const { return !(*this >= b); }
        inline bool operator<=(const vec4x32f& b) const { return !(*this > b); }

        template<typename U, typename std::enable_if<std::is_arithmetic_v<U>>::type* = nullptr>
        inline bool operator==(const U b) const { vec4x32f temp(b); return (*this == temp); }

        template<typename U, typename std::enable_if<std::is_arithmetic_v<U>>::type* = nullptr>
        inline bool operator!=(const U b) const { vec4x32f temp(b); return !(*this == temp); }

        template<typename U, typename std::enable_if<std::is_arithmetic_v<U>>::type* = nullptr>
        inline bool operator>=(const U b) const { vec4x32f temp(b); return (*this >= temp); }

        template<typename U, typename std::enable_if<std::is_arithmetic_v<U>>::type* = nullptr>
        inline bool operator>(const U b) const { vec4x32f temp(b); return (*this > temp); }

        template<typename U, typename std::enable_if<std::is_arithmetic_v<U>>::type* = nullptr>
        inline bool operator<=(const U b) const { vec4x32f temp(b); return (*this <= temp); }

        template<typename U, typename std::enable_if<std::is_arithmetic_v<U>>::type* = nullptr>
        inline bool operator<(const U b) const { vec4x32f temp(b); return (*this < temp); }

        // unary operator overload
        inline vec4x32f operator-() const { return vec4x32f(_mm_xor_ps(m_vec, _mm_castsi128_ps(_mm_set1_epi32(0x80000000)))); }

        // properties
        private:
            __m128 m_vec;
    };

    // numerical operator overloading
    inline vec4x32f operator+(vec4x32f a, const vec4x32f& b) { a += b; return a; }
    inline vec4x32f operator-(vec4x32f a, const vec4x32f& b) { a -= b; return a; }
    inline vec4x32f operator*(vec4x32f a, const vec4x32f& b) { a *= b; return a; }
    inline vec4x32f operator/(vec4x32f a, const vec4x32f& b) { a /= b; return a; }
    inline vec4x32f operator+(vec4x32f a, vec4x32f&& b) { a += std::move(b); return a; }
    inline vec4x32f operator-(vec4x32f a, vec4x32f&& b) { a -= std::move(b); return a; }
    inline vec4x32f operator*(vec4x32f a, vec4x32f&& b) { a *= std::move(b); return a; }
    inline vec4x32f operator/(vec4x32f a, vec4x32f&& b) { a /= std::move(b); return a; }

    template<typename U, typename std::enable_if<std::is_arithmetic_v<U>>::type* = nullptr>
    inline vec4x32f operator+(vec4x32f a, U b) { a += b; return a; }
    template<typename U, typename std::enable_if<std::is_arithmetic_v<U>>::type* = nullptr>
    inline vec4x32f operator+(U b, vec4x32f& a) { a += b; return a; }

    template<typename U, typename std::enable_if<std::is_arithmetic_v<U>>::type* = nullptr>
    inline vec4x32f operator-(vec4x32f a, U b) { a -= b; return a; }
    template<typename U, typename std::enable_if<std::is_arithmetic_v<U>>::type* = nullptr>
    inline vec4x32f operator-(U b, vec4x32f& a) { vec4x32f temp(b); temp -= b; return temp; }

    template<typename U, typename std::enable_if<std::is_arithmetic_v<U>>::type* = nullptr>
    inline vec4x32f operator*(vec4x32f a, U b) { a *= b; return a; }
    template<typename U, typename std::enable_if<std::is_arithmetic_v<U>>::type* = nullptr>
    inline vec4x32f operator*(U b, vec4x32f& a) { a *= b; return a; }

    template<typename U, typename std::enable_if<std::is_arithmetic_v<U>>::type* = nullptr>
    inline vec4x32f operator/(vec4x32f a, U b) { a /= b; return a; }

    // return the negative of a vector
    inline vec4x32f negate(const vec4x32f& a) { return _mm_xor_ps(a, _mm_set1_ps(-0.0)); }

    // return the absolute of a vector
    inline vec4x32f abs(const vec4x32f& a) { return _mm_andnot_ps(_mm_set1_ps(-0.f), a); }

    // return the inverse square root of a vector
    inline vec4x32f rsqrt(const vec4x32f& a) { return _mm_rsqrt_ps(a); }

    // return the square root of a vector
    inline vec4x32f sqrt(const vec4x32f& a) { return _mm_sqrt_ps(a); }

    // return a rounded (towards lower closest integer) vector
    inline vec4x32f floor(const vec4x32f& a) { return _mm_round_ps(a, 0x01 | 0x00); }

    // return a rounded (towards upper closest integer) vector
    inline vec4x32f ceil(const vec4x32f& a) { return _mm_round_ps(a, 0x02 | 0x00); }

    // return the natural logarithm of a vector
    inline vec4x32f log(const vec4x32f& a) { return _mm_log_ps(a); }

    // return the base 10 logarithm of a vector
    inline vec4x32f log10(const vec4x32f& a) { return _mm_log10_ps(a); }

    // return the base 2 logarithm of a vector
    inline vec4x32f log2(const vec4x32f& a) { return _mm_log2_ps(a); }

    // return the cubic root of a vector
    inline vec4x32f cbrt(const vec4x32f& a) { return _mm_cbrt_ps(a); }

    // return the sine of a vector
    inline vec4x32f sin(const vec4x32f& a) { return _mm_sin_ps(a); }

    // return the cosine of a vector
    inline vec4x32f cos(const vec4x32f& a) { return _mm_cos_ps(a); }

    // return the arcsine of a vector
    inline vec4x32f asin(const vec4x32f& a) { return _mm_asin_ps(a); }

    // return the arccos of a vector
    inline vec4x32f acos(const vec4x32f& a) { return _mm_acos_ps(a); }

    // return the tangent of a vector
    inline vec4x32f tan(const vec4x32f& a) { return _mm_tan_ps(a); }

    // return the arctangent of a vector
    inline vec4x32f atan(const vec4x32f& a) { return _mm_atan_ps(a); }

    // return the hyperbolic sine of a vector
    inline vec4x32f sinh(const vec4x32f& a) { return _mm_sinh_ps(a); }

    // return the hyperbolic cosine of a vector
    inline vec4x32f cosh(const vec4x32f& a) { return _mm_cosh_ps(a); }

    // return the hyperbolic arcsine of a vector
    inline vec4x32f asinh(const vec4x32f& a) { return _mm_asinh_ps(a); }

    // return the hyperbolic arccos of a vector
    inline vec4x32f acosh(const vec4x32f& a) { return _mm_acosh_ps(a); }

    // return the 2 argument arctangent of two vector
    inline vec4x32f atan2(const vec4x32f& a, const vec4x32f& b) { return _mm_atan2_ps(a, b); }

    // return a vector composed of the minimal values between current and other vector
    inline vec4x32f min(const vec4x32f& a, const vec4x32f& b) { return vec4x32f(_mm_min_ps(a, b)); }

    // return a vector composed of the maximal values between current and other vector
    inline vec4x32f max(const vec4x32f& a, const vec4x32f& b) { return vec4x32f(_mm_max_ps(a, b)); }

    // return vector length/magnitude/L2 norm
    inline float length(const vec4x32f& a) { return _mm_cvtss_f32(_mm_sqrt_ss(_mm_dp_ps(a, a, 0xFF))); }

    // return the dot product between two vectors
    inline float dot(const vec4x32f& a, const vec4x32f& b) { return _mm_cvtss_f32(_mm_dp_ps(a, b, 0xFF)); }

    // return distance between two vectors
    inline float distance(const vec4x32f& a, const vec4x32f& b) { 
        const vec4x32f diff{ b - a }; 
        return std::sqrtf(dot(diff, diff)); 
    }

    /// return a normalized vector
    inline vec4x32f normalize(const vec4x32f& a) { return vec4x32f(_mm_div_ps(a, _mm_sqrt_ps(_mm_dp_ps(a, a, 0xFF)))); }

    // return the minimal value of a vector
    inline float minElement(const vec4x32f& a) {
        vec4x32f v(a);
        v = _mm_min_ps(v, _mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 1, 0, 3)));
        v = _mm_min_ps(v, _mm_shuffle_ps(v, v, _MM_SHUFFLE(1, 0, 3, 2)));
        return v.x();
    }

    // return the maximal value of a vector
    inline float maxElement(const vec4x32f& a) {
        vec4x32f v(a);
        v = _mm_max_ps(v, _mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 1, 0, 3)));
        v = _mm_max_ps(v, _mm_shuffle_ps(v, v, _MM_SHUFFLE(1, 0, 3, 2)));
        return v.x();
    }

    // return the sum of vector elements
    inline float sum(const vec4x32f& a) {
        vec4x32f v{ _mm_hadd_ps(a, a) };
        v = _mm_hadd_ps(v, v);
        return _mm_cvtss_f32(v);
    }

    /**
    * vec4x32f implementation
    **/
    struct alignas(16) vec16x8i {
        // value constructor
        template<typename U, typename std::enable_if<std::is_arithmetic_v<U>>::type* = nullptr>
        vec16x8i(const U val)      : m_vec(_mm_set1_epi8(static_cast<std::uint8_t>(val))) {}
        vec16x8i(const __m128i& v) : m_vec(v)                                             {}
        vec16x8i(__m128i&& v)      : m_vec(std::move(v))                                  {}
        vec16x8i()                 : vec16x8i(static_cast<std::uint8_t>(0))               {}
        
        // "collection" constructor
        template<typename Collection, typename std::enable_if<is_iterate_able_v<Collection>>::type* = nullptr>
        vec16x8i(const Collection& col) {
            alignas(16) std::array<std::uint8_t, 16> arr;
            std::for_each(std::begin(col), std::end(col), [&arr, i = 0](const auto& c) mutable { arr[i] = static_cast<std::uint8_t>(c); ++i; });
            m_vec = _mm_set_epi8(arr[0], arr[1], arr[2],  arr[3],  arr[4],  arr[5],  arr[6],  arr[7], 
                                 arr[8], arr[9], arr[10], arr[11], arr[12], arr[13], arr[14], arr[15]);
        }
        
        template<typename Collection, typename std::enable_if<is_iterate_able_v<Collection>>::type* = nullptr>
        vec16x8i(Collection&& col) {
            alignas(16) std::array<std::uint8_t, 16> arr;
            std::for_each(std::begin(col), std::end(col), [&arr, i = 0](auto&& c) mutable { arr[i] = static_cast<std::uint8_t>(c); ++i; });
            m_vec = _mm_set_epi8(arr[0], arr[1], arr[2],  arr[3],  arr[4],  arr[5],  arr[6],  arr[7], 
                                 arr[8], arr[9], arr[10], arr[11], arr[12], arr[13], arr[14], arr[15]);
        }
    
        // "load" constructor
        vec16x8i(const void* pointer) : m_vec(_mm_loadu_si128((const __m128i*)pointer)) {}

        // copy semantics
        vec16x8i(const vec16x8i& other) : m_vec(other.m_vec) {}
        vec16x8i& operator=(const vec16x8i& other) {
            if (this != &other) {
                m_vec = other.m_vec;
            }
            return *this;
        }
        
        // move semantics
        vec16x8i(vec16x8i&& other) noexcept : m_vec(std::exchange(other.m_vec, _mm_set1_epi8(0))) {}
        vec16x8i& operator=(vec16x8i&& other) noexcept {
            if (this != &other) {
                m_vec = std::exchange(other.m_vec, _mm_set1_epi8(0));
            }
            return *this;
        }
        
        // value assignment
        template<typename U, typename std::enable_if<std::is_arithmetic_v<U>>::type* = nullptr>
        vec16x8i& operator=(const U other) {
            m_vec = _mm_set1_epi8(static_cast<std::uint8_t>(other));
            return *this;
        }
        
        // "collection" assignment
        template<typename Collection, typename std::enable_if<is_iterate_able_v<Collection>>::type* = nullptr>
        vec16x8i& operator=(Collection&& other) {
            vec16x8i temp(other);
            m_vec = std::exchange(temp.m_vec, _mm_set1_epi8(0));
            return *this;
        }

        template<typename Collection, typename std::enable_if<is_iterate_able_v<Collection>>::type* = nullptr>
        vec16x8i& operator=(const Collection& other) {
            vec16x8i temp(other);
            m_vec = temp;
            return *this;
        }
        
        // casting to standard 128bit register
        operator __m128i() const { return m_vec; }
        operator __m128i()       { return m_vec; }
        
        operator __m128() const { return _mm_cvtepi32_ps(m_vec); }
        operator __m128()       { return _mm_cvtepi32_ps(m_vec); }

        // set an element by its index (run time index)
        template<typename U, typename std::enable_if<std::is_arithmetic_v<U>>::type* = nullptr>
        inline void setElement(const U value, const std::size_t xi_index) {
            assert(xi_index < 16);

            // mask
            alignas(16) constexpr std::int8_t maskArray[32] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                                                -1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
            const __m128i mask{ _mm_load_si128((const __m128i*)(maskArray + 16 - (xi_index & 0x0F))) };

            // set element
            const std::uint8_t v{ static_cast<std::uint8_t>(value) };
            m_vec = _mm_blendv_epi8(m_vec, _mm_set1_epi8(v), mask);
        }

        // set an element by its index (compile time index)
        template<std::size_t I, typename U, typename std::enable_if<(I < 16) && std::is_arithmetic_v<U>>::type* = nullptr>
        inline void setElement(const U value) {
            // mask
            alignas(16) constexpr std::int8_t maskArray[32] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                                                -1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
            const __m128i mask{ _mm_load_si128((const __m128i*)(maskArray + 16 - (I & 0x0F))) };

            // set element
            const std::uint8_t v{ static_cast<std::uint8_t>(value) };
            m_vec = _mm_blendv_epi8(m_vec, _mm_set1_epi8(v), mask);
        }

        // extract a given element using compile time index
        template<std::size_t I, typename std::enable_if<I < 16>::type* = nullptr>
        inline std::uint8_t getElement() const {
            const std::int32_t as32bit{ _mm_extract_epi8(m_vec, I) };
            return static_cast<std::uint8_t>(as32bit & 0xFF);
        }

        // compound numerical operator overloading
        inline vec16x8i& operator+=(const vec16x8i& b) { m_vec = _mm_add_epi8(m_vec, b); return *this; }
        inline vec16x8i& operator-=(const vec16x8i& b) { m_vec = _mm_sub_epi8(m_vec, b); return *this; }
        inline vec16x8i& operator*=(const vec16x8i& b) { m_vec = multiply(m_vec, b); return *this; }
        inline vec16x8i& operator/=(const vec16x8i& b) { m_vec = _mm_div_epi8(m_vec, b); return *this; }
        
        template<typename U, typename std::enable_if<std::is_arithmetic_v<U>>::type* = nullptr>
        inline vec16x8i& operator+=(U b) { m_vec = _mm_add_epi8(m_vec, vec16x8i(b)); return *this; }

        template<typename U, typename std::enable_if<std::is_arithmetic_v<U>>::type* = nullptr>
        inline vec16x8i& operator-=(U b) { m_vec = _mm_sub_epi8(m_vec, vec16x8i(b)); return *this; }

        template<typename U, typename std::enable_if<std::is_arithmetic_v<U>>::type* = nullptr>
        inline vec16x8i& operator*=(U b) { m_vec = multiply(m_vec, vec16x8i(b)); return *this; }

        template<typename U, typename std::enable_if<std::is_arithmetic_v<U>>::type* = nullptr>
        inline vec16x8i& operator/=(U b) { m_vec = _mm_div_epi8(m_vec, vec16x8i(b)); return *this; }
        
        // relational operator overloading
        inline bool operator==(const vec16x8i& b) const { return (((_mm_movemask_epi8(_mm_cmpeq_epi8(m_vec, b))) & 0x7) == 0x7); }
        inline bool operator!=(const vec16x8i& b) const { return !(*this == b); }
        inline bool operator> (const vec16x8i& b) const { return (((_mm_movemask_epi8(_mm_cmpgt_epi8(m_vec, b))) & 0x7) == 0x7); }
        inline bool operator>=(const vec16x8i& b) const { return ((*this > b) || (*this == b)); }
        inline bool operator< (const vec16x8i& b) const { return !(*this >= b); }
        inline bool operator<=(const vec16x8i& b) const { return !(*this > b); }

        template<typename U, typename std::enable_if<std::is_arithmetic_v<U>>::type* = nullptr>
        inline bool operator==(const U b) const { vec16x8i temp(b); return (*this == temp); }

        template<typename U, typename std::enable_if<std::is_arithmetic_v<U>>::type* = nullptr>
        inline bool operator!=(const U b) const { vec16x8i temp(b); return !(*this == temp); }

        template<typename U, typename std::enable_if<std::is_arithmetic_v<U>>::type* = nullptr>
        inline bool operator>=(const U b) const { vec16x8i temp(b); return (*this >= temp); }

        template<typename U, typename std::enable_if<std::is_arithmetic_v<U>>::type* = nullptr>
        inline bool operator>(const U b) const { vec16x8i temp(b); return (*this > temp); }

        template<typename U, typename std::enable_if<std::is_arithmetic_v<U>>::type* = nullptr>
        inline bool operator<=(const U b) const { vec16x8i temp(b); return (*this <= temp); }

        template<typename U, typename std::enable_if<std::is_arithmetic_v<U>>::type* = nullptr>
        inline bool operator<(const U b) const { vec16x8i temp(b); return (*this < temp); }

        // bitwise operator overloading
        inline vec16x8i& operator>>=(const std::uint32_t& b) {
            // even numbered elements (later shifted back to position
            vec16x8i thisEven{ _mm_slli_epi16(m_vec, 8) };
            thisEven = _mm_sra_epi16(thisEven, _mm_cvtsi32_si128(b + 8));

            // move odd numbers
            const vec16x8i thisOde{ _mm_sra_epi16(m_vec, _mm_cvtsi32_si128(b)) },
                           maskEven{ _mm_set1_epi32(0x00FF00FF) };
            m_vec = _mm_blendv_epi8(maskEven, thisEven, thisOde);

            // output
            return *this;
        }
        inline vec16x8i& operator<<=(const std::uint32_t& b) {
            // mask to remove bits that are shifted out
            const std::uint32_t mask{ static_cast<uint32_t>(0xFF) >> static_cast<std::uint32_t>(b) };

            // protect from overflow and shift 16 bits
            const vec16x8i protectOverflow{ _mm_and_si128(m_vec, _mm_set1_epi8(static_cast<std::int8_t>(mask))) };
            m_vec = _mm_sll_epi16(protectOverflow, _mm_cvtsi32_si128(b));

            // output
            return *this;
        }
        inline vec16x8i& operator&=(const vec16x8i& b) { m_vec = _mm_and_si128(m_vec, b); return *this; }
        inline vec16x8i& operator|=(const vec16x8i& b) { m_vec = _mm_or_si128(m_vec,  b); return *this; }
        inline vec16x8i& operator^=(const vec16x8i& b) { m_vec = _mm_xor_si128(m_vec, b); return *this; }
        
        template<typename U, typename std::enable_if<std::is_arithmetic_v<U>>::type* = nullptr>
        inline vec16x8i operator&=(const U b) const { return (*this &= vec16x8i(b)); }
        template<typename U, typename std::enable_if<std::is_arithmetic_v<U>>::type* = nullptr>
        inline vec16x8i operator|=(const U b) const { return (*this |= vec16x8i(b)); }
        template<typename U, typename std::enable_if<std::is_arithmetic_v<U>>::type* = nullptr>
        inline vec16x8i operator^=(const U b) const { return (*this ^= vec16x8i(b)); }

        // unary operator overload
        inline vec16x8i operator-() const { return vec16x8i(_mm_xor_si128(m_vec, _mm_set_epi32(0, 0, 0, 0))); }

        // negation operator overload
        inline vec16x8i operator~() { return _mm_xor_si128(m_vec, _mm_set1_epi32(-1)); }

        // properties
        private:
            __m128i m_vec;

        // internal helpers
        private:

            // multiply two 16x8biti vectors
            inline vec16x8i multiply(const vec16x8i& a, const vec16x8i& b) {
                // split input arguments to two 16bit multiplies
                const __m128i aodd{ _mm_srli_epi16(a, 8) },                 // odd numbered elements of a
                              bodd{ _mm_srli_epi16(b, 8) };                 // odd numbered elements of b

                // product of even numbered elements
                const __m128i muleven{ _mm_mullo_epi16(a, b) };
                
                // product of odd  numbered elements
                __m128i mulodd{ _mm_mullo_epi16(aodd, bodd) };

                // put odd numbered elements back in place
                mulodd = _mm_slli_epi16(mulodd, 8);

                // mix even & odd
                const __m128i mask{ _mm_set1_epi32(0x00FF00FF) },          // mask for even positions
                              product{ _mm_blendv_epi8(mulodd, muleven, mask) };        // interleave even and odd
                return product;
            }
    };

    // numerical operator overloading
    inline vec16x8i operator+(vec16x8i a, const vec16x8i& b) { a += b; return a; }
    inline vec16x8i operator-(vec16x8i a, const vec16x8i& b) { a -= b; return a; }
    inline vec16x8i operator*(vec16x8i a, const vec16x8i& b) { a *= b; return a; }
    inline vec16x8i operator/(vec16x8i a, const vec16x8i& b) { a /= b; return a; }
    inline vec16x8i operator+(vec16x8i a, vec16x8i&& b) { a += std::move(b); return a; }
    inline vec16x8i operator-(vec16x8i a, vec16x8i&& b) { a -= std::move(b); return a; }
    inline vec16x8i operator*(vec16x8i a, vec16x8i&& b) { a *= std::move(b); return a; }
    inline vec16x8i operator/(vec16x8i a, vec16x8i&& b) { a /= std::move(b); return a; }

    template<typename U, typename std::enable_if<std::is_arithmetic_v<U>>::type* = nullptr>
    inline vec16x8i operator+(vec16x8i a, U b) { a += b; return a; }
    template<typename U, typename std::enable_if<std::is_arithmetic_v<U>>::type* = nullptr>
    inline vec16x8i operator+(U b, vec16x8i& a) { a += b; return a; }

    template<typename U, typename std::enable_if<std::is_arithmetic_v<U>>::type* = nullptr>
    inline vec16x8i operator-(vec16x8i a, U b) { a -= b; return a; }
    template<typename U, typename std::enable_if<std::is_arithmetic_v<U>>::type* = nullptr>
    inline vec16x8i operator-(U b, vec16x8i& a) { vec16x8i temp(b); temp -= b; return temp; }

    template<typename U, typename std::enable_if<std::is_arithmetic_v<U>>::type* = nullptr>
    inline vec16x8i operator*(vec16x8i a, U b) { a *= b; return a; }
    template<typename U, typename std::enable_if<std::is_arithmetic_v<U>>::type* = nullptr>
    inline vec16x8i operator*(U b, vec16x8i& a) { a *= b; return a; }

    template<typename U, typename std::enable_if<std::is_arithmetic_v<U>>::type* = nullptr>
    inline vec16x8i operator/(vec16x8i a, U b) { a /= b; return a; }

    // bitwise operator overloading
    inline vec16x8i operator>>(vec16x8i a, const std::uint8_t b) { a >>= b; return a; }
    inline vec16x8i operator<<(vec16x8i a, const std::uint8_t b) { a <<= b; return a; }
    inline vec16x8i operator&(vec16x8i a, const vec16x8i& b) { a &= b; return a; }
    inline vec16x8i operator|(vec16x8i a, const vec16x8i& b) { a |= b; return a; }
    inline vec16x8i operator^(vec16x8i a, const vec16x8i& b) { a ^= b; return a; }
    inline vec16x8i operator&(vec16x8i a, vec16x8i&& b) { a &= std::move(b); return a; }
    inline vec16x8i operator|(vec16x8i a, vec16x8i&& b) { a |= std::move(b); return a; }
    inline vec16x8i operator^(vec16x8i a, vec16x8i&& b) { a ^= std::move(b); return a; }

    template<typename U, typename std::enable_if<std::is_arithmetic_v<U>>::type* = nullptr>
    inline vec16x8i operator>>(vec16x8i a, U b) { a >>= b; return a; }
    template<typename U, typename std::enable_if<std::is_arithmetic_v<U>>::type* = nullptr>
    inline vec16x8i operator>>(U b, vec16x8i& a) { a >>= b; return a; }

    template<typename U, typename std::enable_if<std::is_arithmetic_v<U>>::type* = nullptr>
    inline vec16x8i operator<<(vec16x8i a, U b) { a <<= b; return a; }
    template<typename U, typename std::enable_if<std::is_arithmetic_v<U>>::type* = nullptr>
    inline vec16x8i operator<<(U b, vec16x8i& a) { a <<= b; return a; }

    template<typename U, typename std::enable_if<std::is_arithmetic_v<U>>::type* = nullptr>
    inline vec16x8i operator&(vec16x8i a, U b) { a &= b; return a; }
    template<typename U, typename std::enable_if<std::is_arithmetic_v<U>>::type* = nullptr>
    inline vec16x8i operator&(U b, vec16x8i& a) { a &= b; return a; }

    template<typename U, typename std::enable_if<std::is_arithmetic_v<U>>::type* = nullptr>
    inline vec16x8i operator|(vec16x8i a, U b) { a |= b; return a; }
    template<typename U, typename std::enable_if<std::is_arithmetic_v<U>>::type* = nullptr>
    inline vec16x8i operator|(U b, vec16x8i& a) { a |= b; return a; }

    template<typename U, typename std::enable_if<std::is_arithmetic_v<U>>::type* = nullptr>
    inline vec16x8i operator^(vec16x8i a, U b) { a ^= b; return a; }
    template<typename U, typename std::enable_if<std::is_arithmetic_v<U>>::type* = nullptr>
    inline vec16x8i operator^(U b, vec16x8i& a) { a ^= b; return a; }

    // return the sum of vector elements
    inline std::uint32_t sum(const vec16x8i& a) {
        // summation
        const __m128i sum1{ _mm_sad_epu8(a, _mm_setzero_si128()) },
                      sum2{ _mm_shuffle_epi32(sum1, 2) },
                      sum3{ _mm_add_epi16(sum1, sum2) };

        // output
        return  static_cast<std::uint32_t>(_mm_cvtsi128_si32(sum3));
    }

    /**
    * vec128i implementation
    **/
    struct alignas(16) vec128i {
        // value constructor
        vec128i()                 : vec128i(_mm_set1_epi8(0)) {}
        vec128i(const __m128i& v) : m_vec(v)                  {}
        vec128i(__m128i&& v)      : m_vec(std::move(v))       {}
        
        // "load" constructor
        vec128i(const void* pointer) : m_vec(_mm_loadu_si128((const __m128i*)pointer)) {}
        
        // copy semantics
        vec128i(const vec128i& other) : m_vec(other.m_vec) {}
        vec128i& operator=(const vec128i& other) {
            if (this != &other) {
                m_vec = other.m_vec;
            }
            return *this;
        }
        
        // move semantics
        vec128i(vec128i&& other) noexcept : m_vec(std::exchange(other.m_vec, _mm_set1_epi32(0))) {}
        vec128i& operator=(vec128i&& other) noexcept {
            if (this != &other) {
                m_vec = std::exchange(other.m_vec, _mm_set1_epi8(0));
            }
            return *this;
        }
        
        // value assignment
        template<typename U, typename std::enable_if<std::is_convertible_v<U, std::int32_t>>::type* = nullptr>
        vec128i& operator=(const U other) {
            m_vec = _mm_set1_epi32(static_cast<std::int32_t>(other));
            return *this;
        }
        
        // "collection" assignment
        template<typename Collection, typename std::enable_if<is_iterate_able_v<Collection>>::type* = nullptr>
        vec128i& operator=(Collection&& other) {
            vec128i temp(other);
            m_vec = std::exchange(temp.m_vec, _mm_set1_epi32(0));
            return *this;
        }

        template<typename Collection, typename std::enable_if<is_iterate_able_v<Collection>>::type* = nullptr>
        vec128i& operator=(const Collection& other) {
            vec128i temp(other);
            m_vec = temp;
            return *this;
        }
        
        // casting to standard 128bit register
        operator __m128i() const { return m_vec; }
        operator __m128i()       { return m_vec; }
        
        operator __m128() const { return _mm_cvtepi32_ps(m_vec); }
        operator __m128()       { return _mm_cvtepi32_ps(m_vec); }

        // set to 'true' all elements whose indices's are entered (run time index)
        inline void setElements(std::initializer_list<std::uint8_t>&& xi_indices) {
            // fill boolean array
            std::array<bool, 128> arr;
            std::for_each(xi_indices.begin(), xi_indices.end(), [&arr](auto&& c) {
                assert(c < 127);
                arr[std::min(c, static_cast< std::uint8_t>(127))] = true;
            });

            // send boolean array to vector
            m_vec = _mm_load_si128((const __m128i*)arr.data());
        }

        // set to all elements whose indices's are entered (compile time index) to a given value
        template<std::size_t... Is, typename std::enable_if<are_in_range_v<127, Is...>>::type* = nullptr>
        inline void setElements(const bool value) {
            // fill boolean array
            std::array<bool, 128> arr;
            for (auto&& i : { Is... }) {
                arr[i] = value;
            }

            // send boolean array to vector
            m_vec = _mm_load_si128((const __m128i*)arr.data());
        }

        // test if a given element is true using compile time index
        template<std::size_t I, typename std::enable_if<I < 127>::type* = nullptr>
        inline bool testElement() const {
            alignas(16) bool arr[128] = { false };
            _mm_store_si128((__m128i*)arr, m_vec);
            return (arr[I] == true);
        }

        // clear a vector (transform all elements to zero)
        inline void clear() { m_vec = _mm_set1_epi32(0); }

        // test if vector is entirely 'false'
        inline bool isEmpty() { return (_mm_testz_si128(m_vec, m_vec)); }

        // test if vector is entirely 'true'
        inline bool isFilled() { return (_mm_testc_si128(m_vec, _mm_set1_epi32(-1)) != 0); }

        // calculate how many elements are 'true'
        inline std::size_t countTrue() {
            alignas(16) bool arr[128];
            _mm_store_si128((__m128i*)arr, m_vec);
            return std::count_if(std::begin(arr), std::end(arr), [](bool b) { return (b == true); });
        }

        // bitwise operator overloading
        inline vec128i& operator&=(const vec128i& b) { m_vec = _mm_and_si128(m_vec, b); return *this; }
        inline vec128i& operator|=(const vec128i& b) { m_vec = _mm_or_si128(m_vec,  b); return *this; }
        inline vec128i& operator^=(const vec128i& b) { m_vec = _mm_xor_si128(m_vec, b); return *this; }
        
        template<typename U, typename std::enable_if<std::is_arithmetic_v<U>>::type* = nullptr>
        inline vec128i operator&=(const U b) const { return (*this &= vec128i(b)); }
        template<typename U, typename std::enable_if<std::is_arithmetic_v<U>>::type* = nullptr>
        inline vec128i operator|=(const U b) const { return (*this |= vec128i(b)); }
        template<typename U, typename std::enable_if<std::is_arithmetic_v<U>>::type* = nullptr>
        inline vec128i operator^=(const U b) const { return (*this ^= vec128i(b)); }

        // negation operator overload
        inline vec128i operator~() { return _mm_xor_si128(m_vec, _mm_set1_epi32(-1)); }

        // properties
        private:
            __m128i m_vec;
    };
};
