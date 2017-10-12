#pragma once

#define MOCK class

#if defined(RUN_UNIT_TESTS)

#define TEST_ARG_TREAT_AS_VOID const bool called = true
#define  virtual

#else

#define TEST_ARG_TREAT_AS_VOID void
#define  /**/
#define MOCK( className ) /**/

#endif


