/*! \file
    \brief Тесты производительности и сравнение доморощеного Decimal, double и cpp_dec_float (вариант с хранилищем в std::basic_string)

 */

#if defined(WIN32) || defined(_WIN32)

    #include "windows.h"

#else // Linups users


#endif


#include <iostream>
#include <exception>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <iterator>
#include <map>
#include <set>
#include <optional>
#include <string>
#include <ostream>

#include "safe_main.h"

#include "../../marty_decimal.h"
#include "../../undef_min_max.h"





//#include <boost/multiprecision/mpfr.hpp>
//#include <boost/multiprecision/gmp.hpp>
#include <boost/multiprecision/cpp_dec_float.hpp>

#include <cstdlib>


// #define PROFILING

// #define EXCLUDE_BUILTIN_FROM_COMPARIZON



#if !defined(PROFILING)

    #define NUM_OF_DATA_ITEMS                          1000u
   
    #define NUM_OF_ADDITION_TEST_ITERATIONS            200000u
    #define NUM_OF_SUBTRACTION_TEST_ITERATIONS         200000u
    #define NUM_OF_MULTIPLICATION_ITERATIONS           20000u
    #define NUM_OF_DIVISION_ITERATIONS                 20000u
   
    // For long numbers test
    #define NUM_OF_LNT_ADDITION_TEST_ITERATIONS        1000u
    #define NUM_OF_LNT_SUBTRACTION_TEST_ITERATIONS     1000u
    #define NUM_OF_LNT_MULTIPLICATION_ITERATIONS       50u
    #define NUM_OF_LNT_DIVISION_ITERATIONS             50u

#else

    #define NUM_OF_DATA_ITEMS                          1000u
   
    #define NUM_OF_ADDITION_TEST_ITERATIONS            5000u
    #define NUM_OF_SUBTRACTION_TEST_ITERATIONS         5000u
    #define NUM_OF_MULTIPLICATION_ITERATIONS           500u
    #define NUM_OF_DIVISION_ITERATIONS                 500u
   
    // For long numbers test
    #define NUM_OF_LNT_ADDITION_TEST_ITERATIONS        500u
    #define NUM_OF_LNT_SUBTRACTION_TEST_ITERATIONS     500u
    #define NUM_OF_LNT_MULTIPLICATION_ITERATIONS       5u
    #define NUM_OF_LNT_DIVISION_ITERATIONS             5u

#endif

/*
// MPFR?
typedef boost::multiprecision::mpfr_float_50     mpfr_float_normal;
typedef boost::multiprecision::mpfr_float_1000   mpfr_float_long  ;
*/

// boost::multiprecision::mpfr_float::default_precision(1000); // Не совсем понятно, зачем это, если точность вроде как задана параметром шаблона


/*
// GMP
typedef boost::multiprecision::mpf_float_50     mpf_float_normal;
typedef boost::multiprecision::mpf_float_1000   mpf_float_long  ;

// Так понятнее, что откуда ноги растут
typedef mpf_float_normal  gmp_float_normal ;
typedef mpf_float_long    gmp_float_long   ;

// boost::multiprecision::mpf_float::default_precision(1000);
*/


typedef boost::multiprecision::number<boost::multiprecision::cpp_dec_float<50> >    cpp_float_normal;
typedef boost::multiprecision::number<boost::multiprecision::cpp_dec_float<1000> >  cpp_float_long  ; // Стандартно есть только cpp_dec_float_100


typedef marty::Decimal  decimal_normal;
typedef marty::Decimal  decimal_long  ;


typedef double    builtin_float_normal;
typedef double    builtin_float_long  ;



//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
class FastRandomGenerator
{

public:

    struct seed_t
    {
        std::uint32_t a = 1; // начальные значения могут быть любыми
        std::uint32_t b = 123;
        std::uint32_t c = 456;
        std::uint32_t d = 768;
    };

    typedef std::uint32_t  random_type_t;


    seed_t seed( seed_t s ) { std::swap(m_seed, s); return s; }
    seed_t seed( ) const    { return m_seed; }

    random_type_t generate()
    {
         // https://ru.wikipedia.org/wiki/Xorshift
         /* Algorithm "xor128" from p. 5 of Marsaglia, "Xorshift RNGs" */

         std::uint32_t t = m_seed.d;
        
         std::uint32_t const s = m_seed.a;
         m_seed.d = m_seed.c;
         m_seed.c = m_seed.b;
         m_seed.b = s;
        
         t ^= t << 11;
         t ^= t >> 8;

         return m_seed.a = t ^ s ^ (s >> 19);    
    }


protected:

    seed_t   m_seed;

};

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
// Небольшие числа с небольшим количеством знаков после запятой
inline
std::vector< std::string > generateNormalAriphmeticSources( FastRandomGenerator &rnd, std::size_t resultSetSize )
{
    std::vector< std::string > resVec;

    for( std::size_t r=0; r!=resultSetSize; ++r )
    {
        std::string str;

        if (rnd.generate()%2)
        {
            str.push_back('-');
        }

        auto firstDigit = 1 + rnd.generate()%9; // не ноль
        str.push_back('0'+(char)firstDigit);

        auto intPartSize = rnd.generate()%5; // Еще от нуля до пяти цифр до запятой

        for( auto i=0u; i!=intPartSize; ++i)
        {
            auto digit = rnd.generate()%10;
            str.push_back('0'+(char)digit);
        }

        auto floatPartSize = rnd.generate()%6; // Еще от нуля до шести цифр после запятой

        if (floatPartSize!=0)
            str.push_back('.');

        for( auto i=0u; i!=floatPartSize; ++i)
        {
            auto digit = rnd.generate()%10;
            str.push_back('0'+(char)digit);
        }

        resVec.push_back(str);
    
    }

    return resVec;

}

