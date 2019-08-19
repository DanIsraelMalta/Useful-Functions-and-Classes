/**
* An example of how to emulate a function with named arguments using a functor.
*
* Dan Israel Malta
**/

// a functor which behaves like a function with multiple 
// named input parameters and multiple named outputs.
struct computeExample {
    // input arguments (they can have default values)
    int x{},
        y{ 2 },
        z{ 1 };
    
    // output arguments
    struct {
        int Sum,        // sum of input arguments
            Mult;       // multiplication of input arguments
    } output;
    
    // actual computation (function body)
    computeExample operator()() {
        output.Sum  = x + y + z;
        output.Mult = x * y * z;
        return *this;
    }
};

// ---------------------
// --- example usage ---
// ---------------------
#include <iostream>

int main() {
    // initialize input arguments in the declaration order of the struct (supported by all recent compilers)
    auto r1 = computeExample{1,2,3}(); // x = 1, y = 2, z = 3
    // and get named output results
    std::cout << "Sum = " << r1.output.Sum << " Mult = " << r1.output.Mult << "\n";
    
    // gcc and clang allow the usage of designated initializers (last time I checked, this was not supported by msvc)
    auto r2 = computeExample{.x = 2, .y = 3, .z=5}();
    std::cout << "Sum = " << r2.output.Sum << " Mult = " << r2.output.Mult << "\n";
    
    // gcc and clang, allow us to omit input parameters that have default values
    auto r3 = computeExample{.x = 4}(); // y = 2, z = 1
    std::cout << "Sum = " << r3.output.Sum << " Mult = " << r3.output.Mult << "\n";
    
    // clang allow us to change the input parameters order (last time I checked, this was not supported by gcc/msvc)
    auto r4 = computeExample{.z = 42, .x = 3}(); // y = 2
    std::cout << "Sum = " << r4.output.Sum << " Mult = " << r4.output.Mult << "\n";
}
