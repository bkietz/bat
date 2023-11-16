#include <iostream>
#include <string_view>

#include "bat/ButtonSequence.h"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "bat/doctest.h"

namespace bat {

struct Seq : ButtonSequence {
  Seq(std::initializer_list<char> chars) {
    if (chars.size() > MAX_COUNT) throw 0;
    std::copy(chars.begin(), chars.end(), presses);
    count = chars.size();
  };
};

bool operator==(ButtonSequence const &l, ButtonSequence const &r) {
  return std::equal(l.begin(), l.end(), r.begin(), r.end());
}

std::ostream &operator<<(std::ostream &os, ButtonSequence const &s) {
  os << "[ ";
  for (char t : s) {
    os << (t == Seq::TAP ? "tap" : "hold") << " ";
  }
  return os << "]";
}

TEST_CASE("single tap") {
  ButtonSequence seq;
  seq.press(0);
  seq.release(735);
  CHECK(not seq.ready(1000));
  CHECK(seq.ready(2000));
  CHECK(seq == Seq({Seq::TAP}));
}

TEST_CASE("double tap") {
  ButtonSequence seq;
  seq.press(0);
  seq.release(735);
  seq.press(1200);
  seq.release(1800);
  CHECK(not seq.ready(2000));
  CHECK(seq.ready(3000));
  CHECK(seq == Seq({Seq::TAP, Seq::TAP}));
}

TEST_CASE("single hold") {
  ButtonSequence seq;
  seq.press(0);
  seq.release(1743);
  CHECK(not seq.ready(2000));
  CHECK(seq.ready(2800));
  CHECK(seq == Seq({Seq::HOLD}));
}

TEST_CASE("debounced single tap") {
  ButtonSequence seq;
  seq.press(0);
  seq.release(735);
  seq.press(750);
  seq.release(770);
  CHECK(not seq.ready(1000));
  CHECK(seq.ready(2000));
  CHECK(seq == Seq({Seq::TAP}));
}

TEST_CASE("single tap, long pause, single tap") {
  ButtonSequence seq;
  seq.press(0);
  seq.release(735);
  // the sequence is consumed or ignored here...

  // ... later, a new sequence starts
  seq.press(3800);
  seq.release(4000);
  CHECK(not seq.ready(4900));
  CHECK(seq.ready(5100));
  CHECK(seq == Seq({Seq::TAP}));
}

TEST_CASE("hold, overwritten by three taps") {
  ButtonSequence seq;
  seq.press(0);
  seq.release(1735);
  seq.press(2000);
  seq.release(2100);
  seq.press(2200);
  seq.release(2300);
  seq.press(2400);
  seq.release(2500);
  CHECK(not seq.ready(3400));
  CHECK(seq.ready(3600));
  CHECK(seq == Seq({Seq::TAP, Seq::TAP, Seq::TAP}));
}

}  // namespace bat
