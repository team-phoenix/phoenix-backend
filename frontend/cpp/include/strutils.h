#pragma once

#include <QString>
#include <QStringList>

#include <algorithm>
#include <limits>

class StrUtils
{
public:
  static QString findClosestMatch(QStringList phrases, QString masterPhrase)
  {
    if (phrases.isEmpty()) {
      throw std::runtime_error("No phrases we passed in to match");
    }

    int currentMinDistance = std::numeric_limits<int>::max();
    QString closestMatch;

    for (const QString &phrase : phrases) {
      const int distance = stringDistance(phrase, masterPhrase);

      if (distance < currentMinDistance) {
        currentMinDistance = distance;
        closestMatch = phrase;
      }
    }

    return closestMatch;
  }

private:
  StrUtils() = default;

  static int levenshtein_distance(const QString &s1, const QString &s2)
  {
    // To change the type this function manipulates and returns, change
    // the return type and the types of the two variables below.
    int s1len = s1.size();
    int s2len = s2.size();

    auto column_start = (decltype(s1len))1;

    auto column = new decltype(s1len)[s1len + 1];
    std::iota(column + column_start - 1, column + s1len + 1, column_start - 1);

    for (auto x = column_start; x <= s2len; x++) {
      column[0] = x;
      auto last_diagonal = x - column_start;

      for (auto y = column_start; y <= s1len; y++) {
        auto old_diagonal = column[y];
        auto possibilities = {
          column[y] + 1,
          column[y - 1] + 1,
          last_diagonal + (s1.at(y - 1) == s2.at(x - 1) ? 0 : 1)
        };
        column[y] = std::min(possibilities);
        last_diagonal = old_diagonal;
      }
    }

    auto result = column[s1len];
    delete[] column;
    return result;
  }


  static int stringDistance(QString str1, QString str2)
  {
    return levenshtein_distance(str1, str2);
  }
};
