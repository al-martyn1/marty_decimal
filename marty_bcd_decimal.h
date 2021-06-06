#pragma once


#include <string>
#include <cstdint>
#include <utility>
#include <limits>
#include <iostream>
#include <exception>
#include <stdexcept>
#include <cstdio>

#define MARTY_DECIMAL_H__079F0131_B01B_44ED_BF0F_8DFECAB67FD7__


#ifndef MARTY_DECIMAL_DEFAULT_DIVISION_PRECISION
    #define MARTY_DECIMAL_DEFAULT_DIVISION_PRECISION   18
#endif


#include "marty_bcd.h"
#include "marty_relops_impl.h"


//----------------------------------------------------------------------------
namespace marty
{

class Decimal
{

public:

    //------------------------------
    enum class RoundingMethod
    {
        // https://en.wikipedia.org/wiki/Rounding

        roundingInvalid                               ,


        // Directed rounding to an integer methods

        roundDown                                     , // roundFloor, roundTowardNegInf
        roundFloor        = roundDown                 ,
        roundTowardNegInf = roundDown                 , // round towards negative infinity
        
        roundUp                                       , // roundCeil, roundTowardsPosInf
        roundCeil          = roundUp                  ,
        roundTowardsPosInf = roundUp                  , // round towards positive infinity

        roundTowardsZero                              , // roundAwayFromInf, roundTrunc
        roundAwayFromInf  = roundTowardsZero          ,
        roundTrunc        = roundTowardsZero          ,

        roundTowardsInf                               , // roundAwayFromZero
        roundAwayFromZero = roundTowardsInf           ,


        // Rounding to the nearest integer

        roundHalfUp                                   , // roundHalfTowardsPositiveInf
        roundHalfTowardsPositiveInf = roundHalfUp     ,

        roundHalfDown                                 , // roundHalfTowardsNegativeInf
        roundHalfTowardsNegativeInf = roundHalfDown   ,

        roundHalfTowardsZero                          , // roundHalfAwayFromInf
        roundHalfAwayFromInf  = roundHalfTowardsZero  ,

        roundHalfTowardsInf                           , // roundHalfAwayFromZero, roundMath
        roundHalfAwayFromZero = roundHalfTowardsInf   ,
        roundMath             = roundHalfTowardsInf   ,

        roundHalfToEven                               , // roundBankers, roundBanking, roundConvergent, roundStatistician, roundStatistic, roundDutch, roundGaussian
        roundBankers      = roundHalfToEven           ,
        roundBanking      = roundHalfToEven           ,
        roundConvergent   = roundHalfToEven           ,
        roundStatistician = roundHalfToEven           ,
        roundStatistic    = roundHalfToEven           ,
        roundDutch        = roundHalfToEven           ,
        roundGaussian     = roundHalfToEven           ,

        roundHalfToOdd

    };

    //------------------------------
    static
    const char* getRoundingMethodName( RoundingMethod m );

    


    //------------------------------
    void assignFromString( const char        *pStr );
    void assignFromString( const std::string &str  );

    static Decimal fromString( const char        *pStr ) { Decimal res; res.assignFromString(pStr); return res; }
    static Decimal fromString( const std::string &str  ) { Decimal res; res.assignFromString(str ); return res; }


//----------------------------------------------------------------------------
protected:

    void assignFromIntImpl( std::int64_t  i, int precision = 0 );
    void assignFromIntImpl( std::uint64_t u, int precision = 0 );

    void assignFromDoubleImpl( double d, int precision = 15 );

//----------------------------------------------------------------------------
public:


    //------------------------------
    #define MARTY_BCD_DECIMAL_CLASS_IMPLEMENT_FROM_INT_METHODS( castType, intType )                                                           \
                  void assignFromInt( intType i, int precision = 0 )      { assignFromIntImpl( (castType)i ); m_precision = precision; }      \
                  static Decimal fromInt( intType i, int precision = 0 )  { Decimal res; res.assignFromInt(i, precision);  return res; }

    MARTY_BCD_DECIMAL_CLASS_IMPLEMENT_FROM_INT_METHODS( std::int64_t , std::int64_t  )
    MARTY_BCD_DECIMAL_CLASS_IMPLEMENT_FROM_INT_METHODS( std::int64_t , int           )
    MARTY_BCD_DECIMAL_CLASS_IMPLEMENT_FROM_INT_METHODS( std::uint64_t, std::uint64_t )
    MARTY_BCD_DECIMAL_CLASS_IMPLEMENT_FROM_INT_METHODS( std::uint64_t, unsigned      )


