#pragma once

#define MARTY_DECIMAL_H__F7FEF215_956E_4063_8AD1_1F66F6E17EB7__

#include <string>
#include <cstdint>
#include <utility>
#include <limits>
#include <iostream>
#include <exception>
#include <stdexcept>


//----------------------------------------------------------------------------
namespace marty
{


// Hm-m-m
// МСБ-Лизинг 002P выпуск 2
// Ticker: RU000A100DC4
// Price increment: 0.083336


//----------------------------------------------------------------------------
// 4 294 967 295 uint32_t max
// 1 000 000 000 - must be enough for money or use more wide type in typedef below

typedef std::uint32_t  DecimalDenumeratorRawType;
typedef std::int32_t   DecimalDenumeratorSignedRawType;

//----------------------------------------------------------------------------
//  9 223 372 036 854 775 808
//  1 000 000 000.000 000  000 - 10**9 -  миллиард - бюджет страны верстать не выйдет с точностью 9 нулей после точки
//  1 000 000 000 000 00.0 000 - 10**15 - 100 трлн с точностью до сотых копейки - для домашней бухгалтерии норм

typedef std::int64_t    DecimalNumeratorRawType;
typedef std::uint64_t   DecimalNumeratorUnsignedRawType;



//----------------------------------------------------------------------------
class DecimalDenumerator;
class Decimal;

class DecimalPrecision
{
    friend class DecimalDenumerator;
    friend class Decimal;

    std::uint32_t m_precision;

    // default constructor disabled
    DecimalPrecision( );
    //DecimalPrecision& operator=( const DecimalPrecision &dp );

public:

    typedef std::uint32_t                    precision_t;
    typedef DecimalDenumeratorRawType        denum_t;
    typedef DecimalDenumeratorSignedRawType  sdenum_t;

    static denum_t denum( precision_t pr )
    {
        if ( maxPrecision<denum_t>() < pr )
            throw std::runtime_error("DecimalPrecision::denum - taken precision to big");

        return (denum_t)(getPowers10Table()[pr]);

        /*
        denum_t  denum = 1;
        for(precision_t i=0; i!=pr; ++i)
        {
            denum_t next = denum * 10u;
            if (next<denum)
                throw std::runtime_error("DecimalPrecision precision to big");
            denum = next;
        }

        return denum;
        */
    }

    DecimalPrecision( std::uint32_t pr ) : m_precision(pr) {}
    DecimalPrecision( const DecimalPrecision &pr ) : m_precision(pr.m_precision) {}

    denum_t denum( ) const         { return denum(m_precision);  } // for compatibility
    denum_t denumerator( ) const   { return denum(m_precision);  }

    precision_t prec() const       { return m_precision; }  // for compatibility
    precision_t precision() const  { return m_precision; }

protected:

    template<typename IntType>
    static precision_t maxPrecision()    { throw std::runtime_error("DecimalPrecision::maxPrecision not implemented"); }

    template<> static precision_t maxPrecision<std::int32_t >() { return  9; }
    template<> static precision_t maxPrecision<std::uint32_t>() { return  9; }
    template<> static precision_t maxPrecision<std::int64_t >() { return 18; }
    template<> static precision_t maxPrecision<std::uint64_t>() { return 18; }


    static std::uint64_t* getPowers10Table()
    {
        static std::uint64_t _[] = 
        { 1                             //  0
        , 10                            //  1
        , 100                           //  2
        , 1000                          //  3
        , 10000                         //  4
        , 100000                        //  5
        , 1000000                       //  6
        , 10000000                      //  7
        , 100000000                     //  8
        , 1000000000                    //  9 - max for int32_t
        , 10000000000                   // 10
        , 100000000000                  // 11
        , 1000000000000                 // 12
        , 10000000000000                // 13
        , 100000000000000               // 14
        , 1000000000000000              // 15
        , 10000000000000000             // 16
        , 100000000000000000            // 17
        , 1000000000000000000           // 18 - max for int64_t
        };
    
        return &_[0];
    }


}; // class DecimalPrecision

//----------------------------------------------------------------------------



#define MARTY_DECIMAL_IMPLEMENT_RELATIONAL_OPERATORS( typeT2 )\
                                                              \
    bool operator<( const typeT2 v2 ) const                   \
    {                                                         \
        return compare(v2)<0;                                 \
    }                                                         \
                                                              \
    bool operator<=( const typeT2 v2 ) const                  \
    {                                                         \
        return compare(v2)<=0;                                \
    }                                                         \
                                                              \
    bool operator>( const typeT2 v2 ) const                   \
    {                                                         \
        return compare(v2)>0;                                 \
    }                                                         \
                                                              \
    bool operator>=( const typeT2 v2 ) const                  \
    {                                                         \
        return compare(v2)>=0;                                \
    }                                                         \
                                                              \
    bool operator==( const typeT2 v2 ) const                  \
    {                                                         \
        return compare(v2)==0;                                \
    }                                                         \
                                                              \
    bool operator!=( const typeT2 v2 ) const                  \
    {                                                         \
        return compare(v2)!=0;                                \
    }





//----------------------------------------------------------------------------
class Decimal;

class DecimalDenumerator
{

public:

    typedef DecimalPrecision::denum_t       denum_t    ;
    typedef DecimalPrecision::sdenum_t      sdenum_t   ;
    typedef DecimalPrecision::precision_t   precision_t;

