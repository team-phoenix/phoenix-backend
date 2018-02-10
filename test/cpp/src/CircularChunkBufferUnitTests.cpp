#include "doctest.hpp"
#include "circularchunkbuffer.h"

#include <QVector>

SCENARIO("CircularChunkBufferUnitTests")
{
  GIVEN("a real subject") {

    const QVector<char> src = { 0, 0, 1, 2, 4, 8 };
    QVector<char> dest(src.size());

    const QVector<qint16> shortSrc = { 3, 6, 9, 27, 89, 0};

    REQUIRE(shortSrc.size() == src.size());

    CircularChunkBuffer subject(src.size());

    THEN("There is always one more element is the circular buffer than created with") {
      REQUIRE(subject.capacity() == src.size() + 1);
    }


    WHEN("write() is called on the src buffer") {

      const int wrote = subject.write(src.data(), src.size());

      THEN("all of the src buffer was written") {
        REQUIRE(wrote == src.size());
        REQUIRE(subject.size() == wrote);
      }
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
