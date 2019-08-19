/**
* A flexible range based for loop implementation which allows both
* looping in reverse order and looping with a given stride.
*
* It is equivalent to Python's range loop or Matlab's for indexing with the following syntax:
*
*           python             |    Matlab               |      irange
* -----------------------------+-------------------------+---------------------------------
* range(start, stop, jump)     | start : jump : stop     | irange<stride>(start, stop)
* -----------------------------+-------------------------+---------------------------------
* for i in range(20, 0, -2)    | 20 : -2 : 0             | for (auto i : irange<-2>(20, 0))
* -----------------------------+-------------------------+---------------------------------
* for i in range(13, 38, 5)    | 13 : 5 : 38             | for (auto i : irange<5>(13, 38))
* -----------------------------+-------------------------+---------------------------------
* for i in range(0, 10)        | 0 : 10                  | for (auto i : irange<>(0, 10))
* -----------------------------+-------------------------+---------------------------------
*
*
* Below is a simple example:
*
*  int main() {
*       std::cout << "reverse (20:-2:0)" << std::endl;
*       for (auto i : irange<-2>(20, 0)) std::cout << i << ' ';
*       std::cout << std::endl;
*
*       std::cout << "reverse (20:-2:1)" << std::endl;
*       for (auto i : irange<-2>(20, 1)) std::cout << i << ' ';
*       std::cout << std::endl;
*
*       std::cout << "reverse (20:-2:3)" << std::endl;
*       for (auto i : irange<-2>(20, 3)) std::cout << i << ' ';
*       std::cout << std::endl;
*
*       std::cout << "reverse  (17:-2:-1)" << std::endl;
*       for (auto i : irange<-2>(17, -1)) std::cout << i << ' ';
*       std::cout << std::endl;
*
*       std::cout << "reverse  (17:-3:4)" << std::endl;
*       for (auto i : irange<-3>(1, 37)) std::cout << i << ' ';
*       std::cout << std::endl;
*
*       std::cout << "normal (13:5:38)" << std::endl;
*       for (auto i : irange<5>(13, 38)) std::cout << i << ' ';
*       std::cout << std::endl;
*
*       std::cout << "normal (13:5:34)" << std::endl;
*       for (auto i : irange<5>(13, 34)) std::cout << i << ' ';
*       std::cout << std::endl;
*
*       std::cout << "normal (-5:5:19)" << std::endl;
*       for (auto i : irange<5>(-5, 19)) std::cout << i << ' ';
*       std::cout << std::endl;
*
*       std::cout << "normal (-5:5:21)" << std::endl;
*       for (auto i : irange<5>(-5, 21)) std::cout << i << ' ';
*       std::cout << std::endl;
*
*       std::cout << std::endl;
*  }
*
* Dan Israel Malta
**/
#pragma once

#include <utility>
#include <stdexcept>

/**
* \brief A flexible range based for loop implementation which allows both looping in reverse order
*        and looping with a given stride.
*
* \usage irange<stride>(start, end)
*        notice that when defining a negative stride, the 'start' must be bigger then 'end'
*        example:
*        irange<-2>(20, 0)  -> 20, 18, 16, 14, 12, 10, 8, 6, 4, 2
*        irange<-2>(20, 1)  -> 20, 18, 16, 14, 12, 10, 8, 6, 4, 2
*        irange<-2>(20, 3)  -> 20, 18, 16, 14, 12, 10, 8, 6, 4
*        irange<-2>(17, -1) -> 17, 15, 13, 11, 9, 7, 5, 3, 1
*        irange<5>(13, 38)  -> 13, 18, 23, 28, 33
*        irange<5>(13, 34)  -> 13, 18, 23, 28, 33
*        irange<5>(-5, 19)  -> -5, 0, 5, 10, 15
*
* @param {T, in}  define for loop stride/jump between two consecutive iterations (default is +1)
**/
template<int xi_step = 1> struct irange final {
	static_assert(xi_step != 0, "irange: Stride must be a non zero value.");

    // iterator values
    int mBegin, mEnd;

    // iterator definition
    struct Iterator final {
        // counter
        int value;
        Iterator(const int & xi_value) : value(xi_value) {}

        // '++' override
        Iterator operator++() {
            value += xi_step;
            return *this;
        }

        // '!=' override
        bool operator!=(const Iterator & xi_irange) const {
            return (xi_step > 0) ? (xi_irange.value > value) : (xi_irange.value < value);
        }

        // '()' override
        int operator*() const {
            return value;
        }
    };

    // constructor
    explicit constexpr irange(const int & xi_begin, const int & xi_end) : mBegin(xi_begin), mEnd(xi_end) {
        if ((xi_step < 0) && (mBegin < mEnd)) {
            throw std::out_of_range("irange: Final value is larger then initial value (it should be reversed for a negative stride).");
        }
        else if ((xi_step > 0) && (mEnd < mBegin)) {
            throw std::out_of_range("irange: Initial value is larger then final value.");
        }
    }

    // iterators
    Iterator begin() const { return mBegin; }
    Iterator end()   const { return mEnd; }

    // make it non copyable
    irange()               = delete;
    irange(const irange &) = delete;
};
