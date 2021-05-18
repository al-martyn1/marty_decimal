#pragma once

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
#include <cstdint>
#include <climits>
#include <limits>
#include <exception>
#include <stdexcept>
#include <vector>
#include <cstring>
#include <algorithm>
#include <iterator>
#include <string>
#include <algorithm>

//----------------------------------------------------------------------------


/* Все функции/операции в marty::bcd не готовы к неконсистентным данным
   и предназначены только для внутреннего использования

 */

#ifdef min
    #undef min
#endif

#ifdef max
    #undef max
#endif


// Под отладкой вектора гораздо приятнее разглядывать как последовательности чисел, а не символов. А на производительность - похуй. Алгоритмы отлаживаем на векторах.
#if defined(DEBUG) || defined(_DEBUG)

    #if !defined(MARTY_BCD_USE_VECTOR)
        #define MARTY_BCD_USE_VECTOR
    #endif

#endif


#if defined(_MSC_VER)

    #define MARTY_RAW_BCD_FORCE_INLINE( funcFullSignature ) __forceinline funcFullSignature
    //#define MARTY_RAW_BCD_FORCE_INLINE( funcFullSignature ) inline funcFullSignature

#else

    #define MARTY_RAW_BCD_FORCE_INLINE( funcFullSignature ) funcFullSignature

#endif




#ifndef MARTY_BCD_DEFAULT_DIVISION_PRECISION
    #define MARTY_BCD_DEFAULT_DIVISION_PRECISION   18
#endif


//----------------------------------------------------------------------------
namespace marty
{


//----------------------------------------------------------------------------
namespace bcd
{


//----------------------------------------------------------------------------

// #define MARTY_BCD_SEMI_COMPACT

#if defined(MARTY_BCD_NOT_COMPACT)

    typedef int                   decimal_digit_t;
    typedef unsigned int          decimal_udigit_t;

#elif defined(MARTY_BCD_SEMI_COMPACT)

    typedef short int             decimal_digit_t;
    typedef unsigned short int    decimal_udigit_t;

#else

    typedef signed char           decimal_digit_t;
    typedef unsigned char         decimal_udigit_t;

#endif


#ifndef MARTY_BCD_USE_VECTOR

    typedef std::basic_string<decimal_digit_t> raw_bcd_number_t;

#else