    friend class Decimal;

    DecimalDenumerator( const DecimalPrecision dp )
    : m_precision( dp.prec() )
    , m_denum    ( dp.denum() )
    {}

    DecimalDenumerator( const DecimalDenumerator &dd )
    : m_precision( dd.m_precision )
    , m_denum    ( dd.m_denum     )
    {}

    DecimalDenumerator& operator=( const DecimalDenumerator &dd )
    {
        m_precision = dd.m_precision;
        m_denum     = dd.m_denum    ;
        return *this;
    }

    precision_t prec       () const { return m_precision; } // for compatibility
    precision_t precision  () const { return m_precision; }

    denum_t     denum      () const { return m_denum; }     // for compatibility
    denum_t     denumerator() const { return m_denum; }

    DecimalPrecision decimalPrecision( ) const { return DecimalPrecision(m_precision); }

    static precision_t maxPrecision()
    {
        return DecimalPrecision::maxPrecision<denum_t>();
    }

    void swap( DecimalDenumerator &d2 );

    int compare( const DecimalDenumerator d2 ) const;

    MARTY_DECIMAL_IMPLEMENT_RELATIONAL_OPERATORS(DecimalDenumerator)


protected:

    void incPrec();
    void decPrec();
    denum_t precisionExpandTo( precision_t p );
    denum_t precisionShrinkTo( precision_t p );
    sdenum_t precisionFitTo( precision_t p );
    void tryIncDenum();
    void tryDecDenum();

    // default constructor disabled
    DecimalDenumerator();

    precision_t  m_precision;
    denum_t      m_denum;

}; // class DecimalDenumerator

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
inline
void swap( DecimalDenumerator &d1, DecimalDenumerator &d2 )
{
    d1.swap( d2 );
}

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
class Decimal
{


public:

    //------------------------------
    typedef DecimalNumeratorRawType             num_t      ;
    typedef DecimalNumeratorUnsignedRawType     unum_t     ;
    
    typedef DecimalDenumerator::denum_t         denum_t    ;
    typedef DecimalDenumerator::sdenum_t        sdenum_t   ;
    typedef DecimalDenumerator::precision_t     precision_t;

    typedef DecimalDenumerator                  DenumeratorType;

    //------------------------------
    friend Decimal     decimalFromString( const std::string &d );
    friend void swap( Decimal &d1, Decimal &d2 );

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
    static precision_t maxPrecision()    { return DenumeratorType::maxPrecision(); }
    precision_t precision() const        { return m_denum.precision(); }
    unum_t getPositiveNumerator() const  { return (unum_t)( (m_num<0) ? -m_num : m_num ); }

    //------------------------------
    //! Возвращает число десятичных знаков числа
    static precision_t findCurrentDecimalPower( unum_t unum )
    {
        precision_t curPower = 0;
        for(; unum!=0; unum /= 10, ++curPower ) {}
        return curPower;
    }

    static precision_t findCurrentDecimalPower( num_t num )
    {
        return (num < 0) ? findCurrentDecimalPower( (unum_t)(-num) ) : findCurrentDecimalPower( (unum_t)(num) );
    }

    precision_t findNumCurrentDecimalPower( ) const
    {
        return findCurrentDecimalPower(m_num);
    }

    //------------------------------
    //! Возвращает максимальный десятичный множитель (его степень), на который может быть умножено число без переполнения
    precision_t findMaxDecimalScalePower() const;

    //------------------------------
    static Decimal fromString( const std::string &strDecimal  )     { return decimalFromString(strDecimal ); }
    static Decimal fromString( const char        *pStrDecimal )     { return decimalFromString(std::string(pStrDecimal)); }

    std::string   toString( precision_t p = (Decimal::precision_t)-1 ) const; // { return decimalToString( *this, p ); }

    int           toInt() const                           { return (int)(m_num/m_denum.denum()); }
    unsigned      toUnsigned() const                      { return (unsigned)(m_num/m_denum.denum()); }
    std::int64_t  toInt64() const                         { return (m_num/m_denum.denum()); }
    std::uint64_t toUnsigned64() const                    { return (std::uint64_t)(m_num/m_denum.denum()); }
    float         toFloat() const                         { return (float)((double)m_num/(double)m_denum.denum()); }
    double        toDouble() const                        { return ((double)m_num/(double)m_denum.denum()); }
    //------------------------------



    //------------------------------
    bool checkIsExact( const std::string &strDecimal  ) const { return toString( (Decimal::precision_t)-1 )==strDecimal; }
    bool checkIsExact( const char        *pStrDecimal ) const { return checkIsExact(std::string(pStrDecimal)); }

    //------------------------------




    //------------------------------
    Decimal()                   : m_num(0)      , m_denum(DecimalPrecision(0))   {}
    Decimal( const Decimal &d ) : m_num(d.m_num), m_denum(d.m_denum)             {}

    //------------------------------
    // 
    Decimal( int            v, const DecimalPrecision &prec = DecimalPrecision(0) ) : m_num(v), m_denum(prec) {  /* m_num *= m_denum.denum(); */  }
    Decimal( unsigned       v, const DecimalPrecision &prec = DecimalPrecision(0) ) : m_num(v), m_denum(prec) {  /* m_num *= m_denum.denum(); */  }
    Decimal( std::int64_t   v, const DecimalPrecision &prec = DecimalPrecision(0) ) : m_num(v), m_denum(prec) {  /* m_num *= m_denum.denum(); */  }
    Decimal( std::uint64_t  v, const DecimalPrecision &prec = DecimalPrecision(0) ) : m_num(v), m_denum(prec) {  /* m_num *= m_denum.denum(); */  }

