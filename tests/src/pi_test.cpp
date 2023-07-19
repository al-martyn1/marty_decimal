/*! \file
    \brief Тест производительности доморощеного Decimal на вычислении числа Пи

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


// Можно попробовать использовать ряд обратных квадратов - https://ru.wikipedia.org/wiki/%D0%A0%D1%8F%D0%B4_%D0%BE%D0%B1%D1%80%D0%B0%D1%82%D0%BD%D1%8B%D1%85_%D0%BA%D0%B2%D0%B0%D0%B4%D1%80%D0%B0%D1%82%D0%BE%D0%B2
// == Pi**2 / 6   - сумма 1/N**2 - деление и возведение в степень

// Формула Валлиса - https://ru.wikipedia.org/wiki/%D0%A4%D0%BE%D1%80%D0%BC%D1%83%D0%BB%D0%B0_%D0%92%D0%B0%D0%BB%D0%BB%D0%B8%D1%81%D0%B0
// Только деление и умножение

//----------------------------------------------------------------------------
inline
std::uint32_t getMillisecTick()
{
    return marty::for_decimal_tests::getMillisecTick();
}

//----------------------------------------------------------------------------
void doTest(int divPrecision, unsigned numIterations)
{
    std::uint32_t startTick = getMillisecTick();

    marty::Decimal::setDivisionPrecision(divPrecision); // максимальное число знаков при делении

    marty::Decimal res = 1;
    for(unsigned i=1; i!=numIterations; ++i)
    {
        // Формула Валлиса

        // i = 1   2   3   4   5   6   7   8    9   10
        //
        //     2   2   4   4   6   6   8   8   10   10
        //     - * - * - * - * - * - * - * - * -- * -- * ...
        //     1   3   3   5   5   7   7   9    9   11

        if (i&1)
        {
            res *= marty::Decimal(i+1) / marty::Decimal(i);
        }
        else
        {
            res *= marty::Decimal(i)   / marty::Decimal(i+1);
        }

        #if defined(DEBUG) || defined(_DEBUG)

            std::cout << i << ": " << res << "\n";

        #endif
        
    }

    std::uint32_t endTick = getMillisecTick();

    auto pi = res * 2;
    
    std::cout << "Prec: " << divPrecision << "\n";
    std::cout << "N   : " << numIterations << "\n";
    std::cout << "T el: " << (endTick-startTick) << "\n";
    std::cout << "pi  : " << pi  << "\n";
    std::cout << "\n";

}




MARTY_DECIMAL_MAIN()
{
    doTest( 50,  1000);
    doTest( 50,  2000);
    doTest( 50,  3000);
    doTest( 50,  4000);
    doTest( 50,  5000);
    doTest( 50,  6000);
    doTest( 50,  7000);
    doTest( 50,  8000);
    doTest( 50,  9000);
    doTest( 50, 10000);
    doTest( 50, 11000);
    doTest( 50, 12000);
    doTest( 50, 13000);
    doTest( 50, 14000);
    doTest( 50, 15000);

    doTest(100,  1000);
    doTest(100,  2000);
    doTest(100,  3000);
    doTest(100,  4000);
    doTest(100,  5000);

    doTest(200,  1000);
    doTest(200,  2000);
    doTest(200,  3000);
    doTest(200,  4000);
    doTest(200,  5000);


    return 0;

}

