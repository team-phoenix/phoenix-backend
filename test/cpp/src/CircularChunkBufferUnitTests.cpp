#include "doctest.hpp"
#include "circularchunkbuffer.h"

#include <QVector>

SCENARIO("CircularChunkBufferUnitTests")
{
  GIVEN("a real subject") {

    const QVector<char> src = {1, 2, 4, 8};
    QVector<char> dest(src.size());

    CircularChunkBuffer subject(src.size());

    THEN("The capacity is equal to the set size") {
      REQUIRE(subject.capacity() == src.size());
    }

    WHEN("write() is called on the src buffer") {

      const int wrote = subject.write(src.data(), src.size());

      THEN("all of the src buffer was written") {
        REQUIRE(wrote == src.size());
        REQUIRE(subject.size() == wrote);
      }
    }

    WHEN("") {
      char a[2] = {1, 1};
      subject.write(a, 2);

      REQUIRE(subject.size() == 2);

      qint16 b = 0;
      int wrote = subject.readFramesToShortArray(&b, 1);

      REQUIRE(subject.size() == 0);
      REQUIRE(wrote == 2);

      REQUIRE(subject.writeFramesFromShortArray(&b, 1, 1) == 2);
      char c[2] = {0, 0};

      REQUIRE(subject.read(c, 2) == 2);

      REQUIRE(c[0] == a[0]);
      REQUIRE(c[1] == a[1]);


    }

    WHEN("read() is called without writting anything to the buffer") {
      const int read = subject.read(dest.data(), dest.size());

      THEN("no data was read into the dest buffer") {
        REQUIRE(read == 0);

        const QVector<char> emptyBuffer(dest.size());

        REQUIRE(dest == emptyBuffer);
      }
    }

    WHEN("read() is called after a write with the same size") {
      subject.write(src.data(), src.size());
      const int read = subject.read(dest.data(), src.size());

      THEN("The ammount of data read is same as the data wrote") {
        REQUIRE(read == src.size());
        REQUIRE(dest == src);
      }

      THEN("The subject's internal buffer is empty") {
        REQUIRE(subject.size() == 0);
      }
    }

  }
}
