/**
* helper object to deal with integral types as array of bytes.
*
* example usage:
*
*   using chunk = ArrayOfBytes<std::uint64_t, bool>;
*
*   // construction & boolean operations
*   const chunk A(32),
*               B(4096),
*               C(std::array<bool, 11>{true, false, false, true, false, false, true, false, true, false, true}),    // only the first 8 elements are used in construction
*               D(true, true, true, true, false, false, false, false),
*               E{ C ^ D };
*
*   // relational operators
*   const bool b0{ A == B };
*
*   // casting to underlying type for easy usage as a parameter
*   std::uint64_t e{ E };
*
*   // inspect the internal components
*   std::cout << "E = " << E << std::endl;  // E = {0, 1, 1, 0, 0, 0, 1, 0, 0}
*
* Dan Israel Malta
**/
#pragma once
#include<type_traits>
#include<inttypes.h>
#include<iostream>
#include <string>

// type traits
namespace {
    // type trait to test if an object is iterate-able
    template<typename T, typename = void> struct is_iterate_able : std::false_type {};
    template<typename T> struct is_iterate_able<T, std::void_t<decltype(std::begin(std::declval<T>())), decltype(std::end(std::declval<T>()))>> : std::true_type {};

    // type trait to test if an object 'T' is both integral and convertible to 'U'
    template<typename T, typename U> struct is_IntegralConvertible {
        static constexpr bool value{ std::is_integral<T>::value && std::is_convertible<T, U>::value };
    };

    // type trait to test if an object is a boolean type
    template<typename> struct is_bool       : public std::false_type {};
    template<>         struct is_bool<bool> : public std::true_type  {};
}

