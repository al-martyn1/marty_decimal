#pragma once


#if !defined(MARTY_DECIMAL_H__079F0131_B01B_44ED_BF0F_8DFECAB67FD7__)
    #error "This header file must be used from marty_decimal.h only"
#endif


//----------------------------------------------------------------------------
inline
bool Decimal::checkIsExact( const std::string &strDecimal  ) const 
{
    return toString( -1 )==strDecimal;
}

//----------------------------------------------------------------------------
inline
bool Decimal::checkIsExact( const char        *pStrDecimal ) const
{
    return checkIsExact(std::string(pStrDecimal));
}

//----------------------------------------------------------------------------
inline
const char* Decimal::getRoundingMethodName( RoundingMethod m )
{
    switch(m)
    {
        case RoundingMethod::roundingInvalid     : return "roundingInvalid";
        case RoundingMethod::roundFloor          : return "roundFloor";
        case RoundingMethod::roundCeil           : return "roundCeil";
        case RoundingMethod::roundTowardsZero    : return "roundTowardsZero";
        case RoundingMethod::roundTowardsInf     : return "roundTowardsInf";
        case RoundingMethod::roundHalfUp         : return "roundHalfUp";
        case RoundingMethod::roundHalfDown       : return "roundHalfDown";
        case RoundingMethod::roundHalfTowardsZero: return "roundHalfTowardsZero";
        case RoundingMethod::roundHalfTowardsInf : return "roundHalfTowardsInf";
        case RoundingMethod::roundHalfToEven     : return "roundHalfToEven";
        case RoundingMethod::roundHalfToOdd      : return "roundHalfToOdd";
        default: return "UNKNOWN ROUNDING";
    }
}

//----------------------------------------------------------------------------
inline
void Decimal::assignFromString( const char        *pStr )
{
    const char *pStrOrg = pStr;

    // Skip ws
    while(*pStr==' ' || *pStr=='\t') ++pStr;

    m_sign =  1;

    if (*pStr=='+' || *pStr=='-')
    {
        if (*pStr=='-')
            m_sign = -1;
        else
            m_sign =  1;

        ++pStr;

        // Skip ws
        while(*pStr==' ' || *pStr=='\t') ++pStr;
    }

    const char *pStrEnd = 0;

    m_precision = bcd::makeRawBcdNumber( m_number
                                       , pStr
                                       , (std::size_t)-1
                                       , &pStrEnd
                                       );

    if (*pStrEnd!=0 && *pStrEnd!='0')
    {
        throw std::runtime_error( std::string("Decimal::fromString: invalid number string: ") + pStrOrg );
    }

    if (bcd::checkForZero( m_number ))
    {
        bcd::clearShrink(m_number);
        m_precision = 0;
        m_sign = 0;
    }

}

//----------------------------------------------------------------------------
inline
void Decimal::assignFromString( const std::string &str  )
{
    assignFromString( str.c_str() );
}

//----------------------------------------------------------------------------
inline
void Decimal::assignFromIntImpl( std::int64_t  i, int precision )
{
    m_precision = 0;
    bcd::clearShrink(m_number);

    if (i==0)
    {
        m_sign = 0;
    }
    else if (i>0)
    {
        m_sign = 1;
        bcd::makeRawBcdNumberFromUnsigned( m_number, (std::uint64_t)i );
        m_precision = precision;
    }
    else // i<0
    {
        m_sign = -1;
        //NOTE: !!! Вообще не паримся по поводу несимметричности множества целых относительно нуля
        bcd::makeRawBcdNumberFromUnsigned( m_number, (std::uint64_t)-i ); 
        m_precision = precision;
    }

}

//----------------------------------------------------------------------------
inline
void Decimal::assignFromIntImpl( std::uint64_t u, int precision )
{
    m_precision = 0;
    bcd::clearShrink(m_number);

    if (u==0)
    {
        m_sign = 0;
    }
    else // u>0 - unsigned же
    {
        m_sign = 1;
        bcd::makeRawBcdNumberFromUnsigned( m_number, u );
        m_precision = precision;
    }

}

