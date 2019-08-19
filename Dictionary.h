/**
* compile-time fixed-size bi-directional map (dictionary) for integer types.
*
* Dan Israel Malta
**/
#pragma once
#include <type_traits>

namespace CompileTimeDictionary {

	/**
	* \brief dictionary is composed of entries.
	*        each entry is defined by <key, value> pair.
	*        usage is: using dict = Dictionary<Entry<x1,y1>, Entry<x2,y2>, ...>;
	**/
	template<int K, int V> struct Entry { enum { Key = K, Value = V }; };
	template<typename ...Entrys> struct Dictionary {};

	/**
	* \brief a helper struct which holds its own type
	**/
	template<typename T> struct Identity { using type = T; };

	/**
	* \brief return the amount of entries in dictionary (i.e. - dictionary size)
	* 
	* @param {in}  dictionary
	* @param {out} numbe of entries in dictionary
	**/
	template<typename...B> struct size;
	template<typename...B> struct size<Dictionary<B...>> { enum { Value = sizeof...(B) }; };
	template<typename...B> inline constexpr int size_v = size<B...>::Value;

	/**
	* \brief given a key and a dictionary, extract the appropriate value
	*
	* @param {in}  key
	* @param {in}  dictionary
	* @param {out} value
	**/
	template<int I, typename M> struct GetValueFromKey;
	template<int I, typename A> struct GetValueFromKey<I, Dictionary<A>> {
		using meta = typename std::conditional<I == A::Key, Identity<A>, void>::type;
		using type = typename meta::type;
		enum { value = type::Value };
	};
	template<int I, typename A, typename ...B> struct GetValueFromKey<I, Dictionary<A, B...> > {
		using meta = typename std::conditional<I == A::Key, Identity<A>, GetValueFromKey<I, Dictionary<B...>>>::type;
		using type = typename meta::type;
		enum { value = type::Value };
	};
	template<int I, typename A> inline constexpr int GetValueFromKey_v = GetValueFromKey<I, A>::value;

	/**
	* \brief given a value and a dictionary, extract the appropriate key
	*
	* @param {in}  value
	* @param {in}  dictionary
	* @param {out} key
	**/
	template<int I, typename M> struct GetKeyFromValue;
	template<int I, typename A> struct GetKeyFromValue<I, Dictionary<A>> {
		using meta = typename std::conditional<I == A::Value, Identity<A>, void>::type;
		using type = typename meta::type;
		enum { value = type::Key };
	};
	template<int I, typename A, typename ...B> struct GetKeyFromValue<I, Dictionary<A, B...> > {
		using meta = typename std::conditional<I == A::Value, Identity<A>, GetKeyFromValue<I, Dictionary<B...>>>::type;
		using type = typename meta::type;
		enum { value = type::Key };
	};
	template<int I, typename A> inline constexpr int GetKeyFromValue_v = GetKeyFromValue<I, A>::value;

	/**
	* \brief test if a given key exists in a dictionary
	*
	* @param {in}  key
	* @param {in}  dictionary
	* @param {out} true if key exists, false otherwise
	**/
	template<int I, typename A> struct ContainsKey;
	template<int I, typename A> struct ContainsKey<I, Dictionary<A>> {
		constexpr static bool Value{ I == A::Key };
	};
	template<int I, typename A, typename ... B> struct 	ContainsKey<I, Dictionary<A, B...>> {
		constexpr static bool Value{ (I == A::Key) || ContainsKey<I, Dictionary<B...>>::Value };
	};
	template<int I, typename A> inline constexpr int ContainsKey_v = ContainsKey<I, A>::Value;

	/**
	* \brief test if a given value exists in a dictionary
	*
	* @param {in}  value
	* @param {in}  dictionary
	* @param {out} true if value exists, false otherwise
	**/
	template<int I, typename A> struct ContainsValue;
	template<int I, typename A> struct ContainsValue<I, Dictionary<A>> {
		constexpr static bool Value{ I == A::Value };
	};
	template<int I, typename A, typename ... B> struct 	ContainsValue<I, Dictionary<A, B...>> {
		constexpr static bool Value{ (I == A::Value) || ContainsValue<I, Dictionary<B...>>::Value };
	};
	template<int I, typename A> inline constexpr int ContainsValue_v = ContainsValue<I, A>::Value;
}

//////////////////////////////////////////////////
//////////////// example usage ///////////////////
//////////////////////////////////////////////////

#include "Dictionary.h"
#include <iostream>

// test compile time dictionary
int main() {
	using namespace CompileTimeDictionary;

	// dictionary
	using M = Dictionary<Entry<0, 4>,
						 Entry<1, 8>,
						 Entry<2, 15>>;

	// check dictionary size
	static_assert(size_v<M> == 3);

	// check get dictionary value by key
	static_assert(GetValueFromKey_v<0, M> == 4);
	static_assert(GetValueFromKey_v<1, M> == 8);
	static_assert(GetValueFromKey_v<2, M> == 15);

	// check get dictionary key by value
	static_assert(GetKeyFromValue_v<4, M>  == 0);
	static_assert(GetKeyFromValue_v<8, M>  == 1);
	static_assert(GetKeyFromValue_v<15, M> == 2);

	// check if a given key contained in dictionary?
	static_assert(ContainsKey_v<0, M>  == true);
	static_assert(ContainsKey_v<2, M>  == true);
	static_assert(ContainsKey_v<37, M> == false);

	// check if a given value contained in dictionary?
	static_assert(ContainsValue_v<4, M>  == true);
	static_assert(ContainsValue_v<15, M> == true);
	static_assert(ContainsValue_v<37, M> == false);

	// output
	std::cout << "dictionary size is" << size_v<M> << std::endl;

	std::cout << "GetValueFromKey_v<0, M> = " << GetValueFromKey_v<0, M> << std::endl;
	std::cout << "GetValueFromKey_v<1, M> = " << GetValueFromKey_v<1, M> << std::endl;
	std::cout << "GetValueFromKey_v<2, M> = " << GetValueFromKey_v<2, M> << std::endl;

	std::cout << "GetKeyFromValue_v<4, M>  = " << GetKeyFromValue_v<4, M> << std::endl;
	std::cout << "GetKeyFromValue_v<8, M>  = " << GetKeyFromValue_v<8, M> << std::endl;
	std::cout << "GetKeyFromValue_v<15, M> = " << GetKeyFromValue_v<15, M> << std::endl;

	std::cout << std::boolalpha << "does dictionary contains the key 0? "  << ContainsKey_v<0, M>  << std::endl;
	std::cout << std::boolalpha << "does dictionary contains the key 2? "  << ContainsKey_v<2, M>  << std::endl;
	std::cout << std::boolalpha << "does dictionary contains the key 37? " << ContainsKey_v<37, M> << std::endl;

	std::cout << std::boolalpha << "does dictionary contains the value 4? "  << ContainsValue_v<4, M>  << std::endl;
	std::cout << std::boolalpha << "does dictionary contains the value 15? " << ContainsValue_v<15, M> << std::endl;
	std::cout << std::boolalpha << "does dictionary contains the value 37? " << ContainsValue_v<37, M> << std::endl;

	return 1;
}
