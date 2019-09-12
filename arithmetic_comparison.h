/**
* safe (no implicit casting) arithmetical value comparison with no run time costs
**/
#pragma once
#include <limits>
#include <type_traits>
#include <array>
#include <cmath>

// --------------------------------
// --- safe integral comparison ---
// --------------------------------
namespace integral_comparison_detail {
    // comparison between integers with same size and sign
    template<typename Integral1, typename Integral2,
             typename std::enable_if<(std::is_signed<Integral1>::value == std::is_signed<Integral2>::value) &&
                                      std::is_same<typename std::make_signed<Integral1>::type, typename std::make_signed<Integral2>::type>::value
                                    >::type* = nullptr>
    inline constexpr std::int32_t integral_compare(const Integral1 lhs, const Integral2 rhs) {
        return (lhs < rhs) ? -1 :
               (lhs > rhs) ?  1 :
                              0;
    }

    // comparison between integers with same sign but different sizes
    template<typename Integral1, typename Integral2,
             typename std::enable_if<(std::is_signed<Integral1>::value == std::is_signed<Integral2>::value) &&
                                     !std::is_same<typename std::make_signed<Integral1>::type, typename std::make_signed<Integral2>::type>::value
                                    >::type* = nullptr>
        inline constexpr std::int32_t integral_compare(const Integral1 lhs, const Integral2 rhs) {
        using Common = typename std::common_type<Integral1, Integral2>::type;
        return integral_compare(static_cast<Common>(lhs), static_cast<Common>(rhs));
    }

    // comparison between integers with different sign but same sizes
    template<typename Integral1, typename Integral2,
             typename std::enable_if<std::is_signed<Integral1>::value && std::is_unsigned<Integral2>::value &&
                                     std::is_same<typename std::make_signed<Integral1>::type, typename std::make_signed<Integral2>::type>::value
                                    >::type* = nullptr>
        inline constexpr std::int32_t integral_compare(const Integral1 lhs, const Integral2 rhs) {
        // the rhs is unsigned it will always be bigger then or equal to 0
        return (lhs < 0) ? -1 : 
                           integral_compare(static_cast<typename std::make_unsigned<Integral1>::type>(lhs), static_cast<typename std::make_unsigned<Integral2>::type>(rhs));
    }

    template<typename Integral1, typename Integral2,
             typename std::enable_if<std::is_unsigned<Integral1>::value && std::is_signed<Integral2>::value &&
                                     std::is_same<typename std::make_signed<Integral1>::type, typename std::make_signed<Integral2>::type>::value
                                    >::type* = nullptr>
    inline constexpr std::int32_t integral_compare(const Integral1 lhs, const Integral2 rhs) {
        return -integral_compare(rhs, lhs);
    }

    // comparison between integers with different sign but different sizes
    template<typename Integral1, typename Integral2,
             typename std::enable_if<(std::is_signed<Integral1>::value != std::is_signed<Integral2>::value) &&
                                      !std::is_same<typename std::make_signed<Integral1>::type, typename std::make_signed<Integral2>::type>::value
                                    >::type* = nullptr>
    inline constexpr std::int32_t integral_compare(const Integral1 lhs, const Integral2 rhs) {
        // promote to the integrals of their specific sign, but with common size
        using Common = typename std::common_type<Integral1, Integral2>::type;

        using NewIntegral1 = typename std::conditional<std::is_signed<Integral1>::value, 
                                                       typename std::make_signed<Common>::type,
                                                       typename std::make_unsigned<Common>::type>::type;
        using NewIntegral2 = typename std::conditional<std::is_signed<Integral2>::value,
                                                       typename std::make_signed<Common>::type,
                                                       typename std::make_unsigned<Common>::type>::type;
        return integral_compare(static_cast<NewIntegral1>(lhs), static_cast<NewIntegral2>(rhs));
    }
}

/**
* \brief safe comparison of two integers with potentially different sign and sizes.
* 
* @param {Integer, in} integer 1 
* @param {Integer, in} integer 2
* @param {int32_t, out} -1 if lhs < rhs, 0 if lhs == rhs, 1 if lhs > rhs
**/
template<typename Integral1, typename Integral2> 
inline constexpr std::int32_t integral_compare(const Integral1 lhs, const Integral2 rhs) {
    return integral_comparison_detail::integral_compare(lhs, rhs);
}

// -----------------------------
// --- safe float comparison ---
// -----------------------------

/**
* \brief safe comparison of two floating point numbers with potentially different sizes.
*        Nan is considered to be infinite.
*
* @param {Float, in}  float 1
* @param {Float, in}  float 2
* @param {bool,  out} true lhs < rhs, false if lhs >= rhs
**/
template<typename Float1, typename Float2,
         typename std::enable_if<std::is_arithmetic_v<Float1> && !std::is_integral_v<Float1> &&
                                 std::is_arithmetic_v<Float2> && !std::is_integral_v<Float2>
                                >::type* = nullptr>
inline constexpr bool float_compare(const Float1 lhs, const Float2 rhs) {
    using Common = typename std::common_type<Float1, Float2>::type;

    return std::isnan(lhs) ? false :
           std::isnan(rhs) ? true  :
                             static_cast<Common>(lhs) < static_cast<Common>(rhs);
}