    //------------------------------
    #define MARTY_BCD_DECIMAL_CLASS_IMPLEMENT_FROM_FLOAT_METHODS( castType, floatType, defPrecision )                                          \
                  void assignFromFloat( floatType f, int precision = defPrecision )      { assignFromDoubleImpl( (castType)f, precision ); }             \
                  static Decimal fromFloat( floatType f, int precision = defPrecision )  { Decimal res; res.assignFromFloat(f, precision);  return res; }

    MARTY_BCD_DECIMAL_CLASS_IMPLEMENT_FROM_FLOAT_METHODS( double, double, std::numeric_limits<double>::digits10 /* 15 */  )
    MARTY_BCD_DECIMAL_CLASS_IMPLEMENT_FROM_FLOAT_METHODS( double, float , std::numeric_limits<float> ::digits10 /*  6 */  )


    //------------------------------
    const char* toString( char *pBuf, std::size_t bufSize, int precision = -1, char dot = 0 ) const;
    std::string toString( int precision = -1, char dot = 0 ) const;

    const char* to_string( char *pBuf, std::size_t bufSize, int precision = -1, char dot = 0 ) const { return toString(pBuf, bufSize, precision, dot); }
    std::string to_string( int precision = -1, char dot = 0 )                                  const { return toString(precision, dot); }

#if 0

    int           toInt() const                           { return (int)(m_num/m_denum.denum()); }
    unsigned      toUnsigned() const                      { return (unsigned)(m_num/m_denum.denum()); }
    std::int64_t  toInt64() const                         { return (m_num/m_denum.denum()); }
    std::uint64_t toUnsigned64() const                    { return (std::uint64_t)(m_num/m_denum.denum()); }
    //------------------------------
#endif

    double        toDouble() const;
    float         toFloat () const  { return (float)toDouble(); }


//----------------------------------------------------------------------------
protected:

    template<typename IntType>
    IntType toIntImpl() const
    {
        throw std::runtime_error("marty::Decimal::toInt: not implemented for this type");
    }

    std::uint64_t getAsUint64() const
    {
        return bcd::rawToInt( m_number, m_precision );
    }

    template<>
    std::int64_t toIntImpl<std::int64_t>() const
    {
        if (m_sign==0)
            return 0;
        else if (m_sign>0)
            return (std::int64_t)getAsUint64();
        else 
            return -(std::int64_t)getAsUint64();
    }

    template<>
    std::uint64_t toIntImpl<std::uint64_t>() const
    {
        return (std::uint64_t)toIntImpl<std::int64_t>();
    }


//----------------------------------------------------------------------------
public:

    //------------------------------
    bool checkIsExact( const std::string &strDecimal  ) const;
    bool checkIsExact( const char        *pStrDecimal ) const;

    //------------------------------



    //------------------------------
    Decimal( ) : m_sign(0), m_precision(0), m_number()  {}
    Decimal( const Decimal &d ) : m_sign(d.m_sign), m_precision(d.m_precision), m_number(d.m_number)  {}

    Decimal( int            v, int precision = 0 ) { assignFromInt( v, precision ); }
    Decimal( unsigned       v, int precision = 0 ) { assignFromInt( v, precision ); }
    Decimal( std::int64_t   v, int precision = 0 ) { assignFromInt( v, precision ); }
    Decimal( std::uint64_t  v, int precision = 0 ) { assignFromInt( v, precision ); }

    Decimal( double         d, int precision = 12 ) { assignFromFloat( d, precision ); }
    Decimal( float          d, int precision = 6 ) { assignFromFloat( d, precision ); }

    Decimal( const char        *pStr ) { assignFromString( pStr ); }
    Decimal( const std::string &str  ) { assignFromString( str  ); }

    //------------------------------




    //------------------------------
    int compare( const Decimal &rightOp ) const;
    MARTY_IMPLEMENT_RELATIONAL_OPERATORS_BY_COMPARE( Decimal )

    //------------------------------

    
    
    //------------------------------
    void swap( Decimal &d2 );
    Decimal& operator=( Decimal d2 );

    //------------------------------



    //----------------------------------------------------------------------------
    Decimal& add( const Decimal &d );
    Decimal& sub( const Decimal &d );
    Decimal& mul( const Decimal &d );
    Decimal& div( const Decimal &d, int precision = -1, unsigned numSignificantDigits = (unsigned)-1 );

    Decimal  mod_helper_raw_div( const Decimal &d ) const; //!< Возвращает частное (с одним макс знаком после запятой, по модулю, без учёта знака)
    Decimal  mod_helper( const Decimal &d ) const; //!< Возвращает целое (в виде Decimal), сколько раз 'd' влезает в текущее (по модулю) - для реализации получения частного и остатка от деления, кто как пожелает на свой вкус