    typedef std::vector<decimal_digit_t> raw_bcd_number_t;

#endif

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
inline
void clearShrink( raw_bcd_number_t &bcdNumber )
{
    bcdNumber.clear();
    bcdNumber.shrink_to_fit();
}
//----------------------------------------------------------------------------
//! Returns precision
/*!
    Нужно еще передавать список символов - допустимых десятичных разделителей
                         список символов - разделителей разрядов
    https://ru.wikipedia.org/wiki/%D0%94%D0%B5%D1%81%D1%8F%D1%82%D0%B8%D1%87%D0%BD%D1%8B%D0%B9_%D1%80%D0%B0%D0%B7%D0%B4%D0%B5%D0%BB%D0%B8%D1%82%D0%B5%D0%BB%D1%8C#:~:text=%D0%A0%D0%B0%D0%B7%D0%B4%D0%B5%D0%BB%D0%B8%D1%82%D0%B5%D0%BB%D1%8C%20%D0%B3%D1%80%D1%83%D0%BF%D0%BF%20%D1%80%D0%B0%D0%B7%D1%80%D1%8F%D0%B4%D0%BE%D0%B2,-%D0%94%D0%BB%D1%8F%20%D1%83%D0%BF%D1%80%D0%BE%D1%89%D0%B5%D0%BD%D0%B8%D1%8F%20%D1%87%D1%82%D0%B5%D0%BD%D0%B8%D1%8F&text=%D0%9A%D0%B0%D0%BA%20%D0%BF%D1%80%D0%B0%D0%B2%D0%B8%D0%BB%D0%BE%2C%20%D0%B3%D1%80%D1%83%D0%BF%D0%BF%D1%8B%20%D1%81%D0%BE%D1%81%D1%82%D0%BE%D1%8F%D1%82%20%D0%B8%D0%B7,%D0%B1%D0%BE%D0%BB%D1%8C%D1%88%D0%B5%20%D1%87%D0%B5%D1%82%D1%8B%D1%80%D1%91%D1%85%20%D0%B8%D0%BB%D0%B8%20%D0%BF%D1%8F%D1%82%D0%B8%20%D1%86%D0%B8%D1%84%D1%80.

    Если в качестве десятичного разделителя используется точка, то разделитель групп разрядов может быть представлен запятой, 
    апострофом или пробелом, а если запятая, — то точкой (например, такая запись прежде использовалась в испанском языке[12], 
    теперь устарела[13]) или пробелом. Таким образом, значение точки и запятой оказывается зависимым от контекста (например, 
    запись 1,546 в английской нотации обозначает одна тысяча пятьсот сорок шесть, а в русской — одна целая пятьсот сорок шесть тысячных).
    Поэтому, чтобы избежать неоднозначности, международные стандарты (ISO 31-0, Международное бюро мер и весов, ИЮПАК) рекомендуют 
    использовать для разделителя групп разрядов только неразрывный пробел (или тонкую шпацию при типографском наборе)

    Пока разделители групп разрядов не реализованы.

 */

typedef const char* const_char_ptr;

inline
int makeRawBcdNumber( raw_bcd_number_t &bcdNumber
                    , const char *str
                    , std::size_t strSize = (std::size_t)-1
                    , const_char_ptr * pFirstUnknown = 0 //  std::size_t *pProcessedSymbols = 0     // заменить на указатель на указатель на char - начало строки двигается в функции
                    )
{
    if (strSize==(std::size_t)-1)
        strSize = std::strlen(str);

    bcdNumber.clear();

    int precision = 0;

    std::size_t processedSymbols = 0;

    bool processingFranctionalPart = false;

    bool hasDecimalDot = false;
    for(std::size_t i=0; i!=strSize; ++i)
    {
        if ( (str[i] == '.') || (str[i] == ',') )
        {
            hasDecimalDot = true;
            break;
        }
    }

    if (hasDecimalDot)
    {
        while(strSize!=0 && str[strSize-1]=='0') // "remove" trailing zeros
            --strSize;
    }

    while( strSize>1 && *str=='0') // "remove" leading zeros, but keep one
        { --strSize; ++str; }

    bcdNumber.reserve(strSize+1);


    //for(; *str; ++str, ++processedSymbols /* , ++precision */  )
    std::size_t i = 0;

    for( ; i!=strSize; ++i)
    {
        if ( (str[i] == '.') || (str[i] == ',') )
        {
            if (processingFranctionalPart)
            {
                // Already processing franctional part
                if (pFirstUnknown)
                   *pFirstUnknown = &str[i];
                std::reverse(bcdNumber.begin(), bcdNumber.end());
                return precision;
            }

            processingFranctionalPart = true;

            if (!processedSymbols)
            {
                bcdNumber.push_back(0);
            }

            ++processedSymbols;
            continue;
        }

        if ( (str[i] < '0') || (str[i] > '9') )
        {
            if (pFirstUnknown)
               *pFirstUnknown = &str[i];

            std::reverse(bcdNumber.begin(), bcdNumber.end());
            return precision;
        }

        decimal_digit_t d = (decimal_digit_t)(str[i] - '0');
        ++processedSymbols;

        bcdNumber.push_back(d);

        if (processingFranctionalPart)
            ++precision;
    }

    if (pFirstUnknown)
       *pFirstUnknown = &str[i];

    std::reverse(bcdNumber.begin(), bcdNumber.end());
    return precision;

}

//----------------------------------------------------------------------------
template<typename UnsignedIntegerType>
inline
int makeRawBcdNumberFromUnsigned( raw_bcd_number_t &bcdNumber
                                , UnsignedIntegerType u
                                )
{
    bcdNumber.clear();

    while(u)
    {
        bcdNumber.push_back( u%10 );
        u /= 10;
    }

    if (bcdNumber.empty())
       bcdNumber.push_back( 0 );

    return 0;

}

//----------------------------------------------------------------------------
//! Обрезаем точность, если возможно (в хвосте есть нули) до десятичной точки
//!!!
inline
int reducePrecision( raw_bcd_number_t &bcdNumber, int precision )
{
    if (precision<=0)
        return precision;

    int i = 0, size = (int)bcdNumber.size();

    for(; i!=size && i!=precision; ++i)
    {
        if (bcdNumber[i]!=0)
            break;
    }

    raw_bcd_number_t::iterator eraseEnd = bcdNumber.begin();
    std::advance(eraseEnd, (std::size_t)i);

    bcdNumber.erase( bcdNumber.begin(), eraseEnd );

    return precision - i;
}

//----------------------------------------------------------------------------
//! Обрезаем точность, если возможно (в хвосте есть нули), даже если пересекаем позицию десятичной точки
//!!!
inline
int reducePrecisionFull( raw_bcd_number_t &bcdNumber, int precision )
{
    if (bcdNumber.size()<2)
        return precision;

    int i = 0, size = (int)bcdNumber.size();

    for(; i!=(size-1); ++i)
    {
        if (bcdNumber[i]!=0)
            break;
    }

    if (!i)
        return precision;

    std::size_t numberOfPositionsToReduce = i;

    raw_bcd_number_t::iterator eraseEnd = bcdNumber.begin();
    std::advance(eraseEnd, numberOfPositionsToReduce);

    bcdNumber.erase( bcdNumber.begin(), eraseEnd );

    return precision - numberOfPositionsToReduce;
}

//----------------------------------------------------------------------------
//! Обрезаем ведущие незначащие нули
//!!!
inline
int reduceLeadingZeros( raw_bcd_number_t &bcdNumber, int precision )
{
    while( (bcdNumber.size()>0) && (bcdNumber.back()==0) && (bcdNumber.size() > (precision+1)) )
    {
        bcdNumber.pop_back();
    }

    return precision;
}

//----------------------------------------------------------------------------
//! Обрезаем ведущие нули, включая те, которые после точки
inline
int reduceLeadingZerosFull( raw_bcd_number_t &bcdNumber, int precision )
{
    precision = reduceLeadingZeros( bcdNumber, precision );

    while( (bcdNumber.size()>0) && (bcdNumber.back()==0) )
    {
        bcdNumber.pop_back();
    }

    return precision;
}

//----------------------------------------------------------------------------
//! Расширяем точность до заданной
inline
int extendPrecision( raw_bcd_number_t &bcdNumber, int curPrecision, int newPrecision )
{
    if (newPrecision<0) // Так низя
        newPrecision = 0;

    if (newPrecision<=curPrecision) // Нет расширения
        return curPrecision;

    int deltaPrecision = newPrecision - curPrecision;

    // void insert( iterator pos, size_type count, const T& value );
    bcdNumber.insert( bcdNumber.begin(), (raw_bcd_number_t::size_type)deltaPrecision, (raw_bcd_number_t::value_type)0 );

    return newPrecision;

}

//----------------------------------------------------------------------------
//! Расширяем перед точкой
//!!! Оно не нужно
/*
inline
int extendLeadings( raw_bcd_number_t &bcdNumber, int precision, int requestedLeadings )
{
    int bcdSize = (int)bcdNumber.size();

    if (bcdSize<precision)
        return precision;

    std::size_t curLeadings = bcdSize - precision;

    if (requestedLeadings<curLeadings)
        return precision;

    int leadingsToAdd = requestedLeadings - curLeadings;

    if (!leadingsToAdd)
        return precision;

    bcdNumber.insert( bcdNumber.end(), (raw_bcd_number_t::size_type)leadingsToAdd, (raw_bcd_number_t::value_type)0 );

    return precision;
}
*/
//----------------------------------------------------------------------------
//! Проверка на ноль
inline
bool checkForZero( const raw_bcd_number_t &bcdNumber )
{
    if (bcdNumber.empty())
        return true;

    for( auto it=bcdNumber.begin(); it!=bcdNumber.end(); ++it )
    {
        if (*it!=0)
            return false;
    }

    return true;
}

//----------------------------------------------------------------------------
inline
int getMaxPrecision( int precision1, int precision2 )
{
    int res = (precision1 > precision2) ? precision1 : precision2;
    if (res<0)
        res = 0;
    return res;
}

//----------------------------------------------------------------------------
inline
int getMinPrecision( int precision1, int precision2 )
{
    int res = (precision1 < precision2) ? precision1 : precision2;
    if (res<0)
        res = 0;
    return res;
}

//----------------------------------------------------------------------------
inline
int getIntegerPartSize( const raw_bcd_number_t &bcdNumber, int precision )
{
    int res = (int)bcdNumber.size() - precision;
    if (res<0)
        res = 1;
    return res; // (int)bcdNumber.size() - precision;
}

//----------------------------------------------------------------------------
inline
int getMaxIntegerPartSize( int ip1
                         , int ip2
                         )
{
    return (ip1 > ip2) ? ip1 : ip2;
}

//----------------------------------------------------------------------------
inline
int getMaxIntegerPartSize( const raw_bcd_number_t &bcdNumber1, int precision1
                         , const raw_bcd_number_t &bcdNumber2, int precision2
                         )
{
    return getMaxIntegerPartSize( getIntegerPartSize( bcdNumber1, precision1 )
                                , getIntegerPartSize( bcdNumber2, precision2 )
                                );
}

//----------------------------------------------------------------------------
inline
int getMinIntegerPartSize( int ip1
                         , int ip2
                         )
{
    return (ip1 < ip2) ? ip1 : ip2;
}

//----------------------------------------------------------------------------
inline
int getMinIntegerPartSize( const raw_bcd_number_t &bcdNumber1, int precision1
                         , const raw_bcd_number_t &bcdNumber2, int precision2
                         )
{
    return getMinIntegerPartSize( getIntegerPartSize( bcdNumber1, precision1 )
                                , getIntegerPartSize( bcdNumber2, precision2 )
                                );
}

//----------------------------------------------------------------------------
//! Lookup for Most/Least Significant Digit (MSD/LSD). Младшие цифры идут с начала.
inline
raw_bcd_number_t::size_type getMsdIndex( const raw_bcd_number_t &bcdNumber )
{
    raw_bcd_number_t::size_type i = bcdNumber.size();

    for( ; i!=0; --i)
    {
        if (bcdNumber[i-1]!=0)
            return i-1;
    }

    if (!bcdNumber.empty())
        return 0;

    return (raw_bcd_number_t::size_type)-1;

}

//----------------------------------------------------------------------------
//! Lookup for Most/Least Significant Digit (MSD/LSD). Младшие цифры идут с начала.
inline
raw_bcd_number_t::size_type getLsdIndex( const raw_bcd_number_t &bcdNumber )
{
    raw_bcd_number_t::size_type i = 0, size = bcdNumber.size();
    for( ; i!=size; ++i)
    {
        if (bcdNumber[i]!=0)
            return i;
    }

    if (!bcdNumber.empty())
        return 0;

    return (raw_bcd_number_t::size_type)-1;
}

//----------------------------------------------------------------------------
MARTY_RAW_BCD_FORCE_INLINE( decimal_digit_t getDigitByIndex( const raw_bcd_number_t &bcdNumber, int idx ) )
//inline decimal_digit_t getDigitByIndex( const raw_bcd_number_t &bcdNumber, int idx )
{
    return (((unsigned)idx) < bcdNumber.size()) ? bcdNumber[idx] : (decimal_digit_t)0;
}


//----------------------------------------------------------------------------
#define MARTY_BCD_DECLARE_PRECISION_VIRTUAL_ADJUSTMENT_VARS()                                  \
                    int ipSize1      = getIntegerPartSize( bcdNumber1, precision1 );           \
                    int ipSize2      = getIntegerPartSize( bcdNumber2, precision2 );           \
                                                                                               \
                    int maxIpSize    = getMaxIntegerPartSize( ipSize1, ipSize2 );              \
                    /*std::size_t minIpSize    = getMinIntegerPartSize( ipSize1, ipSize2 );*/  \
                                                                                               \
                    int maxPrecision = getMaxPrecision( precision1, precision2 );              \
                    /*int minPrecision = getMinPrecision( precision1, precision2 );*/          \
                    /*int dtPrecision  = maxPrecision - minPrecision; */                       \
                                                                                               \
                    int offset1      = maxPrecision - precision1;                              \
                    int offset2      = maxPrecision - precision2;                              \
                                                                                               \
                    int totalSize    = maxIpSize; /* virtual size */                           \
                    if (maxPrecision>0)                                                        \
                        totalSize += maxPrecision;


#define MARTY_BCD_DECLARE_PRECISION_VIRTUAL_ADJUSTMENT_VARS_V2( bcdNumber1, precision1, bcdNumber2, precision2 )  \
                int decOrderMin1 = -precision1;                                                                   \
                int decOrderMax1 = decOrderMin1 + (int)bcdNumber1.size();                                         \
                                                                                                                  \
                int decOrderMin2 = -precision2;                                                                   \
                int decOrderMax2 = decOrderMin2 + (int)bcdNumber2.size();                                         \
                                                                                                                  \
                int decOrderMin = std::min( decOrderMin1, decOrderMin2 );                                         \
                int decOrderMax = std::max( decOrderMax1, decOrderMax2 );                                         \
                                                                                                                  \
                int maxPrecision = std::max( precision1, precision2 );                                            \
                                                                                                                  \
                int decOrderDelta = decOrderMax - decOrderMin;


#define MARTY_BCD_PRECISION_GET_DIGITS_BY_VIRTUAL_ADJUSTMENT_VARS( decOrderValue, bcdNumber1, bcdNumber2 )        \
                                                                                                                  \
                decimal_digit_t d1 = getDigitByIndex(bcdNumber1, (decOrderValue) - decOrderMin1 );                \
                decimal_digit_t d2 = getDigitByIndex(bcdNumber2, (decOrderValue) - decOrderMin2 );

//----------------------------------------------------------------------------
inline
int getDecimalOrderByIndex( raw_bcd_number_t::size_type idx, const raw_bcd_number_t &bcdNumber, int precision )
{
    int order = 0;

    if (idx!=(raw_bcd_number_t::size_type)-1)
        order = ((int)idx) - precision;

    return order;
}

//----------------------------------------------------------------------------
inline
int truncatePrecision( raw_bcd_number_t &bcdNumber, int precision, int newPrecision, int *pLastTruncatedDigit = 0, bool *pAllTruncatedAreZeros = 0 )
{
    if (newPrecision<0)
        newPrecision = 0; // truncution of integer part is not allowed

    if (newPrecision>=precision)
    {
        if (pLastTruncatedDigit)
           *pLastTruncatedDigit = 0;

        if (pAllTruncatedAreZeros)
           *pAllTruncatedAreZeros = true; // allTruncatedAreZeros;

        return precision; // newPrecision; // Nothing to do
    }
    // Realy need truncation

    int requestedDecOrder = -newPrecision;

    int lastTruncatedDigit = 0;
    bool allTruncatedAreZeros = true;

    raw_bcd_number_t::size_type digitIndex = 0, bcdSize = bcdNumber.size();

    for(; digitIndex!=bcdSize; ++digitIndex)
    {
        int decOrder = getDecimalOrderByIndex( digitIndex, bcdNumber, precision );

        if (decOrder>=requestedDecOrder)
        {
            if (pLastTruncatedDigit)
               *pLastTruncatedDigit = lastTruncatedDigit;

            if (pAllTruncatedAreZeros)
               *pAllTruncatedAreZeros = allTruncatedAreZeros;

            // обрез цифры
            raw_bcd_number_t::iterator eraseEnd = bcdNumber.begin();
            std::advance(eraseEnd, digitIndex);

            /*
            // А это вроде вообще не нужно
            for(raw_bcd_number_t::iterator it=bcdNumber.begin(); it!=eraseEnd; ++it )
            {
                if (*it!=0)
                    allTruncatedAreZeros = false;
            }
            */

            bcdNumber.erase( bcdNumber.begin(), eraseEnd );

            return newPrecision;
        }

        lastTruncatedDigit = bcdNumber[digitIndex];
        if (lastTruncatedDigit!=0)
            allTruncatedAreZeros = false;
    }

    // Прошлись по всему вектору, но до требуемой точности так и не добрались
    // Судя по всему, ведущие значащие нули обрезаны

    // Значит, последняя обрезаемая цифра - 0, но вот были ли значащие цифры - 
    // нам таки об этом говорит allTruncatedAreZeros

    //clearShrink(bcdNumber);
    bcdNumber.clear();
    bcdNumber.push_back(0);
    bcdNumber.shrink_to_fit();


    if (pLastTruncatedDigit)
       *pLastTruncatedDigit = 0; // lastTruncatedDigit;

    if (pAllTruncatedAreZeros)
       *pAllTruncatedAreZeros = allTruncatedAreZeros;
       
    // return precision;
    return newPrecision;
}

//----------------------------------------------------------------------------
inline
int compareRaws( const raw_bcd_number_t &bcdNumber1, int precision1
               , const raw_bcd_number_t &bcdNumber2, int precision2
               )
{
    // Как быть, вот в чем вопрос?
    //
    // 1) Можно уравнять
    //    а) в обоих bcdNumbers precisions (расширением снизу/at front),
    //    б) целую часть (расширением сверху/at back)
    //    Тогда на каждом шаге не будет лишних проверок и сама операция будет произведена быстро.
    //    Но будут переаллокации вектора(ов) BCD, и их же перемещения.
    // 2) Можно ничего не переаллоцировать/перемещать, но на каждом 
    //    шаге будет смещение и проверки индексов.
    //    Зато не придётся лазать в кучу

    MARTY_BCD_DECLARE_PRECISION_VIRTUAL_ADJUSTMENT_VARS_V2( bcdNumber1, precision1, bcdNumber2, precision2 )


    // Сравнение начинаем со старших разрядов

    for( int decOrder=decOrderMax+1; decOrder!=decOrderMin; --decOrder )
    {

        MARTY_BCD_PRECISION_GET_DIGITS_BY_VIRTUAL_ADJUSTMENT_VARS( decOrder-1, bcdNumber1, bcdNumber2 )

        if (d1<d2)
            return -1;

        if (d1>d2)
            return  1;
    }

    return 0;
}

//----------------------------------------------------------------------------
//! Форматирование "сырого" BCD, форматируется в строку с полной точностью, без какого-либо усечения/округления
inline
const char* formatRawBcdNumber( const raw_bcd_number_t &bcdNumber, int precision, char *pBuf, std::size_t bufSize, char sep = '.' )
{
    if (bufSize < 4 )
        throw std::runtime_error("marty::bcd::formatRawBcdNumber: bufSize is not enough");

    // bufSize must be enough for 0.0\0

    --bufSize; // Reserve space for termination zero

    std::size_t bufPos = 0;

    
    int decOrderMin = -precision;
    if (decOrderMin>0)
        decOrderMin = 0; // Не гоже отрезать значащие нули справа от недробной части


    // Тут мы точно находим максимальный значащий разряд, опуская возможные ведущие нули
    // При операциях на них плевать, тем более что нули во всех операциях оптимизированы
    // А при выводе ведущие нули, хз откуда выплывшие, нам не нужны
    int decOrderMax = getDecimalOrderByIndex( getMsdIndex(bcdNumber), bcdNumber, precision ); 

    // Но нам нужны ведущие нули, если целая часть числа равна нулю
    if (decOrderMax<0)
        decOrderMax = 0;

    //int maxPrecision = std::max( precision1, precision2 );

    int decOrderDelta = decOrderMax - decOrderMin;

    // Форматирование, как и сравение, начинаем со старших разрядов

    for( int decOrder=decOrderMax+1; decOrder!=decOrderMin; --decOrder )
    {
        // Я просто выдрал код из макроса, который для двух чисел, decOrderValue там - параметр макроса.
        // Чтобы было минимум расхождений с макросом.
        // А у нас тут только одно число, и лишние дествия хоть и копеечны, но зачем?
        // Теоретически, макросы для двух чисел можно переделать на макросы для одного числа, 
        // и использовать дважды, но чёй-то лениво, и особо уже и незачем.
        // Форматирование - единственная операция, где участвует одно число.
        // Сравнение, сложение, вычитание, умножение и деление - там всё парнокопытно. И всё же отлажено и работает.
        // Форматирование было написано прежде всего, но когда я пытался использовать код из ф-ии форматирования
        // в арифм. операциях баги полезли сразу. А форматирование до последнего делало вид, что работает правильно.

        const int decOrderValue = decOrder-1; 

        decimal_digit_t d = getDigitByIndex( bcdNumber, (decOrderValue) - (-precision) );

        if (decOrderValue==-1 && bufPos!=0) // Добрались до первой цифры после точки
        {
            if (bufPos<bufSize)
                pBuf[bufPos++] = sep;
            //*pBuf++ = sep;
        }

        if (bufPos<bufSize)
            pBuf[bufPos++] = d + '0';
    }

    if (!bufPos) // место есть, спасибо --bufSize;
    {
        pBuf[bufPos++] = '0';
    }

    pBuf[bufPos] = 0; // место есть, спасибо --bufSize;

    return pBuf;

}

//----------------------------------------------------------------------------
inline
std::string formatRawBcdNumber( const raw_bcd_number_t &bcdNumber, int precision, char sep = '.' )
{
    char buf[1024];
    return formatRawBcdNumber( bcdNumber, precision, &buf[0], sizeof(buf), sep );
}

//----------------------------------------------------------------------------
inline
int getLowestDigit( const raw_bcd_number_t &bcdNumber, int precision )
{
    const int decOrderMin   = -precision;
    const int decOrderValue = decOrderMin; // decOrder-1; 

    decimal_digit_t d = getDigitByIndex( bcdNumber, (decOrderValue) - (-precision) );

    return d;
}

//----------------------------------------------------------------------------
//! Конвертация в целое
inline
std::uint64_t rawToInt( const raw_bcd_number_t &bcdNumber, int precision )
{

    int decOrderMin = -precision;
    if (decOrderMin>0)
        decOrderMin = 0; // Не гоже отрезать значащие нули справа от недробной части


    int decOrderMax = getDecimalOrderByIndex( getMsdIndex(bcdNumber), bcdNumber, precision ); 

    // Но нам нужны ведущие нули, если целая часть числа равна нулю
    if (decOrderMax<0)
        decOrderMax = 0;

    int decOrderDelta = decOrderMax - decOrderMin;

    std::uint64_t res = 0;

    for( int decOrder=decOrderMax+1; decOrder!=decOrderMin; --decOrder )
    {
        const int decOrderValue = decOrder-1; 

        decimal_digit_t d = getDigitByIndex( bcdNumber, (decOrderValue) - (-precision) );

        if (decOrderValue==-1) // Добрались до первой цифры после точки
        {
            return res;
        }

        res *= 10;
        res += (unsigned)d;

    }

    return res;

}

//----------------------------------------------------------------------------
//! Конвертация в целое
inline
double rawToDouble( const raw_bcd_number_t &bcdNumber, int precision )
{

    int decOrderMin = -precision;
    if (decOrderMin>0)
        decOrderMin = 0; // Не гоже отрезать значащие нули справа от недробной части


    int decOrderMax = getDecimalOrderByIndex( getMsdIndex(bcdNumber), bcdNumber, precision ); 

    // Но нам нужны ведущие нули, если целая часть числа равна нулю
    if (decOrderMax<0)
        decOrderMax = 0;

    int decOrderDelta = decOrderMax - decOrderMin;

    double res = 0;
    double devider = 1;

    for( int decOrder=decOrderMax+1; decOrder!=decOrderMin; --decOrder )
    {
        const int decOrderValue = decOrder-1; 

        decimal_digit_t d = getDigitByIndex( bcdNumber, (decOrderValue) - (-precision) );

        if (decOrderValue<=-1) // Добрались до цифр после точки
        {
            devider *= 10;
        }

        res *= 10;
        res += (unsigned)d;

    }

    return res/devider;

}

//----------------------------------------------------------------------------
template< typename IntType >
inline
IntType bcdCorrectOverflow( IntType &d )
{
    IntType res = d / 10;
    d %= 10;
    return res;
}

//----------------------------------------------------------------------------
//MARTY_RAW_BCD_FORCE_INLINE( decimal_digit_t bcdCorrectOverflowAfterSummation( decimal_digit_t &d ) )
inline decimal_digit_t bcdCorrectOverflowAfterSummation( decimal_digit_t &d )
{
    /*
         Коррекцию производим после суммирования одиночных разрядов.
         Значит, у нас на входе может быть число от 0 до 18.
         От нуля до девяти - ничего не делаем, возвращаем 0.
         Если больше - отнимаем 10 от d и возвращаем 1.
         Обходимся без делений.
         Должно получиться лучше, чем с делением и взятием остатка.

         Oo-o-ps. Слагаемых может быть три, так что диапазон на входе до 27ми

         Oo-o-ps #2. Одно из слагаемых - результат предыдущей коррекции - 0 или 1.
         Значит, у нас на входе может быть число от 0 до 19.
     */

    /*
    switch(d)
    {
        case 0 : case  1: case  2: case  3: case  4: case  5: case  6: case  7: case  8: case  9: 
             return 0;

        case 10: case 11: case 12: case 13: case 14: case 15: case 16: case 17: case 18: case 19:
             d -= 10;
             return 1;

        case 20: case 21: case 22: case 23: case 24: case 25: case 26: case 27:
             d -= 20;
             return 2;

        default: 
             throw std::runtime_error("marty::bcd::bcdCorrectOverflowAfterSummation: something goes wrong");

    }
    */

    /*
    if (d<10)
    {
        return 0;
    }
    else if (d<20)
    {
        d -= 10;
        return 1;
    }
    else if (d<28)
    {
        d -= 20;
        return 2;
    }
    else
    {
        throw std::runtime_error("marty::bcd::bcdCorrectOverflowAfterSummation: something goes wrong");
    }
    */

    /*
    decimal_digit_t res = 0;

    while(d>9)
    {
        ++res;
        d -= 10;
    }

    return res;
    */

    // 

    static 
    const
    int correctedDs[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9
                        , 0, 1, 2, 3, 4, 5, 6, 7, 8, 9
                        , 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF // на тестах должно вылететь, если что-то не так
                        };

    static 
    const
    int results    [] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
                        , 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
                        , 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF // на тестах должно вылететь, если что-то не так
                        };
    