    //------------------------------
    //
    Decimal( float          f, const DecimalPrecision &prec = DecimalPrecision(0) ) : m_num(0), m_denum(prec) { fromFloat( f, prec ); }
    Decimal( double         f, const DecimalPrecision &prec = DecimalPrecision(0) ) : m_num(0), m_denum(prec) { fromFloat( f, prec ); }
    //------------------------------

    Decimal( const std::string &strDecimal  ) : m_num(0), m_denum(0) { *this = fromString(strDecimal ); }
    Decimal( const char        *pStrDecimal ) : m_num(0), m_denum(0) { *this = fromString(pStrDecimal); }

    //------------------------------
    #if 0
    template<typename IntType>
    static
    Decimal fromIntWithDenumerator( IntType i, Decimal::precision_t p /* denumenator pow10 */ )
    {
        Decimal resultPrecision= 
    }
    #endif



    //------------------------------



    //------------------------------
    void swap( Decimal &d2 );
    Decimal& operator=( Decimal d2 );
    int compare( Decimal d2 ) const;

    //------------------------------



    //------------------------------
    // операторы преобразования типа 
    explicit operator int() const           { return toInt(); }
    explicit operator unsigned() const      { return toUnsigned(); }
    explicit operator std::int64_t() const  { return toInt64(); }
    explicit operator std::uint64_t() const { return toUnsigned64(); }
    explicit operator float() const         { return toFloat(); }
    explicit operator double() const        { return toDouble(); }

    //------------------------------
    Decimal operator + ( Decimal d2 ) const;
    Decimal operator - ( Decimal d2 ) const;
    Decimal operator - ( ) const;
    Decimal operator * ( Decimal d2 ) const;
    Decimal divide( Decimal devider, precision_t resultPrecision ) const;
    Decimal operator / ( Decimal d2 ) const;
    Decimal operator % ( Decimal d2 ) const;
    Decimal& operator += ( Decimal d2 );
    Decimal& operator -= ( Decimal d2 );
    Decimal& operator *= ( Decimal d2 );
    Decimal& operator /= ( Decimal d2 );
    Decimal& operator %= ( Decimal d2 );

    //------------------------------
    #define MARTY_DECIMAL_IMPLEMENT_ARIPHMETICT_OVERLOADS_FOR_INTEGRAL_TYPE( integralType )         \
                                                                                                    \
                Decimal operator +   ( integralType i ) const { return operator+ ( Decimal(i) ); }  \
                Decimal operator -   ( integralType i ) const { return operator- ( Decimal(i) ); }  \
                Decimal operator *   ( integralType i ) const { return operator* ( Decimal(i) ); }  \
                Decimal operator /   ( integralType i ) const { return operator/ ( Decimal(i) ); }  \
                Decimal operator %   ( integralType i ) const { return operator% ( Decimal(i) ); }  \
                                                                                                    \
                Decimal& operator += ( integralType i )       { return operator+=( Decimal(i) ); }  \
                Decimal& operator -= ( integralType i )       { return operator-=( Decimal(i) ); }  \
                Decimal& operator *= ( integralType i )       { return operator*=( Decimal(i) ); }  \
                Decimal& operator /= ( integralType i )       { return operator/=( Decimal(i) ); }  \
                Decimal& operator %= ( integralType i )       { return operator%=( Decimal(i) ); }


    MARTY_DECIMAL_IMPLEMENT_ARIPHMETICT_OVERLOADS_FOR_INTEGRAL_TYPE( int           )
    MARTY_DECIMAL_IMPLEMENT_ARIPHMETICT_OVERLOADS_FOR_INTEGRAL_TYPE( unsigned      )
    MARTY_DECIMAL_IMPLEMENT_ARIPHMETICT_OVERLOADS_FOR_INTEGRAL_TYPE( std::int64_t  )
    MARTY_DECIMAL_IMPLEMENT_ARIPHMETICT_OVERLOADS_FOR_INTEGRAL_TYPE( std::uint64_t )
    MARTY_DECIMAL_IMPLEMENT_ARIPHMETICT_OVERLOADS_FOR_INTEGRAL_TYPE( float         )
    MARTY_DECIMAL_IMPLEMENT_ARIPHMETICT_OVERLOADS_FOR_INTEGRAL_TYPE( double        )

    //----------------------------------------------------------------------------



    //----------------------------------------------------------------------------

    MARTY_DECIMAL_IMPLEMENT_RELATIONAL_OPERATORS(Decimal)

    //----------------------------------------------------------------------------



protected:


    //------------------------------
    static
    Decimal fromRawNumPrec( num_t num, precision_t prec )
    {
        Decimal res;

        res.m_num   = num;
        res.m_denum = DecimalPrecision(prec);

        return res;
    }

public:

    //------------------------------
    Decimal minimizePrecision() const;
    Decimal& minimizePrecisionInplace();
    Decimal expantPrecisionTo( precision_t p ) const;
    Decimal shrinkPrecisionTo( precision_t p ) const;
    Decimal fitPrecisionTo   ( precision_t p ) const;