//----------------------------------------------------------------------------
inline
void Decimal::assignFromDoubleImpl( double d, int precision )
{
    if (precision<0)
        precision = 0;

    if (precision>12)
        precision = 12;

    // https://docs.microsoft.com/en-us/cpp/c-runtime-library/format-specification-syntax-printf-and-wprintf-functions?view=msvc-160
    // printf( "%.*f", 3, 3.14159265 ); /* 3.142 output */

    char buf[32]; // forever enough for all doubles

    sprintf( &buf[0], "%.*f", precision, d ); 

    assignFromString( &buf[0] );

}

//----------------------------------------------------------------------------
inline
const char* Decimal::toString( char *pBuf, std::size_t bufSize, int precision ) const
{
    if (bufSize < 5 )
        throw std::runtime_error("marty::Decimal::toString: bufSize is not enough");

    if (m_sign==0)
    {
        pBuf[0] = '0'; pBuf[1] = 0;
        return pBuf;
    }

    unsigned idx = 0;

    if (m_sign<0)
    {
        pBuf[idx++] = '-';
    }

    bcd::formatRawBcdNumber( m_number, m_precision, &pBuf[idx], bufSize-1 );

    return pBuf;
}

//----------------------------------------------------------------------------
inline
std::string Decimal::toString( int precision ) const
{
    char buf[256]; // 256 знаком в числе хватит для всех :)
    return toString( &buf[0], sizeof(buf), precision );
}

//----------------------------------------------------------------------------
inline
int Decimal::compare( const Decimal &r ) const
{

    if (m_sign < r.m_sign )
        return -1;

    if (m_sign > r.m_sign )
        return  1;

    // m_sign are equals

    if (m_sign==0)
        return 0; // Нули - равны

    if (m_sign>0) // Больше нуля - сравнение BCD прямое
    {
        return bcd::compareRaws( m_number, m_precision, r.m_number, r.m_precision );
    }

    // Меньше нуля - сравнение BCD инвертируется - в минусах всё инверсное по знаку
    return -bcd::compareRaws( m_number, m_precision, r.m_number, r.m_precision );

}

//----------------------------------------------------------------------------
inline
void Decimal::swap( Decimal &d2 )
{
    std::swap( m_sign     , d2.m_sign      );
    std::swap( m_precision, d2.m_precision );
    m_number.swap(d2.m_number);
}

//----------------------------------------------------------------------------
inline
Decimal& Decimal::operator=( Decimal d2 )
{
    swap(d2);
    return *this;
}

//----------------------------------------------------------------------------
inline
Decimal& Decimal::add( const Decimal &d )
{
    if (d.m_sign==0) // Nothing to add
        return *this;

    if (m_sign==0)
    {
        if (d.m_sign!=0)
            *this = d;
        return *this;
    }

    // Both are non-zero

    if (m_sign==d.m_sign) // same sign
    {
        bcd::raw_bcd_number_t resNumber;

        m_precision = bcd::rawAddition( resNumber
                                      , m_number  , m_precision
                                      , d.m_number, d.m_precision
                                      );

        // При сложении двух ненулевых чисел ноль получится не может. Не проверяем на ноль.

        resNumber.swap(m_number);
        // m_sign не меняется

        return *this;
    }

    // Знаки различны и оба не равны нулю

    int cmpRes = bcd::compareRaws( m_number, m_precision, d.m_number, d.m_precision );

    if (cmpRes==0) // абсолютные значения равны, в результате - ноль
    {
        m_sign      = 0;
        m_precision = 0;
        bcd::clearShrink(m_number);
        return *this;
    }

    bcd::raw_bcd_number_t resNumber;

    // Вычитание модулей - всегда из большего вычитается меньший

    if (cmpRes>0) // this greater than d
       m_precision = bcd::rawSubtraction( resNumber, m_number, m_precision, d.m_number, d.m_precision );
    else // this less than d
       m_precision = bcd::rawSubtraction( resNumber, d.m_number, d.m_precision, m_number, m_precision );

    resNumber.swap(m_number);

    // На ноль не проверяем - модули не равны, ноль не может получится
    // Осталось разобраться со знаком результата

    if (m_sign>0)
    {
        if (cmpRes>0)
            m_sign =  1;
        else
            m_sign = -1;
    }
    else // m_sign<0
    {
        if (cmpRes>0)
            m_sign = -1;
        else
            m_sign =  1;
    }

    return *this;

}

