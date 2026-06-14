#pragma once

#include <cstdint>

namespace Prism
{

struct Scalar
{
    union {
        bool     Bool;
        int32_t  Int;
        uint32_t UInt;
        float    Float;
    };

    Scalar() : Int(0) {}

    static Scalar FromBool(bool v)      { Scalar s; s.Bool = v;  return s; }
    static Scalar FromInt(int32_t v)    { Scalar s; s.Int = v;   return s; }
    static Scalar FromUInt(uint32_t v)  { Scalar s; s.UInt = v;  return s; }
    static Scalar FromFloat(float v)    { Scalar s; s.Float = v; return s; }
};

} // namespace Prism
