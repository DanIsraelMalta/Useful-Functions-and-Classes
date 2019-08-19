
/**
* \brief type trait to test if 'TEnum' underlying type is convertible to 'TOut'
* 
* @param {TEnum, in} enumeration
* @param {TOut,  in} a type
**/
template<typename TEnum, typename TOut> struct underlying_convertible_to {
	static constexpr bool value { std::is_convertible<std::underlying_type_t<TEnum>, TOut>>::value };
};

/**
* \brief type trait to test if number 'TIn' is convertible to enumeration 'TOut' underlying type
* 
* @param {TOut,in} enumeration
* @param {TIn, in} a type
**/
template<typename TOut, typename TIn> struct number_convertible_to_enum {
	static constexpr bool value{ std::is_enum<TOut>() && !std::is_enum<TIn>() && underlying_convertible_to<TOut, TIn>::value };
};

/**
* \brief type trait to test if two types are enumerations
* 
* @param {TOut,in} type
* @param {TIn, in} type
**/
template<typename TOut, typename TIn> struct are_both_enums {
	static constexpr bool value{ std::is_enum<TOut>() && std::is_enum<TIn>() };
}

/**
* \brief converts one arithmetic type to another 
* 
* @param {TIn,  in}  input arithmetic type
* @param {TOut, out} output arithmetic type
**/
template<typename TOut, typename TIn>
inline constexpr auto to_num(const TIn& x) noexcept {
    static_assert(std::is_arithmetic<TOut>{}, "`TOut` output type must be an arithmetic type.");
    static_assert(std::is_arithmetic<TIn>{}, "`TIn` input type must be an arithmetic type.");
    return static_cast<TOut>(x);
}

/**
* \brief Converts an enumeration to a type convertible to its underlying type
* 
* @param {TIn,  in}  enumeration
* @param {TOut, out} a type which is convertible from enumeration underlying type
**/
template<typename TOut, typename TIn>
inline constexpr auto from_enum(const TIn& x) noexcept {
    static_assert(std::is_enum<TIn>{});
    static_assert(underlying_convertible_to<TIn, TOut>{});

    return to_num<TOut>(static_cast<std::underlying_type_t<TIn>>(x));
}

/**
* \brief Converts an enumeration to its underlying type
* 
* @param {TIn,  in}  enumeration
* @param {auto, out} enumeration value in its underlying type
**/
template<typename TIn> inline constexpr auto from_enum(const TIn& x) noexcept {
    return from_enum<std::underlying_type_t<TIn>, TIn>(x);
}

/**
* \brief Converts a number to an enumeration
* 
* @param {TIn,  in}  arithmetic value 
* @param {TOut, out} enumeration
**/
template<typename TOut, typename TIn, std::enable_if_t<number_convertible_to_enum<TOut, TIn>::value, TOut>>
inline constexpr auto to_enum(const TIn& x) noexcept {
    return static_cast<TOut>(to_num<std::underlying_type_t<TOut>>(x));
}

/**
* \brief Converts an enumeration to another enumeration 
*        (their underlying types must be convertible between each other)
* 
* @param {TIn, in}   enumeration
* @param {TOut, out} enumeration
**/
template<typename TOut, typename TIn, std::enable_if_t<are_both_enums<TOut, TIn>::value, TOut>>
inline constexpr auto to_enum(const TIn& x) noexcept {
    static_assert(are_underlying_types_convertible<TOut, TIn>{});
    return to_enum<TOut>(from_enum(x));
}