//----------------------------------------------------------------------------
inline
Decimal& Decimal::sub( const Decimal &d )
{
    Decimal tmp = d;
    if (tmp.m_sign!=0)
        tmp.m_sign = -tmp.m_sign;

    return add(tmp);
}

//----------------------------------------------------------------------------
inline
Decimal& Decimal::mul( const Decimal &d )
{
    m_sign = m_sign * d.m_sign;

    if (m_sign==0)
    {
        m_precision = 0;
        bcd::clearShrink(m_number);
        return *this;
    }

    bcd::raw_bcd_number_t resNumber;

    m_precision = bcd::rawMultiplication( resNumber, m_number, m_precision, d.m_number, d.m_precision );

    // Оба не нулевые, результат точно не ноль, не проверяем

    resNumber.swap(m_number);

    return *this;
}

//----------------------------------------------------------------------------
inline
Decimal& Decimal::div( const Decimal &d, int precision )
{
    if (d.m_sign==0)
    {
        throw std::runtime_error("marty::Decimal::div: division by zero");
    }

    if (bcd::checkForZero( d.m_number ))
    {
        throw std::runtime_error("marty::Decimal::div: division by zero. Divisor m_sign is not a zero, but Divisor m_number is actually zero");
    }

    // Для деления знак получается по тем же правилам, что и для умножения

    m_sign = m_sign * d.m_sign;

    if (m_sign==0)
    {
        // Это могло произойти только в том случае, если делимое равно нулю
        // Если делимое равно нулю, то и частное тоже равно нулю
        // Ничего делать не нужно

        m_precision = 0;
        bcd::clearShrink(m_number);
        return *this;
    }

    bcd::raw_bcd_number_t resNumber;

    m_precision = bcd::rawDivision( resNumber, m_number, m_precision, d.m_number, d.m_precision, precision );

    if (bcd::checkForZero( resNumber ))
    {
        // Получился 0. На самом деле, конечно, реально не ноль, но в условиях ограниченной точности разрядов не хватило, чтобы влезли значимые разряды

        m_sign = 0;
        m_precision = 0;
        bcd::clearShrink(m_number);
        return *this;

    }

    resNumber.swap(m_number);

    return *this;
}

//----------------------------------------------------------------------------
//! Всегда возвращает true
inline
bool Decimal::precisionExpandTo( int p )
{
    if (p<0)
        p = 0;

    if (p<=m_precision)
    {
        return true;
    }

    m_precision = bcd::extendPrecision( m_number, m_precision, p );

    return true;
}

//----------------------------------------------------------------------------
//! Возвращает true, если все обрезанные цифры были нулём
inline
bool Decimal::precisionShrinkTo( int p )
{
    if (p<0)
        p = 0;

    if (p>=m_precision)
    {
        return true;
    }

    int lastTruncatedDigit = 0;
    bool allTruncatedAreZeros = true;

    m_precision = bcd::truncatePrecision( m_number, m_precision, p, &lastTruncatedDigit, &allTruncatedAreZeros );

    return allTruncatedAreZeros;
}

//----------------------------------------------------------------------------
//! Возвращает true, если все обрезанные цифры были нулём (если было обрезание), или true
inline
bool Decimal::precisionFitTo( int p )
{
    if (p<0)
        p = 0;

    if (p>m_precision)
        return precisionExpandTo( p );
    else if (p<m_precision)
        return precisionShrinkTo( p );

    return true;

}

//----------------------------------------------------------------------------
inline
bool Decimal::isDigitEven( int d ) 
{
    switch(d)
    {
        case 0: case 2: case 4: case 6: case 8:
        return true;
    }
    
    return false;
}

//----------------------------------------------------------------------------
//! Цифра нечётна?
inline
bool Decimal::isDigitOdd( int d ) 
{
    switch(d)
    {
        case 1: case 3: case 5: case 7: case 9:
        return true;
    }
    
    return false;
}

//----------------------------------------------------------------------------
inline
int Decimal::getLowestDecimalDigit() const
{
    return bcd::getLowestDigit( m_number, m_precision );
}

//----------------------------------------------------------------------------
inline
Decimal Decimal::makeMinimalPrecisionImplHelper( int val ) const
{
    // Используется при округлении (дробной части), поэтому точность всегда будет >=0
    return Decimal( val, m_precision );
}

