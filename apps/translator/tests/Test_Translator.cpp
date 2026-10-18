#include "TestAssert.h"
#include <LexiBridge/apps/translator/Translator.h>

using namespace LexiBridge::Apps::Translator;

int test_default()
{
    CPPTEST_ASSERT(true);
    return 0;
}

int main()
{
    CPPTEST_RUN(test_default);
    return 0;
}
