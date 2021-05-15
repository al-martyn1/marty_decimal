#pragma once

#include <cstdint>
#include <climits>
#include <limits>
#include <exception>
#include <stdexcept>

namespace marty
{



// cli/numeric.h
namespace int_helpers
{

template<typename IntType> inline
IntType makeCharSizeMask()
{
    return (IntType)(unsigned char)-1;
}

// https://en.cppreference.com/w/cpp/types/numeric_limits
// https://en.cppreference.com/w/cpp/types/climits
// template<> class numeric_limits<unsigned char>;


template<typename IntType> inline
IntType makeHighBitMask( )
{
    return ((IntType)1) << (numBits<IntType>()-1);
}


template<typename IntType> inline
unsigned numBits()
{
    return sizeof(IntType)*numBits<unsigned char>();
}

template<> inline
unsigned numBits<unsigned char>()
{
    return (unsigned)CHAR_BIT;
}

template<typename IntType> inline
std::size_t getShiftSize()
{
    return (sizeof(IntType)*numBits<unsigned char>());
}

template<typename IntType> inline
std::size_t getHalfShiftSize()
{
    return getShiftSize<IntType>() / 2;
}



template<typename IntType> inline
IntType makeMaskLowHalf()
{
    IntType mask = makeCharSizeMask<IntType>(); //  char_size_mask();
    IntType resMask = mask;
    for(unsigned i=0; i!=(unsigned)(sizeof(IntType)/2); ++i)
       {
        resMask |= mask;
        mask   <<= numBits<unsigned char>(); // char_bits();
       }
    return resMask;
}

//! Returns low part of integer in lower bits
template<typename IntType> inline
IntType getLowHalf( IntType i )
{
    return i & makeMaskLowHalf<IntType>();
}

//! Returns high part of integer in lower bits (shifted by size/2)
template<typename IntType> inline
IntType getHighPart( IntType i ) 
{
    return (i >> getHalfShiftSize<IntType>()) & makeMaskLowHalf<IntType>();
}

#if 0


template<typename IntType> inline
IntType getHighPartSigned( IntType i ) 
{
    if (i & makeHighBitMask<IntType>())
    {
        IntType res = getHighPart( i );
        res |= makeMaskLowHalf<IntType>() << getHalfShiftSize<IntType>();
        return res;
    }

    return getHighPart( i );;
}



// returns true if overflow detected
// IntType must be unsigned type
// TODO: !!! Need to be updated with new C++ features
template<IntType> inline
bool unsignedAddition( IntType a, IntType b, IntType *c = 0 )
{
    IntType r = a + b; if (c) *c = r; return !(r>=a && r>=b);
}

template<IntType> inline
bool unsignedAddition( IntType a, IntType b, IntType &c )
{
    IntType r = a + b; c = r; return !(r>=a && r>=b);
}

// returns true if carry detected
template<IntType> inline
bool unsignedSubtraction( IntType a, IntType b, IntType *c = 0 )
{
    IntType r = a - b; if (c) *c = r; return a<b;
}

template<IntType> inline
bool unsignedSubtraction( IntType a, IntType b, IntType &c )
{
    IntType r = a - b; c = r; return a<b;
}




template<typename UnsignedIntType1, typename UnsignedIntType2>
void unsignedMultiplyNumbersMnImpl( UnsignedIntType1 *w
           , const UnsignedIntType1 *u, const UnsignedIntType1 *v
           , unsigned m, unsigned n
           )
{
    UIntT2 k, t; //, b;
    unsigned i, j; // counters

    for (i=0; i<m; i++)
        w[i] = 0;

    for(j=0; j!=n; j++)
       {
        k = 0;
        for(i=0; i!=m; i++)
           {
            //t = u[i]   * v[j] 
            //  + w[i+j] + k;
            t = static_cast<UnsignedIntType1>(u[i])   * static_cast<UnsignedIntType1>(v[j]) 
              + static_cast<UnsignedIntType1>(w[i+j]) + static_cast<UnsignedIntType1>(k);
            w[i+j] = static_cast<UnsignedIntType1>(getLowHalf(t)); // signed? t; // (Т.е., t & OxFFFF).
            k = getHighPart(t); // signed? t >> 16;
           }
        w[j+m] = static_cast<UnsignedIntType1>(k);
       }
}


#endif




} // namespace int_helpers


//! Тупое умножение в столбик беззнаковых чисел произвольной длины, 1 разряд - 2 байта, младшие - сначала
/*! uint16_t используется как 1 разряд для того, чтобы два разряда влезало в 32 бита, что доступно на контроллерах
 */
inline
void multiplicationOfUnsignedLongInts( const std::uint16_t *pM1 , std::size_t szM1
                                     , const std::uint16_t *pM2 , std::size_t szM2
                                     ,       std::uint16_t *pRes, std::size_t szResBuf // high parts will be skipped if szResBuf is not enough
                                     )
{
    const std::size_t resIdxEnd   = szResBuf;

    for( std::size_t k=0; k!=resIdxEnd; ++k)
       pRes[k] = 0;


    for( std::size_t i=0; i!=szM1; ++i)
    {
        if (!*pM1)
            continue;

        for( std::size_t j=0; j!=szM2; ++j)
        {
            std::size_t resIdxBegin = i+j;

            std::uint16_t m1 = pM1[i];
            std::uint16_t m2 = pM2[j];

            if (!m2)
               continue;

            std::uint32_t tmpRes = ((std::uint32_t)m1) * ((std::uint32_t)m2);

            for( std::size_t k=resIdxBegin; k!=resIdxEnd; ++k)
            {
                tmpRes += (std::uint32_t)pRes[k];
                pRes[k] = (std::uint16_t)tmpRes; // усекаем отрубанием старшей части

                tmpRes >>= (unsigned)int_helpers::getHalfShiftSize<std::uint32_t>(); // Удаляем младшую часть, старшую - на её место
                if (!tmpRes)
                    break;
            }

        } // for( std::size_t j=0; j!=szM2; ++j)

    } // for( std::size_t i=0; i!=szM1; ++i)
}

//! Тупое сложение в столбик беззнаковых чисел произвольной длины, 1 разряд - 2 байта, младшие - сначала
/*! uint16_t используется как 1 разряд для того, чтобы два разряда влезало в 32 бита, что доступно на контроллерах
    \returns true, если бпроизошло переполнение
 */
inline
bool summationOfUnsignedLongInts( const std::uint16_t *pS1 , std::size_t szS1
                                , const std::uint16_t *pS2 , std::size_t szS2
                                ,       std::uint16_t *pRes, std::size_t szResBuf // high parts will be skipped if szResBuf is not enough
                                )
{
    if (szS1!=szS2)
        throw std::runtime_error("summationOfUnsignedLongInts: addendums must have the same lenght");
    if (szS1!=szResBuf)
        throw std::runtime_error("summationOfUnsignedLongInts: addendums and result must have the same lenght");

    std::uint32_t tmpRes = 0;

    for(std::size_t i=0; i!=szS1; ++i)
    {
        tmpRes >>= (unsigned)int_helpers::getHalfShiftSize<std::uint32_t>(); // Удаляем младшую часть, старшую - на её место
        tmpRes += pS1[i];
        tmpRes += pS2[i];

        pRes[i] = (std::uint16_t)tmpRes;

    }

    tmpRes >>= (unsigned)int_helpers::getHalfShiftSize<std::uint32_t>(); // Удаляем младшую часть, старшую - на её место

    return tmpRes!=0;
}


template <typename UnsignedType>
constexpr /* inline */ 
std::size_t calcNumberOfUint16ItemsInLong()
{
    return sizeof(UnsignedType) / sizeof(std::uint16_t);
}


template <typename UnsignedType> inline
void convertToLongUnsigned( UnsignedType u
                          , std::uint16_t *pLongUnsigned // buf must be enough 
                          )
{
    const std::size_t numParts = calcNumberOfUint16ItemsInLong<UnsignedType>();

    for( std::size_t i=0; i!=numParts; ++i, ++pLongUnsigned)
    {
        *pLongUnsigned = (std::uint16_t)u;
        u >>= int_helpers::getShiftSize<std::uint16_t>();
    }

}

template <typename UnsignedType>
inline
void convertFromLongUnsigned( UnsignedType &u
                            , std::uint16_t *pLongUnsigned
                            , std::size_t    bufSize // in counts of std::uint16_t
                            )
{
    const std::size_t end = (std::size_t)-1;

    const std::size_t unsignedTypeSizeInUint16Items = sizeof(UnsignedType) / sizeof(std::uint16_t);

    // Выбираем меньшее из двух размеров
    const std::size_t numItems = (unsignedTypeSizeInUint16Items < bufSize) ? unsignedTypeSizeInUint16Items : bufSize;

    u = 0;

    for( std::size_t i=(numItems-1); i!=end; --i)
    {
         u <<= int_helpers::getShiftSize<std::uint16_t>();
         u  |= (UnsignedType)pLongUnsigned[i];
    }

}




// int_helpers::getHalfShiftSize()
// int_helpers::makeMaskLowHalf()
// int_helpers::numBits()



} // namespace marty


