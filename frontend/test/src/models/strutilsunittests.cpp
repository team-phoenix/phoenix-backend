#include "catch.hpp"

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
}
