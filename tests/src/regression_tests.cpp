/*! \file
    \brief Decimal type regression tests

 */

#include <iostream>
#include <exception>
#include <stdexcept>
#include <sstream>
#include <iomanip>

#define MARTY_NO_QT

#include "safe_main.h"
#include "../../marty_decimal.h"
#include "../../undef_min_max.h"


// #define MARTY_DECIMAL_MULLER_TEST_NO_BOOST


#if !defined(MARTY_DECIMAL_MULLER_TEST_NO_BOOST)

    #include <boost/multiprecision/cpp_dec_float.hpp>

#endif


#define THROW_ERROR() do{ throw std::runtime_error("Test failed"); } while(0)
//#define THROW_ERROR() do{ } while(0)



// #include "invest_openapi/utility.h"
// #include "invest_openapi/qt_helpers.h"


#include "cpp.h"


//----------------------------------------------------------------------------
// int - удобнее
inline
int getFirstDiffPos( const std::string &v1, const std::string &v2 )
{
    // std::string::size_type i = 0, sz = std::min( v1.size(), v2.size() );
    std::string::size_type i = 0;
    std::string::size_type sz = std::min( v1.size(), v2.size() );
    //std::string::size_type sz = v1.size() < v2.size() ? v1.size() : v2.size();

    if (!sz)
        return 0;

    for(; i!=sz; ++i )
    {
        if (v1[i]!=v2[i])
           return i;
    }

    return -1;
}

//----------------------------------------------------------------------------
inline
std::string makeDiffMarkerString( int strDiffRes, char marker = '|' )
{
    if (strDiffRes<0)
       return std::string();

    if (marker==0)
        return cpp::expandAtFront( std::string(), strDiffRes );

    return cpp::expandAtFront( std::string(), strDiffRes ) + std::string(1, marker);
}

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
inline
int printDiffPair( const std::string &v1, const std::string &v2, std::string indend, bool printExMarker = true )
{
    using std::cout;
    using std::endl;

    auto mrkPos  = getFirstDiffPos( v1, v2 );

    /*
    if (mrkPos<0 && v1.size()!=v2.size())
    {
        mrkPos = std::min(v1.size(),v2.size());
    }
    */

    auto mrkLine = makeDiffMarkerString(mrkPos);

    std::string mrkPosStr;

    std::string mrkIndend = indend;

    if (printExMarker)
    {
        if (mrkPos<0)
        {
            mrkIndend += "+";
        }
        else
        {
            mrkIndend += "!";
            mrkPosStr += " - ";
            mrkPosStr += std::to_string(mrkPos);
        }

        mrkIndend += " ";

        indend    += "  ";
    }

    cout << indend    << v1 << endl;
    cout << mrkIndend << mrkLine << mrkPosStr << endl;
    cout << indend    << v2 << endl;

    return mrkPos;
}

//----------------------------------------------------------------------------
inline
int printDiffPair( const marty::Decimal &v1, const marty::Decimal &v2, std::string indend, bool printExMarker = true )
{
    return printDiffPair( v1.toString(), v2.toString(), indend, printExMarker );
}

//----------------------------------------------------------------------------





//----------------------------------------------------------------------------
typedef marty::Decimal Decimal;

inline Decimal invalidOperatorImpl( const Decimal &o1, const Decimal &o2 )
{
    throw std::runtime_error("invalidOperatorImpl called");
    return o1;
}

inline Decimal plusOperatorImpl( const Decimal &o1, const Decimal &o2 )
{
    return o1 + o2;
}

inline Decimal minusOperatorImpl( const Decimal &o1, const Decimal &o2 )
{
    return o1 - o2;
}

inline Decimal mulOperatorImpl( const Decimal &o1, const Decimal &o2 )
{
    return o1 * o2;
}

inline Decimal divOperatorImpl( const Decimal &o1, const Decimal &o2 )
{
    return o1 / o2;
}

typedef Decimal (*BinaryOperatorFunctionType)( const Decimal &o1, const Decimal &o2 );

inline BinaryOperatorFunctionType getOperatorFunctionBySign( const char *opSign )
{
    if (!opSign)
        return &invalidOperatorImpl;

    char ch = *opSign;

    switch(ch)
    {
        case '+': return &plusOperatorImpl;
        case '-': return &minusOperatorImpl;
        case '*': return &mulOperatorImpl;
        case '/': return &divOperatorImpl;
        default : return &invalidOperatorImpl;
    
    }

}

//----------------------------------------------------------------------------
// GCC __PRETTY_FUNCTION__
// __func__ exists in C99 / C++11
// __FUNCTION__ is non standard
inline
bool decimalOpTest( unsigned &totalOpTests, unsigned &totalOpTestsFailed
                  , const char *val1, const char *val2, const std::string &strResForCompare
                  , const char *opSign
                  , const char *file
                  , int         line
                  )
{
    using std::cout;
    using std::endl;

    ++totalOpTests;

    BinaryOperatorFunctionType operatorImpl = getOperatorFunctionBySign(opSign);

    Decimal     d1       = Decimal(val1);
    Decimal     d2       = Decimal(val2);
    Decimal     dRes     = operatorImpl(d1,d2);
    std::string strRes   = dRes.toString( /* precision - auto */ );

    auto mrkPos  = getFirstDiffPos( strRes, strResForCompare );

    if (mrkPos<0 && strResForCompare.size() > strRes.size() )
    {
        mrkPos = strRes.size();
    }

    bool bGood /* Johny */ = mrkPos<0; // dRes.checkIsExact(strResForCompare);
    if (!bGood)
       ++totalOpTestsFailed;


    cout << "[" << (bGood ? "+" : "-") << "]  " << d1 << " " << opSign << " " << d2 << " = " << strRes << "";
    if (!bGood)
    {
        cout << endl << "     expected - '" << strResForCompare << "'";
        cout << endl;
        /* int */
        printDiffPair( strRes, strResForCompare, std::string("   "), true  /* printExMarker */  );

        cout << file << ":" << line << endl;

    }
    else
    {
        cout << endl;
    }

    if (!bGood)
    {
        THROW_ERROR();
        return false;
    }

    return true;

}

//----------------------------------------------------------------------------





//----------------------------------------------------------------------------
template<typename IntType1, typename IntType2, typename IntTypeRes>
void martyLongIntMultiplyTestHelper( IntType1 u1, IntType2 u2, IntTypeRes &res )
{
    std::uint16_t ul1[ marty::calcNumberOfUint16ItemsInLong<IntType1>() ];
    marty::convertToLongUnsigned( u1, &ul1[0] );

    std::uint16_t ul2[ marty::calcNumberOfUint16ItemsInLong<IntType2>() ];
    marty::convertToLongUnsigned( u2, &ul2[0] );

    std::uint16_t ulRes[ marty::calcNumberOfUint16ItemsInLong<IntType1>() + marty::calcNumberOfUint16ItemsInLong<IntType2>() ];

    marty::multiplicationOfUnsignedLongInts( &ul1[0], marty::calcNumberOfUint16ItemsInLong<IntType1>()
                                           , &ul2[0], marty::calcNumberOfUint16ItemsInLong<IntType2>()
                                           , &ulRes[0], marty::calcNumberOfUint16ItemsInLong<IntType1>() + marty::calcNumberOfUint16ItemsInLong<IntType2>()
                                           );

    marty::convertFromLongUnsigned( res, &ulRes[0], marty::calcNumberOfUint16ItemsInLong<IntType1>() + marty::calcNumberOfUint16ItemsInLong<IntType2>() );

}

template<typename IntType>
void martyLongIntSummTestHelper( IntType u1, IntType u2, IntType &res )
{
    std::uint16_t ul1[ marty::calcNumberOfUint16ItemsInLong<IntType>() ];
    marty::convertToLongUnsigned( u1, &ul1[0] );

    std::uint16_t ul2[ marty::calcNumberOfUint16ItemsInLong<IntType>() ];
    marty::convertToLongUnsigned( u2, &ul2[0] );

    std::uint16_t ulRes[ marty::calcNumberOfUint16ItemsInLong<IntType>() ];

    marty::summationOfUnsignedLongInts( &ul1[0], marty::calcNumberOfUint16ItemsInLong<IntType>()
                                      , &ul2[0], marty::calcNumberOfUint16ItemsInLong<IntType>()
                                      , &ulRes[0], marty::calcNumberOfUint16ItemsInLong<IntType>()
                                      );

    marty::convertFromLongUnsigned( res, &ulRes[0], marty::calcNumberOfUint16ItemsInLong<IntType>() );

}


template <typename TParam, typename TResult = TParam> inline
TResult quoteOnlyString( TParam p )
{
    return p;
}

template<> inline
std::string quoteOnlyString<std::string, std::string>( std::string p )
{
    return std::string("\"") + p + std::string("\"");
}

template<> inline
std::string quoteOnlyString< const char*, std::string >( const char* p )
{
    return quoteOnlyString< std::string, std::string >(p);
}

template < std::size_t N >
inline
std::string quoteOnlyString( const char( &str )[ N ] )
{
    return quoteOnlyString< std::string, std::string >(&str[0]);
}




template< typename SomeFloatingType >
std::uint32_t mullersRecurrenceFunctionTest( std::size_t testSize, const char *testTitle );