//----------------------------------------------------------------------------
inline
Decimal Decimal::makeMinimalPrecisionOne() const
{
    return makeMinimalPrecisionImplHelper(1);
}

//----------------------------------------------------------------------------
inline
Decimal Decimal::makeMinimalPrecisionTwo() const
{
    return makeMinimalPrecisionImplHelper(2);
}

//----------------------------------------------------------------------------
inline
Decimal Decimal::makeMinimalPrecisionFive() const
{
    return makeMinimalPrecisionImplHelper(5);
}

//----------------------------------------------------------------------------
inline
Decimal& Decimal::roundingImpl2( int requestedPrecision, RoundingMethod roundingMethod )
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
    int    ldd        = getLowestDecimalDigit();

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
                     // Нужно найти остаток от деления на минимальную двойку
                     // Если делится на два - число четное.
                     // А если не делится - то нечётное.
                     // Всё очень просто
                     // Результат - либо минимальная единичка, если минимальная (последняя) цифра числа - нечётная,
                     //             либо ноль, если минимальная (последняя) цифра числа - чётная,

                     // Было:
                     // Decimal thisMod2 = this->mod(makeMinimalPrecisionTwo());
                     // *this = *this + thisMod2;

                     // Стало:

                     int lowestDigit = bcd::getLowestDigit( m_number, m_precision );

                     Decimal thisModLowest2 = isDigitEven(lowestDigit) ? (Decimal(0)) : makeMinimalPrecisionOne();
                     *this = *this + thisModLowest2;

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

//----------------------------------------------------------------------------
inline
Decimal& Decimal::roundingImpl1( int requestedPrecision, RoundingMethod roundingMethod )
{
    if (roundingMethod==RoundingMethod::roundingInvalid)
    {
        throw std::runtime_error("Decimal::roundingImpl: rounding method isInvalid");
    }

    if ( precision() <= requestedPrecision )
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

}

//----------------------------------------------------------------------------
inline
Decimal& Decimal::roundingImpl ( int requestedPrecision, RoundingMethod roundingMethod )
{
    roundingImpl1(requestedPrecision, roundingMethod);

    if (bcd::checkForZero( m_number ))
    {
        m_sign = 0;
        m_precision = 0;
        bcd::clearShrink(m_number);
    }

    return *this;
}

//----------------------------------------------------------------------------
inline
Decimal Decimal::rounded( int precision, RoundingMethod roundingMethod ) const
{
    Decimal res = *this;
    res.roundingImpl( precision, roundingMethod );
    return res;
}

//----------------------------------------------------------------------------
inline
Decimal  Decimal::reciprocated( int precision ) const
{
    if (isZero())
        throw std::runtime_error("marty::Decimal::reciprocated: Zero value can't be reciprocated");
    Decimal d1 = 1u;
    return  d1.div( *this, precision );

}

//----------------------------------------------------------------------------
inline
Decimal& Decimal::reciprocate( int precision )
{
    auto rcprt = reciprocated( precision );
    swap(rcprt);
    return *this;
}

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
inline
Decimal Decimal::getPercentOf( const Decimal &d ) const
{
    //UNDONE: !!! Need to make correct rounding
    Decimal tmp = Decimal(100) * *this;
    //tmp.precisionExpandTo(2); // точнось - сотые доли процента
    return (tmp / d).rounded( 2, RoundingMethod::roundMath );
}

//----------------------------------------------------------------------------
inline
Decimal Decimal::getPermilleOf( const Decimal &d ) const
{
    //UNDONE: !!! Need to make correct rounding
    Decimal tmp = Decimal(1000) * *this;
    //tmp.precisionExpandTo(2); // точнось - сотые доли промилле
    return (tmp / d).rounded( 2, RoundingMethod::roundMath );
}

//----------------------------------------------------------------------------
inline
double Decimal::toDouble() const
{
    return m_sign * bcd::rawToDouble( m_number, m_precision );
}


/*
    int                     m_sign;
    int                     m_precision;
    bcd::raw_bcd_number_t   m_number;

    unsigned bcd::getLowestDigit( const raw_bcd_number_t &bcdNumber, int precision )

*/