//----------------------------------------------------------------------------
// Небольшие числа, в которых большая часть числа состоит и нулей, а после запятой - обычно максимум 4-6 знаков
inline
std::vector< std::string > generateAccountingAriphmeticSources( FastRandomGenerator &rnd, std::size_t resultSetSize )
{
    std::vector< std::string > resVec;

    for( std::size_t r=0; r!=resultSetSize; ++r )
    {
        std::string str;

        if (rnd.generate()%2)
        {
            str.push_back('-');
        }

        auto firstDigit = 1 + rnd.generate()%9; // не ноль
        str.push_back('0'+(char)firstDigit);

        auto intPartSize = rnd.generate()%5; // Еще от 0 до 5 значащих цифр

        for( auto i=0u; i!=intPartSize; ++i)
        {
            auto digit = rnd.generate()%10;
            str.push_back('0'+(char)digit);
        }


        auto zerosPartSize = rnd.generate()%9; // Еще от 0 до 9 нулей до запятой

        for( auto i=0u; i!=zerosPartSize; ++i)
        {
            str.push_back('0');
        }

        // Случайно добавляем дробную часть
        if (rnd.generate()%2)
        {
            str.push_back('.');

            auto floatPartSize = rnd.generate()%3; // От 1 до 4 цифр после запятой
           
            for( auto i=0u; i!=(floatPartSize+1); ++i)
            {
                auto digit = rnd.generate()%10;
                str.push_back('0'+(char)digit);
            }

        }

        resVec.push_back(str);

    }

    return resVec;

}

//----------------------------------------------------------------------------
// Проверяем реально большие числа с 100-1000 знаками до и после запятой
inline
std::vector< std::string > generateRealBigAriphmeticSources( FastRandomGenerator &rnd, std::size_t resultSetSize )
{
    std::vector< std::string > resVec;

    for( std::size_t r=0; r!=resultSetSize; ++r )
    {
        std::string str;

        if (rnd.generate()%2)
        {
            str.push_back('-');
        }

        auto firstDigit = 1 + rnd.generate()%9; // не ноль
        str.push_back('0'+(char)firstDigit);

        auto intPartSize = rnd.generate()%200; // 1000; // Еще от 0 до 1000 цифр до запятой - 1000 - многовато

        for( auto i=0u; i!=intPartSize; ++i)
        {
            auto digit = rnd.generate()%10;
            str.push_back('0'+(char)digit);
        }

        auto floatPartSize = rnd.generate()%200; // 1000; // Еще от 0 до 1000 цифр после запятой

        if (floatPartSize!=0)
            str.push_back('.');

        for( auto i=0u; i!=floatPartSize; ++i)
        {
            auto digit = rnd.generate()%10;
            str.push_back('0'+(char)digit);
        }

        resVec.push_back(str);
    
    }

    return resVec;

}

//----------------------------------------------------------------------------
inline 
void printSourceVector( const std::vector< std::string > &v )
{
    using std::cout;
    using std::endl;

    for( std::vector< std::string >::const_iterator it=v.begin(); it!=v.end(); ++it )
    {
        cout << "    " << *it << endl;
    }

}

//----------------------------------------------------------------------------
template< typename TargetLongNumberType > inline
std::vector< TargetLongNumberType > convertToLongNumbersVector( const std::vector< std::string > &v )
{
    std::vector< TargetLongNumberType > res;
    res.reserve(v.size());

    for( std::vector< std::string >::const_iterator it=v.begin(); it!=v.end(); ++it )
    {
        res.push_back( TargetLongNumberType(*it) );
    }

    return res;

}

template< > inline
std::vector< builtin_float_normal > convertToLongNumbersVector<builtin_float_normal>( const std::vector< std::string > &v )
{
    std::vector< builtin_float_normal > res;
    res.reserve(v.size());

    for( std::vector< std::string >::const_iterator it=v.begin(); it!=v.end(); ++it )
    {
        res.push_back( std::stod(*it) );
    }

    return res;

}



//----------------------------------------------------------------------------
inline
std::uint32_t getMillisecTick()
{
    return marty::for_decimal_tests::getMillisecTick();
}

//----------------------------------------------------------------------------
template< typename VecSrcType, typename VecResType, typename OperationImpl>
inline
std::uint32_t performanceTest( const VecSrcType &src
                             , VecResType       &dst
                             //, const OperationImpl &operationImpl
                             , OperationImpl operationImpl
                             , std::size_t numIters
                             , const char* title
                             )
{
    using std::cout;
    using std::endl;
    using std::flush;

    FastRandomGenerator rnd;


    std::uint32_t startTick = getMillisecTick();

    for( auto i=0u; i!=numIters; ++i)
    {
        for( auto it=dst.begin(); it!=dst.end(); ++it )
        {
            auto idx1 = rnd.generate() % src.size();
            auto idx2 = rnd.generate() % src.size();

            *it = operationImpl(src[idx1], src[idx2]);
        }
    }

    std::uint32_t endTick = getMillisecTick();

    std::uint32_t deltaTick = endTick - startTick;

    cout << title << " elapsed time: "
         << (deltaTick) << endl << flush;

    return deltaTick;
}

//----------------------------------------------------------------------------
template< typename OperandType > inline
OperandType plusOperationImpl( const OperandType &o1, const OperandType &o2 )
{
    return o1 + o2;
}

