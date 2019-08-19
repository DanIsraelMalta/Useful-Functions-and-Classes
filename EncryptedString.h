/**
* A small utility which allows compile time encryption and run time decryption of strings.
* Encryption/Decryption schemes are supplied by the user (default is XOR encryption).
* 
* example usage:
* ```c
* // encrypt string during compilation time
* auto decrypted_string = EncryptString("hello world");
* static_assert(sizeof(decrypted_string) > 0, "");
* 
* // decrypt string during run time
* auto encrypted_string = DecryptString(decrypted_string);
* std::cout << "run time decrypted: " << encrypted_string << std::endl;
* ```
* 
* Dan Israel Malta
**/

// create a numerical value from/during compilation time
struct CompileTimeSeed {
    static constexpr std::uint64_t value = (__TIME__[7] - '0') * 1u    +
                                           (__TIME__[6] - '0') * 10u   +
                                           (__TIME__[4] - '0') * 60u   +
                                           (__TIME__[3] - '0') * 600u  +
                                           (__TIME__[1] - '0') * 3600u +
                                           (__TIME__[0] - '0') * 36000u;
};

// one step linear congruential pseudo random number generator (modulus is 2^32 = 0xFFFFFFFF)
template<std::uint64_t Multiplier, std::uint64_t Incerement> struct RandomGenerator {
    static constexpr std::uint32_t value{ static_cast<std::uint32_t>((Incerement + Multiplier * CompileTimeSeed::value) & 0xFFFFFFFF) };
};

// 'incremental' pseudo random number
struct Random {
    static constexpr std::uint32_t value{ RandomGenerator<1664525u, 1013904223u>::value };
};

// return a pseudo random number within a given region
template<std::uint32_t min, std::uint32_t max> struct RandomInRange {
    static constexpr std::uint32_t value{ min + (Random::value % (max - min + 1)) };
};

// return a random character
struct RandomCharacter {
    static constexpr char value{ static_cast<char>(RandomInRange<0, 255>::value) };
};

/**
* A base object which holds the encryption & decryption scheme's to be used during encryption/decryption.
* Encryption schema should have the signature: char = Encryption(char, uint8_t)
* Decryption schema should have the signature: char = Decryption(char, uint8_t)
* Notice that it should work on a singe character.
*
* User editable...
**/
struct EncryptionSchema {
	// character encryption scheme
	static constexpr char Encryption(const char chr, const std::uint8_t id) {
		return (chr ^ (RandomCharacter::value + id));
	}

	// character decryption scheme
	static constexpr char Decryption(const char chr, const std::uint32_t id) {
		return (chr ^ (RandomCharacter::value + id));
	}
};

// type trait to see if the method 'Encryption' is included in a struct
template<typename T, typename = void> struct has_Encryption_method : std::false_type { };
template<typename T> struct has_Encryption_method<T, decltype(std::declval<T>().Encryption, void())> : std::true_type { };

// type trait to see if the method 'Decryption' is included in a struct
template<typename T, typename = void> struct has_Decryption_method : std::false_type { };
template<typename T> struct has_Decryption_method<T, decltype(std::declval<T>().Decryption, void())> : std::true_type { };

/**
* compile time encrypted, run time decrypted string
* do not use directly, use the more API friendly macros defined after this class
**/
template<class Logic, typename Index> class EncryptedString;
template<class Logic, std::uint8_t...Ids> class EncryptedString<Logic, std::index_sequence<Ids...>> {
	static_assert(std::is_class<Logic>::value, "EncryptedString<Logic,...> - Logic must be a struct with methods 'Encryption' & 'Decryption'.");
	static_assert(has_Encryption_method<Logic>::value, "EncryptedString<Logic,...> - Logic must be a struct with method 'Encryption'.");
	static_assert(has_Decryption_method<Logic>::value, "EncryptedString<Logic,...> - Logic must be a struct with method 'Decryption'.");

    // properties
    private:
        char m_string[sizeof...(Ids) + 1];  // encrypted string

    // constructor ( string is encrypted during construction (compile time))
    public:
        explicit constexpr EncryptedString(const char* str) : m_string{ Logic::Encryption(str[Ids], Ids) ... } {}

    // decryption (during run time)
    public:
        char* Decrypt() {
            for (std::uint8_t i{}; i < sizeof...(Ids); ++i) {
                m_string[i] = Logic::Decryption(m_string[i], i);
            }
            m_string[sizeof...(Ids)] = '\0';
            return m_string;
        }
};

// Compile-time string encryption macro
#define EncryptString(str) (EncryptedString<EncryptionSchema, std::make_index_sequence<sizeof(str)>>(str))

// Run-time string decryption macro
#define DecryptObject(obj) (obj.Decrypt())