    //------------------------------
    Decimal& negate();
    Decimal& invert() { return negate(); }

    int     sign() const;
    int     sgn () const { return sign(); }
    Decimal abs () const;
    Decimal mod (  Decimal d2 ) const;
    Decimal neg () const;
    Decimal inv () const { return neg(); }

    //------------------------------
    Decimal getPercentOf( Decimal d ) const;
    Decimal getPermilleOf( Decimal d ) const;

    Decimal rounded( precision_t precision, RoundingMethod roundingMethod ) const;

    //------------------------------

    static precision_t getOutputPrecision()                   { return m_outputPrecision; }
    static void        setOutputPrecision( precision_t p )    { m_outputPrecision = p; }
    static void        setOutputPrecisionToStreamPrecision( ) { setOutputPrecision(0); }
    static void        setOutputPrecisionToAuto( )            { setOutputPrecision((precision_t)-1); }

    //------------------------------

protected:



    //------------------------------
    Decimal( num_t n, DenumeratorType denum) : m_num(n), m_denum(denum)   {}

    //------------------------------
    static
    void adjustPrecisionsToGreater( Decimal &d1, Decimal &d2 )
    {
        int cmp = d1.m_denum.compare( d2.m_denum );
        if (cmp==0) return;

        if (cmp<0)
           d1.precisionExpandTo(d2.m_denum.prec());
        else
           d2.precisionExpandTo(d1.m_denum.prec());
    }

    //------------------------------
    std::string rtrimZeros( std::string s )
    {
        if (s.empty())
            return s;

        {
            std::string::size_type commaPos = s.find( ',' );
            if (commaPos!=std::string::npos)
                s[commaPos] = '.';
        }
        
        std::string::size_type dotPos = s.find( '.' );

        if (dotPos==std::string::npos)
            return s; // do nothing

        std::string::size_type lastGoodPos = s.size()-1;

        while( lastGoodPos!=dotPos && s[lastGoodPos]=='0')
            --lastGoodPos;

        if (s[lastGoodPos]=='.')
            --lastGoodPos;

        s.erase(lastGoodPos+1);

        if (s.empty())
            s = "0";

        /*
        while( !s.empty() && s.back()=='0' ) s.erase(s.size()-1, 1);
        if ( !s.empty() && (s.back()=='.' || s.back()==',') )
            s.append(1,'0');
        */

        return s;
    }

    //------------------------------
    template<typename FloatType>
    void fromFloat( FloatType f, precision_t p )
    {
        bool precAuto = (p==0);
        *this = decimalFromString(rtrimZeros(std::to_string(f)));
        if (!precAuto)
            precisionFitTo(p);
    }

    //------------------------------
    template<typename FloatType>
    void fromFloat( FloatType f, DecimalPrecision p )
    {
        fromFloat( f, p.prec() );
    }

    //------------------------------
    int compare( num_t n ) const
    {
        if (m_num<n) return -1;
        if (m_num>n) return  1;
        return 0;
    }

    //------------------------------
    //! Возвращает true, если обрезание/удлиннение прошло предельно точно и ничего лишнего не было задето, и все жизненно важные органы остались на месте
    bool precisionExpandTo( precision_t p );
    bool precisionShrinkTo( precision_t p );
    bool precisionFitTo( precision_t p );

    //------------------------------
    static
    bool isDigitEven( unum_t d ) // Чётно?
    {
        switch(d)
        {
            case 0: case 2: case 4: case 6: case 8:
            return true;
        }
        
        return false;
    }
    
    //------------------------------
    static
    bool isDigitOdd( unum_t d ) // Нечётно?
    {
        switch(d)
        {
            case 1: case 3: case 5: case 7: case 9:
            return true;
        }
        
        return false;
    }

    //------------------------------
    Decimal& minimizePrecisionImpl();
    unum_t getLowestDecimalDigit() const;
    void replaceLowestDecimalDigit( unum_t d );

    Decimal makeMinimalPrecisionImplHelper( num_t newNum ) const;
    Decimal makeMinimalPrecisionOne() const;
    Decimal makeMinimalPrecisionTwo() const;
    Decimal makeMinimalPrecisionFive() const;


    //------------------------------

    #if 0
    enum class RoundingMethod
    {
        // https://en.wikipedia.org/wiki/Rounding

        // Directed rounding to an integer methods

    [-] roundDown                           , // roundFloor, roundTowardNegInf
                                              // округление в сторону минус бесконечности
                                              // Для положительных чисел - отбрасываем дробную часть - roundTrunc/roundTowardsZero
                                              // Для отрицательных - если отброшенная дробная часть не нулевая, то отнимаем 1 - roundTowardsInf

        roundUp                             , // roundCeil, roundTowardsPosInf
                                              // округление в сторону плюс бесконечности
                                              // Для положительных чисел  - если отброшенная дробная часть не нулевая, то прибавляем 1 - roundTowardsInf
                                              // Для отрицательных - отбрасываем дробную часть - roundTowardsZero/roundTrunc

        roundTowardsZero                    , // roundAwayFromInf, roundTrunc - тупое отсечение по модулю, для +/- одинаково
        roundTowardsInf                     , // roundAwayFromZero - тупое прибавление 1 по мумодулю, если отброс не равен нулю


        // Rounding to the nearest integer