template< typename OperandType > inline
OperandType minusOperationImpl( const OperandType &o1, const OperandType &o2 )
{
    return o1 - o2;
}

template< typename OperandType > inline
OperandType mulOperationImpl( const OperandType &o1, const OperandType &o2 )
{
    return o1 * o2;
}

template< typename OperandType > inline
OperandType divOperationImpl( const OperandType &o1, const OperandType &o2 )
{
    return o1 / o2;
}

template< typename OperandType > inline
OperandType cmpOperationImpl( const OperandType &o1, const OperandType &o2 )
{
    // Чтобы у компилятора не было возможности сфилонить, как он мог бы, когда результат сравнения просто игнорируется
    return (o1 == o2) ? o1 : o2;
}

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
template< typename OperandType >
bool isZero( const OperandType &v ) //!< Must be specialized. \returns true if v can't be divider (to prevent divisions by zero)
{
    throw std::runtime_error("isZero not implemented for this type");
    return true;
}

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
template< typename OperandType > inline
OperandType reciprocateZero()
{
    return OperandType(88888888);
}

//----------------------------------------------------------------------------
template< typename OperandType > inline
OperandType reciprocate( const OperandType &v ) //!< Must be specialized.
{
    throw std::runtime_error("reciprocate not implemented for this type");
    return reciprocateZero<OperandType>();
}

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
template< typename OperandType > inline
OperandType reciprocateOperationImpl( const OperandType &o1, const OperandType &o2 )
{
    if ( !isZero(o1) )
    {
        return reciprocate(o1);
    }

    if ( !isZero(o2) )
    {
        return reciprocate(o2);
    }

    return reciprocateZero<OperandType>();
}







//----------------------------------------------------------------------------
inline
marty::Decimal makePercentReverseRelatio( const marty::Decimal &d )
{
    if (d==0)
        return 0;

    return (marty::Decimal(100) / d).rounded( 2, marty::Decimal::RoundingMethod::roundMath );
}

//----------------------------------------------------------------------------
inline
void showPerformancePercents( std::uint32_t builtinFloatTick
                            , std::uint32_t cppFloatTick
                            , std::uint32_t decimalTick
                            )
{
    using std::cout;
    using std::endl;
    using std::flush;

    if (!builtinFloatTick)
        builtinFloatTick = 1; // assign minimal delta

    if (!decimalTick)
        decimalTick = 1; // assign minimal delta

    if (!cppFloatTick)
        cppFloatTick = 1; // assign minimal delta


    /*
    if (!builtinFloatTick || !decimalTick || !cppFloatTick)
    {
        cout << "Performance too fast" << endl;
        return;
    }
    */

    marty::Decimal builtinFloatTickDecimal  = builtinFloatTick ;
    marty::Decimal cppFloatTickDecimal      = cppFloatTick;
    marty::Decimal decimalTickDecimal       = decimalTick ;

    #if defined(EXCLUDE_BUILTIN_FROM_COMPARIZON)
        builtinFloatTickDecimal = cppFloatTick;
    #endif

    marty::Decimal perfRating = 0;

    cout << "Performance log" << endl << flush;

    #if !defined(EXCLUDE_BUILTIN_FROM_COMPARIZON)
    perfRating = makePercentReverseRelatio(builtinFloatTickDecimal.getPercentOf(decimalTickDecimal));
    cout << "    builtin_dec_float : " << builtinFloatTickDecimal .getPercentOf(decimalTickDecimal) << "%, x " << perfRating << endl << flush;
    #endif

    perfRating = makePercentReverseRelatio(cppFloatTickDecimal   .getPercentOf(decimalTickDecimal));
    cout << "    cpp_dec_float     : " << cppFloatTickDecimal    .getPercentOf(decimalTickDecimal) << "%, x " << perfRating << endl << flush;

    perfRating = makePercentReverseRelatio(decimalTickDecimal    .getPercentOf(decimalTickDecimal));
    cout << "    Decimal           : " << decimalTickDecimal     .getPercentOf(decimalTickDecimal) << "%, x " << perfRating << endl << flush;

    //cout << std::flush;

}




 /* volatile */  std::vector< decimal_normal >        decimalNormResultNumbers       ;
 /* volatile */  std::vector< decimal_long >          decimalLongResultNumbers       ;
 /*          */ 
 /* volatile */  std::vector< cpp_float_normal >      cppFloatNormResultNumbers      ;
 /* volatile */  std::vector< cpp_float_long >        cppFloatLongResultNumbers      ;

 /*          */ 
 /* volatile */  std::vector< builtin_float_normal >  builtinFloatNormResultNumbers  ;
 /* volatile */  std::vector< builtin_float_long >    builtinFloatLongResultNumbers  ;




