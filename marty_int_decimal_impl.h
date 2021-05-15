#pragma once

#if !defined(MARTY_DECIMAL_H__F7FEF215_956E_4063_8AD1_1F66F6E17EB7__)
    #error "This header file must be used from marty_decimal.h only"
#endif

//----------------------------------------------------------------------------
inline
void DecimalDenumerator::swap( DecimalDenumerator &d2 )
{
    std::swap( m_precision, d2.m_precision );
    std::swap( m_denum    , d2.m_denum     );
}

//----------------------------------------------------------------------------
inline
int DecimalDenumerator::compare( const DecimalDenumerator d2 ) const
{
    if (m_precision<d2.m_precision) return -1;
    if (m_precision>d2.m_precision) return  1;
    return 0;
}

//----------------------------------------------------------------------------
inline
void DecimalDenumerator::incPrec()
{
    tryIncDenum();
    m_precision += 1;
}

//----------------------------------------------------------------------------
inline
void DecimalDenumerator::decPrec()
{
    tryDecDenum();
    m_precision -= 1;
}

//----------------------------------------------------------------------------
inline
DecimalDenumerator::denum_t
DecimalDenumerator::precisionExpandTo( precision_t p )
{
    precision_t pMax = DecimalPrecision::maxPrecision<denum_t>();
    if (p > pMax)
        p = pMax;

    denum_t deltaDenum = 1;
    while(m_precision < p)
    {
        incPrec();
        deltaDenum *= 10;
    }

    return deltaDenum;
}

//----------------------------------------------------------------------------
inline
DecimalDenumerator::denum_t
DecimalDenumerator::precisionShrinkTo( precision_t p )
{
    denum_t deltaDenum = 1;
    while(m_precision > p)
    {
        decPrec();
        deltaDenum *= 10;
    }

    return deltaDenum;
}

//----------------------------------------------------------------------------
inline
DecimalDenumerator::sdenum_t
DecimalDenumerator::precisionFitTo( precision_t p )
{
    if (m_precision<p)
        return   (sdenum_t)precisionExpandTo(p);
    else
        return - (sdenum_t)precisionShrinkTo(p);
}

//----------------------------------------------------------------------------
inline
void DecimalDenumerator::tryIncDenum()
{
    denum_t newDenum = m_denum*10u;
    if (newDenum<m_denum)
        throw std::runtime_error("DecimalDenumerator precision to big to increment");
    m_denum = newDenum;
}

//----------------------------------------------------------------------------
inline
void DecimalDenumerator::tryDecDenum()
{
    if (!m_denum)
        throw std::runtime_error("DecimalDenumerator precision to small to decrement");
    m_denum = m_denum / 10u;
}

//----------------------------------------------------------------------------





//----------------------------------------------------------------------------
inline
Decimal Decimal::minimizePrecision() const
{
    Decimal res = *this;
    return res.minimizePrecisionImpl();
}

//----------------------------------------------------------------------------
inline
Decimal& Decimal::minimizePrecisionInplace()
{
    return minimizePrecisionImpl();
}

//----------------------------------------------------------------------------
inline
Decimal Decimal::expantPrecisionTo( precision_t p ) const
{
    Decimal res = *this;
    res.precisionExpandTo(p);
    return res;
}

//----------------------------------------------------------------------------
inline
Decimal Decimal::shrinkPrecisionTo( precision_t p ) const
{
    Decimal res = *this;
    res.precisionShrinkTo(p);
    return res;
}

//----------------------------------------------------------------------------
inline
Decimal Decimal::fitPrecisionTo( precision_t p ) const
{
    Decimal res = *this;
    res.precisionFitTo(p);
    return res;
}

//----------------------------------------------------------------------------
inline
Decimal::precision_t Decimal::findMaxDecimalScalePower() const
{
    unum_t unum = getPositiveNumerator();

    precision_t maxScalePower = maxPrecision();

    precision_t curScalePower = findCurrentDecimalPower( unum );

    return maxScalePower - curScalePower;
}

//----------------------------------------------------------------------------

    
    
//----------------------------------------------------------------------------
inline
void Decimal::swap( Decimal &d2 )
{
    //using namespace std;
    std::swap( m_num   , d2.m_num   );
    m_denum.swap( d2.m_denum );
}

