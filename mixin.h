/**
* compile time mixin
*
* Dan Israel Malta
**/
#include <utility>

// --- the 'mixin' driver ---
template<template<class> class... Mixins> struct CRTP_HOST : public Mixins<CRTP_HOST<Mixins...>>... {
    CRTP_HOST(Mixins<CRTP_HOST>&&... args) : Mixins<CRTP_HOST>(std::forward<Mixins<CRTP_HOST>>(args))... {}
};

// --- simple usage example #1 ---

template <class CRTP_HOST> struct Mix1 { Mix1(int,double);};
template <class CRTP_HOST> struct Mix2 { Mix2(char); };

int main (void) {
  using TopHost = CRTP_HOST<Mix1, Mix2>;
  delete new TopHost({10,1.3}, {'A'});
}

// --- simple usage example #2 ---
template<struct CRTP_HOST> struct VisibleOnCT {/*...*/};
template<struct CRTP_HOST> struct VisibleOn3D {/*...*/};
template<struct CRTP_HOST> struct AttachedToMesh {/*...*/};
template<struct CRTP_HOST> struct DetachedFromMesh {/*...*/};
template<struct CRTP_HOST> struct EditedByGimbalAll {/*...*/};
template<struct CRTP_HOST> struct EditedByGimbalTranslation {/*...*/};
template<struct CRTP_HOST> struct EditedByGimbalRotation {/*...*/};

using Landmark    = CRTP_HOST<VisibleOn3D, VisibleOnCT, AttachedToMesh, DetachedFromMesh, EditedByGimbalTranslation>;
using Measurement = CRTP_HOST<VisibleOn3D, VisibleOnCT, AttachedToMesh, EditedByGimbalTranslation>;
using RubberBand  = CRTP_HOST<VisibleOn3D, VisibleOnCT>;
using cut         = CRTP_HOST<VisibleOn3D, VisibleOnCT, EditedByGimbalAll>;