//----------------------------------------------------------------------------
MARTY_DECIMAL_MAIN()
{

    using std::cout;
    using std::endl;

    //cout<<"Path to exe   : "<<QCoreApplication::applicationDirPath().toStdString()<<endl;

    //cout << endl;
    //cout << endl;

    //cout << "RAND_MAX: " << RAND_MAX << endl;

    

    /*
        Делаем бенчмарку для marty::Decimal.

        В качестве соперников берём типы из буста, некоторые из них используют 
        сторониие библиотеки работы с числами, только cpp_dec_float, похоже, родной для буста.

        В качестве исходных данных пробуем:

          1) Небольшие числа с небольшим количеством знаков после запятой - 
             проверяем скорость работы с "обычной" "повседневной" арифметикой;

          2) Небольшие числа, в которых большая часть числа состоит и нулей, а после запятой - обычно максимум 4-6 знаков - 
             проверяем скорость работы "бухгалтерских" расчётов - имхо, для них характерно наличие большого количества нулевх разрядов;

          9) Проверяем реально большие числа с 100-1000 знаками до и после запятой


        Значения кладем в глобальный volatile vector - чтобы компилятор не заоптимизировал его, так как он нигже не будет использоваться.

        Делаем так: пробегаем по result вектору, и результат операции кладём в текущуй элемент.
        А операнды для операции берем по случайному индексу из исходного вектора (или нескольких).
        Так сложнее (на мой взгляд) что-то заоптимизировать и компилятору, и процессору.

        Откуда брать рандомы?

        https://habr.com/ru/post/499490/
        https://ru.wikipedia.org/wiki/Xorshift


        1) Хотелось бы, чтобы они были быстрыми;
        2) Хотелось бы, чтобы они были стабильными - т.е при тестировании различных типов чисел 
           они бы обрабатывались в одинаковом порядке.

        Можно нагенерить вектор какой-то длины 32/64К значений, и с учётом некратности размера массива данных
        в принципе будет вполне годно. Но займет большой кусок памяти - если 4байтные числа брать, то это 256К.
        Надо ли кэш напрягать этим?

        Или взять какой-то простой быстрый алгоритм? Xorshift вполне подойдёт.
        Индекс нужного диапазона будем получать через остаток от деления.
       

     */

    #if 0

    FastRandomGenerator randomGen;

    for(auto i=0u; i!=100u; ++i)
    {
        auto rndNum      = randomGen.generate();
        auto rndNum10_1  = rndNum % 10;
        auto rndNum10_2  = (rndNum>>10) % 10;
        auto rndNum8     = rndNum % 8;

        cout << rndNum10_1 << " / " << rndNum10_2 << " / " << rndNum8 << " / " << rndNum << endl;
    }

    #endif

    //FastRandomGenerator rnd;

    std::vector< std::string > normalNumbers     = generateNormalAriphmeticSources    ( FastRandomGenerator(), NUM_OF_DATA_ITEMS );
    std::vector< std::string > accountingNumbers = generateAccountingAriphmeticSources( FastRandomGenerator(), NUM_OF_DATA_ITEMS );
    std::vector< std::string > realBigNumbers    = generateRealBigAriphmeticSources   ( FastRandomGenerator(), NUM_OF_DATA_ITEMS );


    #if 0

        cout << "Normal numbers: " << endl;
        printSourceVector( normalNumbers );
        cout << endl;
        cout << endl;

        cout << "Accounting numbers: " << endl;
        printSourceVector( accountingNumbers );
        cout << endl;
        cout << endl;

        cout << "Real big numbers: " << endl;
        printSourceVector( realBigNumbers );
        cout << endl;
        cout << endl;

    #endif

    /*
    decimalNormResultNumbers .assign( (std::size_t)NUM_OF_DATA_ITEMS, decimal_normal()   );
    decimalLongResultNumbers .assign( (std::size_t)NUM_OF_DATA_ITEMS, decimal_long  ()   );

    cppFloatNormResultNumbers.assign( (std::size_t)NUM_OF_DATA_ITEMS, cpp_float_normal() );
    cppFloatLongResultNumbers.assign( (std::size_t)NUM_OF_DATA_ITEMS, cpp_float_long  () );
    */

    decimalNormResultNumbers  = std::vector< decimal_normal >   ( (std::size_t)NUM_OF_DATA_ITEMS, decimal_normal()   );
    decimalLongResultNumbers  = std::vector< decimal_long >     ( (std::size_t)NUM_OF_DATA_ITEMS, decimal_long  ()   );
                                                              
    cppFloatNormResultNumbers = std::vector< cpp_float_normal > ( (std::size_t)NUM_OF_DATA_ITEMS, cpp_float_normal() );
    cppFloatLongResultNumbers = std::vector< cpp_float_long >   ( (std::size_t)NUM_OF_DATA_ITEMS, cpp_float_long  () );

    builtinFloatNormResultNumbers = std::vector< builtin_float_normal > ( (std::size_t)NUM_OF_DATA_ITEMS, builtin_float_normal(0.0) );
    builtinFloatLongResultNumbers = std::vector< builtin_float_long >   ( (std::size_t)NUM_OF_DATA_ITEMS, builtin_float_long  (0.0) );




    //NUM_OF_TEST_ITERATIONS


    std::vector< decimal_normal >   decimalNormNormalNumbers       = convertToLongNumbersVector<decimal_normal>(normalNumbers    );
    std::vector< decimal_normal >   decimalNormAccountingNumbers   = convertToLongNumbersVector<decimal_normal>(accountingNumbers);
    std::vector< decimal_normal >   decimalNormRealBigNumbers      = convertToLongNumbersVector<decimal_normal>(realBigNumbers   );

    std::vector< decimal_long >     decimalLongNormalNumbers       = convertToLongNumbersVector<decimal_long>(normalNumbers    );
    std::vector< decimal_long >     decimalLongAccountingNumbers   = convertToLongNumbersVector<decimal_long>(accountingNumbers);
    std::vector< decimal_long >     decimalLongRealBigNumbers      = convertToLongNumbersVector<decimal_long>(realBigNumbers   );


    std::vector< cpp_float_normal > cppFloatNormNormalNumbers      = convertToLongNumbersVector<cpp_float_normal>(normalNumbers    );
    std::vector< cpp_float_normal > cppFloatNormAccountingNumbers  = convertToLongNumbersVector<cpp_float_normal>(accountingNumbers);
    std::vector< cpp_float_normal > cppFloatNormRealBigNumbers     = convertToLongNumbersVector<cpp_float_normal>(realBigNumbers   );

    std::vector< cpp_float_long >   cppFloatLongNormalNumbers      = convertToLongNumbersVector<cpp_float_long>(normalNumbers    );
    std::vector< cpp_float_long >   cppFloatLongAccountingNumbers  = convertToLongNumbersVector<cpp_float_long>(accountingNumbers);
    std::vector< cpp_float_long >   cppFloatLongRealBigNumbers     = convertToLongNumbersVector<cpp_float_long>(realBigNumbers   );


    std::vector< builtin_float_normal > builtinFloatNormNormalNumbers      = convertToLongNumbersVector<builtin_float_normal>(normalNumbers    );
    std::vector< builtin_float_normal > builtinFloatNormAccountingNumbers  = convertToLongNumbersVector<builtin_float_normal>(accountingNumbers);
    std::vector< builtin_float_normal > builtinFloatNormRealBigNumbers     = convertToLongNumbersVector<builtin_float_normal>(realBigNumbers   );

    std::vector< builtin_float_long >   builtinFloatLongNormalNumbers      = convertToLongNumbersVector<builtin_float_long>(normalNumbers    );
    std::vector< builtin_float_long >   builtinFloatLongAccountingNumbers  = convertToLongNumbersVector<builtin_float_long>(accountingNumbers);
    std::vector< builtin_float_long >   builtinFloatLongRealBigNumbers     = convertToLongNumbersVector<builtin_float_long>(realBigNumbers   );





    #if 0

    #define TEST_IMPL( src, dst, op, numIters, title )      \
                                                            \
    do                                                      \
    {                                                       \
        FastRandomGenerator rnd;                            \
                                                            \
        std::uint32_t startTick = getMillisecTick();        \
                                                            \
        for( auto i=0u; i!=numIters; ++i)                   \
        {                                                   \
            for( auto it=dst.begin(); it!=dst.end(); ++it ) \
            {                                               \
                auto idx1 = rnd.generate() % src.size();    \
                auto idx2 = rnd.generate() % src.size();    \
                                                            \
                *it = src[idx1] op src[idx2];               \
            }                                               \
        }                                                   \
                                                            \
        std::uint32_t endTick = getMillisecTick();          \
                                                            \
        cout << title << " elapsed time: "                  \
             << (endTick-startTick) << endl;                \
                                                            \
    } while(0)

    #endif

    #define TEST_IMPL( src, dst, op, opType, numIters, title )                            \
                                                                                  \
                 performanceTest( src, dst, op##OperationImpl<opType>, numIters, title );


    #if defined(WIN64) || defined(_WIN64)

        cout<<"Platform: x64" << endl;

    #else

        cout<<"Platform: x86" << endl;

    #endif

    #if defined(MARTY_BCD_USE_VECTOR)

        cout<<"BCD storage type: std::vector" << endl << endl;

    #else

        cout<<"BCD storage type: std::basic_string" << endl << endl;

    #endif


    std::uint32_t t1, t2, t3;


    // Возможно, относительно быстрое деление связано с тем, что по умолчанию при делении используется точность 18 знаков
    // Поэтому зададим точность 50 знаков после запятой
    // Кстати, поэтому, возможно, мой Decimal с setDivisionPrecision(50) отстаёт от cpp_dec_float<50> - последний гарантирует 50 значащих цифр:
    //     The typedefs cpp_dec_float_50 and cpp_dec_float_100 provide arithmetic types at 50 and 100 decimal digits precision respectively
    //     boost::multiprecision::number<boost::multiprecision::cpp_dec_float<50> >
    // тогда как мой Decimal гарантирует 50 значащих цифр после десятичной точки (при задании точности 50)
    
    //NOTE: !!! cpp_dec_float: 1) гарантирует 50 значащих десятичных цифр всего, а не после десятичной точки
    //                         2) Немотря на название, не является десятичным числом - косвенным подтверждением 
    //                            этого является то, что на рекурентном тесте Мюллера его расколбашивает
    //                            точно так же, как и встроенный в язык double.
    //                            dec в названии говорит только о том, что данный тип хранит не менее, чем заданное число десятичных знаков.
    //


    marty::Decimal::setDivisionPrecision(50);

    /*
    std::vector< builtin_float_normal > builtinFloatNormNormalNumbers      = convertToLongNumbersVector<cpp_float_normal>(normalNumbers    );
    std::vector< builtin_float_normal > builtinFloatNormAccountingNumbers  = convertToLongNumbersVector<cpp_float_normal>(accountingNumbers);
    std::vector< builtin_float_normal > builtinFloatNormRealBigNumbers     = convertToLongNumbersVector<cpp_float_normal>(realBigNumbers   );

    std::vector< builtin_float_long >   builtinFloatLongNormalNumbers      = convertToLongNumbersVector<builtin_float_long>(normalNumbers    );
    std::vector< builtin_float_long >   builtinFloatLongAccountingNumbers  = convertToLongNumbersVector<builtin_float_long>(accountingNumbers);
    std::vector< builtin_float_long >   builtinFloatLongRealBigNumbers     = convertToLongNumbersVector<builtin_float_long>(realBigNumbers   );


    std::vector< builtin_float_normal >  builtinFloatNormResultNumbers  ;
    std::vector< builtin_float_long >    builtinFloatLongResultNumbers  ;
    */

    #if !defined(PROFILING)

        t1 = TEST_IMPL( builtinFloatNormNormalNumbers      , builtinFloatNormResultNumbers , plus , builtin_float_normal  , NUM_OF_ADDITION_TEST_ITERATIONS        , "Builtin Float normal precision, normal numbers, '+' " );
        t2 = TEST_IMPL( cppFloatNormNormalNumbers          , cppFloatNormResultNumbers     , plus , cpp_float_normal      , NUM_OF_ADDITION_TEST_ITERATIONS        , "Cpp Float     normal precision, normal numbers, '+' " );
        t3 = TEST_IMPL( decimalNormNormalNumbers           , decimalNormResultNumbers      , plus , decimal_normal        , NUM_OF_ADDITION_TEST_ITERATIONS        , "Decimal       normal precision, normal numbers, '+' " );
        showPerformancePercents( t1, t2, t3 );
        cout << endl;
       
        t1 = TEST_IMPL( builtinFloatNormNormalNumbers      , builtinFloatNormResultNumbers , minus, builtin_float_normal  , NUM_OF_SUBTRACTION_TEST_ITERATIONS     , "Builtin Float normal precision, normal numbers, '-' " );
        t2 = TEST_IMPL( cppFloatNormNormalNumbers          , cppFloatNormResultNumbers     , minus, cpp_float_normal      , NUM_OF_SUBTRACTION_TEST_ITERATIONS     , "Cpp Float     normal precision, normal numbers, '-' " );
        t3 = TEST_IMPL( decimalNormNormalNumbers           , decimalNormResultNumbers      , minus, decimal_normal        , NUM_OF_SUBTRACTION_TEST_ITERATIONS     , "Decimal       normal precision, normal numbers, '-' " );
        showPerformancePercents( t1, t2, t3 );
        cout << endl;
       
        t1 = TEST_IMPL( builtinFloatNormNormalNumbers      , builtinFloatNormResultNumbers , mul  , builtin_float_normal  , NUM_OF_MULTIPLICATION_ITERATIONS       , "Builtin Float normal precision, normal numbers, '*' " );
        t2 = TEST_IMPL( cppFloatNormNormalNumbers          , cppFloatNormResultNumbers     , mul  , cpp_float_normal      , NUM_OF_MULTIPLICATION_ITERATIONS       , "Cpp Float     normal precision, normal numbers, '*' " );
        t3 = TEST_IMPL( decimalNormNormalNumbers           , decimalNormResultNumbers      , mul  , decimal_normal        , NUM_OF_MULTIPLICATION_ITERATIONS       , "Decimal       normal precision, normal numbers, '*' " );
        showPerformancePercents( t1, t2, t3 );
        cout << endl;
       
        t1 = TEST_IMPL( builtinFloatNormNormalNumbers      , builtinFloatNormResultNumbers , div  , builtin_float_normal  , NUM_OF_DIVISION_ITERATIONS             , "Builtin Float normal precision, normal numbers, '/' " );
        t2 = TEST_IMPL( cppFloatNormNormalNumbers          , cppFloatNormResultNumbers     , div  , cpp_float_normal      , NUM_OF_DIVISION_ITERATIONS             , "Cpp Float     normal precision, normal numbers, '/' " );
        t3 = TEST_IMPL( decimalNormNormalNumbers           , decimalNormResultNumbers      , div  , decimal_normal        , NUM_OF_DIVISION_ITERATIONS             , "Decimal       normal precision, normal numbers, '/' " );
        showPerformancePercents( t1, t2, t3 );
        cout << endl;
       
        t1 = TEST_IMPL( builtinFloatNormNormalNumbers      , builtinFloatNormResultNumbers , cmp  , builtin_float_normal  , NUM_OF_DIVISION_ITERATIONS             , "Builtin Float normal precision, normal numbers, '=='" );
        t2 = TEST_IMPL( cppFloatNormNormalNumbers          , cppFloatNormResultNumbers     , cmp  , cpp_float_normal      , NUM_OF_DIVISION_ITERATIONS             , "Cpp Float     normal precision, normal numbers, '=='" );
        t3 = TEST_IMPL( decimalNormNormalNumbers           , decimalNormResultNumbers      , cmp  , decimal_normal        , NUM_OF_DIVISION_ITERATIONS             , "Decimal       normal precision, normal numbers, '=='" );
        showPerformancePercents( t1, t2, t3 );
        cout << endl;
       
        cout << "------------------------------" << endl;
       
        t1 = TEST_IMPL( builtinFloatNormAccountingNumbers  , builtinFloatNormResultNumbers , plus , builtin_float_normal  , NUM_OF_ADDITION_TEST_ITERATIONS        , "Builtin Float normal precision, accounting numbers, '+' " );
        t2 = TEST_IMPL( cppFloatNormAccountingNumbers      , cppFloatNormResultNumbers     , plus , cpp_float_normal      , NUM_OF_ADDITION_TEST_ITERATIONS        , "Cpp Float     normal precision, accounting numbers, '+' " );
        t3 = TEST_IMPL( decimalNormAccountingNumbers       , decimalNormResultNumbers      , plus , decimal_normal        , NUM_OF_ADDITION_TEST_ITERATIONS        , "Decimal       normal precision, accounting numbers, '+' " );
        showPerformancePercents( t1, t2, t3 );
        cout << endl;
       
        t1 = TEST_IMPL( builtinFloatNormAccountingNumbers  , builtinFloatNormResultNumbers , minus, builtin_float_normal  , NUM_OF_SUBTRACTION_TEST_ITERATIONS     , "Builtin Float normal precision, accounting numbers, '-' " );
        t2 = TEST_IMPL( cppFloatNormAccountingNumbers      , cppFloatNormResultNumbers     , minus, cpp_float_normal      , NUM_OF_SUBTRACTION_TEST_ITERATIONS     , "Cpp Float     normal precision, accounting numbers, '-' " );
        t3 = TEST_IMPL( decimalNormAccountingNumbers       , decimalNormResultNumbers      , minus, decimal_normal        , NUM_OF_SUBTRACTION_TEST_ITERATIONS     , "Decimal       normal precision, accounting numbers, '-' " );
        showPerformancePercents( t1, t2, t3 );
        cout << endl;
       
        t1 = TEST_IMPL( builtinFloatNormAccountingNumbers  , builtinFloatNormResultNumbers , mul  , builtin_float_normal  , NUM_OF_MULTIPLICATION_ITERATIONS       , "Builtin Float normal precision, accounting numbers, '*' " );
        t2 = TEST_IMPL( cppFloatNormAccountingNumbers      , cppFloatNormResultNumbers     , mul  , cpp_float_normal      , NUM_OF_MULTIPLICATION_ITERATIONS       , "Cpp Float     normal precision, accounting numbers, '*' " );
        t3 = TEST_IMPL( decimalNormAccountingNumbers       , decimalNormResultNumbers      , mul  , decimal_normal        , NUM_OF_MULTIPLICATION_ITERATIONS       , "Decimal       normal precision, accounting numbers, '*' " );
        showPerformancePercents( t1, t2, t3 );
        cout << endl;
       
        t1 = TEST_IMPL( builtinFloatNormAccountingNumbers  , builtinFloatNormResultNumbers , div  , builtin_float_normal  , NUM_OF_DIVISION_ITERATIONS             , "Builtin Float normal precision, accounting numbers, '/' " );
        t2 = TEST_IMPL( cppFloatNormAccountingNumbers      , cppFloatNormResultNumbers     , div  , cpp_float_normal      , NUM_OF_DIVISION_ITERATIONS             , "Cpp Float     normal precision, accounting numbers, '/' " );
        t3 = TEST_IMPL( decimalNormAccountingNumbers       , decimalNormResultNumbers      , div  , decimal_normal        , NUM_OF_DIVISION_ITERATIONS             , "Decimal       normal precision, accounting numbers, '/' " );
        showPerformancePercents( t1, t2, t3 );
        cout << endl;
       
        t1 = TEST_IMPL( builtinFloatNormAccountingNumbers  , builtinFloatNormResultNumbers , cmp  , builtin_float_normal  , NUM_OF_DIVISION_ITERATIONS             , "Builtin Float normal precision, accounting numbers, '=='" );
        t2 = TEST_IMPL( cppFloatNormAccountingNumbers      , cppFloatNormResultNumbers     , cmp  , cpp_float_normal      , NUM_OF_DIVISION_ITERATIONS             , "Cpp Float     normal precision, accounting numbers, '=='" );
        t3 = TEST_IMPL( decimalNormAccountingNumbers       , decimalNormResultNumbers      , cmp  , decimal_normal        , NUM_OF_DIVISION_ITERATIONS             , "Decimal       normal precision, accounting numbers, '=='" );
        showPerformancePercents( t1, t2, t3 );
        cout << endl;
       
        cout << "------------------------------" << endl;
       
       
        marty::Decimal::setDivisionPrecision(1000);
       
        t1 = TEST_IMPL( builtinFloatLongRealBigNumbers     , builtinFloatLongResultNumbers , plus , builtin_float_long    , NUM_OF_LNT_ADDITION_TEST_ITERATIONS    , "Builtin Float long precision, big numbers, '+' " );
        t2 = TEST_IMPL( cppFloatLongRealBigNumbers         , cppFloatLongResultNumbers     , plus , cpp_float_long        , NUM_OF_LNT_ADDITION_TEST_ITERATIONS    , "Cpp Float     long precision, big numbers, '+' " );
        t3 = TEST_IMPL( decimalLongRealBigNumbers          , decimalLongResultNumbers      , plus , decimal_long          , NUM_OF_LNT_ADDITION_TEST_ITERATIONS    , "Decimal       long precision, big numbers, '+' " );
        showPerformancePercents( t1, t2, t3 );
        cout << endl;
       
        t1 = TEST_IMPL( builtinFloatLongRealBigNumbers     , builtinFloatLongResultNumbers , minus, builtin_float_long    , NUM_OF_SUBTRACTION_TEST_ITERATIONS     , "Builtin Float long precision, big numbers, '-' " );
        t2 = TEST_IMPL( cppFloatLongRealBigNumbers         , cppFloatLongResultNumbers     , minus, cpp_float_long        , NUM_OF_SUBTRACTION_TEST_ITERATIONS     , "Cpp Float     long precision, big numbers, '-' " );
        t3 = TEST_IMPL( decimalLongRealBigNumbers          , decimalLongResultNumbers      , minus, decimal_long          , NUM_OF_LNT_SUBTRACTION_TEST_ITERATIONS , "Decimal       long precision, big numbers, '-' " );
        showPerformancePercents( t1, t2, t3 );
        cout << endl;
       
        t1 = TEST_IMPL( builtinFloatLongRealBigNumbers     , builtinFloatLongResultNumbers , mul  , builtin_float_long    , NUM_OF_LNT_MULTIPLICATION_ITERATIONS   , "Builtin Float long precision, big numbers, '*' " );
        t2 = TEST_IMPL( cppFloatLongRealBigNumbers         , cppFloatLongResultNumbers     , mul  , cpp_float_long        , NUM_OF_LNT_MULTIPLICATION_ITERATIONS   , "Cpp Float     long precision, big numbers, '*' " );
        t3 = TEST_IMPL( decimalLongRealBigNumbers          , decimalLongResultNumbers      , mul  , decimal_long          , NUM_OF_LNT_MULTIPLICATION_ITERATIONS   , "Decimal       long precision, big numbers, '*' " );
        showPerformancePercents( t1, t2, t3 );
        cout << endl;
       
        t1 = TEST_IMPL( builtinFloatLongRealBigNumbers     , builtinFloatLongResultNumbers , div  , builtin_float_long    , NUM_OF_LNT_DIVISION_ITERATIONS         , "Builtin Float long precision, big numbers, '/' " );
        t2 = TEST_IMPL( cppFloatLongRealBigNumbers         , cppFloatLongResultNumbers     , div  , cpp_float_long        , NUM_OF_LNT_DIVISION_ITERATIONS         , "Cpp Float     long precision, big numbers, '/' " );
        t3 = TEST_IMPL( decimalLongRealBigNumbers          , decimalLongResultNumbers      , div  , decimal_long          , NUM_OF_LNT_DIVISION_ITERATIONS         , "Decimal       long precision, big numbers, '/' " );
        showPerformancePercents( t1, t2, t3 );
        cout << endl;
       
        t1 = TEST_IMPL( builtinFloatLongRealBigNumbers     , builtinFloatLongResultNumbers , cmp  , builtin_float_long    , NUM_OF_LNT_DIVISION_ITERATIONS         , "Builtin Float long precision, big numbers, '=='" );
        t2 = TEST_IMPL( cppFloatLongRealBigNumbers         , cppFloatLongResultNumbers     , cmp  , cpp_float_long        , NUM_OF_LNT_DIVISION_ITERATIONS         , "Cpp Float     long precision, big numbers, '=='" );
        t3 = TEST_IMPL( decimalLongRealBigNumbers          , decimalLongResultNumbers      , cmp  , decimal_long          , NUM_OF_LNT_DIVISION_ITERATIONS         , "Decimal       long precision, big numbers, '=='" );
        showPerformancePercents( t1, t2, t3 );
        cout << endl;
       
        cout << "------------------------------------------------------------------------------------------------------------------------------------------------------------------------" << endl;

    #else

        t2 = TEST_IMPL( decimalNormNormalNumbers       , decimalNormResultNumbers      , plus , decimal_normal   , NUM_OF_ADDITION_TEST_ITERATIONS        , "Decimal   normal precision, normal numbers, + " );
        t2 = TEST_IMPL( decimalNormNormalNumbers       , decimalNormResultNumbers      , minus, decimal_normal   , NUM_OF_SUBTRACTION_TEST_ITERATIONS     , "Decimal   normal precision, normal numbers, - " );
        t2 = TEST_IMPL( decimalNormNormalNumbers       , decimalNormResultNumbers      , mul  , decimal_normal   , NUM_OF_MULTIPLICATION_ITERATIONS       , "Decimal   normal precision, normal numbers, * " );
        t2 = TEST_IMPL( decimalNormNormalNumbers       , decimalNormResultNumbers      , div  , decimal_normal   , NUM_OF_DIVISION_ITERATIONS             , "Decimal   normal precision, normal numbers, / " );
        t2 = TEST_IMPL( decimalNormAccountingNumbers   , decimalNormResultNumbers      , plus , decimal_normal   , NUM_OF_ADDITION_TEST_ITERATIONS        , "Decimal   normal precision, accounting numbers, + " );
        t2 = TEST_IMPL( decimalNormAccountingNumbers   , decimalNormResultNumbers      , minus, decimal_normal   , NUM_OF_SUBTRACTION_TEST_ITERATIONS     , "Decimal   normal precision, accounting numbers, - " );
        t2 = TEST_IMPL( decimalNormAccountingNumbers   , decimalNormResultNumbers      , mul  , decimal_normal   , NUM_OF_MULTIPLICATION_ITERATIONS       , "Decimal   normal precision, accounting numbers, * " );
        t2 = TEST_IMPL( decimalNormAccountingNumbers   , decimalNormResultNumbers      , div  , decimal_normal   , NUM_OF_DIVISION_ITERATIONS             , "Decimal   normal precision, accounting numbers, / " );
       
        marty::Decimal::setDivisionPrecision(1000);
       
        t2 = TEST_IMPL( decimalLongRealBigNumbers      , decimalLongResultNumbers      , plus , decimal_long     , NUM_OF_LNT_ADDITION_TEST_ITERATIONS    , "Decimal   long precision, big numbers, + " );
        t2 = TEST_IMPL( decimalLongRealBigNumbers      , decimalLongResultNumbers      , minus, decimal_long     , NUM_OF_LNT_SUBTRACTION_TEST_ITERATIONS , "Decimal   long precision, big numbers, - " );
        t2 = TEST_IMPL( decimalLongRealBigNumbers      , decimalLongResultNumbers      , mul  , decimal_long     , NUM_OF_LNT_MULTIPLICATION_ITERATIONS   , "Decimal   long precision, big numbers, * " );
        t2 = TEST_IMPL( decimalLongRealBigNumbers      , decimalLongResultNumbers      , div  , decimal_long     , NUM_OF_LNT_DIVISION_ITERATIONS         , "Decimal   long precision, big numbers, / " );

    #endif

/*

*/



/*

NUM_OF_LNT_ADDITION_TEST_ITERATIONS

#define NUM_OF_ADDITION_TEST_ITERATIONS    10000u
#define NUM_OF_SUBTRACTION_TEST_ITERATIONS 10000u
#define NUM_OF_MULTIPLICATION_ITERATIONS   10000u
#define NUM_OF_DIVISION_ITERATIONS         10000u

*/

    
    return 0;
}