        // Тут четыре случая. 
        // 1) Отбрасываемая часть равна 0, и младшая цифра равна 0                         - ничего не округляем, всё и так кругло
        // 2) Отбрасываемая часть не равна 0, или младшая цифра не равна 0, но меньше 5    - округляем в сторону меньшего по модулю - roundTrunc/roundTowardsZero
        // 3) Отбрасываемая часть равна нулю, и младшая цифра равна пяти                   - тут по-разному для каждого из случаев
        // 4) Младшая цифра больше пяти                                                    - округляем в сторону большего по модулю

        roundHalfUp                         , // roundHalfTowardsPositiveInf
        roundHalfDown                       , // roundHalfTowardsNegativeInf

        roundHalfTowardsZero                , // roundHalfAwayFromInf
        roundHalfTowardsInf                 , // roundHalfAwayFromZero, roundMath
        roundHalfToEven                     , // roundBankers, roundBanking, roundConvergent, roundStatistician, roundStatistic, roundDutch, roundGaussian

        roundHalfToOdd

    };

    #endif

    Decimal& roundingImpl2( precision_t requestedPrecision, RoundingMethod roundingMethod )
    {
        int thisSign = sgn();
        if (!thisSign)
        {
            precisionFitTo(requestedPrecision); // Усечение нуля рояли не играет
            return *this;
        }

        if (thisSign<0)
        {
            // Для embed asert поставить
            throw std::runtime_error("Decimal::roundingImpl2: negative decimals not allowed here");
        }

        bool   fitExact   = precisionFitTo(requestedPrecision + 1);
        unum_t ldd        = getLowestDecimalDigit();

        bool lddExactZero = (ldd==0) && fitExact;

        if (lddExactZero)
        {
            // Ничего не отброшено, и младшая десятичная цифра равна нулю.
            precisionShrinkTo( requestedPrecision ); // Ничего не делаем, просто обрезаем младший десятичный разряд (нулевой)
            return *this;
        }

        bool lddExactFive = (ldd==5) && fitExact;

        // Тут уже нет отрицательных значений.
        // Тут уже нет чисел, у которых нечего отбрасывать, кроме незначащих нулей

        switch(roundingMethod)
        {
            case RoundingMethod::roundTowardsInf:

                     precisionShrinkTo( requestedPrecision );
                     *this += makeMinimalPrecisionOne();
                     return *this;


            case RoundingMethod::roundTowardsZero:

                     precisionShrinkTo( requestedPrecision );
                     return *this;

            case RoundingMethod::roundHalfTowardsZero :

                     if (lddExactFive || ldd<5)
                     {
                         precisionShrinkTo( requestedPrecision );
                     }
                     else
                     {
                         precisionShrinkTo( requestedPrecision );
                         *this += makeMinimalPrecisionOne();
                     }

                     return *this;


            case RoundingMethod::roundHalfTowardsInf  :

                     if (lddExactFive || ldd>=5)
                     {
                         precisionShrinkTo( requestedPrecision );
                         *this += makeMinimalPrecisionOne();
                     }
                     else
                     {
                         precisionShrinkTo( requestedPrecision );
                     }

                     return *this;


            case RoundingMethod::roundHalfToEven      :

                     precisionShrinkTo( requestedPrecision );

                     if (lddExactFive)
                     {
                         Decimal thisMod2 = this->mod(makeMinimalPrecisionTwo());
                         *this = *this + thisMod2;
                     }
                     else if (ldd<5)
                     {
                         // do nothing
                     }
                     else
                     {
                         *this += makeMinimalPrecisionOne();
                     }

                     return *this;


            case RoundingMethod::roundHalfToOdd       :

                     return *this;

        }

        throw std::runtime_error("Decimal::roundingImpl2: something goes wrong");


    }