//------------------------------
inline
Decimal& Decimal::operator=( Decimal d2 )
{
    swap(d2);
    return *this;
}

//------------------------------
inline
int Decimal::compare( Decimal d2 ) const
{
    if (m_denum.prec()==d2.m_denum.prec())
    {
        return compare(d2.m_num);
    }

    Decimal d1 = *this;
    adjustPrecisionsToGreater( d1, d2 );
    return d1.compare(d2);
}
//----------------------------------------------------------------------------





//----------------------------------------------------------------------------
inline
Decimal Decimal::operator + ( Decimal d2 ) const
{
    Decimal d1 = *this;
    adjustPrecisionsToGreater( d1, d2 );
    d1.m_num += d2.m_num;
    return d1;
}

//------------------------------
inline
Decimal Decimal::operator - ( Decimal d2 ) const
{
    Decimal d1 = *this;
    adjustPrecisionsToGreater( d1, d2 );
    d1.m_num -= d2.m_num;
    return d1;
}

//------------------------------
inline
Decimal Decimal::operator - ( ) const
{
    Decimal res = *this;
    res.m_num = -res.m_num;
    return res;
}

//------------------------------
inline
Decimal Decimal::operator * ( Decimal d2 ) const
{
    //minimizePrecisionImpl();
    d2.minimizePrecisionInplace();
    Decimal d1 = minimizePrecision();

    //precision_t findMaxDecimalScalePower() const;
    //precision_t findCurrentDecimalPower( unum_t unum )

    precision_t maxResultPower = DecimalPrecision::maxPrecision<num_t>();
    precision_t resultPower    = d1.findNumCurrentDecimalPower() + d2.findNumCurrentDecimalPower();

    precision_t needReducePrecision1 = (resultPower > maxResultPower) ? (resultPower - maxResultPower) : 0;

    precision_t resultPrecision      = d1.precision() + d2.precision();
    precision_t needReducePrecision2 = (resultPrecision > maxPrecision()) ? (resultPrecision - maxPrecision()) : 0;

    precision_t needReducePrecision  = (needReducePrecision1 > needReducePrecision2) ? needReducePrecision1 : needReducePrecision2;


    // precision_t reducePrecision1 = needReducePrecision/2;
    // Пытаемся сделать пропорциональное уменьшение точности

    precision_t reducePrecision1 = needReducePrecision; // Уменьшаем первую точность по максимуму по умолчанию

    /*
    if (d2.precision()!=0)
    {
        reducePrecision1 = d1.precision()*needReducePrecision / d2.precision();
    }
    */

    if (resultPrecision!=0)
    {
        reducePrecision1 = d1.precision()*needReducePrecision / resultPrecision;
    }

    if (reducePrecision1 > d1.precision())
    {
        reducePrecision1 = d1.precision();
    }

    if (reducePrecision1)
        d1 = d1.rounded( d1.precision()-reducePrecision1, RoundingMethod::roundStatistician ); // RoundingMethod::roundMath


    precision_t reducePrecision2 = needReducePrecision - reducePrecision1;

    if (reducePrecision2 > d2.precision())
    {
        reducePrecision2 = d2.precision();
    }

    if (reducePrecision2)
        d2 = d2.rounded( d2.precision()-reducePrecision2, RoundingMethod::roundStatistician ); // RoundingMethod::roundMath

    // Вообще-то надо бы уменьшать точность пропорционально исходным
    // Может возникнуть такая ситуация, что точность у d1 большая, а у d2 - маленькая.
    // Но точность d2 мы уменьшили вполовину суммарной, и еще есть место для манёвра, 
    // а точность d2 уменьшать некуда


    num_t num = d1.m_num * d2.m_num;

    return Decimal( num, d1.precision() + d2.precision() ).minimizePrecision();
    /*
    if (d1.m_denum < d2.m_denum)
    {
        //num /= m_denum.denum();
        return Decimal( num, d2.m_denum ).minimizePrecisionImpl();
    }
    else
    {
        //num /= d2.m_denum.denum();
        return Decimal( num, d1.m_denum ).minimizePrecisionImpl();
    }
    */

}

//------------------------------
// 1200.00000 / 22.000 = 54.5454545454545
// 1200 00000 / 22 000 = 5454.54545454
// 1200.00000 / 25.000 = 48