/**
* \brief helper object to deal with integral types as array of bytes
*
* @param {Type, in} integral type to be handled as array
* @param {Byte, in} a 'component' from which the 'Type' is composed
**/
template<class Type, class Byte = bool> union ArrayOfBytes {
    static_assert(std::is_integral<Type>::value && std::is_integral<Byte>::value, "ArrayOfBytes<Type, Byte>: both 'Type' and 'Byte' must be integral types.");
    static_assert(sizeof(Type) % sizeof(Byte) == 0, "ArrayOfBytes<Type, Byte>: 'Byte' size should be an integer division of 'Type'.");

    // 'constants'
    enum : std::size_t { ByteCount = sizeof(Type) / sizeof(Byte) }; // how many 'Byte' there are in 'Type'
    enum : std::size_t { ByteSizeInBits = sizeof(Byte) * 8 };       // 'Byte' size in bits

    // properties
    Type value;                 // 'Type' itself
    Byte values[ByteCount];     // 'Type' as an array of 'Byte'

    // construct by 'Type'
    template<class U> explicit constexpr ArrayOfBytes(const U& v, typename std::enable_if<is_IntegralConvertible<U, Type>::value>::type* = nullptr) : value(static_cast<Type>(v)) {}

    // construct from collection of 'Bytes' (should have no more then 'ByteCount' elements)
    template<typename Collection, typename std::enable_if<is_iterate_able<Collection>::value>::type* = nullptr>
    explicit constexpr ArrayOfBytes(const Collection& xi_collection) {
        std::size_t i{};
        for (const auto& c : xi_collection) {
            values[i] = c;
            ++i;
            if (i == ByteCount) break;
        }
    }

    // construct using 'ByteCount' number of 'Bytes'
    template<typename ...Us> explicit constexpr ArrayOfBytes(Us... bytes) {
        constexpr std::size_t len{ sizeof...(Us) };
        static_assert(len == ByteCount, "ArrayOfBytes is attempted to be constructed with a wrong number of input arguments.");

        Byte _bytes[] = { static_cast<Byte>(bytes)... };

        for (std::size_t i{}; i < len; ++i) {
            values[i] = _bytes[i];
        }
    }

    // copy constructors
    ArrayOfBytes (const ArrayOfBytes& other) { 
        value = other.value; 
    }

    // move constructors
    ArrayOfBytes(ArrayOfBytes&& other) noexcept {
        value = std::move(other.value);
        other.value = Type{};
    }

    // copy assignments
    ArrayOfBytes& operator=(const Type& v) {
        value = v;
        return *this;
    }
    ArrayOfBytes& operator=(const ArrayOfBytes& other) { return operator=(other.value); }

    // move assignments
    ArrayOfBytes& operator=(Type&& v) {
        value = std::move(v);
        return *this;
    }
    ArrayOfBytes& operator=(ArrayOfBytes&& other) noexcept { 
        return operator=(other.value); 
    }

    // cast operator
    operator Type() const { return value; }

    // set/get components at specific index using the '[]' operator
    constexpr Byte  operator[](const std::size_t i) const { assert(i < ByteCount); return values[i]; }
    constexpr Byte& operator[](const std::size_t i)       { assert(i < ByteCount); return values[i]; }

    // --- relational (equality) operators ---
    constexpr bool operator!=(const Type& other)         const { return !operator==(other); }
    constexpr bool operator==(const Type& other)         const { return this->value == other; }
    constexpr bool operator!=(const ArrayOfBytes& other) const { return !operator==(other); }
    constexpr bool operator==(const ArrayOfBytes& other) const { return operator==(other.value); }

    // print components
    friend std::ostream& operator<<(std::ostream& xio_stream, const ArrayOfBytes& arr) {
        xio_stream << "{";

        for (std::size_t i{}; i < arr.ByteCount; ++i) {
            xio_stream << std::to_string(arr[i]) << ", ";
        }
        xio_stream << arr[ByteCount - 1] << "}";

        return xio_stream;
    }

    // --- logical operations with boolean components ---
#define M_LOGICAL_OPERATOR(OP, AOP)                                                                    \
    template<class b = Byte, typename std::enable_if<is_bool<b>::value>::type* = nullptr>              \
    friend constexpr ArrayOfBytes operator OP (const ArrayOfBytes& lhs, const Type rhs) {              \
        ArrayOfBytes<Type, Byte> temp(rhs);                                                            \
        for (std::size_t i{}; i < temp.ByteCount; ++i) {                                               \
            temp[i] AOP lhs[i];                                                                        \
        }                                                                                              \
        return temp;                                                                                   \
    }                                                                                                  \
    template<class b = Byte, typename std::enable_if<is_bool<b>::value>::type* = nullptr>              \
    friend constexpr ArrayOfBytes operator OP (const Type rhs, const ArrayOfBytes& lhs) {              \
        ArrayOfBytes<Type, Byte> temp(rhs);                                                            \
        for (std::size_t i{}; i < temp.ByteCount; ++i) {                                               \
            temp[i] AOP lhs[i];                                                                        \
        }                                                                                              \
        return temp;                                                                                   \
    }                                                                                                  \
    template<class b = Byte, typename std::enable_if<is_bool<b>::value>::type* = nullptr>              \
    friend constexpr ArrayOfBytes operator OP (const ArrayOfBytes& lhs, const ArrayOfBytes& rhs) {     \
        ArrayOfBytes<Type, Byte> temp(rhs);                                                            \
        for (std::size_t i{}; i < temp.ByteCount; ++i) {                                               \
            temp[i] AOP lhs[i];                                                                        \
        }                                                                                              \
        return temp;                                                                                   \
    }                                                                                                  \
    template<class b = Byte, typename std::enable_if<is_bool<b>::value>::type* = nullptr>              \
    friend constexpr ArrayOfBytes operator OP (ArrayOfBytes&& lhs, const ArrayOfBytes& rhs) {          \
        for (std::size_t i{}; i < lhs.ByteCount; ++i) {                                                \
            lhs[i] AOP rhs[i];                                                                         \
        }                                                                                              \
        return lhs;                                                                                    \
    }                                                                                                  \
    template<class b = Byte, typename std::enable_if<is_bool<b>::value>::type* = nullptr>              \
    friend constexpr ArrayOfBytes operator OP (const ArrayOfBytes& lhs, ArrayOfBytes&& rhs) {          \
        for (std::size_t i{}; i < lhs.ByteCount; ++i) {                                                \
            rhs[i] AOP lhs[i];                                                                         \
        }                                                                                              \
        return rhs;                                                                                    \
    }                                                                                                  \
    template<class b = Byte, typename std::enable_if<is_bool<b>::value>::type* = nullptr>              \
    friend constexpr ArrayOfBytes operator OP (ArrayOfBytes&& lhs, ArrayOfBytes&& rhs) {               \
        for (std::size_t i{}; i < lhs.ByteCount; ++i) {                                                \
            lhs[i] AOP rhs[i];                                                                         \
        }                                                                                              \
        return lhs;                                                                                    \
    }

    M_LOGICAL_OPERATOR(&, &= );
    M_LOGICAL_OPERATOR(|, |= );
    M_LOGICAL_OPERATOR(^, ^= );

#undef M_LOGICAL_OPERATOR
};