    // negate()
    //------------------------------
    Decimal& roundingImpl( precision_t requestedPrecision, RoundingMethod roundingMethod )
    {
        if (roundingMethod==RoundingMethod::roundingInvalid)
        {
            throw std::runtime_error("Decimal::roundingImpl: rounding method isInvalid");
        }

        if ( m_denum.precision() <= requestedPrecision )
            return *this;

        switch(roundingMethod)
        {
            case RoundingMethod::roundDown: // roundFloor, roundTowardNegInf
                 if (sgn()>0)
                     return roundingImpl2( requestedPrecision, RoundingMethod::roundTrunc ); // roundTowardsZero
                 else
                     return negate().roundingImpl2( requestedPrecision, RoundingMethod::roundTowardsInf).negate();


            case RoundingMethod::roundUp: // roundCeil, roundTowardsPosInf
                 if (sgn()>0)
                     return roundingImpl2( requestedPrecision, RoundingMethod::roundTowardsInf );
                 else
                     return negate().roundingImpl2( requestedPrecision, RoundingMethod::roundTrunc).negate(); // roundTowardsZero, roundAwayFromInf
                       

            case RoundingMethod::roundTowardsZero: // roundAwayFromInf, roundTrunc
                 if (sgn()>0)
                     return roundingImpl2( requestedPrecision, roundingMethod );
                 else
                     return negate().roundingImpl2( requestedPrecision, roundingMethod ).negate();


            case RoundingMethod::roundTowardsInf: // roundAwayFromZero
                 if (sgn()>0)
                     return roundingImpl2( requestedPrecision, roundingMethod );
                 else
                     return negate().roundingImpl2( requestedPrecision, roundingMethod ).negate();


            // Rounding to the nearest integer

            case RoundingMethod::roundHalfUp          : // roundHalfTowardsPositiveInf
                 if (sgn()>0)
                     return roundingImpl2( requestedPrecision, RoundingMethod::roundHalfTowardsInf );
                 else
                     return negate().roundingImpl2( requestedPrecision, RoundingMethod::roundHalfTowardsZero ).negate();


            case RoundingMethod::roundHalfDown        :
                 if (sgn()>0)
                     return roundingImpl2( requestedPrecision, RoundingMethod::roundHalfTowardsZero );
                 else
                     return negate().roundingImpl2( requestedPrecision, RoundingMethod::roundHalfTowardsInf ).negate();

            case RoundingMethod::roundHalfTowardsZero :
            case RoundingMethod::roundHalfTowardsInf  :
            case RoundingMethod::roundHalfToEven      :
            case RoundingMethod::roundHalfToOdd       :
                 if (sgn()>0)
                     return roundingImpl2( requestedPrecision, roundingMethod );
                 else
                     return negate().roundingImpl2( requestedPrecision, roundingMethod ).negate();



        }

        throw std::runtime_error("Decimal::roundingImpl: rounding method not implemented yet");

        return *this;


        #if 0

        // Нужно привести точность к precision+1
        // Если необходимо, то расширить точность
        // Нужно проверить, возможно ли такое расширение

        // Если невозможно расширение точности, 
        // то: а) вернуть, как есть
        //     б) кинуть исключение
        // Наверное - (а), так как если точность меньше требуемой, то и округления никакого не нужно

        // В итоге - если текущая точность меньше или равна запрошенной, то ничего делать и не надо 

        if ( m_denum.precision() <= requestedPrecision )
            return *this;


        bool fitExact = precisionFitTo(requestedPrecision + 1);
        unum_t ldd = getLowestDecimalDigit();

        bool lddExactZero = (ldd==0) && fitExact;

        if (lddExactZero)
        {
            // Ничего не отброшено, и младшая десятичная цифра равна нулю.
            precisionShrinkTo( requestedPrecision ); // Ничего не делаем, просто обрезаем младший десятичный разряд
            return *this;
        }

        bool lddExactFive = (ldd==5) && fitExact;



        

        switch(roundingMethod)
        {
            case RoundingMethod::roundDown: // roundFloor
                 {
                     precisionShrinkTo( requestedPrecision );

                     if (ldd==0)
                         break;

                     if (sign()<0)
                     {
                         *this -= makeMinimalPrecisionOne();
                     }
                 }
                 break;
        
            case RoundingMethod::roundUp: // roundCeil
                 {
                     precisionShrinkTo( requestedPrecision );

                     if (ldd==0)
                         break;

                     if (sign()>0)
                     {
                         *this += makeMinimalPrecisionOne();
                     }
                 }
                 break;
        
            case RoundingMethod::roundTowardsZero: // roundAwayFromInf, roundTrunc
                 {
                     precisionShrinkTo( requestedPrecision );
                 }
                 break;
        
            case RoundingMethod::roundTowardsInf: // roundAwayFromZero
                 {
                     precisionShrinkTo( requestedPrecision );
                     *this += makeMinimalPrecisionOne() * sign();
                 }
                 break;
        
            case RoundingMethod::roundHalfUp: // roundHalfTowardsPositiveInf
                 {
                     if (ldd==5 || sgn()>0)
                         *this += makeMinimalPrecisionFive();
                     else if (ldd>=5 || sgn()<0)
                         *this -= makeMinimalPrecisionFive();
                     else
                         { /* simple truncation */ }

                     precisionShrinkTo( requestedPrecision );
                 }
                 break;

            case RoundingMethod::roundHalfDown: // roundHalfTowardsNegativeInf
                 {
                     if (ldd==5 /* || sgn()>0 */ )
                         *this -= makeMinimalPrecisionFive(); // Ok
                     else if (ldd>=5)
                         *this += makeMinimalPrecisionFive() * sgn();
                     else
                         { /* simple truncation */ }

                     precisionShrinkTo( requestedPrecision );
                 }
                 break;
        
            case RoundingMethod::roundHalfTowardsZero: // roundHalfAwayFromInf
                 {
                     if (ldd<=5)
                         { /* simple truncation */ }
                     else
                     {
                         *this += makeMinimalPrecisionFive() * sgn();
                     }

                     //*this -= makeMinimalPrecisionFive() * sign();
                     precisionShrinkTo( requestedPrecision );
                 }
                 break;
        
            case RoundingMethod::roundHalfTowardsInf: // roundHalfAwayFromZero, roundMath
                 {
                     *this += makeMinimalPrecisionFive() * sign();
                     precisionShrinkTo( requestedPrecision );
                 }
                 break;
        
            case RoundingMethod::roundHalfToEven: // roundBankers, roundBanking
                 {
                     precisionFitTo(requestedPrecision);

                     if (ldd==5)
                     {
                         int     thisSign     = sign();
                         Decimal thisAbs      = abs();
                         Decimal thisAbsMod2  = abs();
                         thisAbsMod2 = thisAbsMod2.mod(makeMinimalPrecisionTwo());
                         *this = (thisAbs+thisAbsMod2) * thisSign;
                     }
                     else if (ldd>5)
                     {
                         *this += makeMinimalPrecisionOne() * sign();
                     }
                 }
                 break;
        
            case RoundingMethod::roundHalfToOdd:
                 {
                     //throw std::runtime_error("RoundingMethod::roundHalfToOdd not implemented");

                     // Soryan, not implemented yet
                     precisionFitTo(requestedPrecision);
                     // Currently is the same as "trunc"

                 }
                 break;
        
        
        };

        return *this;

        #endif

    }

