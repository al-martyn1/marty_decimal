#pragma once

#ifdef USE_MARTY_INT_DECIMAL
    #include "marty_int_decimal.h"
#else
    #include "marty_bcd_decimal.h"
#endif


#include <vector>


#if defined(WIN32) || defined(_WIN32)

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

        return (std::uint32_t)GetTickCount();

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

    std::vector< SomeFloatingType >::size_type i = 2, size = testResults.size();
    for(; i!=size; ++i)
    {
        testResults[i] = mullersRecurrenceFunction( testResults[i-1], testResults[i-2] );
    }

    return true;

}




//----------------------------------------------------------------------------

} // namespace for_decimal_tests

} // namespace marty