    //----------------------------------------------------------------------------

    
    
    //----------------------------------------------------------------------------
    Decimal operator + ( Decimal d2 ) const { Decimal res = *this; return res.add(d2); }
    Decimal operator - ( Decimal d2 ) const { Decimal res = *this; return res.sub(d2); }
    Decimal operator - (            ) const { Decimal res = *this; if (res.m_sign!=0) res.m_sign = -res.m_sign; return res; }
    Decimal operator * ( Decimal d2 ) const { Decimal res = *this; return res.mul(d2); }
    //Decimal divide( Decimal devider, precision_t resultPrecision ) const;
    Decimal operator / ( Decimal d2 ) const { Decimal res = *this; return res.div(d2, m_divisionPrecision); }
    // Decimal operator % ( Decimal d2 ) const;
    Decimal& operator += ( Decimal d2 ) { return add(d2); }
    Decimal& operator -= ( Decimal d2 ) { return sub(d2); }
    Decimal& operator *= ( Decimal d2 ) { return mul(d2); }
    Decimal& operator /= ( Decimal d2 ) { return div(d2, m_divisionPrecision); }
    // Decimal& operator %= ( Decimal d2 );

    //----------------------------------------------------------------------------
    #define MARTY_DECIMAL_IMPLEMENT_ARIPHMETICT_OVERLOADS_FOR_INTEGRAL_TYPE( integralType )         \
                                                                                                    \
                Decimal operator +   ( integralType i ) const { return operator+ ( Decimal(i) ); }  \
                Decimal operator -   ( integralType i ) const { return operator- ( Decimal(i) ); }  \
                Decimal operator *   ( integralType i ) const { return operator* ( Decimal(i) ); }  \
                Decimal operator /   ( integralType i ) const { return operator/ ( Decimal(i) ); }  \
                /*Decimal operator %   ( integralType i ) const { return operator% ( Decimal(i) ); }*/  \
                                                                                                    \
                Decimal& operator += ( integralType i )       { return operator+=( Decimal(i) ); }  \
                Decimal& operator -= ( integralType i )       { return operator-=( Decimal(i) ); }  \
                Decimal& operator *= ( integralType i )       { return operator*=( Decimal(i) ); }  \
                Decimal& operator /= ( integralType i )       { return operator/=( Decimal(i) ); }  \
                /*Decimal& operator %= ( integralType i )       { return operator%=( Decimal(i) ); }*/


    MARTY_DECIMAL_IMPLEMENT_ARIPHMETICT_OVERLOADS_FOR_INTEGRAL_TYPE( int           )
    MARTY_DECIMAL_IMPLEMENT_ARIPHMETICT_OVERLOADS_FOR_INTEGRAL_TYPE( unsigned      )
    MARTY_DECIMAL_IMPLEMENT_ARIPHMETICT_OVERLOADS_FOR_INTEGRAL_TYPE( std::int64_t  )
    MARTY_DECIMAL_IMPLEMENT_ARIPHMETICT_OVERLOADS_FOR_INTEGRAL_TYPE( std::uint64_t )
    MARTY_DECIMAL_IMPLEMENT_ARIPHMETICT_OVERLOADS_FOR_INTEGRAL_TYPE( float         )
    MARTY_DECIMAL_IMPLEMENT_ARIPHMETICT_OVERLOADS_FOR_INTEGRAL_TYPE( double        )



    //------------------------------
    // операторы преобразования типа 
    explicit operator std::int64_t() const  { return           toIntImpl<std:: int64_t>(); }
    explicit operator std::uint64_t() const { return           toIntImpl<std::uint64_t>(); }
    explicit operator int() const           { return (int)     toIntImpl<std:: int64_t>(); }
    explicit operator unsigned() const      { return (unsigned)toIntImpl<std::uint64_t>(); }
    explicit operator float() const         { return           toFloat(); }
    explicit operator double() const        { return           toDouble(); }


    //----------------------------------------------------------------------------

    Decimal& zerate()   { m_sign  =  0; return *this; } //!< Zerate-Mazerate ;) makes *this zero valued
    Decimal& zeroise()  { m_sign  =  0; return *this; }
    Decimal& negate()   { m_sign *= -1; return *this; }
    Decimal& invert()   { return negate(); }
    Decimal& reciprocate( int precision = MARTY_DECIMAL_DEFAULT_DIVISION_PRECISION ); //!< multiplicative inverse this: *this = 1/(*this) \returns *this

    //------------------------------