inline
Decimal Decimal::divide( Decimal devider, precision_t resultPrecision ) const
{
    devider.minimizePrecisionInplace();

    precision_t maxAllowedPrecisionIncrement  = findMaxDecimalScalePower();
    precision_t maxAllowedPrecision           = this->m_denum.precision() + maxAllowedPrecisionIncrement;
    if (maxAllowedPrecision > maxPrecision())
        maxAllowedPrecision = maxPrecision();

    precision_t requiredSrcPrecision = resultPrecision + devider.m_denum.precision();

    // Точность результата = точность делимого - точность делителя
    // Для этого точность делимого нужно расширить до resultPrecision + точность делителя
    // Но если расширение точности невозможно, то уменьшаем точность делителя
    // Можно без цикла, но лень думать

    while( (requiredSrcPrecision>maxAllowedPrecision) && (devider.m_denum.precision()!=0) )
    {
        devider = devider.shrinkPrecisionTo( devider.m_denum.precision()-1 );
        requiredSrcPrecision = resultPrecision + devider.m_denum.precision();
    }

    if (requiredSrcPrecision>maxAllowedPrecision)
        requiredSrcPrecision = maxAllowedPrecision;

    Decimal d1 = expantPrecisionTo( requiredSrcPrecision );

    return Decimal( d1.m_num / devider.m_num, DenumeratorType(d1.m_denum.precision()-devider.m_denum.precision()) );
    //DecimalPrecision
}

//------------------------------
inline
Decimal Decimal::operator / ( Decimal d2 ) const
{
    d2.minimizePrecisionInplace();

    // Проблема в том, что если у делимого точность нулевая или минимальная, 
    // и значения сравнимого порядка, то точность у результата будет никакая

    precision_t recomendedNumPrecision = 2*d2.m_denum.prec();
    if (recomendedNumPrecision < 3)
        recomendedNumPrecision = 3;

    precision_t numPrecision = m_denum.prec() + d2.m_denum.prec();
    if (numPrecision < recomendedNumPrecision)
        numPrecision = recomendedNumPrecision;

    if (numPrecision < d2.m_denum.prec())
        numPrecision = d2.m_denum.prec();

    return divide( d2, numPrecision - d2.m_denum.prec() );

    //Decimal d1 = *this;
    //d1.precisionExpandTo( m_denum.prec() + d2.m_denum.prec() );
    //return Decimal( d1.m_num / d2.m_num, m_denum);
}

//------------------------------
// 1200.00000   % 22.000    = 12.000
// 1200         % 22        = 12
// 1200 00000   % 22 000    = 12000
// 1200 000000  % 22 000    = 10000
// 1200.00000   % 22.00000  = 12.00000
// 120000000    % 2200000   = 12 00000

inline
Decimal Decimal::operator % ( Decimal d2 ) const
{
    return mod(d2);
    /*
    Decimal d1 = *this;
    adjustPrecisionsToGreater( d1, d2 );
    d1.m_num %= d2.m_num;
    return d1;
    */
}

//------------------------------
inline
Decimal& Decimal::operator += ( Decimal d2 )
{
    *this = operator+(d2);
    return *this;
}

//------------------------------
inline
Decimal& Decimal::operator -= ( Decimal d2 )
{
    *this = operator-(d2);
    return *this;
}

//------------------------------
inline
Decimal& Decimal::operator *= ( Decimal d2 )
{
    *this = operator*(d2);
    return *this;
}

//------------------------------
inline
Decimal& Decimal::operator /= ( Decimal d2 )
{
    *this = operator/(d2);
    return *this;
}

//------------------------------
inline
Decimal& Decimal::operator %= ( Decimal d2 )
{
    *this = operator%(d2);
    return *this;
}

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
inline
Decimal Decimal::getPercentOf( Decimal d ) const
{
    //UNDONE: !!! Need to make correct rounding
    Decimal tmp = Decimal(100) * *this;
    tmp.precisionExpandTo(2u); // точнось - сотые доли процента
    return tmp / d;
}

//------------------------------
inline
Decimal Decimal::getPermilleOf( Decimal d ) const
{
    //UNDONE: !!! Need to make correct rounding
    Decimal tmp = Decimal(1000) * *this;
    tmp.precisionExpandTo(2u); // точнось - сотые доли промилле
    return tmp / d;
}

