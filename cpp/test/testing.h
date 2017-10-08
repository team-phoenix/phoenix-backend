#pragma once

#define MOCK class

#if defined(RUN_UNIT_TESTS)

#define TEST_ARG_TREAT_AS_VOID const bool called = true
#define MOCKABLE virtual

#else

#define TEST_ARG_TREAT_AS_VOID void
#define MOCKABLE /**/
#define MOCK( className ) /**/

#endif