    int      signum() const { return m_sign; }
    int      sign  () const { return m_sign; }
    int      sgn   () const { return m_sign; }
    Decimal  abs   () const { Decimal res = *this; if (res.m_sign) res.m_sign = 1; return res; } 
    //Decimal  mod   (  Decimal d2 ) const;
    Decimal  neg   () const { Decimal res = *this; res.negate(); return res; }
    Decimal  inv   () const { return neg(); }

    //------------------------------
    Decimal  zerated()   const { return Decimal(0u); }
    Decimal  zeroised()  const { return Decimal(0u); }
    Decimal  negated()   const { return neg(); }
    Decimal  inverted()  const { return inv(); }
    

    //! return multiplicative inverse - 1/(*this)
    Decimal  reciprocated( int precision = MARTY_DECIMAL_DEFAULT_DIVISION_PRECISION ) const;

    //------------------------------
    bool     zer   () const { return m_sign==0; }
    bool     zero  () const { return m_sign==0; }
    bool     isZero() const { return m_sign==0; }
    bool     zeq   () const { return m_sign==0; } //!< zeq means "Zero EQual" - ==0. Also this is the russian joke - зек (ЗК) - means a prisoner

    //------------------------------

    int      msp   () const { return getMostSignificantDigitPower(); } //!< return Most Significant digit Power 

    //------------------------------




    //------------------------------
    //! Возвращает true, если обрезание/удлинение прошло предельно точно и ничего лишнего не было задето, и все жизненно важные органы остались на месте
    bool precisionExpandTo( int p ); //!< Всегда возвращает true
    bool precisionShrinkTo( int p ); //!< Возвращает true, если все обрезанные цифры были нулём
    bool precisionFitTo( int p );    //!< Возвращает true, если все обрезанные цифры были нулём (если было обрезание), или true (если было расширение или ничего)

    //------------------------------
    //! Цифра чётна?
    static
    bool isDigitEven( int d );

    //! Цифра нечётна?
    static
    bool isDigitOdd( int d );

    //------------------------------
    //Decimal& minimizePrecisionImpl();
    int getLowestDecimalDigit() const;
    //void replaceLowestDecimalDigit( unum_t d );

    int getMostSignificantDigitPower() const;


//----------------------------------------------------------------------------
protected:

    Decimal makeMinimalPrecisionImplHelper( int newNum ) const;


//----------------------------------------------------------------------------
public:

    Decimal makeMinimalPrecisionOne() const;
    Decimal makeMinimalPrecisionTwo() const;
    Decimal makeMinimalPrecisionFive() const;

    //------------------------------



    //------------------------------

    static int  getOutputPrecision()                   { return m_outputPrecision; }
    static void setOutputPrecision( int p )            { m_outputPrecision = p; }
    static void setOutputPrecisionToStreamPrecision( ) { setOutputPrecision(0); }
    static void setOutputPrecisionToAuto( )            { setOutputPrecision(-1); }

    int precision() const { return m_precision; }

    //------------------------------
    static int  getDivisionPrecision()                                                  { return m_divisionPrecision; }
    static void setDivisionPrecision( int p = MARTY_DECIMAL_DEFAULT_DIVISION_PRECISION) { m_divisionPrecision = p; }

    static unsigned  getDivisionNumberOfSignificantDigits()             { return m_divisionNumberOfSignificantDigits; }
    static void      setDivisionNumberOfSignificantDigits( unsigned n ) { m_divisionNumberOfSignificantDigits    = n; }


    //------------------------------
    //! Return what part of d is *this (in percents/permilles) - d is 100%
    Decimal  getPercentOf ( const Decimal &d ) const;
    Decimal  getPermilleOf( const Decimal &d ) const;

    Decimal  getExPercentOf ( const Decimal &d, int precision, unsigned numSignificantDigits ) const;
    Decimal  getExPermilleOf( const Decimal &d, int precision, unsigned numSignificantDigits ) const;


    //! Return what part of *this is d (in percents/permilles) - *this is 100%
    Decimal  getPercent   ( const Decimal &d ) const { return d.getPercentOf (*this); }
    Decimal  getPermille  ( const Decimal &d ) const { return d.getPermilleOf(*this); }

    Decimal  getExPercent ( const Decimal &d, int precision, unsigned numSignificantDigits ) const { return d.getExPercentOf (*this, precision, numSignificantDigits); }
    Decimal  getExPermille( const Decimal &d, int precision, unsigned numSignificantDigits ) const { return d.getExPermilleOf(*this, precision, numSignificantDigits); }