//------------------------------
inline
Decimal Decimal::rounded( precision_t precision, RoundingMethod roundingMethod ) const
{
    Decimal res = *this;
    res.roundingImpl( precision, roundingMethod );
    return res;
}
//------------------------------
inline
int Decimal::sign() const
{
    if (m_num<0)
        return -1;

    if (m_num>0)
        return 1;

    return 0;
}

//------------------------------
inline
Decimal Decimal::abs() const
{
    Decimal res = *this;

    if (res.m_num<0)
       res.m_num = -res.m_num;

    return res;
}

//------------------------------
inline
Decimal Decimal::mod( Decimal d2 ) const
{
    d2.minimizePrecisionInplace();

    Decimal d1 = *this;
    d1.minimizePrecisionInplace();

    adjustPrecisionsToGreater( d1, d2 );

    d1.m_num %= d2.m_num;
    return d1;
}

//------------------------------
inline
Decimal& Decimal::negate()
{
    m_num = -m_num;
    return *this;
}

//------------------------------
inline
Decimal Decimal::neg () const
{
    Decimal res = *this;
    return res.negate();
}
//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
inline
bool Decimal::precisionExpandTo( precision_t p )
{
    denum_t adjust = m_denum.precisionExpandTo(p);
    m_num *= adjust;
    return true; // При расширении точости жизненно важные органы не задеваются
}

//------------------------------
inline
bool Decimal::precisionShrinkTo( precision_t p )
{
    denum_t adjust   = m_denum.precisionShrinkTo(p);
    num_t   reminder = m_num % adjust;
    m_num /= adjust;
    return reminder==0; // Возврат true, если ничего лишнего не обрезали
}

//------------------------------
inline
bool Decimal::precisionFitTo( precision_t p )
{
    sdenum_t adjust = m_denum.precisionFitTo(p);
    if (adjust<0)
    {
       num_t   reminder = m_num % -adjust;
       m_num /= (denum_t)-adjust;
       return reminder==0;
    }
    else if (adjust==0)
    {
       // Ничего делать не надо
       return true; // Точность не меняется, жизненно важные органы не задеты
    }
    else
    {
       m_num *= (denum_t)adjust;
       return true; // При расширении точости жизненно важные органы не задеваются
    }
}

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
inline
Decimal Decimal::makeMinimalPrecisionImplHelper( Decimal::num_t newNum ) const
{
    Decimal res;
    res.m_denum = m_denum;
    res.m_num   = newNum;
    return res;
}

//------------------------------
inline
Decimal Decimal::makeMinimalPrecisionOne() const
{
    return makeMinimalPrecisionImplHelper(1);
}

//------------------------------
inline
Decimal Decimal::makeMinimalPrecisionTwo() const
{
    return makeMinimalPrecisionImplHelper(2);
}

//------------------------------
inline
Decimal Decimal::makeMinimalPrecisionFive() const
{
    return makeMinimalPrecisionImplHelper(5);
}

//------------------------------
inline
Decimal& Decimal::minimizePrecisionImpl()
{
    while( ((m_num%10)==0) && (m_denum.prec()>0) )
    {
        m_num /= 10;
        m_denum.decPrec();
    }

    return *this;
}

//------------------------------
inline
Decimal::unum_t Decimal::getLowestDecimalDigit() const
{
    unum_t unum = getPositiveNumerator();
    return unum % 10;
}

//------------------------------
inline
void Decimal::replaceLowestDecimalDigit( Decimal::unum_t d )
{
    // Clear place for new lowest digit
    m_num /= 10;
    m_num *= 10;

    if (d>9)
        d = 9;

    if (m_num<0)
        m_num -= (num_t)d;
    else
        m_num += (num_t)d;
}

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
inline
std::string  Decimal::toString( precision_t p ) const
{
    Decimal d = *this;
    if (p==(Decimal::precision_t)-1)
    {
        p = d.precision();
    }
    else
    {
        if (p > Decimal::maxPrecision())
            p = Decimal::maxPrecision();
        d = d.expantPrecisionTo( p );
    }

    d = d.rounded( p, RoundingMethod::roundMath );

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

}
//{ return decimalToString( *this, p ); }
