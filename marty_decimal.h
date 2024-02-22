#pragma once

#ifndef MARTY_DECIMAL_ASSERT_FAIL

    #define MARTY_DECIMAL_ASSERT_FAIL(msg)    throw std::runtime_error(msg)

#endif

//----------------------------------------------------------------------------
#ifndef MARTY_ARG_USED

    //! Подавление варнинга о неиспользованном аргументе
    #define MARTY_ARG_USED(x)                   (void)(x)

#endif

//----------------------------------------------------------------------------


#ifdef USE_MARTY_INT_DECIMAL
    #include "marty_int_decimal.h"
#else
    #include "marty_bcd_decimal.h"
#endif


#include <vector>


#if defined(WIN32) || defined(_WIN32)

    #include <winsock2.h>
    #include <windows.h>

#endif


namespace marty{

namespace for_decimal_tests{

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
inline
std::uint32_t getMillisecTick()
{
    #if defined(WIN32) || defined(_WIN32)

        // https://devblogs.microsoft.com/cppblog/visual-c-compiler-version/
        // https://learn.microsoft.com/en-us/cpp/overview/compiler-versions?view=msvc-170

        #if defined(_MSC_VER) && _MSC_VER>1929
            #pragma warning(push)
            #pragma warning(disable:28159) // warning C28159: Consider using 'GetTickCount64' instead of 'GetTickCount'
        #endif

        return (std::uint32_t)GetTickCount();

        #if defined(_MSC_VER) && _MSC_VER>1929
            #pragma warning(pop)
        #endif

    #else // Linups users can add native millisec counter getter or new std chrono fns

        return 0;

    #endif
}

//----------------------------------------------------------------------------





//----------------------------------------------------------------------------
// Muller's Recurrence - https://latkin.org/blog/2014/11/22/mullers-recurrence-roundoff-gone-wrong/
template< typename SomeFloatingType > inline
SomeFloatingType mullersRecurrenceFunction( const SomeFloatingType &y, const SomeFloatingType &z )
{
    return SomeFloatingType(108) - ( (SomeFloatingType(815) - SomeFloatingType(1500) / z) / y);
}

//----------------------------------------------------------------------------
template< typename SomeFloatingType > inline
bool mullersRecurrenceFunctionTest( std::vector< SomeFloatingType > &testResults ) // testResults must already have required size
{
    if (testResults.size()<3)
        return false; // do nothing

    testResults[0] = SomeFloatingType(4   ); // x0
    testResults[1] = SomeFloatingType(4.25); // x1

    typename std::vector< SomeFloatingType >::size_type i = 2, size = testResults.size();
    for(; i!=size; ++i)
    {
        testResults[i] = mullersRecurrenceFunction( testResults[i-1], testResults[i-2] );
    }

    return true;

}




//----------------------------------------------------------------------------

} // namespace for_decimal_tests

} // namespace marty