    //----------------------------------------------------------------------------






    //----------------------------------------------------------------------------
    // Class members goes here

    num_t               m_num;
    DenumeratorType     m_denum;



    // Global affecting options

    // if == -1  - Exact Decimal number precision will be used
    // if ==  0  - Output stream precision will be used
    // if >   0  - Exact precision will be used
    inline static precision_t         m_outputPrecision = (precision_t)-1; 

    // RoundingMethod::roundingInvalid
    // RoundingMethod::roundHalfTowardsInf
    // RoundingMethod::roundHalfToEven

    inline static RoundingMethod      m_divideRoundingMethod   = RoundingMethod::roundHalfToEven; 

    inline static precision_t         m_dividePrecision        = 6;
    




}; // class Decimal

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
#include "marty_int_decimal_impl.h"
//----------------------------------------------------------------------------





//----------------------------------------------------------------------------
#define MARTY_DECIMAL_IMPLEMENT_FRIEND_OVERLOADS_FOR_INTEGRAL_TYPE( integralType )                                          \
                                                                                                                            \
    inline bool operator< ( integralType i, const Decimal &d ) { return Decimal(i) <  d; }                                  \
    inline bool operator<=( integralType i, const Decimal &d ) { return Decimal(i) <= d; }                                  \
    inline bool operator> ( integralType i, const Decimal &d ) { return Decimal(i) >  d; }                                  \
    inline bool operator>=( integralType i, const Decimal &d ) { return Decimal(i) >= d; }                                  \
    inline bool operator==( integralType i, const Decimal &d ) { return Decimal(i) == d; }                                  \
    inline bool operator!=( integralType i, const Decimal &d ) { return Decimal(i) != d; }                                  \
                                                                                                                            \
    inline Decimal operator + ( integralType i, const Decimal &d ) { return Decimal(i).operator+ ( d ); }                   \
    inline Decimal operator - ( integralType i, const Decimal &d ) { return Decimal(i).operator- ( d ); }                   \
    inline Decimal operator * ( integralType i, const Decimal &d ) { return Decimal(i).operator* ( d ); }                   \
    inline Decimal operator / ( integralType i, const Decimal &d ) { return Decimal(i).operator/ ( d ); }                   \
    inline Decimal operator % ( integralType i, const Decimal &d ) { return Decimal(i).operator% ( d ); }

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
MARTY_DECIMAL_IMPLEMENT_FRIEND_OVERLOADS_FOR_INTEGRAL_TYPE( int           )
MARTY_DECIMAL_IMPLEMENT_FRIEND_OVERLOADS_FOR_INTEGRAL_TYPE( unsigned      )
MARTY_DECIMAL_IMPLEMENT_FRIEND_OVERLOADS_FOR_INTEGRAL_TYPE( std::int64_t  )
MARTY_DECIMAL_IMPLEMENT_FRIEND_OVERLOADS_FOR_INTEGRAL_TYPE( std::uint64_t )
MARTY_DECIMAL_IMPLEMENT_FRIEND_OVERLOADS_FOR_INTEGRAL_TYPE( float         )
MARTY_DECIMAL_IMPLEMENT_FRIEND_OVERLOADS_FOR_INTEGRAL_TYPE( double        )

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
inline
void swap( Decimal &d1, Decimal &d2 )
{
    d1.swap(d2);
}

//----------------------------------------------------------------------------
inline
std::string decimalToString  ( Decimal d, Decimal::precision_t p = (Decimal::precision_t)-1 )
{
    return d.toString(p);
    /*
    d = d.expantPrecisionTo( p );

    Decimal::num_t p1 = d.m_num / d.m_denum.denum();
    Decimal::num_t p2 = d.m_num % d.m_denum.denum();

    std::string res = std::to_string(p1);

    std::size_t prec = d.m_denum.prec();

    if (!prec)
        return res; // No decimal digits after dot

    if (p2<0)
        p2 = -p2;

    std::string strP2 = std::to_string(p2);

    std::size_t leadingZerosNum = 0;
    if (prec>strP2.size())
        leadingZerosNum = prec - strP2.size();

    return res + std::string(1, '.') + std::string(leadingZerosNum, '0') + strP2;
    */
}

//----------------------------------------------------------------------------
// For compatibility with old code
inline
std::string toString  ( const Decimal     &d, Decimal::precision_t p = (Decimal::precision_t)-1 )
{
    return decimalToString(d,p);
}

//----------------------------------------------------------------------------
template<typename CharType>
inline
int charToDecimalDigit( CharType ch )
{
    if (ch<(CharType)'0') return -1;
    if (ch>(CharType)'9') return -1;
    return (int)(ch - (CharType)'0');
}

//----------------------------------------------------------------------------
template<typename CharType>
inline
bool charIsDecimalSeparator( CharType ch )
{
    if (ch==(CharType)'.') return true;
    if (ch==(CharType)',') return true;
    return false;
}

//----------------------------------------------------------------------------
template<typename CharType>
inline
int charIsSpaceOrTab( CharType ch )
{
    switch((char)ch)
    {
        case ' ' : return true;
        case '\t': return true;
        default  : return false;
    };
}

//----------------------------------------------------------------------------
inline
bool isDecimalStringCandidateExactPlainDecimalValueChar( char ch )
{
    switch(ch)
    {
        case '0': case '1': case '2': case '3': case '4':            
        case '5': case '6': case '7': case '8': case '9':
            return true; // good char - this is a digit

        case ',': case '.':
            return true; // good char - this is a decimal separator

        case ' ': case '\'': case '`':
            return true; // good char - this is a group separator

        case '+': case '-':
            return true; // nice signum sign

        default: return false;
    }

    return true;
}

//----------------------------------------------------------------------------
inline
bool isDecimalStringCandidateExactPlainDecimalValueString( const std::string &numberStr )
{
    typedef std::string::size_type sz_t;
    sz_t pos = 0, sz = numberStr.size();

    for(; pos!=sz; ++pos)
    {
        if (!isDecimalStringCandidateExactPlainDecimalValueChar(numberStr[pos]))
            return false;
    }

    return true;
}

//----------------------------------------------------------------------------
inline
char decimalStringPrepareConvertChar( char ch )
{
    if (ch==' ' || ch=='\'' || ch=='`') return 0;
    if (ch==',' ) return '.';
    return ch;
}

//----------------------------------------------------------------------------
inline
std::string decimalStringPrepareForConvert( const std::string &numberStr )
{
    typedef std::string::size_type sz_t;
    sz_t pos = 0, sz = numberStr.size();
    std::string res;

    for(; pos!=sz; ++pos)
    {
        char ch = decimalStringPrepareConvertChar(numberStr[pos]);
        if (ch)
           res.append(1,ch);
    }

    return res;
}

//----------------------------------------------------------------------------
inline
Decimal     decimalFromString( const std::string &numberStr_ )
{
    std::string numberStr = decimalStringPrepareForConvert(numberStr_); // change ',' to '.' and skip thousands separator

    typedef std::string::size_type sz_t;
    sz_t pos = 0, sz = numberStr.size();

    if (!isDecimalStringCandidateExactPlainDecimalValueString(numberStr))
    {
        // may be double in exponential format
        // Лень самому парсить
        sz_t cnt = 0;
        double dbl = std::stod( numberStr, &cnt );

        if (cnt!=sz)
            throw std::runtime_error("Decimal fromString - possible exponential form parsing failed");

        return Decimal(dbl);
    }


    bool neg = false;

    while( charIsSpaceOrTab(numberStr[pos]) && pos!=sz ) pos++;
    if (pos==sz) throw std::runtime_error("Decimal fromString - empty string taken as number string");

    if (numberStr[pos]=='+')                        { pos++; }
    else if (numberStr[pos]=='-')                   { pos++; neg = true; }
    else if (charToDecimalDigit(numberStr[pos])>=0) {}
    else throw std::runtime_error("Decimal fromString - invalid character found");

    while( charIsSpaceOrTab(numberStr[pos]) ) pos++;

    std::int64_t  num = 0;
    std::uint32_t precision = 0;

    int dig = charToDecimalDigit(numberStr[pos]);
    while(dig>=0)
    {
        num *= 10;
        num += (std::int64_t)dig;
        pos++;
        dig = charToDecimalDigit(numberStr[pos]);
    }

    if (!charIsDecimalSeparator(numberStr[pos]))
    {
        if (neg)
            num = -num;

        return Decimal( num, DecimalPrecision(0) );
    }

    pos++;

    unsigned prec = 0;

    dig = charToDecimalDigit(numberStr[pos]);
    while( sz != pos && (dig>=0  /* && (d[pos]!=' ' && d[pos]!='\'') */ ) )
    {
        num *= 10;
        num += (std::int64_t)dig;
        pos++;
        dig = charToDecimalDigit(numberStr[pos]);
        prec++;
    }

    if (neg)
        num = -num;

    return Decimal::fromRawNumPrec( num, prec );

}

//----------------------------------------------------------------------------
// For compatibility with old code
inline
Decimal     fromString( const std::string &numberStr_ )
{
    return decimalFromString(numberStr_);
}


//----------------------------------------------------------------------------
inline
std::ostream& operator<<( std::ostream& os, const Decimal &v )
{
    Decimal::precision_t  precision = Decimal::getOutputPrecision();

    if (precision==(Decimal::precision_t)-1)
    {
        // Exact Decimal number precision will be used
        precision = v.precision();
        if (precision > Decimal::maxPrecision())
            precision = Decimal::maxPrecision();
    }
    else if (precision>0)
    {
        // Exact global output precision will be used
        if (precision > Decimal::maxPrecision())
            precision = Decimal::maxPrecision();
    }
    else // precision==0
    {
        // Output stream precision will be used
        precision = (Decimal::precision_t)os.width();
        if (precision > Decimal::maxPrecision())
            precision = Decimal::maxPrecision();
    }


    //auto minPrecision = (Decimal::precision_t)os.precision();
    //if (minPrecision<1)
    //    minPrecision = 1;
    os << v.toString(precision);
    return os;
}

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------

} // namespace marty