    int res = results[d];

    d = (decimal_digit_t)correctedDs[d];

    return res;
}

//----------------------------------------------------------------------------
MARTY_RAW_BCD_FORCE_INLINE( decimal_digit_t bcdCorrectCarry( decimal_digit_t &d ) )
// inline decimal_digit_t bcdCorrectCarry( decimal_digit_t &d )
{
    /*
    if (d>=0)
        return 0;

    // Here d is less than zero

    d = ((decimal_digit_t)10) + d;

    return 1;
    */

    if (d<0)
    {
        d = ((decimal_digit_t)10) + d;
        return 1;
    }

    return 0;
}

//----------------------------------------------------------------------------
//! Сложение "сырых" BCD чисел с "плавающей" точкой. Реализация.
// MARTY_RAW_BCD_FORCE_INLINE(
//        int rawAdditionImpl( raw_bcd_number_t &bcdRes
//                    , const raw_bcd_number_t &bcdNumber1, int precision1
//                    , const raw_bcd_number_t &bcdNumber2, int precision2
//                    )
// )
inline int rawAdditionImpl( raw_bcd_number_t &bcdRes
                   , const raw_bcd_number_t &bcdNumber1, int precision1
                   , const raw_bcd_number_t &bcdNumber2, int precision2
                   )
{

    MARTY_BCD_DECLARE_PRECISION_VIRTUAL_ADJUSTMENT_VARS_V2( bcdNumber1, precision1, bcdNumber2, precision2 )

    bcdRes.reserve(decOrderDelta+1);

    bcdRes.resize( (std::size_t)decOrderDelta, (decimal_digit_t)0 );

    decimal_digit_t *pCurDigit = &bcdRes[0];


    decimal_digit_t dPrevOverflow = 0;

    for( int decOrder=decOrderMin; decOrder!=decOrderMax; ++decOrder )
    {
        MARTY_BCD_PRECISION_GET_DIGITS_BY_VIRTUAL_ADJUSTMENT_VARS( decOrder, bcdNumber1, bcdNumber2 )

        *pCurDigit = dPrevOverflow + d1 + d2;
        dPrevOverflow = bcdCorrectOverflowAfterSummation(*pCurDigit);
        ++pCurDigit;

    }

    if (dPrevOverflow)
        bcdRes.push_back(dPrevOverflow);

    return maxPrecision;
}

//----------------------------------------------------------------------------
//! Сложение "сырых" BCD чисел с "плавающей" точкой
inline
int rawAddition( raw_bcd_number_t &bcdRes
               , const raw_bcd_number_t &bcdNumber1, int precision1
               , const raw_bcd_number_t &bcdNumber2, int precision2
               )
{
    int resultPrecision = rawAdditionImpl( bcdRes
                                         , bcdNumber1, precision1
                                         , bcdNumber2, precision2
                                         );

    resultPrecision = reduceLeadingZeros( bcdRes, resultPrecision );
    resultPrecision = reducePrecision   ( bcdRes, resultPrecision );

    return resultPrecision;
}

//----------------------------------------------------------------------------
//! Вычитание "сырых" BCD чисел с "плавающей" точкой. Уменьшаемое должно быть больше или равно вычитаемому. Реализация.
// MARTY_RAW_BCD_FORCE_INLINE(
//         int rawSubtractionImpl( raw_bcd_number_t &bcdRes
//                               , const raw_bcd_number_t &bcdNumber1, int precision1
//                               , const raw_bcd_number_t &bcdNumber2, int precision2
//                               )
// )
inline int rawSubtractionImpl( raw_bcd_number_t &bcdRes
                             , const raw_bcd_number_t &bcdNumber1, int precision1
                             , const raw_bcd_number_t &bcdNumber2, int precision2
                             )
{
    MARTY_BCD_DECLARE_PRECISION_VIRTUAL_ADJUSTMENT_VARS_V2( bcdNumber1, precision1, bcdNumber2, precision2 )

    bcdRes.clear();
    bcdRes.reserve(decOrderDelta+1);

    decimal_digit_t dPrev = 0;

    for( int decOrder=decOrderMin; decOrder!=decOrderMax; ++decOrder )
    {
        MARTY_BCD_PRECISION_GET_DIGITS_BY_VIRTUAL_ADJUSTMENT_VARS( decOrder, bcdNumber1, bcdNumber2 )

        decimal_digit_t dCur = d1 - dPrev; // - d2;
        dPrev = bcdCorrectCarry(dCur);

        dCur = dCur - d2;
        dPrev += bcdCorrectCarry(dCur);

        bcdRes.push_back(dCur);

    }

    if (dPrev)
        bcdRes.push_back(dPrev);

    return maxPrecision;
}

//----------------------------------------------------------------------------
//! Вычитание "сырых" BCD чисел с "плавающей" точкой. Уменьшаемое должно быть больше или равно вычитаемому.
inline
int rawSubtraction( raw_bcd_number_t &bcdRes
                  , const raw_bcd_number_t &bcdNumber1, int precision1
                  , const raw_bcd_number_t &bcdNumber2, int precision2
                  )
{
    int resultPrecision = rawSubtractionImpl( bcdRes
                                            , bcdNumber1, precision1
                                            , bcdNumber2, precision2
                                            );

    resultPrecision = reduceLeadingZeros( bcdRes, resultPrecision );
    resultPrecision = reducePrecision   ( bcdRes, resultPrecision );

    return resultPrecision;
}
//----------------------------------------------------------------------------
inline
raw_bcd_number_t rawBcdMakeZeroPrecisionFromNegative( const raw_bcd_number_t &bcdNumber, int precision )
{
    if (precision>=0)
        return bcdNumber;

    precision = -precision;

    raw_bcd_number_t res;
    res.reserve( bcdNumber.size() + (std::size_t)precision );

    res.insert( res.end(), (std::size_t)precision, 0 ); // extends with zeros on lowest positions
    res.insert( res.end(), bcdNumber.begin(), bcdNumber.end() );

    return res; // precision is zero here
}

//----------------------------------------------------------------------------
//! Умножение "сырых" BCD чисел с "плавающей" точкой.
/*! Тупое умножение в столбик.
    Небольшая оптимизация - нулевые частичные суммы не вычисляем.
    Чем больше нулей в числе - тем быстрее производится умножение.
 */
inline
int rawMultiplication( raw_bcd_number_t &multRes
                  , const raw_bcd_number_t &bcdNumberArg1, int precision1
                  , const raw_bcd_number_t &bcdNumberArg2, int precision2
                  )
{
    //MARTY_BCD_DECLARE_PRECISION_VIRTUAL_ADJUSTMENT_VARS();

    typedef const raw_bcd_number_t*  const_raw_bcd_number_ptr_t;

    const_raw_bcd_number_ptr_t ptrBcdNumber1 = &bcdNumberArg1;
    const_raw_bcd_number_ptr_t ptrBcdNumber2 = &bcdNumberArg2;

    raw_bcd_number_t correctedBcdNumber1;
    raw_bcd_number_t correctedBcdNumber2;

    if (precision1<0)
    {
        correctedBcdNumber1 = rawBcdMakeZeroPrecisionFromNegative( bcdNumberArg1, precision1 );
        precision1          = 0;
        ptrBcdNumber1       = &correctedBcdNumber1;
    }

    if (precision2<0)
    {
        correctedBcdNumber2 = rawBcdMakeZeroPrecisionFromNegative( bcdNumberArg2, precision2 );
        precision2          = 0;
        ptrBcdNumber2       = &correctedBcdNumber2;
    }

    const raw_bcd_number_t &bcdNumber1 = *ptrBcdNumber1;
    const raw_bcd_number_t &bcdNumber2 = *ptrBcdNumber2;


    multRes.clear();
    multRes.reserve( bcdNumber1.size() + bcdNumber2.size() );

    // Алгоритм Фюрера - https://ru.wikipedia.org/wiki/%D0%90%D0%BB%D0%B3%D0%BE%D1%80%D0%B8%D1%82%D0%BC_%D0%A4%D1%8E%D1%80%D0%B5%D1%80%D0%B0
    // Интересно про свёртки

    // Проверил на бамажке - вроде работает. А что самое прикольное - гораздо проще всратого умножения столбиком.
    // В статье по ссылке - число маленькое, было интересно, когда элемент свертки перевалит за два разряда.
    // Будет не лень - настучу свой пример.

    // Используем basic_string - при небольших размерах она не лезет в динамическую память.
    // А ведёт себя так же, как вектор
    // Но с вектором отлаживаться удобнее - он красивше выглядит в отладчике

    #if defined(MARTY_BCD_USE_VECTOR)

        typedef std::vector<unsigned>       convolution_vector_t; 

    #else

        typedef std::basic_string<unsigned> convolution_vector_t; 

    #endif

    convolution_vector_t convolution = convolution_vector_t( bcdNumber1.size() + bcdNumber2.size(), 0 );


    // Итак, младшие разряды у нас идут первыми

    // Внешний цикл - по множителю (но на самом деле скорее всего пофик - но удобнее сравнивать с тем, что посчитано ручками)

    for( std::size_t i=0; i!=bcdNumber2.size(); ++i)
    {
        for( std::size_t j=0; j!=bcdNumber1.size(); ++j)
        {
            convolution[i+j] += (((unsigned)bcdNumber2[i]) * ((unsigned)bcdNumber1[j]));
        }
    }

    // Убираем ведущие нули - они могли появится, если на входе были числа с ведущими нулями, или, если старшие разяды были мелковаты 
    // и из них некторыхбыло переноса
    std::size_t convSize = convolution.size();
    for(; convSize!=0; --convSize )
    {
        if (convolution[convSize-1]!=0)
            break;
    }

    unsigned overflow = 0;

    for( std::size_t i=0; i!=convSize; ++i )
    {
        convolution[i] += overflow;
        overflow = bcdCorrectOverflow(convolution[i]);
        multRes.push_back((decimal_digit_t)convolution[i]);
    }

    if (overflow)
    {
        multRes.push_back((decimal_digit_t)overflow);
    }


    int precisionRes = precision1+precision2;

    return reducePrecision( multRes, precisionRes );

}

//----------------------------------------------------------------------------
inline
bool rawDivisionCheckContinueCondition( int dividendPrecision, int divisorPrecision
                                      , int  deltaCmp
                                      //, bool relativeDelta
                                      )
{
    int delta = dividendPrecision - divisorPrecision;

    //if (relativeDelta)
    {
        if (delta < deltaCmp)
            return true; // continue division
        return false;
    }
    // else
    // {
    //     // absolute precision required
    //     // Hm-m-m
    //     throw std::runtime_error("rawDivisionCheckContinueCondition: absolute precision not implemented");
    // }

    return false;

}

//----------------------------------------------------------------------------
//! Деление "сырых" BCD чисел с "плавающей" точкой.
inline
int rawDivision( raw_bcd_number_t &quotient
               , raw_bcd_number_t dividend, int dividendPrecision
               , raw_bcd_number_t divisor , int divisorPrecision
               , int  deltaCmp = MARTY_BCD_DEFAULT_DIVISION_PRECISION
               , unsigned nonZeroDigitsMin = 3
               )
{
    //bool relativeDelta = true;

    // Удаляем все нули справа от значащих цифр, до точки или после - не важно
    dividendPrecision = reducePrecisionFull( dividend, dividendPrecision );
    divisorPrecision  = reducePrecisionFull( divisor , divisorPrecision  );
    // Или, таки важно? А то чёй-то не так работает
    // dividendPrecision = reducePrecision( dividend, dividendPrecision );
    // divisorPrecision  = reducePrecision( divisor , divisorPrecision  );

    dividendPrecision = reduceLeadingZerosFull( dividend, dividendPrecision );
    divisorPrecision  = reduceLeadingZerosFull( divisor , divisorPrecision  );


    // Нужно выровнять размеры
    if (dividend.size() < divisor.size())
    {
        //std::cout<<"NNN1" << std::endl;
        std::size_t delta = divisor.size() - dividend.size();
        dividend.insert( dividend.begin(), delta, 0 );
        dividendPrecision += (int)delta;
    }
    else if (dividend.size() > divisor.size())
    {
        //std::cout<<"NNN2" << std::endl;
        std::size_t delta = dividend.size() - divisor.size();
        divisor.insert( divisor.begin(), delta, 0 );
        divisorPrecision += (int)delta;
    }


    // Не размеры надо выровнять, а порядок старшей значащей цифры (вернее, положение в векторе, 
    // так как порядок далее везде берём нулевой при вычислениях)

    quotient.clear();
    quotient.reserve( dividend.size() );

    // Делимое должно быть больше делителя, поэтому умножаем на 10 и инкрементируем показатель степени
    // Но сравниваем так, как-будто степень у обоих чисел одинаковая - нулевая

    raw_bcd_number_t tmp;

    unsigned nonZeroDigits = 0;

    quotient.insert( quotient.begin(), 1, 0 );

    // Делим до тех пор, пока не получим хотя бы 3 значащих цифры, или до заданной точности
    while( rawDivisionCheckContinueCondition( dividendPrecision, divisorPrecision, deltaCmp /* , relativeDelta */  )  || nonZeroDigits<nonZeroDigitsMin ) 
    {
        if (checkForZero( dividend ) )
            break;

        while( compareRaws( dividend, 0, divisor , 0 ) < 0 )
        {
            dividend.insert( dividend.begin(), 1, 0 );
            ++dividendPrecision;
            quotient.insert( quotient.begin(), 1, 0 );
        }

        while( compareRaws( dividend, 0, divisor , 0 ) >= 0 )
        {
            rawSubtraction( tmp, dividend, 0, divisor , 0 );
            tmp.swap(dividend);
            ++quotient[0];
        }

        if (quotient[0]>0)
           ++nonZeroDigits;

    }
    
    int 
    resultPrecision = dividendPrecision - divisorPrecision;

    resultPrecision = reducePrecision( quotient, resultPrecision );
    resultPrecision = reduceLeadingZeros( quotient, resultPrecision );

    //quotient.shrink_to_fit();

    return resultPrecision;
}


/* Шаблон перебора цифр от старших к младшим - сравнение, возможно - деление

inline
int tplFromMsdToLsd( const raw_bcd_number_t &bcdNumber1, int precision1
                   , const raw_bcd_number_t &bcdNumber2, int precision2
                   )
{
    MARTY_BCD_DECLARE_PRECISION_VIRTUAL_ADJUSTMENT_VARS();

    for( int idxFromBegin=0; idxFromBegin!=totalSize; ++idxFromBegin)
    {
        int idx  = (int)idxFromBegin;

        int idx1 = totalSize - idx - 1;
        int idx2 = totalSize - idx - 1;

        idx1 -= (int)maxPrecision - (int)precision1;
        idx2 -= (int)maxPrecision - (int)precision2;


        decimal_digit_t d1 = 0;
        decimal_digit_t d2 = 0;

        if (idx1>=0 && idx1<(int)bcdNumber1.size())
            d1 = bcdNumber1[idx1];

        if (idx2>=0 && idx2<(int)bcdNumber2.size())
            d2 = bcdNumber2[idx2];

        // Do something with digits here

    }

    return 0;
}

*/


/* Шаблон перебора цифр от младших к старшим - сложение, вычитание, умножение

inline
int templateForOperation( const raw_bcd_number_t &bcdNumber1, int precision1
                        , const raw_bcd_number_t &bcdNumber2, int precision2
                        )
{
    MARTY_BCD_DECLARE_PRECISION_VIRTUAL_ADJUSTMENT_VARS();

    for( int idxFromBegin=0; idxFromBegin!=totalSize; ++idxFromBegin)
    {
        int idx  = (int)idxFromBegin;

        int idx1 = idx - (int)maxPrecision;
        int idx2 = idx - (int)maxPrecision;

        idx1 += (int)precision1;
        idx2 += (int)precision2;

        decimal_digit_t d1 = 0;
        decimal_digit_t d2 = 0;

        if (idx1>=0 && idx1<(int)bcdNumber1.size())
            d1 = bcdNumber1[idx1];

        if (idx2>=0 && idx2<(int)bcdNumber2.size())
            d2 = bcdNumber2[idx2];

        std::cout<<(unsigned)(d1);

    }

    return 0;
}
*/




/* Что ещё необходимо:

     Идея generale в том, что (главная идея):

       а) мы не создаём копий объекта для последующей нормализации при
          как минимум сравнении. И, наверное, получится так же сделать при сложении, вычитании и умножении.
       б) Из плюсов тут то, что не надо вектору-хранилищу лишний раз лазать в кучу и что-то аллоцировать - 
          операция не слишком лёгкая и сразу даст замедление работы
       в) Из минусов - код конкретного метода будет, наверное, чуть побольше, чем если тупо расширять BCD-числа.
          Но размер кода - обычно не проблема (Flash всегда жирнее ОЗУ), а вот ОЗУ обычно мало (на некторых 
          чипах - 20Кб и меньше), и больше, чем мало (на MCU).

          Куча (Heap) vs немного кода

          Код исполняется всегда, а в кучу лазаем не всегда ( у вектора есть capacity(), который в 
          итоге мы не сможем превысить до обращения в кучу)

          Код иполняется вегда, а в кучу лазаем редко. Что быбрать?
          Наверное, таки без копирования


     Намётки, как чё
     (уже неактуально, но пусть будет, покажет, если что, как блуждала моя мюсля)

     Сравнение
       сместить стартовые точки относительно заданной precision обоих векторов;
       если в одном из двух векторов idx выходит за пределы - то используем 0;
       диапазон индексов начинаем с наименьшего индекса с меньшей precision,
         большего - начинаем с наибольшего индекса макс разряда числа

       Сравнение возвращает -1, 0, 1

     Операторы (функции) сравнения
        Необходимы для реализации вычитания.
        Едионая  0 - провкрить н ноль обаслас гаемвх) UPD - чего я тут хотел сказать? :)

     Сложение
     Вычитание
       а) расширить точность до одинаковой (или сделать алгоритм сложнее без расширения)
       б) сместить стартовые точки относительно заданной precision обоих векторов;
          если в одном из двух векторов idx выходит за пределы - то используем 0;
          диапазон индексов начинаем с наименьшего индекса с меньшей precision,
            большего - начинаем с наибольшего индекса до макс разряда числа


          tckb (U1 <=> U2)
          пре сравнеии U1 - U2 сранения со .


     Умножение.
       Умножаем столбиком - не слишком эффективно, зато просто и я так уже делал.
       Про другие способы пишут, что они быстрее после ~250 знаков числа. Нам вроде такое не грозит в нормальной ситуации.

     Деление.
       Тут жопа - тут только делал тыщу лет назад что-то похожее.
       UPD Сделал тупо на вычитаниях и сдвигах - алгоритм практически тот же, что и при делении столбиком 
           (который все в начальной школе изучали)
           Просто при делении столбиком человек видит, что на что делится и надо ли сдвигать, и видит множитель, который
           и пишет в частное, а компьютер тупой, поэтому он в цикле вычитает.

     UPD Умножение и деление. 
         Нули в числах бывают, особенно в тех, которые задаёт человек (и тут очень и очень часто).
         Для нулевых цифр в числе все шаги алгоритма пропускаются


     ЗЫ Дёрнул же меня чёрт писать Decimal самому

     ЗЗЫ На несколько вечеров есть чем заняться :)


     //TODO: !!! Оптимизация умножения/деления на степень 10
     // В этом случае ващет достаточно подвинуть десятичную точку. 
     // Поэтому надо проверять параметры, не являются ли они 10 в степени N.
     // Для умножения - оба, для деления - только делитель.
     // Проверка довольно копеечная, но если числа вида степень десяти очень редки,
     // то минус копеечка набежит, а профита не будет.
     // Надо будет сделать опционально.
 */


} // namespace bcd

} // namespace marty



