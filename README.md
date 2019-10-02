The following functions/examples exist in the repository:

* enumerate.h - intuitive enumeration of a list of things

* zip_iterator.h - parallel-iterate over several controlled heterogeneous sequences simultaneously

* ArrayOfBytes.h - helper object to deal with integral types as array of bytes

* expand_stl.h - extend many STL algorithms to operate on homogeneous parameter packs or a variadic amount of collections where each collection can be of a different type but must hold the same underlying type.

* ContainerSOA.h - allow user to iterate a given collection either in SoA style or in AoS style.

* irange.h - A flexible range based for loop implementation which allows both looping in reverse order and looping with a given stride

* FSM.h - minimal generic finite state machine

* LazyStringSplit.h - Lazy string splitting and iteration

* EncryptedString.h - compile time encrypted, run time decrypted string

* Reactive.h - minimal functional reactive programming kit (in less then 150 lines of code)

* async_deferred.h - an emulated 'std::async' with 'deferred' option with the added value that it doesn't block until thread is finished when returned future is destructed

* mixin.h - implementing the 'mixin' design pattern without any run-time costs

* enumeration_casts.h - a set of utilities to safely handle enumeration<->number casting (with compile time checks)

* GenericVisitor.h - A generic visitor (with example usage)

* NamedArguments.cpp - An example of how to emulate a function with named arguments in c++

* Dictionary.h - compile-time fixed-size bi-directional map (dictionary) for integer types

* arithmetic_comparison.h - safe (no implicit casting) arithmetical value comparison with no run time costs