MARTY_DECIMAL_MAIN()
{
    using std::cout;
    using std::endl;

    using marty::Decimal;
    // using marty::DecimalPrecision;
    // using marty::toString;
    // using marty::fromString;

    #define TEST_QUOTE_ONLY_STRING(param) \
                 cout << #param << ": " << param << endl;

    /*
    TEST_QUOTE_ONLY_STRING( quoteOnlyString(7) );
    TEST_QUOTE_ONLY_STRING( quoteOnlyString("7c") );
    TEST_QUOTE_ONLY_STRING( quoteOnlyString(std::string("7s")) );
    */

    // simple pretest
    {

        std::uint32_t u1 = 213251u; // u1  = 0x00034103á ul1 = { 0x4103, 0x0003 }
        std::uint32_t u2 = 153754u; // u2  = 0x0002589a  ul2  = { 0x589a, 0x0002 }
        // 0x4103 * 0x589a = 377496526 // 0x168023CE

        std::uint64_t uRes; // 213251 * 153754 = 32788194254 (0x07A25423CE)
        std::uint32_t sumRes;

        martyLongIntMultiplyTestHelper( u1, u2, uRes );
        cout << u1 << " x " << u2 << " = " << uRes << endl;

        martyLongIntSummTestHelper( u1, u2, sumRes );
        cout << u1 << " + " << u2 << " = " << sumRes << endl; // 213251+153754 = 367005


        u1 = 54213251u;
        u2 = 91153754u;

        martyLongIntMultiplyTestHelper( u1, u2, uRes );
        cout << u1 << " x " << u2 << " = " << uRes << endl;
        // 54213251 * 91153754 = 4941741345194254

        martyLongIntSummTestHelper( u1, u2, sumRes );
        cout << u1 << " + " << u2 << " = " << sumRes << endl; // 54213251+91153754 = 145367005


        u1 = 4213251u;  // 0x0040 4A03
        u2 = 1153754u;  // 0x0011 9ADA
        // 0x4A03*0x9ADA = 0x2CC4 D48E
        // 0x4A03*0x0011 = 0x0004 EA33
        //                             0x2CC9 BEC1
        // 0x0040*0x9ADA = 0x0026 B680
        //                             0x2CF0 7541
        // 0x0040*0x0011 = 0x0000 0440
        //                             0x2CF07981
        
        martyLongIntMultiplyTestHelper( u1, u2, uRes );
        cout << u1 << " x " << u2 << " = " << uRes << endl;
        // 4213251 * 1153754 =    4861055194254

        martyLongIntSummTestHelper( u1, u2, sumRes );
        cout << u1 << " + " << u2 << " = " << sumRes << endl; // 4213251+1153754 = 5367005

     }

    cout << endl;
    cout << "------------------------------" << endl;
    cout << endl;


    if (0)
    {
        cout << "Precision truncation calls for debug" << endl;
        cout << endl;

        marty::bcd::raw_bcd_number_t bcdNumber;
        int bcdNumberPrecision = marty::bcd::makeRawBcdNumber( bcdNumber, "3.141592654" );
        int lastDigit = 0;

        cout << "Input BCD: " << marty::bcd::formatRawBcdNumber( bcdNumber, bcdNumberPrecision ) << endl;
        bcdNumberPrecision = marty::bcd::truncatePrecision( bcdNumber, bcdNumberPrecision, 3, &lastDigit );
        cout << "Trunc BCD: "<< marty::bcd::formatRawBcdNumber( bcdNumber, bcdNumberPrecision ) << ", last truncated digit: " << lastDigit << endl;
        cout << endl;

        marty::bcd::makeRawBcdNumber( bcdNumber, "3" ); bcdNumberPrecision = -3; // 3000
        cout << "Input BCD: " << marty::bcd::formatRawBcdNumber( bcdNumber, bcdNumberPrecision ) << endl;
        bcdNumberPrecision = marty::bcd::truncatePrecision( bcdNumber, bcdNumberPrecision, 3, &lastDigit );
        cout << "Trunc BCD: "<< marty::bcd::formatRawBcdNumber( bcdNumber, bcdNumberPrecision ) << ", last truncated digit: " << lastDigit << endl;
        cout << endl;

        bcdNumberPrecision = marty::bcd::makeRawBcdNumber( bcdNumber, "3.1" );
        cout << "Input BCD: " << marty::bcd::formatRawBcdNumber( bcdNumber, bcdNumberPrecision ) << endl;
        bcdNumberPrecision = marty::bcd::truncatePrecision( bcdNumber, bcdNumberPrecision, 3, &lastDigit );
        cout << "Trunc BCD: "<< marty::bcd::formatRawBcdNumber( bcdNumber, bcdNumberPrecision ) << ", last truncated digit: " << lastDigit << endl;
        cout << endl;

        bcdNumberPrecision = marty::bcd::makeRawBcdNumber( bcdNumber, "3.141" );
        cout << "Input BCD: " << marty::bcd::formatRawBcdNumber( bcdNumber, bcdNumberPrecision ) << endl;
        bcdNumberPrecision = marty::bcd::truncatePrecision( bcdNumber, bcdNumberPrecision, 3, &lastDigit );
        cout << "Trunc BCD: "<< marty::bcd::formatRawBcdNumber( bcdNumber, bcdNumberPrecision ) << ", last truncated digit: " << lastDigit << endl;
        cout << endl;


        cout << endl;
        cout << "------------------------------" << endl;
        cout << endl;
        
    }


    if (0)
    {
        cout << "Precision extention calls for debug" << endl;
        cout << endl;

        marty::bcd::raw_bcd_number_t bcdNumber;
        int bcdNumberPrecision = marty::bcd::makeRawBcdNumber( bcdNumber, "3.141592654" );
        int lastDigit = 0;

        cout << "Input BCD: " << marty::bcd::formatRawBcdNumber( bcdNumber, bcdNumberPrecision ) << endl;
        bcdNumberPrecision = marty::bcd::extendPrecision( bcdNumber, bcdNumberPrecision, 3 );
        cout << "Extnd BCD: "<< marty::bcd::formatRawBcdNumber( bcdNumber, bcdNumberPrecision ) << endl;
        cout << endl;

        marty::bcd::makeRawBcdNumber( bcdNumber, "3" ); bcdNumberPrecision = -3; // 3000
        cout << "Input BCD: " << marty::bcd::formatRawBcdNumber( bcdNumber, bcdNumberPrecision ) << endl;
        bcdNumberPrecision = marty::bcd::extendPrecision( bcdNumber, bcdNumberPrecision, 3 );
        cout << "Extnd BCD: "<< marty::bcd::formatRawBcdNumber( bcdNumber, bcdNumberPrecision ) << endl;
        cout << endl;

        bcdNumberPrecision = marty::bcd::makeRawBcdNumber( bcdNumber, "3.1" );
        cout << "Input BCD: " << marty::bcd::formatRawBcdNumber( bcdNumber, bcdNumberPrecision ) << endl;
        bcdNumberPrecision = marty::bcd::extendPrecision( bcdNumber, bcdNumberPrecision, 3 );
        cout << "Extnd BCD: "<< marty::bcd::formatRawBcdNumber( bcdNumber, bcdNumberPrecision ) << endl;
        cout << endl;

        bcdNumberPrecision = marty::bcd::makeRawBcdNumber( bcdNumber, "3.141" );
        cout << "Input BCD: " << marty::bcd::formatRawBcdNumber( bcdNumber, bcdNumberPrecision ) << endl;
        bcdNumberPrecision = marty::bcd::extendPrecision( bcdNumber, bcdNumberPrecision, 3 );
        cout << "Extnd BCD: "<< marty::bcd::formatRawBcdNumber( bcdNumber, bcdNumberPrecision ) << endl;
        cout << endl;


        cout << endl;
        cout << "------------------------------" << endl;
        cout << endl;
        
    }


    #define RAW_BCD_FROM_STRING_TEST( strVal, strResForCompare )                            \
                do                                                                          \
                {                                                                           \
                    ++totalRawBcdFromStringConvertTests;                                    \
                    marty::bcd::raw_bcd_number_t bcdNumber;                                 \
                    int                          bcdNumberPrecision;                        \
                                                                                            \
                    bcdNumberPrecision = marty::bcd::makeRawBcdNumber( bcdNumber, strVal ); \
                                                                                            \
                    std::string bcdStrFormatted = marty::bcd::formatRawBcdNumber( bcdNumber, bcdNumberPrecision );\
                                                                                            \
                    bool bGood = bcdStrFormatted==strResForCompare;                         \
                                                                                            \
                    if (!bGood)                                                             \
                       ++totalRawBcdFromStringConvertTestsFailed;                           \
                                                                                            \
                                                                                            \
                    cout << "[" << (bGood ? "+" : "-") << "]  " << strVal << " converted to BCD is '" << bcdStrFormatted << "'" << endl; \
                    if (!bGood)                                                             \
                    {                                                                       \
                        cout << "    expected '" << strResForCompare << "'" << endl;        \
                        cout << __FILE__ << ":" << __LINE__ << endl;                        \
                    }                                                                       \
                                                                                            \
                    if (!bGood)                                                             \
                        THROW_ERROR();                                                      \
                                                                                            \
                } while(0)


    #define RAW_BCD_COMPARE_TEST( strVal1, strVal2, cmpResForCompare )                      \
                do                                                                          \
                {                                                                           \
                    ++totalRawBcdCompareTests;                                              \
                    marty::bcd::raw_bcd_number_t bcdNumber1;                                \
                    int                          bcdNumberPrecision1;                       \
                    marty::bcd::raw_bcd_number_t bcdNumber2;                                \
                    int                          bcdNumberPrecision2;                       \
                                                                                            \
                    bcdNumberPrecision1 = marty::bcd::makeRawBcdNumber( bcdNumber1, strVal1 ); \
                    bcdNumberPrecision2 = marty::bcd::makeRawBcdNumber( bcdNumber2, strVal2 ); \
                                                                                            \
                    int cmpRes = marty::bcd::compareRaws( bcdNumber1, bcdNumberPrecision1, bcdNumber2, bcdNumberPrecision2 );\
                                                                                            \
                    bool bGood = cmpRes==cmpResForCompare;                                  \
                                                                                            \
                    if (!bGood)                                                             \
                       ++totalRawBcdCompareTestsFailed;                                     \
                                                                                            \
                                                                                            \
                    cout << "[" << (bGood ? "+" : "-") << "]  " << strVal1 << " compare with " << strVal2 << " is " << cmpRes << "" << endl; \
                    if (!bGood)                                                             \
                    {                                                                       \
                        cout << "    expected " << cmpResForCompare << "" << endl;          \
                        cout << __FILE__ << ":" << __LINE__ << endl;                        \
                    }                                                                       \
                                                                                            \
                    if (!bGood)                                                             \
                        THROW_ERROR();                                                      \
                                                                                            \
                } while(0)

    #define RAW_BCD_ZERO_TEST( strVal1, cmpResForCompare )                                  \
                do                                                                          \
                {                                                                           \
                    ++totalRawBcdZeroTests;                                                 \
                    marty::bcd::raw_bcd_number_t bcdNumber1;                                \
                    int                          bcdNumberPrecision1;                       \
                                                                                            \
                    bcdNumberPrecision1 = marty::bcd::makeRawBcdNumber( bcdNumber1, strVal1 ); \
                                                                                            \
                    bool zeroRes = marty::bcd::checkForZero( bcdNumber1 );                  \
                                                                                            \
                    bool bGood = zeroRes==cmpResForCompare;                                 \
                                                                                            \
                    if (!bGood)                                                             \
                       ++totalRawBcdZeroTestsFailed;                                        \
                                                                                            \
                                                                                            \
                    cout << "[" << (bGood ? "+" : "-") << "]  " << strVal1 << " is zero: " << (zeroRes?"true":"false") << "" << endl; \
                    if (!bGood)                                                             \
                    {                                                                       \
                        cout << "    expected " << cmpResForCompare << "" << endl;          \
                        cout << __FILE__ << ":" << __LINE__ << endl;                        \
                    }                                                                       \
                                                                                            \
                    if (!bGood)                                                             \
                        THROW_ERROR();                                                      \
                                                                                            \
                } while(0)


    #define RAW_BCD_OP_TEST( strVal1, strVal2, op, strResForCompare )                       \
                do                                                                          \
                {                                                                           \
                    ++totalRawBcdOpTests;                                                   \
                    marty::bcd::raw_bcd_number_t bcdNumber1;                                \
                    int                          bcdNumberPrecision1;                       \
                    marty::bcd::raw_bcd_number_t bcdNumber2;                                \
                    int                          bcdNumberPrecision2;                       \
                    marty::bcd::raw_bcd_number_t bcdNumberRes;                                 \
                    int                          bcdNumberPrecisionRes;                        \
                                                                                               \
                    bcdNumberPrecision1 = marty::bcd::makeRawBcdNumber( bcdNumber1, strVal1 ); \
                    bcdNumberPrecision2 = marty::bcd::makeRawBcdNumber( bcdNumber2, strVal2 ); \
                                                                                               \
                    bcdNumberPrecisionRes = marty::bcd:: op ( bcdNumberRes, bcdNumber1, bcdNumberPrecision1, bcdNumber2, bcdNumberPrecision2 );  \
                                                                                            \
                    std::string bcdStrFormatted = marty::bcd::formatRawBcdNumber( bcdNumberRes, bcdNumberPrecisionRes );\
                                                                                            \
                    bool bGood = bcdStrFormatted==strResForCompare;                         \
                                                                                            \
                    if (!bGood)                                                             \
                       ++totalRawBcdOpTestsFailed;                                          \
                                                                                            \
                                                                                            \
                    cout << "[" << (bGood ? "+" : "-") << "]  " << strVal1 << " " << #op << " " << strVal2 << " = " << bcdStrFormatted << "" << endl; \
                    if (!bGood)                                                             \
                    {                                                                       \
                        cout << "    expected " << strResForCompare << "" << endl;          \
                        cout << __FILE__ << ":" << __LINE__ << endl;                        \
                    }                                                                       \
                                                                                            \
                    if (!bGood)                                                             \
                        THROW_ERROR();                                                      \
                                                                                            \
                } while(0)


    unsigned totalRawBcdFromStringConvertTests        = 0;
    unsigned totalRawBcdFromStringConvertTestsFailed  = 0;

    cout << "RAW_BCD_FROM_STRING_TEST test suite" << endl;

    RAW_BCD_FROM_STRING_TEST( "0.003"          , "0.003"           );
    RAW_BCD_FROM_STRING_TEST( "300"            , "300"             );
    RAW_BCD_FROM_STRING_TEST( "0"              , "0"               );
    RAW_BCD_FROM_STRING_TEST( "3.141592654"    , "3.141592654"     );
    RAW_BCD_FROM_STRING_TEST( "314.15"         , "314.15"          );
    RAW_BCD_FROM_STRING_TEST( "31.4159"        , "31.4159"         );
    RAW_BCD_FROM_STRING_TEST( "317.291592654"  , "317.291592654"   );
    RAW_BCD_FROM_STRING_TEST( "311.008407346"  , "311.008407346"   );
    RAW_BCD_FROM_STRING_TEST( "34.557492654"   , "34.557492654"    );
    RAW_BCD_FROM_STRING_TEST( "28.274307346"   , "28.274307346"    );
    RAW_BCD_FROM_STRING_TEST( "303.141592654"  , "303.141592654"   );
    RAW_BCD_FROM_STRING_TEST( "296.858407346"  , "296.858407346"   );
    RAW_BCD_FROM_STRING_TEST( "3003.141592654" , "3003.141592654"  );
    RAW_BCD_FROM_STRING_TEST( "2996.858407346" , "2996.858407346"  );
    RAW_BCD_FROM_STRING_TEST( ".141"           , "0.141"           );
    RAW_BCD_FROM_STRING_TEST( ".141000"        , "0.141"           );
    RAW_BCD_FROM_STRING_TEST( "25.141"         , "25.141"          );
    RAW_BCD_FROM_STRING_TEST( "24.859"         , "24.859"          );
    //RAW_BCD_FROM_STRING_TEST();

    cout << endl;

    cout << "Failed " << totalRawBcdFromStringConvertTestsFailed << " BCD convert tests from total " << totalRawBcdFromStringConvertTests << endl;
    if (!totalRawBcdFromStringConvertTestsFailed)
        cout << "+++ All BCD convert tests passed"  << endl;

    cout << "------------------------------" << endl << endl << endl;



    unsigned totalRawBcdZeroTests       = 0;
    unsigned totalRawBcdZeroTestsFailed = 0;

    cout << "RAW_BCD_ZERO_TEST test suite" << endl;

    RAW_BCD_ZERO_TEST( "3.141592654"    , false );
    RAW_BCD_ZERO_TEST( "314.15"         , false );
    RAW_BCD_ZERO_TEST( "31.4159"        , false );
    RAW_BCD_ZERO_TEST( "317.291592654"  , false );
    RAW_BCD_ZERO_TEST( "311.008407346"  , false );
    RAW_BCD_ZERO_TEST( "34.557492654"   , false );
    RAW_BCD_ZERO_TEST( "28.274307346"   , false );
    RAW_BCD_ZERO_TEST( "303.141592654"  , false );
    RAW_BCD_ZERO_TEST( "296.858407346"  , false );
    RAW_BCD_ZERO_TEST( "3003.141592654" , false );
    RAW_BCD_ZERO_TEST( "2996.858407346" , false );
    RAW_BCD_ZERO_TEST( "0.003"          , false );
    RAW_BCD_ZERO_TEST( ".141"           , false );
    RAW_BCD_ZERO_TEST( ".141000"        , false );
    RAW_BCD_ZERO_TEST( "25.141"         , false );
    RAW_BCD_ZERO_TEST( "24.859"         , false );
    RAW_BCD_ZERO_TEST( "0"              , true  );
    RAW_BCD_ZERO_TEST( "00"             , true  );
    RAW_BCD_ZERO_TEST( "000"            , true  );
    RAW_BCD_ZERO_TEST( "00.0"           , true  );
    RAW_BCD_ZERO_TEST( "0.000"          , true  );
    RAW_BCD_ZERO_TEST( "00.00"          , true  );


    cout << endl;

    cout << "Failed " << totalRawBcdZeroTestsFailed << " BCD zero tests from total " << totalRawBcdZeroTests << endl;
    if (!totalRawBcdZeroTestsFailed)
        cout << "+++ All BCD zero tests passed"  << endl;

    cout << "------------------------------" << endl << endl << endl;



    unsigned totalRawBcdCompareTests       = 0;
    unsigned totalRawBcdCompareTestsFailed = 0;

    cout << "RAW_BCD_COMPARE_TEST test suite" << endl;

    RAW_BCD_COMPARE_TEST( "3.141592654" , "314.15"      , -1 );
    RAW_BCD_COMPARE_TEST( "314.15"      , "3.141592654" ,  1 );
    RAW_BCD_COMPARE_TEST( "3.141592654" , "3.141592654" ,  0 );
    RAW_BCD_COMPARE_TEST( "3.141592654" , "31.4159"     , -1 );
    RAW_BCD_COMPARE_TEST( "31.4159"     , "3.141592654" ,  1 );
    RAW_BCD_COMPARE_TEST( "31.4159"     , "31.4159"     ,  0 );
    RAW_BCD_COMPARE_TEST( "314.15"      , "31.4159"     ,  1 );
    RAW_BCD_COMPARE_TEST( "31.4159"     , "314.15"      , -1 );
    RAW_BCD_COMPARE_TEST( "314.15"      , "314.15"      ,  0 );
    RAW_BCD_COMPARE_TEST( "314.15"      , "300.0"       ,  1 );
    RAW_BCD_COMPARE_TEST( "300"         , "314.15"      , -1 );
    RAW_BCD_COMPARE_TEST( "300"         , "300.0"       ,  0 );
    RAW_BCD_COMPARE_TEST( "3000"        , "300"         ,  1 );
    RAW_BCD_COMPARE_TEST( "300"         , "3000"        , -1 );
    RAW_BCD_COMPARE_TEST( "3000"        , "3000"        ,  0 );
    RAW_BCD_COMPARE_TEST( "3000.00"     , "3000"        ,  0 );

    cout << endl;

    cout << "Failed " << totalRawBcdCompareTestsFailed << " BCD compare tests from total " << totalRawBcdCompareTests << endl;
    if (!totalRawBcdCompareTestsFailed)
        cout << "+++ All BCD compare tests passed"  << endl;

    cout << "------------------------------" << endl << endl << endl;



    unsigned totalRawBcdOpTests         = 0;
    unsigned totalRawBcdOpTestsFailed   = 0;

    cout << "RAW_BCD_OP_TEST test suite" << endl;

    RAW_BCD_OP_TEST( "3.141592654"   , "314.15"        , rawAddition       , "317.291592654"  ); // 3.141592654 + 314.15 = 317.291592654
    RAW_BCD_OP_TEST( "3.141592654"   , "31.4159"       , rawAddition       , "34.557492654"   ); // 3.141592654+31.4159 = 34.557492654
    RAW_BCD_OP_TEST( "3.141592654"   , "300"           , rawAddition       , "303.141592654"  ); // 3.141592654+300 = 303.141592654
    RAW_BCD_OP_TEST( "3.141592654"   , "3000"          , rawAddition       , "3003.141592654" ); // 3.141592654+3000 = 3003.141592654
    RAW_BCD_OP_TEST( "3.141592654"   , "0.003"         , rawAddition       , "3.144592654"    ); // 3.141592654+0.003 = 3.144592654
    RAW_BCD_OP_TEST( ".141"          , ".141000"       , rawAddition       , "0.282"          ); // .141+.141000 = 0.282
    RAW_BCD_OP_TEST( "25.000"        , ".141000"       , rawAddition       , "25.141"         ); // 25.000+.141000 = 25.141
    RAW_BCD_OP_TEST( "025.000"       , ".141000"       , rawAddition       , "25.141"         ); // 025.000+.141000 = 25.141
    //RAW_BCD_OP_TEST( ""      , "" , rawAddition   , "" ); // 

    cout << endl;

    RAW_BCD_OP_TEST( "314.15"        , "3.141592654"   , rawSubtraction    , "311.008407346"  ); // 314.15 - 3.141592654 = 311.008407346
    RAW_BCD_OP_TEST( "31.4159"       , "3.141592654"   , rawSubtraction    , "28.274307346"   ); // 31.4159-3.141592654 = 28.274307346
    RAW_BCD_OP_TEST( "300"           , "3.141592654"   , rawSubtraction    , "296.858407346"  ); // 300-3.141592654 = 296.858407346
    RAW_BCD_OP_TEST( "3000"          , "3.141592654"   , rawSubtraction    , "2996.858407346" ); // 3000-3.141592654 = 2996.858407346
    RAW_BCD_OP_TEST( "3.141592654"   , "0.003"         , rawSubtraction    , "3.138592654"    ); // 3.141592654-0.003 = 3.138592654
    RAW_BCD_OP_TEST( ".141"          , ".141000"       , rawSubtraction    , "0"              ); // .141-.141000 = 0
    RAW_BCD_OP_TEST( "25.000"        , ".141000"       , rawSubtraction    , "24.859"         ); // 25.000-.141000 = 24.859
    RAW_BCD_OP_TEST( "025.000"       , ".141000"       , rawSubtraction    , "24.859"         ); // 025.000-.141000 = 24.859
    //RAW_BCD_OP_TEST( ""      , "" , rawSubtraction, "" ); // 

    cout << endl;

    RAW_BCD_OP_TEST( "12345"         , "5439876"       , rawMultiplication , "67155269220"      ); // 
    RAW_BCD_OP_TEST( "123"           , "456"           , rawMultiplication , "56088"            ); // 

    RAW_BCD_OP_TEST( "12.345"        , "5439.876"      , rawMultiplication , "67155.26922"      ); // 
    RAW_BCD_OP_TEST( "12.3"          , "4.56"          , rawMultiplication , "56.088"           ); // 

    RAW_BCD_OP_TEST( "31.4159"       , "94500"         , rawMultiplication , "2968802.55"       ); // 31.4159*94500 = 2968802.55
    RAW_BCD_OP_TEST( "314.15"        , "3.141592654"   , rawMultiplication , "986.9313322541"   ); // 314.15 * 3.141592654 = 986.9313322541
    RAW_BCD_OP_TEST( "31.4159"       , "3.141592654"   , rawMultiplication , "98.6959606587986" ); // 31.4159*3.141592654 = 98.6959606587986
    RAW_BCD_OP_TEST( "300"           , "3.141592654"   , rawMultiplication , "942.4777962"      ); // 300*3.141592654 = 942.4777962
    RAW_BCD_OP_TEST( "3000"          , "3.141592654"   , rawMultiplication , "9424.777962"      ); // 3000*3.141592654 = 9424.777962
    RAW_BCD_OP_TEST( "3.141592654"   , "0.003"         , rawMultiplication , "0.009424777962"   ); // 3.141592654*0.003 = 0.009424777962
    RAW_BCD_OP_TEST( ".141"          , ".141000"       , rawMultiplication , "0.019881"         ); // .141*.141000 = 0.019881
    RAW_BCD_OP_TEST( "25.000"        , ".141000"       , rawMultiplication , "3.525"            ); // 25.000*.141000 = 3.525
    RAW_BCD_OP_TEST( "025.000"       , ".141000"       , rawMultiplication , "3.525"            ); // 025.000*.141000 = 3.525

    cout << endl;

    RAW_BCD_OP_TEST( "2"             , "10"            , rawDivision , "0.2"                    ); // 
    RAW_BCD_OP_TEST( "0.94500"       , "0.500"         , rawDivision , "1.89"                   ); // 0.94500/0.500 = 1.89
    RAW_BCD_OP_TEST( "9450.005"      , "0.005"         , rawDivision , "1890001"                ); // 9450.005/0.005 = 1890001
    RAW_BCD_OP_TEST( "94500"         , "500"           , rawDivision , "189"                    ); // 94500/500 = 189
    RAW_BCD_OP_TEST( "94500"         , "50"            , rawDivision , "1890"                   ); // 94500/50 = 1890
    RAW_BCD_OP_TEST( "94500"         , "5"             , rawDivision , "18900"                  ); // 94500/5 = 18900
    RAW_BCD_OP_TEST( "94500"         , "0.5"           , rawDivision , "189000"                 ); // 94500/0.5 = 189000
    RAW_BCD_OP_TEST( "94500"         , "0.05"          , rawDivision , "1890000"                ); // 94500/0.05 = 1890000
    RAW_BCD_OP_TEST( "94500"         , "0.005"         , rawDivision , "18900000"               ); // 94500/0.005 = 18900000
                                                                     // 3008.030965211883154708 28465840545710
    RAW_BCD_OP_TEST( "94500"         , "31.4159"       , rawDivision , "3008.030965211883154708" ); // 94500/31.4159 = 3008.0309652118 8315470828 4658405457 1092981579 3913273215 1553831021
                                                                     // 0.000000529152915291 005291005291005291005291005291005291005291
    RAW_BCD_OP_TEST( "500"           , "94500"         , rawDivision , "0.005291005291005291" ); // 500/94500 = 0.0052910052 9100529100 5291005291 0052910052 9100529100 5291005291

    RAW_BCD_OP_TEST( "31.4159"       , "94500"         , rawDivision ,    "0.000332443386243386"    ); // 31.4159/94500 = 0.0003324433 8624338624 3386243386 243386243386243386243386243386243386243386243386243386243386243386243386243386243386243386243386243386243386
    RAW_BCD_OP_TEST( "314.15"        , "3.141592654"   , rawDivision ,   "99.9970507315809378003"   ); // 314.15 / 3.141592654 = 99.9970507315 8093780034 6664548827 9143397818 75489450390088669974334616584572647781598740649461679063373567463148263396722342857884719258068395
    RAW_BCD_OP_TEST( "31.4159"       , "3.141592654"   , rawDivision ,    "9.999991552055621785"    ); // 31.4159/3.141592654 = 9.9999915520 5562178526 7899980262 6862139333 23005535650198907041370997552631786870736679536429804753420460474504279891914975174244852942032631
    RAW_BCD_OP_TEST( "300"           , "3.141592654"   , rawDivision ,   "95.492965842668411077"    ); // 300/3.141592654 = 95.4929658426 6841107784 1793298259 985045152196870396680014645845298058174030858935157161212282361034576063151184080913629485460338741930353393
    RAW_BCD_OP_TEST( "3000"          , "3.141592654"   , rawDivision ,  "954.929658426684110778"    ); // 3000/3.141592654 = 954.9296584266 8411077841 7932982599 850451521968703966800146458452980581740308589351571612122823610345760631511840809136294854603387419303533933
    RAW_BCD_OP_TEST( "3.141592654"   , "0.003"         , rawDivision , "1047.197551333333333333"    ); // 3.141592654/0.003 = 1047.1975513333 3333333333 3333333333 333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333
    RAW_BCD_OP_TEST( ".141"          , ".141000"       , rawDivision ,    "1"                       ); // .141/.141000 = 1
    RAW_BCD_OP_TEST( "25.000"        , ".141000"       , rawDivision ,  "177.304964539007092198"    ); // 25.000/.141000 = 177.3049645390 0709219858 1560283687 943262411347517730496453900709219858156028368794326241134751773049645390070921985815602836879432624113475177
    RAW_BCD_OP_TEST( "025.000"       , ".141000"       , rawDivision ,  "177.304964539007092198"    ); // 025.000/.141000 = -//-
    RAW_BCD_OP_TEST( "1020314.159", "0.0000002718281828459045235", rawDivision, "3753526026322.301472254944911079" ); // 1020314.159/0.0000002718281828459045235 = 3753526026322.3014722549 4491107928 005226011384158046314264105198045620005998298872351439449250954737806651337555563555122563988343495323608632
    

    cout << endl;

    cout << "Failed " << totalRawBcdOpTestsFailed << " BCD operation tests from total " << totalRawBcdOpTests << endl;
    if (!totalRawBcdOpTestsFailed)
        cout << "+++ All BCD operation tests passed"  << endl;

    cout << "------------------------------" << endl << endl << endl;


    unsigned totalCtorTests       = 0;
    unsigned totalCtorTestsFailed = 0;



    #define DECIMAL_CTOR_TEST( val, strResForCompare )                                      \
                do                                                                          \
                {                                                                           \
                    ++totalCtorTests;                                                       \
                    Decimal     decimal    = Decimal(val);                                  \
                    std::string strDecimal = decimal.toString( /* precision - auto */ );    \
                                                                                            \
                    bool bGood /* Johny */ = decimal.checkIsExact(strResForCompare);        \
                    if (!bGood)                                                             \
                       ++totalCtorTestsFailed;                                              \
                                                                                            \
                                                                                            \
                    cout << "[" << (bGood ? "+" : "-") << "]  " << (decimal<0 ? "" : " ") << quoteOnlyString(val) << " converted to Decimal is \"" << strDecimal << "\"" << endl; \
                    if (!bGood)                                                             \
                    {                                                                       \
                        cout << "    expected '" << strResForCompare << "'" << endl;        \
                        cout << __FILE__ << ":" << __LINE__ << endl;                        \
                    }                                                                       \
                                                                                            \
                    if (!bGood)                                                             \
                        THROW_ERROR();                                                      \
                                                                                            \
                } while(0)


    #define DECIMAL_CTOR_TEST_WITH_PREC( val, prec, strResForCompare )                      \
                do                                                                          \
                {                                                                           \
                    ++totalCtorTests;                                                       \
                    Decimal     decimal    = Decimal(val,  /* marty::DecimalPrecision */ prec);   \
                    std::string strDecimal = decimal.toString( /* precision - auto */ );    \
                                                                                            \
                    bool bGood /* Johny */ = decimal.checkIsExact(strResForCompare);        \
                    if (!bGood)                                                             \
                       ++totalCtorTestsFailed;                                              \
                                                                                            \
                                                                                            \
                    cout << "[" << (bGood ? "+" : "-") << "]  " << (decimal<0 ? "" : " ") << val << " converted to Decimal with precision " << prec << " is \"" << strDecimal << "\"" << endl; \
                    if (!bGood)                                                             \
                    {                                                                       \
                        cout << "    expected '" << strResForCompare << "'" << endl;        \
                        cout << __FILE__ << ":" << __LINE__ << endl;                        \
                    }                                                                       \
                                                                                            \
                    if (!bGood)                                                             \
                        THROW_ERROR();                                                      \
                                                                                            \
                } while(0)


    cout << "double precision: " << std::numeric_limits<double>::digits10 << endl;
    cout << "float precision : " << std::numeric_limits<float> ::digits10 << endl;
    cout << endl;

    cout << "DECIMAL_CTOR_TEST test suite" << endl;

    DECIMAL_CTOR_TEST( (std::uint64_t)3    ,       "3" );
    DECIMAL_CTOR_TEST( (std::int64_t )7    ,       "7" );
    DECIMAL_CTOR_TEST( (std::int64_t )-10  ,     "-10" );
    DECIMAL_CTOR_TEST( "123.01"            ,     "123.01" );
    DECIMAL_CTOR_TEST( "13.101"            ,     "13.101" );
    DECIMAL_CTOR_TEST( "3.1415"            ,     "3.1415" );
    DECIMAL_CTOR_TEST( 3.1415              ,     "3.1415" );
    DECIMAL_CTOR_TEST( -3.1415             ,    "-3.1415" );
    DECIMAL_CTOR_TEST( -100.10101          ,  "-100.10101" );
    //DECIMAL_CTOR_TEST(  3.1415926535897932  ,  "3.1415926535897932" );
    // DECIMAL_CTOR_TEST(  3.1415926          ,  "3.141593" ); // Для int реализации, там округление double 6 знаков
    DECIMAL_CTOR_TEST(  3.1415926          ,  "3.1415926" ); // Для BCD реализации, там округление double 9 знаков
    // DECIMAL_CTOR_TEST();

    //Decimal decimal01    = Decimal(((std::uint64_t)3141592654ULL), marty::DecimalPrecision(9));

    // 3.141592654
    //                                             .
    DECIMAL_CTOR_TEST_WITH_PREC(  ((std::uint64_t)3141592654ULL), 9, "3.141592654" );


    cout << endl;

    cout << "Failed " << totalCtorTestsFailed << " CTOR tests from total " << totalCtorTests << endl;
    if (!totalCtorTestsFailed)
        cout << "+++ All CTOR tests passed"  << endl;

    cout << "------------------------------" << endl << endl << endl;




    #define DECIMAL_OP_TEST( val1, op, val2, strResForCompare )                             \
                    decimalOpTest( totalOpTests, totalOpTestsFailed                         \
                                 , val1, val2, strResForCompare                             \
                                 , #op                                                      \
                                 , __FILE__, __LINE__                                       \
                                 )


    unsigned totalOpTests       = 0;
    unsigned totalOpTestsFailed = 0;

    cout << "DECIMAL_OP_TEST test suite" << endl;

/*
    +2632041719345252483380694049835872197434.9766493
    +6156169830305019652590488156540831.7027942076808
    +781466404305113780602.80990487935255093934990570
    +277145.35491132254485207055096587993977783302753
    +1184055350.1862902754587418620655046842893720190
    +5235415554108378043687227.9880018482293335743662
    +18458345086503572578057123591092037706279741.151
    +174233558770663615740569957179488308019576.48254
    +8.3645424522417012204289292807950462439988531619
    +56380.329107833270343558230233235487388849158556
    +597147583001091464692.45462985860111824175838980
    +1099062944468939421856224.7410255529071444836214

    -18458345086503572578057123591092037706279741.151
    -174233558770663615740569957179488308019576.48254
    -8.3645424522417012204289292807950462439988531619
    -56380.329107833270343558230233235487388849158556
    -597147583001091464692.45462985860111824175838980
    -1099062944468939421856224.7410255529071444836214
    -2632041719345252483380694049835872197434.9766493
    -6156169830305019652590488156540831.7027942076808
    -781466404305113780602.80990487935255093934990570
    -277145.35491132254485207055096587993977783302753
    -1184055350.1862902754587418620655046842893720190
    -5235415554108378043687227.9880018482293335743662

    +2632041719345252483380694049835872197434.9766493    -18458345086503572578057123591092037706279741.151
    +6156169830305019652590488156540831.7027942076808    -174233558770663615740569957179488308019576.48254
    +781466404305113780602.80990487935255093934990570    -8.3645424522417012204289292807950462439988531619
    +277145.35491132254485207055096587993977783302753    -56380.329107833270343558230233235487388849158556
    +1184055350.1862902754587418620655046842893720190    -597147583001091464692.45462985860111824175838980
    +5235415554108378043687227.9880018482293335743662    -1099062944468939421856224.7410255529071444836214
    +18458345086503572578057123591092037706279741.151    -2632041719345252483380694049835872197434.9766493
    +174233558770663615740569957179488308019576.48254    -6156169830305019652590488156540831.7027942076808
    +8.3645424522417012204289292807950462439988531619    -781466404305113780602.80990487935255093934990570
    +56380.329107833270343558230233235487388849158556    -277145.35491132254485207055096587993977783302753
    +597147583001091464692.45462985860111824175838980    -1184055350.1862902754587418620655046842893720190
    +1099062944468939421856224.7410255529071444836214    -5235415554108378043687227.9880018482293335743662

    -18458345086503572578057123591092037706279741.151    +2632041719345252483380694049835872197434.9766493
    -174233558770663615740569957179488308019576.48254    +6156169830305019652590488156540831.7027942076808
    -8.3645424522417012204289292807950462439988531619    +781466404305113780602.80990487935255093934990570
    -56380.329107833270343558230233235487388849158556    +277145.35491132254485207055096587993977783302753
    -597147583001091464692.45462985860111824175838980    +1184055350.1862902754587418620655046842893720190
    -1099062944468939421856224.7410255529071444836214    +5235415554108378043687227.9880018482293335743662
    -2632041719345252483380694049835872197434.9766493    +18458345086503572578057123591092037706279741.151
    -6156169830305019652590488156540831.7027942076808    +174233558770663615740569957179488308019576.48254
    -781466404305113780602.80990487935255093934990570    +8.3645424522417012204289292807950462439988531619
    -277145.35491132254485207055096587993977783302753    +56380.329107833270343558230233235487388849158556
    -1184055350.1862902754587418620655046842893720190    +597147583001091464692.45462985860111824175838980
    -5235415554108378043687227.9880018482293335743662    +1099062944468939421856224.7410255529071444836214

    -2632041719345252483380694049835872197434.9766493    +18458345086503572578057123591092037706279741.151
    -6156169830305019652590488156540831.7027942076808    +174233558770663615740569957179488308019576.48254
    -781466404305113780602.80990487935255093934990570    +8.3645424522417012204289292807950462439988531619
    -277145.35491132254485207055096587993977783302753    +56380.329107833270343558230233235487388849158556
    -1184055350.1862902754587418620655046842893720190    +597147583001091464692.45462985860111824175838980
    -5235415554108378043687227.9880018482293335743662    +1099062944468939421856224.7410255529071444836214
    -18458345086503572578057123591092037706279741.151    +2632041719345252483380694049835872197434.9766493
    -174233558770663615740569957179488308019576.48254    +6156169830305019652590488156540831.7027942076808
    -8.3645424522417012204289292807950462439988531619    +781466404305113780602.80990487935255093934990570
    -56380.329107833270343558230233235487388849158556    +277145.35491132254485207055096587993977783302753
    -597147583001091464692.45462985860111824175838980    +1184055350.1862902754587418620655046842893720190
    -1099062944468939421856224.7410255529071444836214    +5235415554108378043687227.9880018482293335743662

    +18458345086503572578057123591092037706279741.151    -2632041719345252483380694049835872197434.9766493
    +174233558770663615740569957179488308019576.48254    -6156169830305019652590488156540831.7027942076808
    +8.3645424522417012204289292807950462439988531619    -781466404305113780602.80990487935255093934990570
    +56380.329107833270343558230233235487388849158556    -277145.35491132254485207055096587993977783302753
    +597147583001091464692.45462985860111824175838980    -1184055350.1862902754587418620655046842893720190
    +1099062944468939421856224.7410255529071444836214    -5235415554108378043687227.9880018482293335743662
    +2632041719345252483380694049835872197434.9766493    -18458345086503572578057123591092037706279741.151
    +6156169830305019652590488156540831.7027942076808    -174233558770663615740569957179488308019576.48254
    +781466404305113780602.80990487935255093934990570    -8.3645424522417012204289292807950462439988531619
    +277145.35491132254485207055096587993977783302753    -56380.329107833270343558230233235487388849158556
    +1184055350.1862902754587418620655046842893720190    -597147583001091464692.45462985860111824175838980
    +5235415554108378043687227.9880018482293335743662    -1099062944468939421856224.7410255529071444836214

*/

    // Default precision - 18+ digits after decimal dot


    cout << "-------" << endl;
    DECIMAL_OP_TEST(  "10", + ,  "-2",  "8" );
    DECIMAL_OP_TEST(   "2", + ,  "-2",  "0" );
    DECIMAL_OP_TEST(   "2", + , "-10", "-8" );
    DECIMAL_OP_TEST(  "10", + ,   "2", "12" );
    DECIMAL_OP_TEST(   "2", + ,   "2",  "4" );
    DECIMAL_OP_TEST(   "2", + ,  "10", "12" );
    DECIMAL_OP_TEST( "-10", + ,   "2", "-8" );
    DECIMAL_OP_TEST(  "-2", + ,   "2",  "0" );
    DECIMAL_OP_TEST(  "-2", + ,  "10",  "8" );
    DECIMAL_OP_TEST(  "10", + ,   "2", "12" );
    DECIMAL_OP_TEST(   "2", + ,   "2",  "4" );
    DECIMAL_OP_TEST(   "2", + ,  "10", "12" );
    cout << "---" << endl;
    DECIMAL_OP_TEST( "+2632041719345252483380694049835872197434.9766493", + , "-18458345086503572578057123591092037706279741.151", "-18455713044784227325573742897042201834082306.1743507"       ); // +2632041719345252483380694049835872197434.9766493 = -18458345086503572578057123591092037706279741.151    = -18455713044784227325573742897042201834082306.1743507
    DECIMAL_OP_TEST( "+6156169830305019652590488156540831.7027942076808", + , "-174233558770663615740569957179488308019576.48254", "-174233552614493785435550304589000151478744.7797457923192"   ); // +6156169830305019652590488156540831.7027942076808 +  -174233558770663615740569957179488308019576.48254   = -174233552614493785435550304589000151478744.7797457923192
    DECIMAL_OP_TEST( "+781466404305113780602.80990487935255093934990570", + , "-8.3645424522417012204289292807950462439988531619", "781466404305113780594.4453624271108497189209764192049537560011468381" ); // +781466404305113780602.80990487935255093934990570 +  -8.3645424522417012204289292807950462439988531619 = 781466404305113780594.4453624271108497189209764192049537560011468381
    DECIMAL_OP_TEST( "+277145.35491132254485207055096587993977783302753", + , "-56380.329107833270343558230233235487388849158556", "220765.025803489274508512320732644452388983868974"           ); // +277145.35491132254485207055096587993977783302753 +  -56380.329107833270343558230233235487388849158556   = 220765.025803489274508512320732644452388983868974
    DECIMAL_OP_TEST( "+1184055350.1862902754587418620655046842893720190", + , "-597147583001091464692.45462985860111824175838980", "-597147582999907409342.268339583142376379692885115710627981" ); // +1184055350.1862902754587418620655046842893720190 +  -597147583001091464692.45462985860111824175838980   = -597147582999907409342.268339583142376379692885115710627981
    DECIMAL_OP_TEST( "+5235415554108378043687227.9880018482293335743662", + , "-1099062944468939421856224.7410255529071444836214", "4136352609639438621831003.2469762953221890907448"            ); // +5235415554108378043687227.9880018482293335743662 +  -1099062944468939421856224.7410255529071444836214   = 4136352609639438621831003.2469762953221890907448
    DECIMAL_OP_TEST( "+18458345086503572578057123591092037706279741.151", + , "-2632041719345252483380694049835872197434.9766493", "18455713044784227325573742897042201834082306.1743507"        ); // +18458345086503572578057123591092037706279741.151 +  -2632041719345252483380694049835872197434.9766493   = 18455713044784227325573742897042201834082306.1743507
    DECIMAL_OP_TEST( "+174233558770663615740569957179488308019576.48254", + , "-6156169830305019652590488156540831.7027942076808", "174233552614493785435550304589000151478744.7797457923192"    ); // +174233558770663615740569957179488308019576.48254 +  -6156169830305019652590488156540831.7027942076808   = 174233552614493785435550304589000151478744.7797457923192
    DECIMAL_OP_TEST( "+8.3645424522417012204289292807950462439988531619", + , "-781466404305113780602.80990487935255093934990570", "-781466404305113780594.4453624271108497189209764192049537560011468381" ); // +8.3645424522417012204289292807950462439988531619 +  -781466404305113780602.80990487935255093934990570 = -781466404305113780594.4453624271108497189209764192049537560011468381
    DECIMAL_OP_TEST( "+56380.329107833270343558230233235487388849158556", + , "-277145.35491132254485207055096587993977783302753", "-220765.025803489274508512320732644452388983868974"          ); // +56380.329107833270343558230233235487388849158556 +  -277145.35491132254485207055096587993977783302753 = -220765.025803489274508512320732644452388983868974
    DECIMAL_OP_TEST( "+597147583001091464692.45462985860111824175838980", + , "-1184055350.1862902754587418620655046842893720190", "597147582999907409342.268339583142376379692885115710627981"  ); // +597147583001091464692.45462985860111824175838980 +  -1184055350.1862902754587418620655046842893720190 = 597147582999907409342.268339583142376379692885115710627981
    DECIMAL_OP_TEST( "+1099062944468939421856224.7410255529071444836214", + , "-5235415554108378043687227.9880018482293335743662", "-4136352609639438621831003.2469762953221890907448"           ); // +1099062944468939421856224.7410255529071444836214 +  -5235415554108378043687227.9880018482293335743662 = -4136352609639438621831003.2469762953221890907448
    cout << "---" << endl;
    DECIMAL_OP_TEST( "-18458345086503572578057123591092037706279741.151", + , "+2632041719345252483380694049835872197434.9766493", "-18455713044784227325573742897042201834082306.1743507"       ); // -18458345086503572578057123591092037706279741.151 +  +2632041719345252483380694049835872197434.9766493 = -18455713044784227325573742897042201834082306.1743507
    DECIMAL_OP_TEST( "-174233558770663615740569957179488308019576.48254", + , "+6156169830305019652590488156540831.7027942076808", "-174233552614493785435550304589000151478744.7797457923192"   ); // -174233558770663615740569957179488308019576.48254 +  +6156169830305019652590488156540831.7027942076808 = -174233552614493785435550304589000151478744.7797457923192
    DECIMAL_OP_TEST( "-8.3645424522417012204289292807950462439988531619", + , "+781466404305113780602.80990487935255093934990570", "781466404305113780594.4453624271108497189209764192049537560011468381" ); // -8.3645424522417012204289292807950462439988531619 +  +781466404305113780602.80990487935255093934990570 = 781466404305113780594.4453624271108497189209764192049537560011468381
    DECIMAL_OP_TEST( "-56380.329107833270343558230233235487388849158556", + , "+277145.35491132254485207055096587993977783302753", "220765.025803489274508512320732644452388983868974"           ); // -56380.329107833270343558230233235487388849158556 +  +277145.35491132254485207055096587993977783302753 = 220765.025803489274508512320732644452388983868974
    DECIMAL_OP_TEST( "-597147583001091464692.45462985860111824175838980", + , "+1184055350.1862902754587418620655046842893720190", "-597147582999907409342.268339583142376379692885115710627981" ); // -597147583001091464692.45462985860111824175838980 +  +1184055350.1862902754587418620655046842893720190 = -597147582999907409342.268339583142376379692885115710627981
    DECIMAL_OP_TEST( "-1099062944468939421856224.7410255529071444836214", + , "+5235415554108378043687227.9880018482293335743662", "4136352609639438621831003.2469762953221890907448"            ); // -1099062944468939421856224.7410255529071444836214 +  +5235415554108378043687227.9880018482293335743662 = 4136352609639438621831003.2469762953221890907448
    DECIMAL_OP_TEST( "-2632041719345252483380694049835872197434.9766493", + , "+18458345086503572578057123591092037706279741.151", "18455713044784227325573742897042201834082306.1743507"        ); // -2632041719345252483380694049835872197434.9766493 +  +18458345086503572578057123591092037706279741.151 = 18455713044784227325573742897042201834082306.1743507
    DECIMAL_OP_TEST( "-6156169830305019652590488156540831.7027942076808", + , "+174233558770663615740569957179488308019576.48254", "174233552614493785435550304589000151478744.7797457923192"    ); // -6156169830305019652590488156540831.7027942076808 +  +174233558770663615740569957179488308019576.48254 = 174233552614493785435550304589000151478744.7797457923192
    DECIMAL_OP_TEST( "-781466404305113780602.80990487935255093934990570", + , "+8.3645424522417012204289292807950462439988531619", "-781466404305113780594.4453624271108497189209764192049537560011468381" ); // -781466404305113780602.80990487935255093934990570 +  +8.3645424522417012204289292807950462439988531619 = -781466404305113780594.4453624271108497189209764192049537560011468381
    DECIMAL_OP_TEST( "-277145.35491132254485207055096587993977783302753", + , "+56380.329107833270343558230233235487388849158556", "-220765.025803489274508512320732644452388983868974"          ); // -277145.35491132254485207055096587993977783302753 +  +56380.329107833270343558230233235487388849158556 = -220765.025803489274508512320732644452388983868974
    DECIMAL_OP_TEST( "-1184055350.1862902754587418620655046842893720190", + , "+597147583001091464692.45462985860111824175838980", "597147582999907409342.268339583142376379692885115710627981"  ); // -1184055350.1862902754587418620655046842893720190 +  +597147583001091464692.45462985860111824175838980 = 597147582999907409342.268339583142376379692885115710627981
    DECIMAL_OP_TEST( "-5235415554108378043687227.9880018482293335743662", + , "+1099062944468939421856224.7410255529071444836214", "-4136352609639438621831003.2469762953221890907448"           ); // -5235415554108378043687227.9880018482293335743662 +  +1099062944468939421856224.7410255529071444836214 = -4136352609639438621831003.2469762953221890907448
    cout << "---" << endl;
    DECIMAL_OP_TEST( "-2632041719345252483380694049835872197434.9766493", + , "+18458345086503572578057123591092037706279741.151", "18455713044784227325573742897042201834082306.1743507"        ); // -2632041719345252483380694049835872197434.9766493 +  +18458345086503572578057123591092037706279741.151 = 18455713044784227325573742897042201834082306.1743507
    DECIMAL_OP_TEST( "-6156169830305019652590488156540831.7027942076808", + , "+174233558770663615740569957179488308019576.48254", "174233552614493785435550304589000151478744.7797457923192"    ); // -6156169830305019652590488156540831.7027942076808 +  +174233558770663615740569957179488308019576.48254 = 174233552614493785435550304589000151478744.7797457923192
    DECIMAL_OP_TEST( "-781466404305113780602.80990487935255093934990570", + , "+8.3645424522417012204289292807950462439988531619", "-781466404305113780594.4453624271108497189209764192049537560011468381" ); // -781466404305113780602.80990487935255093934990570 +  +8.3645424522417012204289292807950462439988531619 = -781466404305113780594.4453624271108497189209764192049537560011468381
    DECIMAL_OP_TEST( "-277145.35491132254485207055096587993977783302753", + , "+56380.329107833270343558230233235487388849158556", "-220765.025803489274508512320732644452388983868974"          ); // -277145.35491132254485207055096587993977783302753 +  +56380.329107833270343558230233235487388849158556 = -220765.025803489274508512320732644452388983868974
    DECIMAL_OP_TEST( "-1184055350.1862902754587418620655046842893720190", + , "+597147583001091464692.45462985860111824175838980", "597147582999907409342.268339583142376379692885115710627981"  ); // -1184055350.1862902754587418620655046842893720190 +  +597147583001091464692.45462985860111824175838980 = 597147582999907409342.268339583142376379692885115710627981
    DECIMAL_OP_TEST( "-5235415554108378043687227.9880018482293335743662", + , "+1099062944468939421856224.7410255529071444836214", "-4136352609639438621831003.2469762953221890907448"           ); // -5235415554108378043687227.9880018482293335743662 +  +1099062944468939421856224.7410255529071444836214 = -4136352609639438621831003.2469762953221890907448
    DECIMAL_OP_TEST( "-18458345086503572578057123591092037706279741.151", + , "+2632041719345252483380694049835872197434.9766493", "-18455713044784227325573742897042201834082306.1743507"       ); // -18458345086503572578057123591092037706279741.151 +  +2632041719345252483380694049835872197434.9766493 = -18455713044784227325573742897042201834082306.1743507
    DECIMAL_OP_TEST( "-174233558770663615740569957179488308019576.48254", + , "+6156169830305019652590488156540831.7027942076808", "-174233552614493785435550304589000151478744.7797457923192"   ); // -174233558770663615740569957179488308019576.48254 +  +6156169830305019652590488156540831.7027942076808 = -174233552614493785435550304589000151478744.7797457923192
    DECIMAL_OP_TEST( "-8.3645424522417012204289292807950462439988531619", + , "+781466404305113780602.80990487935255093934990570", "781466404305113780594.4453624271108497189209764192049537560011468381" ); // -8.3645424522417012204289292807950462439988531619 +  +781466404305113780602.80990487935255093934990570 = 781466404305113780594.4453624271108497189209764192049537560011468381
    DECIMAL_OP_TEST( "-56380.329107833270343558230233235487388849158556", + , "+277145.35491132254485207055096587993977783302753", "220765.025803489274508512320732644452388983868974"           ); // -56380.329107833270343558230233235487388849158556 +  +277145.35491132254485207055096587993977783302753 = 220765.025803489274508512320732644452388983868974
    DECIMAL_OP_TEST( "-597147583001091464692.45462985860111824175838980", + , "+1184055350.1862902754587418620655046842893720190", "-597147582999907409342.268339583142376379692885115710627981" ); // -597147583001091464692.45462985860111824175838980 +  +1184055350.1862902754587418620655046842893720190 = -597147582999907409342.268339583142376379692885115710627981
    DECIMAL_OP_TEST( "-1099062944468939421856224.7410255529071444836214", + , "+5235415554108378043687227.9880018482293335743662", "4136352609639438621831003.2469762953221890907448"            ); // -1099062944468939421856224.7410255529071444836214 +  +5235415554108378043687227.9880018482293335743662 = 4136352609639438621831003.2469762953221890907448
    cout << "---" << endl;
    DECIMAL_OP_TEST( "+18458345086503572578057123591092037706279741.151", + , "-2632041719345252483380694049835872197434.9766493", "18455713044784227325573742897042201834082306.1743507"        ); // +18458345086503572578057123591092037706279741.151 +  -2632041719345252483380694049835872197434.9766493 = 18455713044784227325573742897042201834082306.1743507
    DECIMAL_OP_TEST( "+174233558770663615740569957179488308019576.48254", + , "-6156169830305019652590488156540831.7027942076808", "174233552614493785435550304589000151478744.7797457923192"    ); // +174233558770663615740569957179488308019576.48254 +  -6156169830305019652590488156540831.7027942076808 = 174233552614493785435550304589000151478744.7797457923192
    DECIMAL_OP_TEST( "+8.3645424522417012204289292807950462439988531619", + , "-781466404305113780602.80990487935255093934990570", "-781466404305113780594.4453624271108497189209764192049537560011468381" ); // +8.3645424522417012204289292807950462439988531619 +  -781466404305113780602.80990487935255093934990570 = -781466404305113780594.4453624271108497189209764192049537560011468381
    DECIMAL_OP_TEST( "+56380.329107833270343558230233235487388849158556", + , "-277145.35491132254485207055096587993977783302753", "-220765.025803489274508512320732644452388983868974"          ); // +56380.329107833270343558230233235487388849158556 +  -277145.35491132254485207055096587993977783302753 = -220765.025803489274508512320732644452388983868974
    DECIMAL_OP_TEST( "+597147583001091464692.45462985860111824175838980", + , "-1184055350.1862902754587418620655046842893720190", "597147582999907409342.268339583142376379692885115710627981"  ); // +597147583001091464692.45462985860111824175838980 +  -1184055350.1862902754587418620655046842893720190 = 597147582999907409342.268339583142376379692885115710627981
    DECIMAL_OP_TEST( "+1099062944468939421856224.7410255529071444836214", + , "-5235415554108378043687227.9880018482293335743662", "-4136352609639438621831003.2469762953221890907448"           ); // +1099062944468939421856224.7410255529071444836214 +  -5235415554108378043687227.9880018482293335743662 = -4136352609639438621831003.2469762953221890907448
    DECIMAL_OP_TEST( "+2632041719345252483380694049835872197434.9766493", + , "-18458345086503572578057123591092037706279741.151", "-18455713044784227325573742897042201834082306.1743507"       ); // +2632041719345252483380694049835872197434.9766493 +  -18458345086503572578057123591092037706279741.151 = -18455713044784227325573742897042201834082306.1743507
    DECIMAL_OP_TEST( "+6156169830305019652590488156540831.7027942076808", + , "-174233558770663615740569957179488308019576.48254", "-174233552614493785435550304589000151478744.7797457923192"   ); // +6156169830305019652590488156540831.7027942076808 +  -174233558770663615740569957179488308019576.48254 = -174233552614493785435550304589000151478744.7797457923192
    DECIMAL_OP_TEST( "+781466404305113780602.80990487935255093934990570", + , "-8.3645424522417012204289292807950462439988531619", "781466404305113780594.4453624271108497189209764192049537560011468381" ); // +781466404305113780602.80990487935255093934990570 +  -8.3645424522417012204289292807950462439988531619 = 781466404305113780594.4453624271108497189209764192049537560011468381
    DECIMAL_OP_TEST( "+277145.35491132254485207055096587993977783302753", + , "-56380.329107833270343558230233235487388849158556", "220765.025803489274508512320732644452388983868974"           ); // +277145.35491132254485207055096587993977783302753 +  -56380.329107833270343558230233235487388849158556 = 220765.025803489274508512320732644452388983868974
    DECIMAL_OP_TEST( "+1184055350.1862902754587418620655046842893720190", + , "-597147583001091464692.45462985860111824175838980", "-597147582999907409342.268339583142376379692885115710627981" ); // +1184055350.1862902754587418620655046842893720190 +  -597147583001091464692.45462985860111824175838980 = -597147582999907409342.268339583142376379692885115710627981
    DECIMAL_OP_TEST( "+5235415554108378043687227.9880018482293335743662", + , "-1099062944468939421856224.7410255529071444836214", "4136352609639438621831003.2469762953221890907448"            ); // +5235415554108378043687227.9880018482293335743662 +  -1099062944468939421856224.7410255529071444836214 = 4136352609639438621831003.2469762953221890907448

    //DECIMAL_OP_TEST( "", + , "", "" ); // 

    cout << endl;


    cout << "-------" << endl;
    DECIMAL_OP_TEST(  "10", - ,  "-2",  "12" );
    DECIMAL_OP_TEST(   "2", - ,  "-2",   "4" );
    DECIMAL_OP_TEST(   "2", - , "-10",  "12" );
    DECIMAL_OP_TEST(  "10", - ,   "2",   "8" );
    DECIMAL_OP_TEST(   "2", - ,   "2",   "0" );
    DECIMAL_OP_TEST(   "2", - ,  "10",  "-8" );
    DECIMAL_OP_TEST( "-10", - ,   "2", "-12" );
    DECIMAL_OP_TEST(  "-2", - ,   "2",  "-4" );
    DECIMAL_OP_TEST(  "-2", - ,  "10", "-12" );
    DECIMAL_OP_TEST(  "10", - ,   "2",   "8" );
    DECIMAL_OP_TEST(   "2", - ,   "2",   "0" );
    DECIMAL_OP_TEST(   "2", - ,  "10",  "-8" );
    cout << "---" << endl;
    DECIMAL_OP_TEST( "+2632041719345252483380694049835872197434.9766493", - , "-18458345086503572578057123591092037706279741.151", "18460977128222917830540504285141873578477176.1276493"        ); // +2632041719345252483380694049835872197434.9766493 -  -18458345086503572578057123591092037706279741.151 = 18460977128222917830540504285141873578477176.1276493
    DECIMAL_OP_TEST( "+6156169830305019652590488156540831.7027942076808", - , "-174233558770663615740569957179488308019576.48254", "174233564926833446045589609769976464560408.1853342076808"    ); // +6156169830305019652590488156540831.7027942076808 -  -174233558770663615740569957179488308019576.48254 = 174233564926833446045589609769976464560408.1853342076808
    DECIMAL_OP_TEST( "+781466404305113780602.80990487935255093934990570", - , "-8.3645424522417012204289292807950462439988531619", "781466404305113780611.1744473315942521597788349807950462439988531619" ); // +781466404305113780602.80990487935255093934990570 -  -8.3645424522417012204289292807950462439988531619 = 781466404305113780611.1744473315942521597788349807950462439988531619
    DECIMAL_OP_TEST( "+277145.35491132254485207055096587993977783302753", - , "-56380.329107833270343558230233235487388849158556", "333525.684019155815195628781199115427166682186086"           ); // +277145.35491132254485207055096587993977783302753 -  -56380.329107833270343558230233235487388849158556 = 333525.684019155815195628781199115427166682186086
    DECIMAL_OP_TEST( "+1184055350.1862902754587418620655046842893720190", - , "-597147583001091464692.45462985860111824175838980", "597147583002275520042.640920134059860103823894484289372019"  ); // +1184055350.1862902754587418620655046842893720190 -  -597147583001091464692.45462985860111824175838980 = 597147583002275520042.640920134059860103823894484289372019
    DECIMAL_OP_TEST( "+5235415554108378043687227.9880018482293335743662", - , "-1099062944468939421856224.7410255529071444836214", "6334478498577317465543452.7290274011364780579876"            ); // +5235415554108378043687227.9880018482293335743662 -  -1099062944468939421856224.7410255529071444836214 = 6334478498577317465543452.7290274011364780579876
    DECIMAL_OP_TEST( "+18458345086503572578057123591092037706279741.151", - , "-2632041719345252483380694049835872197434.9766493", "18460977128222917830540504285141873578477176.1276493"        ); // +18458345086503572578057123591092037706279741.151 -  -2632041719345252483380694049835872197434.9766493 = 18460977128222917830540504285141873578477176.1276493
    DECIMAL_OP_TEST( "+174233558770663615740569957179488308019576.48254", - , "-6156169830305019652590488156540831.7027942076808", "174233564926833446045589609769976464560408.1853342076808"    ); // +174233558770663615740569957179488308019576.48254 -  -6156169830305019652590488156540831.7027942076808 = 174233564926833446045589609769976464560408.1853342076808
    DECIMAL_OP_TEST( "+8.3645424522417012204289292807950462439988531619", - , "-781466404305113780602.80990487935255093934990570", "781466404305113780611.1744473315942521597788349807950462439988531619" ); // +8.3645424522417012204289292807950462439988531619 -  -781466404305113780602.80990487935255093934990570 = 781466404305113780611.1744473315942521597788349807950462439988531619
    DECIMAL_OP_TEST( "+56380.329107833270343558230233235487388849158556", - , "-277145.35491132254485207055096587993977783302753", "333525.684019155815195628781199115427166682186086"           ); // +56380.329107833270343558230233235487388849158556 -  -277145.35491132254485207055096587993977783302753 = 333525.684019155815195628781199115427166682186086
    DECIMAL_OP_TEST( "+597147583001091464692.45462985860111824175838980", - , "-1184055350.1862902754587418620655046842893720190", "597147583002275520042.640920134059860103823894484289372019"  ); // +597147583001091464692.45462985860111824175838980 -  -1184055350.1862902754587418620655046842893720190 = 597147583002275520042.640920134059860103823894484289372019
    DECIMAL_OP_TEST( "+1099062944468939421856224.7410255529071444836214", - , "-5235415554108378043687227.9880018482293335743662", "6334478498577317465543452.7290274011364780579876"            ); // +1099062944468939421856224.7410255529071444836214 -  -5235415554108378043687227.9880018482293335743662 = 6334478498577317465543452.7290274011364780579876
    cout << "---" << endl;
    DECIMAL_OP_TEST( "-18458345086503572578057123591092037706279741.151", - , "+2632041719345252483380694049835872197434.9766493", "-18460977128222917830540504285141873578477176.1276493"       ); // -18458345086503572578057123591092037706279741.151 -  +2632041719345252483380694049835872197434.9766493 = -18460977128222917830540504285141873578477176.1276493
    DECIMAL_OP_TEST( "-174233558770663615740569957179488308019576.48254", - , "+6156169830305019652590488156540831.7027942076808", "-174233564926833446045589609769976464560408.1853342076808"   ); // -174233558770663615740569957179488308019576.48254 -  +6156169830305019652590488156540831.7027942076808 = -174233564926833446045589609769976464560408.1853342076808
    DECIMAL_OP_TEST( "-8.3645424522417012204289292807950462439988531619", - , "+781466404305113780602.80990487935255093934990570", "-781466404305113780611.1744473315942521597788349807950462439988531619" ); // -8.3645424522417012204289292807950462439988531619 -  +781466404305113780602.80990487935255093934990570 = -781466404305113780611.1744473315942521597788349807950462439988531619
    DECIMAL_OP_TEST( "-56380.329107833270343558230233235487388849158556", - , "+277145.35491132254485207055096587993977783302753", "-333525.684019155815195628781199115427166682186086"          ); // -56380.329107833270343558230233235487388849158556 -  +277145.35491132254485207055096587993977783302753 = -333525.684019155815195628781199115427166682186086
    DECIMAL_OP_TEST( "-597147583001091464692.45462985860111824175838980", - , "+1184055350.1862902754587418620655046842893720190", "-597147583002275520042.640920134059860103823894484289372019" ); // -597147583001091464692.45462985860111824175838980 -  +1184055350.1862902754587418620655046842893720190 = -597147583002275520042.640920134059860103823894484289372019
    DECIMAL_OP_TEST( "-1099062944468939421856224.7410255529071444836214", - , "+5235415554108378043687227.9880018482293335743662", "-6334478498577317465543452.7290274011364780579876"           ); // -1099062944468939421856224.7410255529071444836214 -  +5235415554108378043687227.9880018482293335743662 = -6334478498577317465543452.7290274011364780579876
    DECIMAL_OP_TEST( "-2632041719345252483380694049835872197434.9766493", - , "+18458345086503572578057123591092037706279741.151", "-18460977128222917830540504285141873578477176.1276493"       ); // -2632041719345252483380694049835872197434.9766493 -  +18458345086503572578057123591092037706279741.151 = -18460977128222917830540504285141873578477176.1276493
    DECIMAL_OP_TEST( "-6156169830305019652590488156540831.7027942076808", - , "+174233558770663615740569957179488308019576.48254", "-174233564926833446045589609769976464560408.1853342076808"   ); // -6156169830305019652590488156540831.7027942076808 -  +174233558770663615740569957179488308019576.48254 = -174233564926833446045589609769976464560408.1853342076808
    DECIMAL_OP_TEST( "-781466404305113780602.80990487935255093934990570", - , "+8.3645424522417012204289292807950462439988531619", "-781466404305113780611.1744473315942521597788349807950462439988531619" ); // -781466404305113780602.80990487935255093934990570 -  +8.3645424522417012204289292807950462439988531619 = -781466404305113780611.1744473315942521597788349807950462439988531619
    DECIMAL_OP_TEST( "-277145.35491132254485207055096587993977783302753", - , "+56380.329107833270343558230233235487388849158556", "-333525.684019155815195628781199115427166682186086"          ); // -277145.35491132254485207055096587993977783302753 -  +56380.329107833270343558230233235487388849158556 = -333525.684019155815195628781199115427166682186086
    DECIMAL_OP_TEST( "-1184055350.1862902754587418620655046842893720190", - , "+597147583001091464692.45462985860111824175838980", "-597147583002275520042.640920134059860103823894484289372019" ); // -1184055350.1862902754587418620655046842893720190 -  +597147583001091464692.45462985860111824175838980 = -597147583002275520042.640920134059860103823894484289372019
    DECIMAL_OP_TEST( "-5235415554108378043687227.9880018482293335743662", - , "+1099062944468939421856224.7410255529071444836214", "-6334478498577317465543452.7290274011364780579876"           ); // -5235415554108378043687227.9880018482293335743662 -  +1099062944468939421856224.7410255529071444836214 = -6334478498577317465543452.7290274011364780579876
    cout << "---" << endl;
    DECIMAL_OP_TEST( "-2632041719345252483380694049835872197434.9766493", - , "+18458345086503572578057123591092037706279741.151", "-18460977128222917830540504285141873578477176.1276493"       ); // -2632041719345252483380694049835872197434.9766493 -  +18458345086503572578057123591092037706279741.151 = -18460977128222917830540504285141873578477176.1276493
    DECIMAL_OP_TEST( "-6156169830305019652590488156540831.7027942076808", - , "+174233558770663615740569957179488308019576.48254", "-174233564926833446045589609769976464560408.1853342076808"   ); // -6156169830305019652590488156540831.7027942076808 -  +174233558770663615740569957179488308019576.48254 = -174233564926833446045589609769976464560408.1853342076808
    DECIMAL_OP_TEST( "-781466404305113780602.80990487935255093934990570", - , "+8.3645424522417012204289292807950462439988531619", "-781466404305113780611.1744473315942521597788349807950462439988531619" ); // -781466404305113780602.80990487935255093934990570 -  +8.3645424522417012204289292807950462439988531619 = -781466404305113780611.1744473315942521597788349807950462439988531619
    DECIMAL_OP_TEST( "-277145.35491132254485207055096587993977783302753", - , "+56380.329107833270343558230233235487388849158556", "-333525.684019155815195628781199115427166682186086"          ); // -277145.35491132254485207055096587993977783302753 -  +56380.329107833270343558230233235487388849158556 = -333525.684019155815195628781199115427166682186086
    DECIMAL_OP_TEST( "-1184055350.1862902754587418620655046842893720190", - , "+597147583001091464692.45462985860111824175838980", "-597147583002275520042.640920134059860103823894484289372019" ); // -1184055350.1862902754587418620655046842893720190 -  +597147583001091464692.45462985860111824175838980 = -597147583002275520042.640920134059860103823894484289372019
    DECIMAL_OP_TEST( "-5235415554108378043687227.9880018482293335743662", - , "+1099062944468939421856224.7410255529071444836214", "-6334478498577317465543452.7290274011364780579876"           ); // -5235415554108378043687227.9880018482293335743662 -  +1099062944468939421856224.7410255529071444836214 = -6334478498577317465543452.7290274011364780579876
    DECIMAL_OP_TEST( "-18458345086503572578057123591092037706279741.151", - , "+2632041719345252483380694049835872197434.9766493", "-18460977128222917830540504285141873578477176.1276493"       ); // -18458345086503572578057123591092037706279741.151 -  +2632041719345252483380694049835872197434.9766493 = -18460977128222917830540504285141873578477176.1276493
    DECIMAL_OP_TEST( "-174233558770663615740569957179488308019576.48254", - , "+6156169830305019652590488156540831.7027942076808", "-174233564926833446045589609769976464560408.1853342076808"   ); // -174233558770663615740569957179488308019576.48254 -  +6156169830305019652590488156540831.7027942076808 = -174233564926833446045589609769976464560408.1853342076808
    DECIMAL_OP_TEST( "-8.3645424522417012204289292807950462439988531619", - , "+781466404305113780602.80990487935255093934990570", "-781466404305113780611.1744473315942521597788349807950462439988531619" ); // -8.3645424522417012204289292807950462439988531619 -  +781466404305113780602.80990487935255093934990570 = -781466404305113780611.1744473315942521597788349807950462439988531619
    DECIMAL_OP_TEST( "-56380.329107833270343558230233235487388849158556", - , "+277145.35491132254485207055096587993977783302753", "-333525.684019155815195628781199115427166682186086"          ); // -56380.329107833270343558230233235487388849158556 -  +277145.35491132254485207055096587993977783302753 = -333525.684019155815195628781199115427166682186086
    DECIMAL_OP_TEST( "-597147583001091464692.45462985860111824175838980", - , "+1184055350.1862902754587418620655046842893720190", "-597147583002275520042.640920134059860103823894484289372019" ); // -597147583001091464692.45462985860111824175838980 -  +1184055350.1862902754587418620655046842893720190 = -597147583002275520042.640920134059860103823894484289372019
    DECIMAL_OP_TEST( "-1099062944468939421856224.7410255529071444836214", - , "+5235415554108378043687227.9880018482293335743662", "-6334478498577317465543452.7290274011364780579876"           ); // -1099062944468939421856224.7410255529071444836214 -  +5235415554108378043687227.9880018482293335743662 = -6334478498577317465543452.7290274011364780579876
    cout << "---" << endl;
    DECIMAL_OP_TEST( "+18458345086503572578057123591092037706279741.151", - , "-2632041719345252483380694049835872197434.9766493", "18460977128222917830540504285141873578477176.1276493"        ); // +18458345086503572578057123591092037706279741.151 -  -2632041719345252483380694049835872197434.9766493 = 18460977128222917830540504285141873578477176.1276493
    DECIMAL_OP_TEST( "+174233558770663615740569957179488308019576.48254", - , "-6156169830305019652590488156540831.7027942076808", "174233564926833446045589609769976464560408.1853342076808"    ); // +174233558770663615740569957179488308019576.48254 -  -6156169830305019652590488156540831.7027942076808 = 174233564926833446045589609769976464560408.1853342076808
    DECIMAL_OP_TEST( "+8.3645424522417012204289292807950462439988531619", - , "-781466404305113780602.80990487935255093934990570", "781466404305113780611.1744473315942521597788349807950462439988531619" ); // +8.3645424522417012204289292807950462439988531619 -  -781466404305113780602.80990487935255093934990570 = 781466404305113780611.1744473315942521597788349807950462439988531619
    DECIMAL_OP_TEST( "+56380.329107833270343558230233235487388849158556", - , "-277145.35491132254485207055096587993977783302753", "333525.684019155815195628781199115427166682186086"           ); // +56380.329107833270343558230233235487388849158556 -  -277145.35491132254485207055096587993977783302753 = 333525.684019155815195628781199115427166682186086
    DECIMAL_OP_TEST( "+597147583001091464692.45462985860111824175838980", - , "-1184055350.1862902754587418620655046842893720190", "597147583002275520042.640920134059860103823894484289372019"  ); // +597147583001091464692.45462985860111824175838980 -  -1184055350.1862902754587418620655046842893720190 = 597147583002275520042.640920134059860103823894484289372019
    DECIMAL_OP_TEST( "+1099062944468939421856224.7410255529071444836214", - , "-5235415554108378043687227.9880018482293335743662", "6334478498577317465543452.7290274011364780579876"            ); // +1099062944468939421856224.7410255529071444836214 -  -5235415554108378043687227.9880018482293335743662 = 6334478498577317465543452.7290274011364780579876
    DECIMAL_OP_TEST( "+2632041719345252483380694049835872197434.9766493", - , "-18458345086503572578057123591092037706279741.151", "18460977128222917830540504285141873578477176.1276493"        ); // +2632041719345252483380694049835872197434.9766493 -  -18458345086503572578057123591092037706279741.151 = 18460977128222917830540504285141873578477176.1276493
    DECIMAL_OP_TEST( "+6156169830305019652590488156540831.7027942076808", - , "-174233558770663615740569957179488308019576.48254", "174233564926833446045589609769976464560408.1853342076808"    ); // +6156169830305019652590488156540831.7027942076808 -  -174233558770663615740569957179488308019576.48254 = 174233564926833446045589609769976464560408.1853342076808
    DECIMAL_OP_TEST( "+781466404305113780602.80990487935255093934990570", - , "-8.3645424522417012204289292807950462439988531619", "781466404305113780611.1744473315942521597788349807950462439988531619" ); // +781466404305113780602.80990487935255093934990570 -  -8.3645424522417012204289292807950462439988531619 = 781466404305113780611.1744473315942521597788349807950462439988531619
    DECIMAL_OP_TEST( "+277145.35491132254485207055096587993977783302753", - , "-56380.329107833270343558230233235487388849158556", "333525.684019155815195628781199115427166682186086"           ); // +277145.35491132254485207055096587993977783302753 -  -56380.329107833270343558230233235487388849158556 = 333525.684019155815195628781199115427166682186086
    DECIMAL_OP_TEST( "+1184055350.1862902754587418620655046842893720190", - , "-597147583001091464692.45462985860111824175838980", "597147583002275520042.640920134059860103823894484289372019"  ); // +1184055350.1862902754587418620655046842893720190 -  -597147583001091464692.45462985860111824175838980 = 597147583002275520042.640920134059860103823894484289372019
    DECIMAL_OP_TEST( "+5235415554108378043687227.9880018482293335743662", - , "-1099062944468939421856224.7410255529071444836214", "6334478498577317465543452.7290274011364780579876"            ); // +5235415554108378043687227.9880018482293335743662 -  -1099062944468939421856224.7410255529071444836214 = 6334478498577317465543452.7290274011364780579876

    cout << endl;


    cout << "-------" << endl;
    DECIMAL_OP_TEST( "12345", * , "5439876", "67155269220");
    DECIMAL_OP_TEST(   "123", * ,     "456", "56088");

    DECIMAL_OP_TEST(   "0", * ,  "-2",   "0" );
    DECIMAL_OP_TEST(  "10", * ,   "0",   "0" );
    DECIMAL_OP_TEST(  "10", * ,  "-2", "-20" );
    DECIMAL_OP_TEST(   "2", * ,  "-2",  "-4" );
    DECIMAL_OP_TEST(   "2", * , "-10", "-20" );
    DECIMAL_OP_TEST(  "10", * ,   "2",  "20" );
    DECIMAL_OP_TEST(   "2", * ,   "2",   "4" );
    DECIMAL_OP_TEST(   "2", * ,  "10",  "20" );
    DECIMAL_OP_TEST( "-10", * ,   "2", "-20" );
    DECIMAL_OP_TEST(  "-2", * ,   "2",  "-4" );
    DECIMAL_OP_TEST(  "-2", * ,  "10", "-20" );
    DECIMAL_OP_TEST(  "10", * ,   "2",  "20" );
    DECIMAL_OP_TEST(   "2", * ,   "2",   "4" );
    DECIMAL_OP_TEST(   "2", * ,  "10",  "20" );
    cout << "---" << endl;
    DECIMAL_OP_TEST( "+2632041719345252483380694049835872197434.9766493", * , "-18458345086503572578057123591092037706279741.151", "-48583134337748856348203923622087391013486529983282036600113616828850196220864498121.3333053443"  ); // +2632041719345252483380694049835872197434.9766493 *  -18458345086503572578057123591092037706279741.151 = -48583134337748856348203923622087391013486529983282036600113616828850196220864498121.3333053443
    DECIMAL_OP_TEST( "+6156169830305019652590488156540831.7027942076808", * , "-174233558770663615740569957179488308019576.48254", "-1072611377930635899866642094201046004917180606443610497843484826433015313054.225362276315093232" ); // +6156169830305019652590488156540831.7027942076808 *  -174233558770663615740569957179488308019576.48254 = -1072611377930635899866642094201046004917180606443610497843484826433015313054.225362276315093232
    DECIMAL_OP_TEST( "+781466404305113780602.80990487935255093934990570", * , "-8.3645424522417012204289292807950462439988531619", "-6536608913810801162187.17067562402112383021937217334593336704980813014469291978975353891183283"  ); // +781466404305113780602.80990487935255093934990570 *  -8.3645424522417012204289292807950462439988531619 = -6536608913810801162187.17067562402112383021937217334593336704980813014469291978975353891183283
    DECIMAL_OP_TEST( "+277145.35491132254485207055096587993977783302753", * , "-56380.329107833270343558230233235487388849158556", "-15625546320.60762088449103168148392495929776927544054022086436264879135752937428344610648304668" ); // +277145.35491132254485207055096587993977783302753 *  -56380.329107833270343558230233235487388849158556 = -15625546320.60762088449103168148392495929776927544054022086436264879135752937428344610648304668
    DECIMAL_OP_TEST( "+1184055350.1862902754587418620655046842893720190", * , "-597147583001091464692.45462985860111824175838980", "-707055790503254192335232257527.8927714354905923182679046718530461315279166294731785666150062"    ); // +1184055350.1862902754587418620655046842893720190 *  -597147583001091464692.45462985860111824175838980 = -707055790503254192335232257527.8927714354905923182679046718530461315279166294731785666150062
    DECIMAL_OP_TEST( "+5235415554108378043687227.9880018482293335743662", * , "-1099062944468939421856224.7410255529071444836214", "-5754051234416838010880514705725906954064227176449.74906132610473677005243402438507643478575668"  ); // +5235415554108378043687227.9880018482293335743662 *  -1099062944468939421856224.7410255529071444836214 = -5754051234416838010880514705725906954064227176449.74906132610473677005243402438507643478575668
    DECIMAL_OP_TEST( "+18458345086503572578057123591092037706279741.151", * , "-2632041719345252483380694049835872197434.9766493", "-48583134337748856348203923622087391013486529983282036600113616828850196220864498121.3333053443"  ); // +18458345086503572578057123591092037706279741.151 *  -2632041719345252483380694049835872197434.9766493 = -48583134337748856348203923622087391013486529983282036600113616828850196220864498121.3333053443
    DECIMAL_OP_TEST( "+174233558770663615740569957179488308019576.48254", * , "-6156169830305019652590488156540831.7027942076808", "-1072611377930635899866642094201046004917180606443610497843484826433015313054.225362276315093232" ); // +174233558770663615740569957179488308019576.48254 *  -6156169830305019652590488156540831.7027942076808 = -1072611377930635899866642094201046004917180606443610497843484826433015313054.225362276315093232
    DECIMAL_OP_TEST( "+8.3645424522417012204289292807950462439988531619", * , "-781466404305113780602.80990487935255093934990570", "-6536608913810801162187.17067562402112383021937217334593336704980813014469291978975353891183283"  ); // +8.3645424522417012204289292807950462439988531619 *  -781466404305113780602.80990487935255093934990570 = -6536608913810801162187.17067562402112383021937217334593336704980813014469291978975353891183283
    DECIMAL_OP_TEST( "+56380.329107833270343558230233235487388849158556", * , "-277145.35491132254485207055096587993977783302753", "-15625546320.60762088449103168148392495929776927544054022086436264879135752937428344610648304668" ); // +56380.329107833270343558230233235487388849158556 *  -277145.35491132254485207055096587993977783302753 = -15625546320.60762088449103168148392495929776927544054022086436264879135752937428344610648304668
    DECIMAL_OP_TEST( "+597147583001091464692.45462985860111824175838980", * , "-1184055350.1862902754587418620655046842893720190", "-707055790503254192335232257527.8927714354905923182679046718530461315279166294731785666150062"    ); // +597147583001091464692.45462985860111824175838980 *  -1184055350.1862902754587418620655046842893720190 = -707055790503254192335232257527.8927714354905923182679046718530461315279166294731785666150062
    DECIMAL_OP_TEST( "+1099062944468939421856224.7410255529071444836214", * , "-5235415554108378043687227.9880018482293335743662", "-5754051234416838010880514705725906954064227176449.74906132610473677005243402438507643478575668"  ); // +1099062944468939421856224.7410255529071444836214 *  -5235415554108378043687227.9880018482293335743662 = -5754051234416838010880514705725906954064227176449.74906132610473677005243402438507643478575668
    cout << "---" << endl;
    DECIMAL_OP_TEST( "-18458345086503572578057123591092037706279741.151", * , "+2632041719345252483380694049835872197434.9766493", "-48583134337748856348203923622087391013486529983282036600113616828850196220864498121.3333053443"  ); // -18458345086503572578057123591092037706279741.151 *  +2632041719345252483380694049835872197434.9766493 = -48583134337748856348203923622087391013486529983282036600113616828850196220864498121.3333053443
    DECIMAL_OP_TEST( "-174233558770663615740569957179488308019576.48254", * , "+6156169830305019652590488156540831.7027942076808", "-1072611377930635899866642094201046004917180606443610497843484826433015313054.225362276315093232" ); // -174233558770663615740569957179488308019576.48254 *  +6156169830305019652590488156540831.7027942076808 = -1072611377930635899866642094201046004917180606443610497843484826433015313054.225362276315093232
    DECIMAL_OP_TEST( "-8.3645424522417012204289292807950462439988531619", * , "+781466404305113780602.80990487935255093934990570", "-6536608913810801162187.17067562402112383021937217334593336704980813014469291978975353891183283"  ); // -8.3645424522417012204289292807950462439988531619 *  +781466404305113780602.80990487935255093934990570 = -6536608913810801162187.17067562402112383021937217334593336704980813014469291978975353891183283
    DECIMAL_OP_TEST( "-56380.329107833270343558230233235487388849158556", * , "+277145.35491132254485207055096587993977783302753", "-15625546320.60762088449103168148392495929776927544054022086436264879135752937428344610648304668" ); // -56380.329107833270343558230233235487388849158556 *  +277145.35491132254485207055096587993977783302753 = -15625546320.60762088449103168148392495929776927544054022086436264879135752937428344610648304668
    DECIMAL_OP_TEST( "-597147583001091464692.45462985860111824175838980", * , "+1184055350.1862902754587418620655046842893720190", "-707055790503254192335232257527.8927714354905923182679046718530461315279166294731785666150062"    ); // -597147583001091464692.45462985860111824175838980 *  +1184055350.1862902754587418620655046842893720190 = -707055790503254192335232257527.8927714354905923182679046718530461315279166294731785666150062
    DECIMAL_OP_TEST( "-1099062944468939421856224.7410255529071444836214", * , "+5235415554108378043687227.9880018482293335743662", "-5754051234416838010880514705725906954064227176449.74906132610473677005243402438507643478575668"  ); // -1099062944468939421856224.7410255529071444836214 *  +5235415554108378043687227.9880018482293335743662 = -5754051234416838010880514705725906954064227176449.74906132610473677005243402438507643478575668
    DECIMAL_OP_TEST( "-2632041719345252483380694049835872197434.9766493", * , "+18458345086503572578057123591092037706279741.151", "-48583134337748856348203923622087391013486529983282036600113616828850196220864498121.3333053443"  ); // -2632041719345252483380694049835872197434.9766493 *  +18458345086503572578057123591092037706279741.151 = -48583134337748856348203923622087391013486529983282036600113616828850196220864498121.3333053443
    DECIMAL_OP_TEST( "-6156169830305019652590488156540831.7027942076808", * , "+174233558770663615740569957179488308019576.48254", "-1072611377930635899866642094201046004917180606443610497843484826433015313054.225362276315093232" ); // -6156169830305019652590488156540831.7027942076808 *  +174233558770663615740569957179488308019576.48254 = -1072611377930635899866642094201046004917180606443610497843484826433015313054.225362276315093232
    DECIMAL_OP_TEST( "-781466404305113780602.80990487935255093934990570", * , "+8.3645424522417012204289292807950462439988531619", "-6536608913810801162187.17067562402112383021937217334593336704980813014469291978975353891183283"  ); // -781466404305113780602.80990487935255093934990570 *  +8.3645424522417012204289292807950462439988531619 = -6536608913810801162187.17067562402112383021937217334593336704980813014469291978975353891183283
    DECIMAL_OP_TEST( "-277145.35491132254485207055096587993977783302753", * , "+56380.329107833270343558230233235487388849158556", "-15625546320.60762088449103168148392495929776927544054022086436264879135752937428344610648304668" ); // -277145.35491132254485207055096587993977783302753 *  +56380.329107833270343558230233235487388849158556 = -15625546320.60762088449103168148392495929776927544054022086436264879135752937428344610648304668
    DECIMAL_OP_TEST( "-1184055350.1862902754587418620655046842893720190", * , "+597147583001091464692.45462985860111824175838980", "-707055790503254192335232257527.8927714354905923182679046718530461315279166294731785666150062"    ); // -1184055350.1862902754587418620655046842893720190 *  +597147583001091464692.45462985860111824175838980 = -707055790503254192335232257527.8927714354905923182679046718530461315279166294731785666150062
    DECIMAL_OP_TEST( "-5235415554108378043687227.9880018482293335743662", * , "+1099062944468939421856224.7410255529071444836214", "-5754051234416838010880514705725906954064227176449.74906132610473677005243402438507643478575668"  ); // -5235415554108378043687227.9880018482293335743662 *  +1099062944468939421856224.7410255529071444836214 = -5754051234416838010880514705725906954064227176449.74906132610473677005243402438507643478575668
    cout << "---" << endl;
    DECIMAL_OP_TEST( "-2632041719345252483380694049835872197434.9766493", * , "+18458345086503572578057123591092037706279741.151", "-48583134337748856348203923622087391013486529983282036600113616828850196220864498121.3333053443"  ); // -2632041719345252483380694049835872197434.9766493 *  +18458345086503572578057123591092037706279741.151 = -48583134337748856348203923622087391013486529983282036600113616828850196220864498121.3333053443
    DECIMAL_OP_TEST( "-6156169830305019652590488156540831.7027942076808", * , "+174233558770663615740569957179488308019576.48254", "-1072611377930635899866642094201046004917180606443610497843484826433015313054.225362276315093232" ); // -6156169830305019652590488156540831.7027942076808 *  +174233558770663615740569957179488308019576.48254 = -1072611377930635899866642094201046004917180606443610497843484826433015313054.225362276315093232
    DECIMAL_OP_TEST( "-781466404305113780602.80990487935255093934990570", * , "+8.3645424522417012204289292807950462439988531619", "-6536608913810801162187.17067562402112383021937217334593336704980813014469291978975353891183283"  ); // -781466404305113780602.80990487935255093934990570 *  +8.3645424522417012204289292807950462439988531619 = -6536608913810801162187.17067562402112383021937217334593336704980813014469291978975353891183283
    DECIMAL_OP_TEST( "-277145.35491132254485207055096587993977783302753", * , "+56380.329107833270343558230233235487388849158556", "-15625546320.60762088449103168148392495929776927544054022086436264879135752937428344610648304668" ); // -277145.35491132254485207055096587993977783302753 *  +56380.329107833270343558230233235487388849158556 = -15625546320.60762088449103168148392495929776927544054022086436264879135752937428344610648304668
    DECIMAL_OP_TEST( "-1184055350.1862902754587418620655046842893720190", * , "+597147583001091464692.45462985860111824175838980", "-707055790503254192335232257527.8927714354905923182679046718530461315279166294731785666150062"    ); // -1184055350.1862902754587418620655046842893720190 *  +597147583001091464692.45462985860111824175838980 = -707055790503254192335232257527.8927714354905923182679046718530461315279166294731785666150062
    DECIMAL_OP_TEST( "-5235415554108378043687227.9880018482293335743662", * , "+1099062944468939421856224.7410255529071444836214", "-5754051234416838010880514705725906954064227176449.74906132610473677005243402438507643478575668"  ); // -5235415554108378043687227.9880018482293335743662 *  +1099062944468939421856224.7410255529071444836214 = -5754051234416838010880514705725906954064227176449.74906132610473677005243402438507643478575668
    DECIMAL_OP_TEST( "-18458345086503572578057123591092037706279741.151", * , "+2632041719345252483380694049835872197434.9766493", "-48583134337748856348203923622087391013486529983282036600113616828850196220864498121.3333053443"  ); // -18458345086503572578057123591092037706279741.151 *  +2632041719345252483380694049835872197434.9766493 = -48583134337748856348203923622087391013486529983282036600113616828850196220864498121.3333053443
    DECIMAL_OP_TEST( "-174233558770663615740569957179488308019576.48254", * , "+6156169830305019652590488156540831.7027942076808", "-1072611377930635899866642094201046004917180606443610497843484826433015313054.225362276315093232" ); // -174233558770663615740569957179488308019576.48254 *  +6156169830305019652590488156540831.7027942076808 = -1072611377930635899866642094201046004917180606443610497843484826433015313054.225362276315093232
    DECIMAL_OP_TEST( "-8.3645424522417012204289292807950462439988531619", * , "+781466404305113780602.80990487935255093934990570", "-6536608913810801162187.17067562402112383021937217334593336704980813014469291978975353891183283"  ); // -8.3645424522417012204289292807950462439988531619 *  +781466404305113780602.80990487935255093934990570 = -6536608913810801162187.17067562402112383021937217334593336704980813014469291978975353891183283
    DECIMAL_OP_TEST( "-56380.329107833270343558230233235487388849158556", * , "+277145.35491132254485207055096587993977783302753", "-15625546320.60762088449103168148392495929776927544054022086436264879135752937428344610648304668" ); // -56380.329107833270343558230233235487388849158556 *  +277145.35491132254485207055096587993977783302753 = -15625546320.60762088449103168148392495929776927544054022086436264879135752937428344610648304668
    DECIMAL_OP_TEST( "-597147583001091464692.45462985860111824175838980", * , "+1184055350.1862902754587418620655046842893720190", "-707055790503254192335232257527.8927714354905923182679046718530461315279166294731785666150062"    ); // -597147583001091464692.45462985860111824175838980 *  +1184055350.1862902754587418620655046842893720190 = -707055790503254192335232257527.8927714354905923182679046718530461315279166294731785666150062
    DECIMAL_OP_TEST( "-1099062944468939421856224.7410255529071444836214", * , "+5235415554108378043687227.9880018482293335743662", "-5754051234416838010880514705725906954064227176449.74906132610473677005243402438507643478575668"  ); // -1099062944468939421856224.7410255529071444836214 *  +5235415554108378043687227.9880018482293335743662 = -5754051234416838010880514705725906954064227176449.74906132610473677005243402438507643478575668
    cout << "---" << endl;
    DECIMAL_OP_TEST( "+18458345086503572578057123591092037706279741.151", * , "-2632041719345252483380694049835872197434.9766493", "-48583134337748856348203923622087391013486529983282036600113616828850196220864498121.3333053443"  ); // +18458345086503572578057123591092037706279741.151 *  -2632041719345252483380694049835872197434.9766493 = -48583134337748856348203923622087391013486529983282036600113616828850196220864498121.3333053443
    DECIMAL_OP_TEST( "+174233558770663615740569957179488308019576.48254", * , "-6156169830305019652590488156540831.7027942076808", "-1072611377930635899866642094201046004917180606443610497843484826433015313054.225362276315093232" ); // +174233558770663615740569957179488308019576.48254 *  -6156169830305019652590488156540831.7027942076808 = -1072611377930635899866642094201046004917180606443610497843484826433015313054.225362276315093232
    DECIMAL_OP_TEST( "+8.3645424522417012204289292807950462439988531619", * , "-781466404305113780602.80990487935255093934990570", "-6536608913810801162187.17067562402112383021937217334593336704980813014469291978975353891183283"  ); // +8.3645424522417012204289292807950462439988531619 *  -781466404305113780602.80990487935255093934990570 = -6536608913810801162187.17067562402112383021937217334593336704980813014469291978975353891183283
    DECIMAL_OP_TEST( "+56380.329107833270343558230233235487388849158556", * , "-277145.35491132254485207055096587993977783302753", "-15625546320.60762088449103168148392495929776927544054022086436264879135752937428344610648304668" ); // +56380.329107833270343558230233235487388849158556 *  -277145.35491132254485207055096587993977783302753 = -15625546320.60762088449103168148392495929776927544054022086436264879135752937428344610648304668
    DECIMAL_OP_TEST( "+597147583001091464692.45462985860111824175838980", * , "-1184055350.1862902754587418620655046842893720190", "-707055790503254192335232257527.8927714354905923182679046718530461315279166294731785666150062"    ); // +597147583001091464692.45462985860111824175838980 *  -1184055350.1862902754587418620655046842893720190 = -707055790503254192335232257527.8927714354905923182679046718530461315279166294731785666150062
    DECIMAL_OP_TEST( "+1099062944468939421856224.7410255529071444836214", * , "-5235415554108378043687227.9880018482293335743662", "-5754051234416838010880514705725906954064227176449.74906132610473677005243402438507643478575668"  ); // +1099062944468939421856224.7410255529071444836214 *  -5235415554108378043687227.9880018482293335743662 = -5754051234416838010880514705725906954064227176449.74906132610473677005243402438507643478575668
    DECIMAL_OP_TEST( "+2632041719345252483380694049835872197434.9766493", * , "-18458345086503572578057123591092037706279741.151", "-48583134337748856348203923622087391013486529983282036600113616828850196220864498121.3333053443"  ); // +2632041719345252483380694049835872197434.9766493 *  -18458345086503572578057123591092037706279741.151 = -48583134337748856348203923622087391013486529983282036600113616828850196220864498121.3333053443
    DECIMAL_OP_TEST( "+6156169830305019652590488156540831.7027942076808", * , "-174233558770663615740569957179488308019576.48254", "-1072611377930635899866642094201046004917180606443610497843484826433015313054.225362276315093232" ); // +6156169830305019652590488156540831.7027942076808 *  -174233558770663615740569957179488308019576.48254 = -1072611377930635899866642094201046004917180606443610497843484826433015313054.225362276315093232
    DECIMAL_OP_TEST( "+781466404305113780602.80990487935255093934990570", * , "-8.3645424522417012204289292807950462439988531619", "-6536608913810801162187.17067562402112383021937217334593336704980813014469291978975353891183283"  ); // +781466404305113780602.80990487935255093934990570 *  -8.3645424522417012204289292807950462439988531619 = -6536608913810801162187.17067562402112383021937217334593336704980813014469291978975353891183283
    DECIMAL_OP_TEST( "+277145.35491132254485207055096587993977783302753", * , "-56380.329107833270343558230233235487388849158556", "-15625546320.60762088449103168148392495929776927544054022086436264879135752937428344610648304668" ); // +277145.35491132254485207055096587993977783302753 *  -56380.329107833270343558230233235487388849158556 = -15625546320.60762088449103168148392495929776927544054022086436264879135752937428344610648304668
    DECIMAL_OP_TEST( "+1184055350.1862902754587418620655046842893720190", * , "-597147583001091464692.45462985860111824175838980", "-707055790503254192335232257527.8927714354905923182679046718530461315279166294731785666150062"    ); // +1184055350.1862902754587418620655046842893720190 *  -597147583001091464692.45462985860111824175838980 = -707055790503254192335232257527.8927714354905923182679046718530461315279166294731785666150062
    DECIMAL_OP_TEST( "+5235415554108378043687227.9880018482293335743662", * , "-1099062944468939421856224.7410255529071444836214", "-5754051234416838010880514705725906954064227176449.74906132610473677005243402438507643478575668"  ); // +5235415554108378043687227.9880018482293335743662 *  -1099062944468939421856224.7410255529071444836214 = -5754051234416838010880514705725906954064227176449.74906132610473677005243402438507643478575668

    cout << endl;


    cout << "-------" << endl;
    DECIMAL_OP_TEST(   "0", / ,  "-2",   "0" );
    // DECIMAL_OP_TEST(  10, / ,   0,   "0" ); // throws "division by zero" std::runtime_error
    DECIMAL_OP_TEST(  "10", / ,  "-2",  "-5" );
    DECIMAL_OP_TEST(   "2", / ,  "-2",  "-1" );
    DECIMAL_OP_TEST(   "2", / , "-10","-0.2" );
    DECIMAL_OP_TEST(  "10", / ,   "2",   "5" );
    DECIMAL_OP_TEST(   "2", / ,   "2",   "1" );
    DECIMAL_OP_TEST(   "2", / ,  "10", "0.2" );
    DECIMAL_OP_TEST( "-10", / ,   "2",  "-5" );
    DECIMAL_OP_TEST(  "-2", / ,   "2",  "-1" );
    DECIMAL_OP_TEST(  "-2", / ,  "10","-0.2" );
    DECIMAL_OP_TEST(  "10", / ,   "2",   "5" );
    DECIMAL_OP_TEST(   "2", / ,   "2",   "1" );
    DECIMAL_OP_TEST(   "2", / ,  "10", "0.2" );
    cout << "---" << endl;
    marty::Decimal::setDivisionPrecision(500);
    // Эталонные значения посчитаны в калькуляторе Far'а. Он даёт порядка 140 знаков точности, и округляет последний разряд
    // Реальные значения в данном тесте длиннее. Если бы калькулятор Far'а просто обрезал - то проблем бы не было. Но он - округляет. Поэтому последний знак у эталона - обрезан
    DECIMAL_OP_TEST( "+2632041719345252483380694049835872197434.9766493", / , "-18458345086503572578057123591092037706279741.151", "-0.00014259359151702914808567791735666219703647970387789316932539971157744249100212096589225788821764932029458242484728094699899571026393925"          ); // +2632041719345252483380694049835872197434.9766493 /  -18458345086503572578057123591092037706279741.151 = -0.00014259359151702914808567791735666219703647970387789316932539971157744249100212096589225788821764932029458242484728094699899571026393925
    DECIMAL_OP_TEST( "-6156169830305019652590488156540831.7027942076808", / , "-174233558770663615740569957179488308019576.48254", "0.0000000353328593741698742835730936706750278403524119315845640364583784167951390172561112289920418933131238936234368011036015898988383824"           ); // +6156169830305019652590488156540831.7027942076808 /  -174233558770663615740569957179488308019576.48254 = -0.00000003533285937416987428357309367067502784035241193158456403645837841679513901725611122899204189331312389362343680110360158989883838244
    DECIMAL_OP_TEST( "+781466404305113780602.80990487935255093934990570", / , "+8.3645424522417012204289292807950462439988531619", "93426079043412640603.1093995564991780570475281096665830425369229104684956101813937282550204282550536762765864200174735725916412536837560058770739489" ); // +781466404305113780602.80990487935255093934990570 /  -8.3645424522417012204289292807950462439988531619 = -93426079043412640603.1093995564991780570475281096665830425369229104684956101813937282550204282550536762765864200174735725916412536837560058770739489
    DECIMAL_OP_TEST( "-277145.35491132254485207055096587993977783302753", / , "+56380.329107833270343558230233235487388849158556", "-4.9156391829720095447343371323525594558626670930255875653889031887940317811560627090169783126082131574609978226363340688096311714220694647"           ); // +277145.35491132254485207055096587993977783302753 /  -56380.329107833270343558230233235487388849158556 = -4.9156391829720095447343371323525594558626670930255875653889031887940317811560627090169783126082131574609978226363340688096311714220694647
    DECIMAL_OP_TEST( "+1184055350.1862902754587418620655046842893720190", / , "-597147583001091464692.45462985860111824175838980", "-0.00000000000198285211879376569858898618936113144115909070020110428359310570440770740490935449788328490616242780925913323412757"                      ); // +1184055350.1862902754587418620655046842893720190 /  -597147583001091464692.45462985860111824175838980 = -0.0000000000019828521187937656985889861893611314411590907002011042835931057044077074049093544978832849061624278092591332341275705
    DECIMAL_OP_TEST( "-5235415554108378043687227.9880018482293335743662", / , "-1099062944468939421856224.7410255529071444836214", "4.76352658458346917127605305961347072751430254382447414267705419237835721787308618139397237728970369395792561992746134338301460108438073433"          ); // +5235415554108378043687227.9880018482293335743662 /  -1099062944468939421856224.7410255529071444836214 = -4.76352658458346917127605305961347072751430254382447414267705419237835721787308618139397237728970369395792561992746134338301460108438073433
    DECIMAL_OP_TEST( "+18458345086503572578057123591092037706279741.151", / , "+2632041719345252483380694049835872197434.9766493", "7012.93788424268451260357338740581282332462483776801837811546160359571819272428037158067038009296169473060593467099267998584476429221226863104"       ); // +18458345086503572578057123591092037706279741.151 /  -2632041719345252483380694049835872197434.9766493 = -7012.93788424268451260357338740581282332462483776801837811546160359571819272428037158067038009296169473060593467099267998584476429221226863104
    DECIMAL_OP_TEST( "-174233558770663615740569957179488308019576.48254", / , "+6156169830305019652590488156540831.7027942076808", "-28302266.43731673469189139861911390043386693518709744104716349359833067279527756305994108995563434221988682099693589115651184296896301683575725163"   ); // +174233558770663615740569957179488308019576.48254 /  -6156169830305019652590488156540831.7027942076808 = -28302266.43731673469189139861911390043386693518709744104716349359833067279527756305994108995563434221988682099693589115651184296896301683575725163
    DECIMAL_OP_TEST( "+8.3645424522417012204289292807950462439988531619", / , "-781466404305113780602.80990487935255093934990570", "-0.000000000000000000010703649454616695549056034584327389776262891529059101634740661851946986137071584058516353586485364070671148"                     ); // +8.3645424522417012204289292807950462439988531619 /  -781466404305113780602.80990487935255093934990570 = -0.0000000000000000000107036494546166955490560345843273897762628915290591016347406618519469861370715840585163535864853640706711487
    DECIMAL_OP_TEST( "-56380.329107833270343558230233235487388849158556", / , "-277145.35491132254485207055096587993977783302753", "0.20343234374566058623534286851175627498220603163693860894738119538267002431608951126462691916831161704094199841923339605824989196874638298"          ); // +56380.329107833270343558230233235487388849158556 /  -277145.35491132254485207055096587993977783302753 = -0.20343234374566058623534286851175627498220603163693860894738119538267002431608951126462691916831161704094199841923339605824989196874638298
    DECIMAL_OP_TEST( "+597147583001091464692.45462985860111824175838980", / , "+1184055350.1862902754587418620655046842893720190", "504324044401.4216054515078517021892695671616294780843129406280737254141173306856786191443022214657806166838054941488557720236041392674534962"         ); // +597147583001091464692.45462985860111824175838980 /  -1184055350.1862902754587418620655046842893720190 = -504324044401.4216054515078517021892695671616294780843129406280737254141173306856786191443022214657806166838054941488557720236041392674534962
    DECIMAL_OP_TEST( "-1099062944468939421856224.7410255529071444836214", / , "+5235415554108378043687227.9880018482293335743662", "-0.20992850197086528759211307032496311526251725644028411309854689668320991540956226806877290698907804748112973168686659111255910811846192119"          ); // +1099062944468939421856224.7410255529071444836214 /  -5235415554108378043687227.9880018482293335743662 = -0.20992850197086528759211307032496311526251725644028411309854689668320991540956226806877290698907804748112973168686659111255910811846192119
    /*
    cout << "---" << endl;
    DECIMAL_OP_TEST( "-18458345086503572578057123591092037706279741.151", / , "+2632041719345252483380694049835872197434.9766493", "" ); // -18458345086503572578057123591092037706279741.151 /  +2632041719345252483380694049835872197434.9766493 = -7012.93788424268451260357338740581282332462483776801837811546160359571819272428037158067038009296169473060593467099267998584476429221226863104
    DECIMAL_OP_TEST( "-174233558770663615740569957179488308019576.48254", / , "+6156169830305019652590488156540831.7027942076808", "" ); // -174233558770663615740569957179488308019576.48254 /  +6156169830305019652590488156540831.7027942076808 = -28302266.43731673469189139861911390043386693518709744104716349359833067279527756305994108995563434221988682099693589115651184296896301683575725163
    DECIMAL_OP_TEST( "-8.3645424522417012204289292807950462439988531619", / , "+781466404305113780602.80990487935255093934990570", "" ); // -8.3645424522417012204289292807950462439988531619 /  +781466404305113780602.80990487935255093934990570 = -0.0000000000000000000107036494546166955490560345843273897762628915290591016347406618519469861370715840585163535864853640706711487
    DECIMAL_OP_TEST( "-56380.329107833270343558230233235487388849158556", / , "+277145.35491132254485207055096587993977783302753", "" ); // -56380.329107833270343558230233235487388849158556 /  +277145.35491132254485207055096587993977783302753 =
    DECIMAL_OP_TEST( "-597147583001091464692.45462985860111824175838980", / , "+1184055350.1862902754587418620655046842893720190", "" ); // -597147583001091464692.45462985860111824175838980 /  +1184055350.1862902754587418620655046842893720190 =
    DECIMAL_OP_TEST( "-1099062944468939421856224.7410255529071444836214", / , "+5235415554108378043687227.9880018482293335743662", "" ); // -1099062944468939421856224.7410255529071444836214 /  +5235415554108378043687227.9880018482293335743662 =
    DECIMAL_OP_TEST( "-2632041719345252483380694049835872197434.9766493", / , "+18458345086503572578057123591092037706279741.151", "" ); // -2632041719345252483380694049835872197434.9766493 /  +18458345086503572578057123591092037706279741.151 =
    DECIMAL_OP_TEST( "-6156169830305019652590488156540831.7027942076808", / , "+174233558770663615740569957179488308019576.48254", "" ); // -6156169830305019652590488156540831.7027942076808 /  +174233558770663615740569957179488308019576.48254 =
    DECIMAL_OP_TEST( "-781466404305113780602.80990487935255093934990570", / , "+8.3645424522417012204289292807950462439988531619", "" ); // -781466404305113780602.80990487935255093934990570 /  +8.3645424522417012204289292807950462439988531619 =
    DECIMAL_OP_TEST( "-277145.35491132254485207055096587993977783302753", / , "+56380.329107833270343558230233235487388849158556", "" ); // -277145.35491132254485207055096587993977783302753 /  +56380.329107833270343558230233235487388849158556 =
    DECIMAL_OP_TEST( "-1184055350.1862902754587418620655046842893720190", / , "+597147583001091464692.45462985860111824175838980", "" ); // -1184055350.1862902754587418620655046842893720190 /  +597147583001091464692.45462985860111824175838980 =
    DECIMAL_OP_TEST( "-5235415554108378043687227.9880018482293335743662", / , "+1099062944468939421856224.7410255529071444836214", "" ); // -5235415554108378043687227.9880018482293335743662 /  +1099062944468939421856224.7410255529071444836214 =
    cout << "---" << endl;
    DECIMAL_OP_TEST( "-2632041719345252483380694049835872197434.9766493", / , "+18458345086503572578057123591092037706279741.151", "" ); // -2632041719345252483380694049835872197434.9766493 /  +18458345086503572578057123591092037706279741.151 =
    DECIMAL_OP_TEST( "-6156169830305019652590488156540831.7027942076808", / , "+174233558770663615740569957179488308019576.48254", "" ); // -6156169830305019652590488156540831.7027942076808 /  +174233558770663615740569957179488308019576.48254 =
    DECIMAL_OP_TEST( "-781466404305113780602.80990487935255093934990570", / , "+8.3645424522417012204289292807950462439988531619", "" ); // -781466404305113780602.80990487935255093934990570 /  +8.3645424522417012204289292807950462439988531619 =
    DECIMAL_OP_TEST( "-277145.35491132254485207055096587993977783302753", / , "+56380.329107833270343558230233235487388849158556", "" ); // -277145.35491132254485207055096587993977783302753 /  +56380.329107833270343558230233235487388849158556 =
    DECIMAL_OP_TEST( "-1184055350.1862902754587418620655046842893720190", / , "+597147583001091464692.45462985860111824175838980", "" ); // -1184055350.1862902754587418620655046842893720190 /  +597147583001091464692.45462985860111824175838980 =
    DECIMAL_OP_TEST( "-5235415554108378043687227.9880018482293335743662", / , "+1099062944468939421856224.7410255529071444836214", "" ); // -5235415554108378043687227.9880018482293335743662 /  +1099062944468939421856224.7410255529071444836214 =
    DECIMAL_OP_TEST( "-18458345086503572578057123591092037706279741.151", / , "+2632041719345252483380694049835872197434.9766493", "" ); // -18458345086503572578057123591092037706279741.151 /  +2632041719345252483380694049835872197434.9766493 =
    DECIMAL_OP_TEST( "-174233558770663615740569957179488308019576.48254", / , "+6156169830305019652590488156540831.7027942076808", "" ); // -174233558770663615740569957179488308019576.48254 /  +6156169830305019652590488156540831.7027942076808 =
    DECIMAL_OP_TEST( "-8.3645424522417012204289292807950462439988531619", / , "+781466404305113780602.80990487935255093934990570", "" ); // -8.3645424522417012204289292807950462439988531619 /  +781466404305113780602.80990487935255093934990570 =
    DECIMAL_OP_TEST( "-56380.329107833270343558230233235487388849158556", / , "+277145.35491132254485207055096587993977783302753", "" ); // -56380.329107833270343558230233235487388849158556 /  +277145.35491132254485207055096587993977783302753 =
    DECIMAL_OP_TEST( "-597147583001091464692.45462985860111824175838980", / , "+1184055350.1862902754587418620655046842893720190", "" ); // -597147583001091464692.45462985860111824175838980 /  +1184055350.1862902754587418620655046842893720190 =
    DECIMAL_OP_TEST( "-1099062944468939421856224.7410255529071444836214", / , "+5235415554108378043687227.9880018482293335743662", "" ); // -1099062944468939421856224.7410255529071444836214 /  +5235415554108378043687227.9880018482293335743662 =
    cout << "---" << endl;
    DECIMAL_OP_TEST( "+18458345086503572578057123591092037706279741.151", / , "-2632041719345252483380694049835872197434.9766493", "" ); // +18458345086503572578057123591092037706279741.151 /  -2632041719345252483380694049835872197434.9766493 =
    DECIMAL_OP_TEST( "+174233558770663615740569957179488308019576.48254", / , "-6156169830305019652590488156540831.7027942076808", "" ); // +174233558770663615740569957179488308019576.48254 /  -6156169830305019652590488156540831.7027942076808 =
    DECIMAL_OP_TEST( "+8.3645424522417012204289292807950462439988531619", / , "-781466404305113780602.80990487935255093934990570", "" ); // +8.3645424522417012204289292807950462439988531619 /  -781466404305113780602.80990487935255093934990570 =
    DECIMAL_OP_TEST( "+56380.329107833270343558230233235487388849158556", / , "-277145.35491132254485207055096587993977783302753", "" ); // +56380.329107833270343558230233235487388849158556 /  -277145.35491132254485207055096587993977783302753 =
    DECIMAL_OP_TEST( "+597147583001091464692.45462985860111824175838980", / , "-1184055350.1862902754587418620655046842893720190", "" ); // +597147583001091464692.45462985860111824175838980 /  -1184055350.1862902754587418620655046842893720190 =
    DECIMAL_OP_TEST( "+1099062944468939421856224.7410255529071444836214", / , "-5235415554108378043687227.9880018482293335743662", "" ); // +1099062944468939421856224.7410255529071444836214 /  -5235415554108378043687227.9880018482293335743662 =
    DECIMAL_OP_TEST( "+2632041719345252483380694049835872197434.9766493", / , "-18458345086503572578057123591092037706279741.151", "" ); // +2632041719345252483380694049835872197434.9766493 /  -18458345086503572578057123591092037706279741.151 =
    DECIMAL_OP_TEST( "+6156169830305019652590488156540831.7027942076808", / , "-174233558770663615740569957179488308019576.48254", "" ); // +6156169830305019652590488156540831.7027942076808 /  -174233558770663615740569957179488308019576.48254 =
    DECIMAL_OP_TEST( "+781466404305113780602.80990487935255093934990570", / , "-8.3645424522417012204289292807950462439988531619", "" ); // +781466404305113780602.80990487935255093934990570 /  -8.3645424522417012204289292807950462439988531619 =
    DECIMAL_OP_TEST( "+277145.35491132254485207055096587993977783302753", / , "-56380.329107833270343558230233235487388849158556", "" ); // +277145.35491132254485207055096587993977783302753 /  -56380.329107833270343558230233235487388849158556 =
    DECIMAL_OP_TEST( "+1184055350.1862902754587418620655046842893720190", / , "-597147583001091464692.45462985860111824175838980", "" ); // +1184055350.1862902754587418620655046842893720190 /  -597147583001091464692.45462985860111824175838980 =
    DECIMAL_OP_TEST( "+5235415554108378043687227.9880018482293335743662", / , "-1099062944468939421856224.7410255529071444836214", "" ); // +5235415554108378043687227.9880018482293335743662 /  -1099062944468939421856224.7410255529071444836214 =
    */
    marty::Decimal::setDivisionPrecision(18);

    cout << endl;


    cout << "-------" << endl;
    DECIMAL_OP_TEST(         "3", + ,      "4.12", "7.12" );
    DECIMAL_OP_TEST(  "35.64745", + , "89.253464", "124.900914" );

    DECIMAL_OP_TEST(         "3", - ,      "4.12", "-1.12" );
    DECIMAL_OP_TEST(  "35.64745", - , "89.253464", "-53.606014" );
    DECIMAL_OP_TEST(      "4.12", - ,         "3", "1.12" );
    DECIMAL_OP_TEST( "89.253464", - ,  "35.64745", "53.606014" );
    //DECIMAL_OP_TEST( 4.12, * , 3.457, "14.24284" );
    DECIMAL_OP_TEST( "89.253464", * ,  "35.64745", "3181.6583952668" ); // 89.253464*35.64745 = 3181.6583952668
    DECIMAL_OP_TEST( "89.253464", / ,  "35.64745", "2.503782570702813244" ); // 89.253464/35.64745 = 2.5037825707 0281324470 6143076152 712185584102088648697171887470211754276953891512576635916454052113124501191529828921844339496934563341837915
    DECIMAL_OP_TEST( "0.0000001", / ,  "3000000000000000000000", "0.0000000000000000000000000000333" );


                                            // 3181.6583952668
                                            // 3181.653790004
    // Это с округлением (для того, чтобы результат влезал в int64) перед умножением
    // Сильно бьёт по точности
    // 89.253464 * 35.64745 = 3181.653790004 (expected '3181.659678575')
    // 89.25346 * 35.6474 = 3181.653790004 (expected '3181.659678575')




    cout << endl;

    cout << "Failed " << totalOpTestsFailed << " OP tests from total " << totalOpTests << endl;
    if (!totalOpTestsFailed)
        cout << "+++ All OP tests passed"  << endl;

    cout << "------------------------------" << endl << endl << endl;



    #define DECIMAL_MIN_PART_TEST( val, strMinOne, strMinTwo, strMinFive )                  \
                do                                                                          \
                {                                                                           \
                    ++totalMinPartTests;                                                    \
                    Decimal     d        = Decimal(val);                                    \
                                                                                            \
                    Decimal     min1     = d.makeMinimalPrecisionOne();                     \
                    Decimal     min2     = d.makeMinimalPrecisionTwo();                     \
                    Decimal     min5     = d.makeMinimalPrecisionFive();                    \
                                                                                            \
                    bool bGood /* Johny */ = min1.checkIsExact(strMinOne) && min2.checkIsExact(strMinTwo) && min5.checkIsExact(strMinFive);  \
                    if (!bGood)                                                             \
                       ++totalMinPartTestsFailed;                                                \
                                                                                            \
                                                                                            \
                    cout << "[" << (bGood ? "+" : "-") << "]  " << d << " / " << min1 << " / " << min2 << " / " << min5 << "" << endl; \
                    if (!bGood)                                                             \
                    {                                                                       \
                        cout << "    expected " << strMinOne << " / " << strMinTwo << " / " << strMinFive << "" << endl;                 \
                        cout << __FILE__ << ":" << __LINE__ << endl;                        \
                    }                                                                       \
                                                                                            \
                    if (!bGood)                                                             \
                        THROW_ERROR();                                                      \
                                                                                            \
                } while(0)


    unsigned totalMinPartTests       = 0;
    unsigned totalMinPartTestsFailed = 0;

    cout << "DECIMAL_MIN_PART_TEST test suite" << endl;

    DECIMAL_MIN_PART_TEST(               "123.01",                  "0.01",                  "0.02",                  "0.05" );
    DECIMAL_MIN_PART_TEST(               "13.101",                 "0.001",                 "0.002",                 "0.005" );
    DECIMAL_MIN_PART_TEST(               "3.1415",                "0.0001",                "0.0002",                "0.0005" );
    DECIMAL_MIN_PART_TEST(           "-100.10101",               "0.00001",               "0.00002",               "0.00005" );
    DECIMAL_MIN_PART_TEST(   "3.1415926535897932",    "0.0000000000000001",    "0.0000000000000002",    "0.0000000000000005" );
    DECIMAL_MIN_PART_TEST(             "3.141593",              "0.000001",              "0.000002",              "0.000005" );
    DECIMAL_MIN_PART_TEST(            "3.1415926",             "0.0000001",             "0.0000002",             "0.0000005" );
    DECIMAL_MIN_PART_TEST(           "2968802.55",                  "0.01",                  "0.02",                  "0.05" );
    DECIMAL_MIN_PART_TEST(       "986.9313322541",          "0.0000000001",          "0.0000000002",          "0.0000000005" );
    DECIMAL_MIN_PART_TEST(     "98.6959606587986",       "0.0000000000001",       "0.0000000000002",       "0.0000000000005" );
    DECIMAL_MIN_PART_TEST(          "942.4777962",             "0.0000001",             "0.0000002",             "0.0000005" );
    DECIMAL_MIN_PART_TEST(          "9424.777962",              "0.000001",              "0.000002",              "0.000005" );
    DECIMAL_MIN_PART_TEST(       "0.009424777962",        "0.000000000001",        "0.000000000002",        "0.000000000005" );
    DECIMAL_MIN_PART_TEST(             "0.019881",              "0.000001",              "0.000002",              "0.000005" );
    DECIMAL_MIN_PART_TEST(                "3.525",                 "0.001",                 "0.002",                 "0.005" );
    DECIMAL_MIN_PART_TEST(                  "0.2",                   "0.1",                   "0.2",                   "0.5" );
    DECIMAL_MIN_PART_TEST(                 "1.89",                  "0.01",                  "0.02",                  "0.05" );
    DECIMAL_MIN_PART_TEST( "8.030965211883154708",  "0.000000000000000001",  "0.000000000000000002",  "0.000000000000000005" );
    DECIMAL_MIN_PART_TEST( "0.005291005291005291",  "0.000000000000000001",  "0.000000000000000002",  "0.000000000000000005" );
    DECIMAL_MIN_PART_TEST( "0.000332443386243386",  "0.000000000000000001",  "0.000000000000000002",  "0.000000000000000005" );
    DECIMAL_MIN_PART_TEST( "9.997050731580937803",  "0.000000000000000001",  "0.000000000000000002",  "0.000000000000000005" );
    DECIMAL_MIN_PART_TEST( "9.999991552055621785",  "0.000000000000000001",  "0.000000000000000002",  "0.000000000000000005" );
    DECIMAL_MIN_PART_TEST( "5.492965842668411077",  "0.000000000000000001",  "0.000000000000000002",  "0.000000000000000005" );
    DECIMAL_MIN_PART_TEST( "4.929658426684110778",  "0.000000000000000001",  "0.000000000000000002",  "0.000000000000000005" );
    DECIMAL_MIN_PART_TEST( "1047.197551333333333",     "0.000000000000001",     "0.000000000000002",     "0.000000000000005" );
    DECIMAL_MIN_PART_TEST( "7.304964539007092198",  "0.000000000000000001",  "0.000000000000000002",  "0.000000000000000005" );
    DECIMAL_MIN_PART_TEST( "2.301472254944911079",  "0.000000000000000001",  "0.000000000000000002",  "0.000000000000000005" );


    // Decimal makeMinimalPrecisionOne() const;
    // Decimal makeMinimalPrecisionTwo() const;
    // Decimal makeMinimalPrecisionFive() const;


    cout << endl;

    cout << "Failed " << totalMinPartTestsFailed << " Min Part tests from total " << totalMinPartTests << endl;
    if (!totalMinPartTestsFailed)
        cout << "+++ All Min Part tests passed"  << endl;

    cout << "------------------------------" << endl << endl << endl;

    /*
    #define PRINT( var ) cout << #var << ": "<< var << endl;

    Decimal d1_u64_3   = (std::uint64_t)(3)  ;     PRINT(d1_u64_3);
    Decimal d1_i64_7   = (std::int64_t )(7)  ;     PRINT(d1_i64_7);
    Decimal d1_i64_m10 = (std::int64_t )(-10);     PRINT(d1_i64_m10);
    Decimal d1_i64_15_000 = Decimal(15, DecimalPrecision(3)); PRINT(d1_i64_15_000);
    Decimal d1 = Decimal::fromString("123.01");    PRINT(d1);
    Decimal d2 = Decimal::fromString("13.101");    PRINT(d2);
    Decimal d1_2_sum = d1 + d2;                    PRINT(d1_2_sum);  // 123.01 + 13.101 =  136.111
    Decimal d1_2_raz = d1 - d2;                    PRINT(d1_2_raz);  // 123.01 - 13.101 =  109.909
    Decimal d1_2_mul = d1 * d2;                    PRINT(d1_2_mul);  // 123.01 * 13.101 = 1611.55401
    Decimal d1_2_div = d1 / d2;                    PRINT(d1_2_div);  // 123.01 / 13.101 =    9.389359590870925883520341958629112281505228608503167697122357071979238226089611480039691626593389817571177772689107701702160140447294099687
    Decimal d1_2_ost = d1 % d2;                    PRINT(d1_2_ost);  // 123.01 % 13.101 =    5.101

    auto    d1_2_sum_plus_15_a = d1_2_sum + (std::int64_t )(15); PRINT(d1_2_sum_plus_15_a); // 136.111 + 15 = 151.111
    auto    d1_2_sum_plus_15_b = d1_2_sum + (std::uint64_t)(15); PRINT(d1_2_sum_plus_15_b);
    auto    d1_2_sum_plus_15_c = d1_2_sum + (int          )(15); PRINT(d1_2_sum_plus_15_c);
    auto    d1_2_sum_plus_15_d = d1_2_sum + (unsigned     )(15); PRINT(d1_2_sum_plus_15_d);
    auto    d1_2_sum_plus_15_e = d1_2_sum + (double       )(15); PRINT(d1_2_sum_plus_15_e);

    Decimal divideD1   = Decimal( 100, DecimalPrecision(2) );
    Decimal divideD2   = Decimal(  25, DecimalPrecision(1) );
    Decimal divideRes  = divideD1.divide(divideD2, 6);
    
    cout << divideRes.toString() << endl;
    */

    #if 1

    unsigned totalRoundingTests       = 0;
    unsigned totalRoundingTestsFailed = 0;

    auto roundingTest = [&totalRoundingTests, &totalRoundingTestsFailed]( double dblVal, const std::string &strResForCompare, int roundingPrecision, marty::Decimal::RoundingMethod roundingMethod )
                        {

                            ++totalRoundingTests;
                            Decimal decimal    = Decimal(dblVal);
                            Decimal roundedVal = decimal.rounded( roundingPrecision, roundingMethod );
                            /*std::ostringstream os;*/
                            std::string strRes = roundedVal.toString(roundingPrecision);

                            bool bGood /* Johny */ = roundedVal /* decimal */ .checkIsExact(strResForCompare); // Блджад, а как это раньше работало?
                            if (!bGood)
                               ++totalRoundingTestsFailed;


                            cout << "[" << (bGood ? "+" : "-") << "]  " << (decimal<0 ? "" : " ") << decimal.toString( /* roundingPrecision+1 */ ) << " rounded to " << roundingPrecision << " signs with " << marty::Decimal::getRoundingMethodName(roundingMethod) << " rounding method is " << strRes;
                            if (!bGood)
                            {
                                cout << " (expected " << strResForCompare << ")";
                            }
                            cout << endl;

                            if (!bGood)
                                THROW_ERROR();
                        
                        };

    #if 0
    #define ROUNDING_TEST( dblVal, strResForCompare, roundingPrecision, roundingMethod ) \
                do                                                                       \
                {                                                                        \
                    ++totalRoundingTests;                                                \
                    Decimal decimal    = Decimal(dblVal);                                \
                    Decimal roundedVal = decimal.rounded( roundingPrecision, Decimal::RoundingMethod::roundingMethod ); \
                    /*std::ostringstream os;*/                                           \
                    std::string strRes = roundedVal.toString(roundingPrecision);         \
                                                                                         \
                    bool bGood /* Johny */ = decimal.checkIsExact(strResForCompare);        \
                    if (!bGood)                                                          \
                       ++totalRoundingTestsFailed;                                       \
                                                                                         \
                                                                                         \
                    cout << "[" << (bGood ? "+" : "-") << "]  " << (decimal<0 ? "" : " ") << decimal.toString( /* roundingPrecision+1 */ ) << " rounded to " << roundingPrecision << " signs with " << #roundingMethod << " rounding method is " << strRes; \
                    if (!bGood)                                                          \
                    {                                                                    \
                        cout << " (expected " << strResForCompare << ")";                \
                    }                                                                    \
                    cout << endl;                                                        \
                                                                                         \
                    if (!bGood)                                                             \
                        THROW_ERROR();                                                      \
                                                                                            \
                } while(0)

    #else

    #define ROUNDING_TEST( dblVal, strResForCompare, roundingPrecision, roundingMethod ) \
             roundingTest( dblVal, strResForCompare, roundingPrecision, marty::Decimal::RoundingMethod:: roundingMethod )

    #endif

    // https://en.wikipedia.org/wiki/Rounding#Round_half_to_odd

    //cout << endl;
    //cout << "------------------------------" << endl;

    cout << "ROUNDING_TEST test suite" << endl;

    ROUNDING_TEST(   23.0  ,  "23", 0, roundFloor           );
    ROUNDING_TEST(   23.01 ,  "23", 0, roundFloor           );
    ROUNDING_TEST(   23.001,  "23", 0, roundFloor           );
    ROUNDING_TEST(   23.112,  "23", 0, roundFloor           );
    ROUNDING_TEST(   23.49 ,  "23", 0, roundFloor           );
    ROUNDING_TEST(   23.499,  "23", 0, roundFloor           );
    ROUNDING_TEST(   23.5  ,  "23", 0, roundFloor           );
    ROUNDING_TEST(   23.51 ,  "23", 0, roundFloor           );
    ROUNDING_TEST(   23.501,  "23", 0, roundFloor           );
    ROUNDING_TEST(   23.511,  "23", 0, roundFloor           );
    ROUNDING_TEST(   23.69 ,  "23", 0, roundFloor           );
    ROUNDING_TEST(   23.699,  "23", 0, roundFloor           );
    ROUNDING_TEST(   23.7  ,  "23", 0, roundFloor           );
    ROUNDING_TEST(   23.71 ,  "23", 0, roundFloor           );
    ROUNDING_TEST(   23.701,  "23", 0, roundFloor           );
    ROUNDING_TEST(   23.911,  "23", 0, roundFloor           );
    ROUNDING_TEST(   23.999,  "23", 0, roundFloor           );
    ROUNDING_TEST(   24.0  ,  "24", 0, roundFloor           );
    ROUNDING_TEST(   24.01 ,  "24", 0, roundFloor           );
    ROUNDING_TEST(   24.001,  "24", 0, roundFloor           );

    cout << endl;
    ROUNDING_TEST(  -23.0  , "-23", 0, roundFloor           );
    ROUNDING_TEST(  -23.01 , "-24", 0, roundFloor           );
    ROUNDING_TEST(  -23.001, "-24", 0, roundFloor           );
    ROUNDING_TEST(  -23.112, "-24", 0, roundFloor           );
    ROUNDING_TEST(  -23.2  , "-24", 0, roundFloor           );
    ROUNDING_TEST(  -23.21 , "-24", 0, roundFloor           );
    ROUNDING_TEST(  -23.49 , "-24", 0, roundFloor           );
    ROUNDING_TEST(  -23.499, "-24", 0, roundFloor           );
    ROUNDING_TEST(  -23.5  , "-24", 0, roundFloor           );
    ROUNDING_TEST(  -23.51 , "-24", 0, roundFloor           );
    ROUNDING_TEST(  -23.501, "-24", 0, roundFloor           );
    ROUNDING_TEST(  -23.511, "-24", 0, roundFloor           );
    ROUNDING_TEST(  -23.911, "-24", 0, roundFloor           );
    ROUNDING_TEST(  -23.999, "-24", 0, roundFloor           );
    ROUNDING_TEST(  -24.0  , "-24", 0, roundFloor           );
    ROUNDING_TEST(  -24.01 , "-25", 0, roundFloor           );
    ROUNDING_TEST(  -24.011, "-25", 0, roundFloor           );

    cout << endl;
    cout << "------------------------------" << endl;
    ROUNDING_TEST(   23.0  ,  "23", 0, roundCeil            );
    ROUNDING_TEST(   23.01 ,  "24", 0, roundCeil            );
    ROUNDING_TEST(   23.001,  "24", 0, roundCeil            );
    ROUNDING_TEST(   23.2  ,  "24", 0, roundCeil            );
    ROUNDING_TEST(   23.49 ,  "24", 0, roundCeil            );
    ROUNDING_TEST(   23.499,  "24", 0, roundCeil            );
    ROUNDING_TEST(   23.5  ,  "24", 0, roundCeil            );
    ROUNDING_TEST(   23.51 ,  "24", 0, roundCeil            );
    ROUNDING_TEST(   23.501,  "24", 0, roundCeil            );
    ROUNDING_TEST(   23.999,  "24", 0, roundCeil            );
    ROUNDING_TEST(   24.0  ,  "24", 0, roundCeil            );
    ROUNDING_TEST(   24.001,  "25", 0, roundCeil            );

    cout << endl;
    ROUNDING_TEST(  -23.0  , "-23", 0, roundCeil            );
    ROUNDING_TEST(  -23.01 , "-23", 0, roundCeil            );
    ROUNDING_TEST(  -23.001, "-23", 0, roundCeil            );
    ROUNDING_TEST(  -23.49 , "-23", 0, roundCeil            );
    ROUNDING_TEST(  -23.499, "-23", 0, roundCeil            );
    ROUNDING_TEST(  -23.5  , "-23", 0, roundCeil            );
    ROUNDING_TEST(  -23.51 , "-23", 0, roundCeil            );
    ROUNDING_TEST(  -23.501, "-23", 0, roundCeil            );
    ROUNDING_TEST(  -23.7  , "-23", 0, roundCeil            );
    ROUNDING_TEST(  -23.701, "-23", 0, roundCeil            );
    ROUNDING_TEST(  -23.999, "-23", 0, roundCeil            );
    ROUNDING_TEST(  -24.0  , "-24", 0, roundCeil            );
    ROUNDING_TEST(  -24.01 , "-24", 0, roundCeil            );

    cout << endl;
    cout << "------------------------------" << endl;
    ROUNDING_TEST(   23.0  ,  "23", 0, roundTrunc           );
    ROUNDING_TEST(   23.01 ,  "23", 0, roundTrunc           );
    ROUNDING_TEST(   23.49 ,  "23", 0, roundTrunc           );
    ROUNDING_TEST(   23.499,  "23", 0, roundTrunc           );
    ROUNDING_TEST(   23.5  ,  "23", 0, roundTrunc           );
    ROUNDING_TEST(   23.51 ,  "23", 0, roundTrunc           );
    ROUNDING_TEST(   23.7  ,  "23", 0, roundTrunc           );
    ROUNDING_TEST(   23.99 ,  "23", 0, roundTrunc           );
    ROUNDING_TEST(   24.0  ,  "24", 0, roundTrunc           );
    ROUNDING_TEST(   24.01 ,  "24", 0, roundTrunc           );

    cout << endl;
    ROUNDING_TEST(  -23.0  , "-23", 0, roundTrunc           );
    ROUNDING_TEST(  -23.01 , "-23", 0, roundTrunc           );
    ROUNDING_TEST(  -23.49 , "-23", 0, roundTrunc           );
    ROUNDING_TEST(  -23.499, "-23", 0, roundTrunc           );
    ROUNDING_TEST(  -23.5  , "-23", 0, roundTrunc           );
    ROUNDING_TEST(  -23.51 , "-23", 0, roundTrunc           );
    ROUNDING_TEST(  -23.7  , "-23", 0, roundTrunc           );
    ROUNDING_TEST(  -23.99 , "-23", 0, roundTrunc           );
    ROUNDING_TEST(  -24.0  , "-24", 0, roundTrunc           );
    ROUNDING_TEST(  -24.01 , "-24", 0, roundTrunc           );

    cout << endl;
    cout << "------------------------------" << endl;
    ROUNDING_TEST(   23.0  ,  "23", 0, roundTowardsInf      );
    ROUNDING_TEST(   23.01 ,  "24", 0, roundTowardsInf      );
    ROUNDING_TEST(   23.11 ,  "24", 0, roundTowardsInf      );
    ROUNDING_TEST(   23.2  ,  "24", 0, roundTowardsInf      );
    ROUNDING_TEST(   23.49 ,  "24", 0, roundTowardsInf      );
    ROUNDING_TEST(   23.499,  "24", 0, roundTowardsInf      );
    ROUNDING_TEST(   23.5  ,  "24", 0, roundTowardsInf      );
    ROUNDING_TEST(   23.51 ,  "24", 0, roundTowardsInf      );
    ROUNDING_TEST(   23.7  ,  "24", 0, roundTowardsInf      );
    ROUNDING_TEST(   23.99 ,  "24", 0, roundTowardsInf      );
    ROUNDING_TEST(   24.0  ,  "24", 0, roundTowardsInf      );
    ROUNDING_TEST(   24.01 ,  "25", 0, roundTowardsInf      );

    cout << endl;
    ROUNDING_TEST(  -23.0  , "-23", 0, roundTowardsInf      );
    ROUNDING_TEST(  -23.01 , "-24", 0, roundTowardsInf      );
    ROUNDING_TEST(  -23.2  , "-24", 0, roundTowardsInf      );
    ROUNDING_TEST(  -23.49 , "-24", 0, roundTowardsInf      );
    ROUNDING_TEST(  -23.499, "-24", 0, roundTowardsInf      );
    ROUNDING_TEST(  -23.5  , "-24", 0, roundTowardsInf      );
    ROUNDING_TEST(  -23.5  , "-24", 0, roundTowardsInf      );
    ROUNDING_TEST(  -23.51 , "-24", 0, roundTowardsInf      );
    ROUNDING_TEST(  -23.501, "-24", 0, roundTowardsInf      );
    ROUNDING_TEST(  -23.7  , "-24", 0, roundTowardsInf      );
    ROUNDING_TEST(  -23.99 , "-24", 0, roundTowardsInf      );
    ROUNDING_TEST(  -24.0  , "-24", 0, roundTowardsInf      );
    ROUNDING_TEST(  -24.01 , "-25", 0, roundTowardsInf      );

    cout << endl;
    cout << "------------------------------" << endl;
    ROUNDING_TEST(   23.0  ,  "23", 0, roundHalfUp          );
    ROUNDING_TEST(   23.1  ,  "23", 0, roundHalfUp          );
    ROUNDING_TEST(   23.01 ,  "23", 0, roundHalfUp          );
    ROUNDING_TEST(   23.3  ,  "23", 0, roundHalfUp          );
    ROUNDING_TEST(   23.49 ,  "23", 0, roundHalfUp          );
    ROUNDING_TEST(   23.499,  "23", 0, roundHalfUp          );
    ROUNDING_TEST(   23.5  ,  "24", 0, roundHalfUp          );
    ROUNDING_TEST(   23.51 ,  "24", 0, roundHalfUp          );
    ROUNDING_TEST(   23.501,  "24", 0, roundHalfUp          );
    ROUNDING_TEST(   23.7  ,  "24", 0, roundHalfUp          );
    ROUNDING_TEST(   23.9  ,  "24", 0, roundHalfUp          );
    ROUNDING_TEST(   23.99 ,  "24", 0, roundHalfUp          );
    ROUNDING_TEST(   24.0  ,  "24", 0, roundHalfUp          );
    ROUNDING_TEST(   24.01 ,  "24", 0, roundHalfUp          );

    cout << endl;
    ROUNDING_TEST(  -23.0  , "-23", 0, roundHalfUp          );
    ROUNDING_TEST(  -23.1  , "-23", 0, roundHalfUp          );
    ROUNDING_TEST(  -23.49 , "-23", 0, roundHalfUp          );
    ROUNDING_TEST(  -23.499, "-23", 0, roundHalfUp          );
    ROUNDING_TEST(  -23.5  , "-23", 0, roundHalfUp          );
    ROUNDING_TEST(  -23.51 , "-24", 0, roundHalfUp          );
    ROUNDING_TEST(  -23.501, "-24", 0, roundHalfUp          );
    ROUNDING_TEST(  -23.9  , "-24", 0, roundHalfUp          );
    ROUNDING_TEST(  -23.99 , "-24", 0, roundHalfUp          );
    ROUNDING_TEST(  -24.0  , "-24", 0, roundHalfUp          );
    ROUNDING_TEST(  -24.01 , "-24", 0, roundHalfUp          );

    cout << endl;
    cout << "------------------------------" << endl;
    ROUNDING_TEST(   23.0  ,  "23", 0, roundHalfDown        );
    ROUNDING_TEST(   23.1  ,  "23", 0, roundHalfDown        );
    ROUNDING_TEST(   23.3  ,  "23", 0, roundHalfDown        );
    ROUNDING_TEST(   23.49 ,  "23", 0, roundHalfDown        );
    ROUNDING_TEST(   23.499,  "23", 0, roundHalfDown        );
    ROUNDING_TEST(   23.5  ,  "23", 0, roundHalfDown        );
    ROUNDING_TEST(   23.51 ,  "24", 0, roundHalfDown        );
    ROUNDING_TEST(   23.501,  "24", 0, roundHalfDown        );
    ROUNDING_TEST(   23.9  ,  "24", 0, roundHalfDown        );
    ROUNDING_TEST(   23.99 ,  "24", 0, roundHalfDown        );
    ROUNDING_TEST(   24.0  ,  "24", 0, roundHalfDown        );
    ROUNDING_TEST(   24.01 ,  "24", 0, roundHalfDown        );

    cout << endl;
    ROUNDING_TEST(  -23.0  , "-23", 0, roundHalfDown        );
    ROUNDING_TEST(  -23.1  , "-23", 0, roundHalfDown        );
    ROUNDING_TEST(  -23.3  , "-23", 0, roundHalfDown        );
    ROUNDING_TEST(  -23.49 , "-23", 0, roundHalfDown        );
    ROUNDING_TEST(  -23.499, "-23", 0, roundHalfDown        );
    ROUNDING_TEST(  -23.5  , "-24", 0, roundHalfDown        );
    ROUNDING_TEST(  -23.51 , "-24", 0, roundHalfDown        );
    ROUNDING_TEST(  -23.501, "-24", 0, roundHalfDown        );
    ROUNDING_TEST(  -23.9  , "-24", 0, roundHalfDown        );
    ROUNDING_TEST(  -23.99 , "-24", 0, roundHalfDown        );
    ROUNDING_TEST(  -24.0  , "-24", 0, roundHalfDown        );
    ROUNDING_TEST(  -24.1  , "-24", 0, roundHalfDown        );

    cout << endl;
    cout << "------------------------------" << endl;
    ROUNDING_TEST(   23.0  ,  "23", 0, roundHalfTowardsZero );
    ROUNDING_TEST(   23.1  ,  "23", 0, roundHalfTowardsZero );
    ROUNDING_TEST(   23.49 ,  "23", 0, roundHalfTowardsZero );
    ROUNDING_TEST(   23.499,  "23", 0, roundHalfTowardsZero );
    ROUNDING_TEST(   23.5  ,  "23", 0, roundHalfTowardsZero );
    ROUNDING_TEST(   23.51 ,  "24", 0, roundHalfTowardsZero );
    ROUNDING_TEST(   23.511,  "24", 0, roundHalfTowardsZero );
    ROUNDING_TEST(   23.9  ,  "24", 0, roundHalfTowardsZero );
    ROUNDING_TEST(   23.99 ,  "24", 0, roundHalfTowardsZero );
    ROUNDING_TEST(   24.0  ,  "24", 0, roundHalfTowardsZero );
    ROUNDING_TEST(   24.1  ,  "24", 0, roundHalfTowardsZero );

    cout << endl;
    ROUNDING_TEST(  -23.0  , "-23", 0, roundHalfTowardsZero );
    ROUNDING_TEST(  -23.1  , "-23", 0, roundHalfTowardsZero );
    ROUNDING_TEST(  -23.49 , "-23", 0, roundHalfTowardsZero );
    ROUNDING_TEST(  -23.499, "-23", 0, roundHalfTowardsZero );
    ROUNDING_TEST(  -23.5  , "-23", 0, roundHalfTowardsZero );
    ROUNDING_TEST(  -23.51 , "-24", 0, roundHalfTowardsZero );
    ROUNDING_TEST(  -23.501, "-24", 0, roundHalfTowardsZero );
    ROUNDING_TEST(  -23.9  , "-24", 0, roundHalfTowardsZero );
    ROUNDING_TEST(  -23.99 , "-24", 0, roundHalfTowardsZero );
    ROUNDING_TEST(  -24.0  , "-24", 0, roundHalfTowardsZero );
    ROUNDING_TEST(  -24.1  , "-24", 0, roundHalfTowardsZero );

    // roundMath
    cout << endl;
    cout << "------------------------------" << endl;
    ROUNDING_TEST(   23.0  ,  "23", 0, roundHalfTowardsInf  );
    ROUNDING_TEST(   23.1  ,  "23", 0, roundHalfTowardsInf  );
    ROUNDING_TEST(   23.49 ,  "23", 0, roundHalfTowardsInf  );
    ROUNDING_TEST(   23.499,  "23", 0, roundHalfTowardsInf  );
    ROUNDING_TEST(   23.5  ,  "24", 0, roundHalfTowardsInf  );
    ROUNDING_TEST(   23.51 ,  "24", 0, roundHalfTowardsInf  );
    ROUNDING_TEST(   23.501,  "24", 0, roundHalfTowardsInf  );
    ROUNDING_TEST(   23.9  ,  "24", 0, roundHalfTowardsInf  );
    ROUNDING_TEST(   23.99 ,  "24", 0, roundHalfTowardsInf  );
    ROUNDING_TEST(   24.0  ,  "24", 0, roundHalfTowardsInf  );
    ROUNDING_TEST(   24.1  ,  "24", 0, roundHalfTowardsInf  );

    cout << endl;
    ROUNDING_TEST(  -23.0  , "-23", 0, roundHalfTowardsInf  );
    ROUNDING_TEST(  -23.1  , "-23", 0, roundHalfTowardsInf  );
    ROUNDING_TEST(  -23.49 , "-23", 0, roundHalfTowardsInf  );
    ROUNDING_TEST(  -23.499, "-23", 0, roundHalfTowardsInf  );
    ROUNDING_TEST(  -23.5  , "-24", 0, roundHalfTowardsInf  );
    ROUNDING_TEST(  -23.51 , "-24", 0, roundHalfTowardsInf  );
    ROUNDING_TEST(  -23.501, "-24", 0, roundHalfTowardsInf  );
    ROUNDING_TEST(  -23.9  , "-24", 0, roundHalfTowardsInf  );
    ROUNDING_TEST(  -23.99 , "-24", 0, roundHalfTowardsInf  );
    ROUNDING_TEST(  -24.0  , "-24", 0, roundHalfTowardsInf  );
    ROUNDING_TEST(  -24.1  , "-24", 0, roundHalfTowardsInf  );

    cout << endl;
    cout << "------------------------------" << endl;
    ROUNDING_TEST(   23.0  ,  "23", 0, roundHalfToEven      );
    ROUNDING_TEST(   23.1  ,  "23", 0, roundHalfToEven      );
    ROUNDING_TEST(   23.49 ,  "23", 0, roundHalfToEven      );
    ROUNDING_TEST(   23.499,  "23", 0, roundHalfToEven      );
    ROUNDING_TEST(   23.5  ,  "24", 0, roundHalfToEven      );
    ROUNDING_TEST(   23.51 ,  "24", 0, roundHalfToEven      );
    ROUNDING_TEST(   23.501,  "24", 0, roundHalfToEven      );
    ROUNDING_TEST(   23.9  ,  "24", 0, roundHalfToEven      );
    ROUNDING_TEST(   23.99 ,  "24", 0, roundHalfToEven      );

    cout << endl;
    ROUNDING_TEST(   24.0  ,  "24", 0, roundHalfToEven      );
    ROUNDING_TEST(   24.1  ,  "24", 0, roundHalfToEven      );
    ROUNDING_TEST(   24.49 ,  "24", 0, roundHalfToEven      );
    ROUNDING_TEST(   24.499,  "24", 0, roundHalfToEven      );
    ROUNDING_TEST(   24.5  ,  "24", 0, roundHalfToEven      );
    ROUNDING_TEST(   24.51 ,  "25", 0, roundHalfToEven      );
    ROUNDING_TEST(   24.501,  "25", 0, roundHalfToEven      );
    ROUNDING_TEST(   24.9  ,  "25", 0, roundHalfToEven      );
    ROUNDING_TEST(   24.99 ,  "25", 0, roundHalfToEven      );

    cout << endl;
    ROUNDING_TEST(  -23.0  , "-23", 0, roundHalfToEven      );
    ROUNDING_TEST(  -23.1  , "-23", 0, roundHalfToEven      );
    ROUNDING_TEST(  -23.49 , "-23", 0, roundHalfToEven      );
    ROUNDING_TEST(  -23.499, "-23", 0, roundHalfToEven      );
    ROUNDING_TEST(  -23.5  , "-24", 0, roundHalfToEven      );
    ROUNDING_TEST(  -23.51 , "-24", 0, roundHalfToEven      );
    ROUNDING_TEST(  -23.501, "-24", 0, roundHalfToEven      );
    ROUNDING_TEST(  -23.9  , "-24", 0, roundHalfToEven      );
    ROUNDING_TEST(  -23.99 , "-24", 0, roundHalfToEven      );

    cout << endl;
    ROUNDING_TEST(  -24.0  , "-24", 0, roundHalfToEven      );
    ROUNDING_TEST(  -24.1  , "-24", 0, roundHalfToEven      );
    ROUNDING_TEST(  -24.49 , "-24", 0, roundHalfToEven      );
    ROUNDING_TEST(  -24.499, "-24", 0, roundHalfToEven      );
    ROUNDING_TEST(  -24.5  , "-24", 0, roundHalfToEven      );
    ROUNDING_TEST(  -24.51 , "-25", 0, roundHalfToEven      );
    ROUNDING_TEST(  -24.501, "-25", 0, roundHalfToEven      );
    ROUNDING_TEST(  -24.9  , "-25", 0, roundHalfToEven      );
    ROUNDING_TEST(  -24.99 , "-25", 0, roundHalfToEven      );

    /*
    cout << "------------------------------" << endl;
    ROUNDING_TEST(   23.5,  "23", 0, roundHalfToOdd       );
    ROUNDING_TEST(   22.5,  "23", 0, roundHalfToOdd       );
    ROUNDING_TEST(  -23.5, "-23", 0, roundHalfToOdd       );
    ROUNDING_TEST(  -22.5, "-23", 0, roundHalfToOdd       );
    */

    cout << endl;

    cout << "Failed " << totalRoundingTestsFailed << " rounding tests from total " << totalRoundingTests << endl;
    if (!totalRoundingTestsFailed)
        cout << "+++ All rounding tests passed"  << endl;

    cout << "------------------------------" << endl << endl << endl << endl << endl;


    #endif // Rounding tests

    cout << "***********************" << endl;
    cout << "* For sweet" << endl;
    cout << "***********************" << endl;
    cout << endl << endl << endl;
    
    // return marty::for_decimal_tests::getMillisecTick();

    #if !defined(MARTY_DECIMAL_MULLER_TEST_NO_BOOST)

        typedef boost::multiprecision::number<boost::multiprecision::cpp_dec_float<50> >    cpp_float_normal;
        typedef boost::multiprecision::number<boost::multiprecision::cpp_dec_float<200> >   cpp_float_long  ; // Стандартно есть только cpp_dec_float_100

        std::uint32_t cppFloatT1     = mullersRecurrenceFunctionTest<cpp_float_normal>( 64, "cpp_float_normal" ); 
        std::uint32_t cppFloatT2     = mullersRecurrenceFunctionTest<cpp_float_long>  ( 176, "cpp_float_long" ); 

    #endif

    // Достаточно 20 итераций, и процесс сходится в ошибочной точке
    // В расколбас же активно начинает сваливаться на 15ом шаге
    std::uint32_t doubleT     = mullersRecurrenceFunctionTest<double>( 24, "double" ); 

    // marty::Decimal::setDivisionPrecision(50); // 18 - default
    // Достаточно 20 итераций (99.48), и процесс начинает сходится в ошибочную точку, самое годное - на 14-15 (4.997662  - 5.003854)
    std::uint32_t decimalT18  = mullersRecurrenceFunctionTest<marty::Decimal>( 42, "marty::Decimal with precision 18" ); // 

    marty::Decimal::setDivisionPrecision(16); // 16 - примерно как у double
    std::uint32_t decimalT16  = mullersRecurrenceFunctionTest<marty::Decimal>( 20, "marty::Decimal with precision 16" ); // 

    marty::Decimal::setDivisionPrecision(15); // 15 - примерно как у double
    std::uint32_t decimalT15  = mullersRecurrenceFunctionTest<marty::Decimal>( 20, "marty::Decimal with precision 15" ); // 

    marty::Decimal::setDivisionPrecision(12); // 12 - хуже, чем у double
    std::uint32_t decimalT12  = mullersRecurrenceFunctionTest<marty::Decimal>( 20, "marty::Decimal with precision 12" ); // 

    marty::Decimal::setDivisionPrecision(9); // 9 - хуже, чем у double
    std::uint32_t decimalT9  = mullersRecurrenceFunctionTest<marty::Decimal>( 20, "marty::Decimal with precision 9" ); // 

    marty::Decimal::setDivisionPrecision(6); // 6 - совсем хуже, чем у double
    std::uint32_t decimalT6  = mullersRecurrenceFunctionTest<marty::Decimal>( 20, "marty::Decimal with precision 6" ); // 

    marty::Decimal::setDivisionPrecision(3); // 3 - ваще хуже, чем у double
    std::uint32_t decimalT3  = mullersRecurrenceFunctionTest<marty::Decimal>( 20, "marty::Decimal with precision 3" ); // 

    // Начиная с 41 итераций процесс уходит от правильной точки и на 46ой - 99.992  , самое годное - на 35-36 (4.9999999 - 5.0000000)
    marty::Decimal::setDivisionPrecision(50);
    std::uint32_t decimalT50  = mullersRecurrenceFunctionTest<marty::Decimal>( 50, "marty::Decimal with precision 50" );

    // Начиная с 79 итераций процесс уходит от правильной точки и на 84ой - 99.79   , самое годное - на 69-70 (4.999999999999999681 - 5.000000000000025473)
    marty::Decimal::setDivisionPrecision(100);
    std::uint32_t decimalT100 = mullersRecurrenceFunctionTest<marty::Decimal>( 90, "marty::Decimal with precision 100" );

    // Начиная со 156 итераций процесс уходит от правильной точки и на 161ой - 99.96, самое годное - на 134-135 (4.999999999999999999999999999995 - 5.000000000000000000000000000034)
    marty::Decimal::setDivisionPrecision(200);
    std::uint32_t decimalT200 = mullersRecurrenceFunctionTest<marty::Decimal>( 170, "marty::Decimal with precision 200" );

    #if !defined(MARTY_DECIMAL_MULLER_TEST_NO_BOOST)

        cout << "Elapsed for cpp_float_normal   : " << cppFloatT1 << endl;
        cout << "Elapsed for cpp_float_long     : " << cppFloatT2 << endl;

    #endif
    
    cout << "Muller's Recurrence Test: Elapsed for double             : " << doubleT     << endl;
    cout << "Muller's Recurrence Test: Elapsed for marty::Decimal(3)  : " << decimalT3   << endl;
    cout << "Muller's Recurrence Test: Elapsed for marty::Decimal(6)  : " << decimalT6   << endl;
    cout << "Muller's Recurrence Test: Elapsed for marty::Decimal(9)  : " << decimalT9   << endl;
    cout << "Muller's Recurrence Test: Elapsed for marty::Decimal(12) : " << decimalT12  << endl;
    cout << "Muller's Recurrence Test: Elapsed for marty::Decimal(15) : " << decimalT15  << endl;
    cout << "Muller's Recurrence Test: Elapsed for marty::Decimal(16) : " << decimalT16  << endl;
    cout << "Muller's Recurrence Test: Elapsed for marty::Decimal(18) : " << decimalT18  << endl;
    cout << "Muller's Recurrence Test: Elapsed for marty::Decimal(50) : " << decimalT50  << endl;
    cout << "Muller's Recurrence Test: Elapsed for marty::Decimal(100): " << decimalT100 << endl;
    cout << "Muller's Recurrence Test: Elapsed for marty::Decimal(200): " << decimalT200 << endl;

    cout << "------------------------------" << endl << endl << endl;


    return 0;
}

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
template< typename SomeFloatingType > inline
std::uint32_t mullersRecurrenceFunctionTest( std::size_t testSize, const char *testTitle )
{
    std::vector< SomeFloatingType >  testResults  = std::vector< SomeFloatingType > ( (std::size_t)testSize, SomeFloatingType(0.0) );

    std::uint32_t startTick = marty::for_decimal_tests::getMillisecTick();

        bool isPerformed = marty::for_decimal_tests::mullersRecurrenceFunctionTest( testResults );

    std::uint32_t endTick   = marty::for_decimal_tests::getMillisecTick();

    std::uint32_t deltaTick = endTick - startTick;


    using std::cout;
    using std::endl;


    cout << "------------------------------" << endl;
    cout << "Muller's Recurrence Test for " << testTitle << endl;

    cout << "Execution time: " << deltaTick << " ms" << endl << endl;

    if (isPerformed)
    {
        cout << "Test results:" << endl << endl;

        std::size_t i = 0;

        auto fmtWidth = cout.width();

        for( const auto &v : testResults )
        {
            ++i;
            cout << "    " << std::setw(3) << i << std::setw(fmtWidth) << ": " << v << endl;
        }
    }
    else
    {
        cout << "!!! Test not performed" << endl;
    }

    cout << endl << endl << endl;


    return deltaTick;

}



