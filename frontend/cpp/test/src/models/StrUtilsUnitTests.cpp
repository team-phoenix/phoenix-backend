#include "doctest.hpp"

#include "strutils.h"

SCENARIO("StrUtils")
{
  GIVEN("a list of strings") {

    const QStringList stringPhrases({ "Apple", "Dog", "Apple But", "Apple Butt.", "Apple Butt Dog"});
    WHEN("findClosestMatch is called on a master string with") {

      const QString masterString = "Apple Butt. Dog";
      QString actualResult = StrUtils::findClosestMatch(stringPhrases, masterString);

      REQUIRE(actualResult == "Apple Butt Dog");
    }
  }

  WHEN("normalizePathStr is used")  {
    const QString expectedNormalizedString = "moo";

    const QString pathStringWithCrazyMiddle =
      "/path/to/file_item|||`~!@#$%^&*()-_=+[{]}'\";:.>,</?```::moo.nes";
    const QString pathStringWithCrazySuffix =
      "/path/to/file_item/moo.|||`~!@#$%^&*()-_=+[{]}'\";:.>,</?```::";
    const QString actualNormalizedString = StrUtils::normalizePathStr(pathStringWithCrazyMiddle);

    THEN("the returned string returns no special characters or file extension") {
      REQUIRE(actualNormalizedString == expectedNormalizedString);

//      const QString actualString2 = StrUtils::normalizePathStr(pathStringWithCrazySuffix);
//      REQUIRE(actualString2 == expectedNormalizedString);
    }

  }
}