    //------------------------------
    Decimal& round  ( int precision, RoundingMethod roundingMethod );
    Decimal  rounded( int precision, RoundingMethod roundingMethod ) const;

    //------------------------------
    static char setDefaultDecimalSeparator( char sep ) { char res = decimalSeparator; decimalSeparator = sep; return res; }
    static char getDefaultDecimalSeparator( )          { return decimalSeparator; }



//----------------------------------------------------------------------------
protected:

    Decimal& roundingImpl2( int requestedPrecision, RoundingMethod roundingMethod );
    Decimal& roundingImpl1( int requestedPrecision, RoundingMethod roundingMethod );
    Decimal& roundingImpl ( int requestedPrecision, RoundingMethod roundingMethod );

    Decimal implGetPercentOf ( const Decimal &scale, const Decimal &d ) const;
    Decimal implGetExPercentOf ( const Decimal &scale, const Decimal &d, int precision, unsigned numSignificantDigits ) const;


    Decimal( int sign, int precision, bcd::raw_bcd_number_t number )
    : m_sign(sign), m_precision(precision), m_number(number)
    {}

    int                     m_sign;
    int                     m_precision;
    bcd::raw_bcd_number_t   m_number;
    //------------------------------
    // Global affecting options

    // if == -1  - Exact Decimal number precision will be used
    // if ==  0  - Output stream precision will be used
    // if >   0  - Exact precision will be used
    inline static int       m_outputPrecision = -1;

    inline static int       m_divisionPrecision          = MARTY_DECIMAL_DEFAULT_DIVISION_PRECISION;
    inline static unsigned  m_divisionNumberOfSignificantDigits = 3;

    //------------------------------
    inline static char decimalSeparator = '.';



}; // class Decimal

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------

#include "marty_bcd_decimal_impl.h"

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
inline
std::ostream& operator<<( std::ostream& os, const Decimal &v )
{
    int precision = Decimal::getOutputPrecision();

    if (precision==-1 || precision<0)
    {
        // Exact Decimal number precision will be used
        precision = v.precision();
        // if (precision > Decimal::maxPrecision())
        //     precision = Decimal::maxPrecision();
    }
    else if (precision>0)
    {
        // Exact global output precision will be used
        // if (precision > Decimal::maxPrecision())
        //     precision = Decimal::maxPrecision();
    }
    else // precision==0
    {
        // Output stream precision will be used
        precision = (int)os.width();
        // if (precision > Decimal::maxPrecision())
        //     precision = Decimal::maxPrecision();
    }


    //auto minPrecision = (Decimal::precision_t)os.precision();
    //if (minPrecision<1)
    //    minPrecision = 1;
    os << v.toString(precision);
    return os;
}

//----------------------------------------------------------------------------
#define MARTY_DECIMAL_IMPLEMENT_ARIPHMETICT_OVERLOADS_FOR_INTEGRAL_TYPE_FRIENDS( integralType )               \
                                                                                                      \
            inline Decimal operator +   ( integralType i, const Decimal d ) { return Decimal(i) + d; }  \
            inline Decimal operator -   ( integralType i, const Decimal d ) { return Decimal(i) - d; }  \
            inline Decimal operator *   ( integralType i, const Decimal d ) { return Decimal(i) * d; }  \
            inline Decimal operator /   ( integralType i, const Decimal d ) { return Decimal(i) / d; }  \
            /*Decimal operator %   ( integralType i ) { return operator% ( Decimal(i) ); }*/

MARTY_DECIMAL_IMPLEMENT_ARIPHMETICT_OVERLOADS_FOR_INTEGRAL_TYPE_FRIENDS( int           )
MARTY_DECIMAL_IMPLEMENT_ARIPHMETICT_OVERLOADS_FOR_INTEGRAL_TYPE_FRIENDS( unsigned      )
MARTY_DECIMAL_IMPLEMENT_ARIPHMETICT_OVERLOADS_FOR_INTEGRAL_TYPE_FRIENDS( std::int64_t  )
MARTY_DECIMAL_IMPLEMENT_ARIPHMETICT_OVERLOADS_FOR_INTEGRAL_TYPE_FRIENDS( std::uint64_t )
MARTY_DECIMAL_IMPLEMENT_ARIPHMETICT_OVERLOADS_FOR_INTEGRAL_TYPE_FRIENDS( float         )
MARTY_DECIMAL_IMPLEMENT_ARIPHMETICT_OVERLOADS_FOR_INTEGRAL_TYPE_FRIENDS( double        )


// Obsilette
//template<typename T>
//Decimal fromString( const T &t ) { return Decimal::fromString(t); }




} // namespace marty







//----------------------------------------------------------------------------

