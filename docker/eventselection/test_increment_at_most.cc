#include "../code/catch.hpp"
#include "increment_at_most.hpp"
#include <forward_list>

namespace mock {
  struct event {
    int r;
    int sr;
    int evt;
  };
  int
  event_number(event const& e)
  {
    return e.evt;
  }
  int
  subrun_number(event const& e)
  {
    return e.sr;
  }
  int
  run_number(event const& e)
  {
    return e.r;
  }
}

TEST_CASE("Evetn set spans multiple runs with the same subrun number")
{
  std::forward_list<mock::event> const data{{10, 1, 1}, {10, 1, 2}, {11, 1, 1}};
  auto i = data.begin();
  auto const e = data.end();
  increment_at_most(i, e, 5);
  CHECK(i != e);
  CHECK(event_number(*i) == 1);
  CHECK(subrun_number(*i) == 1);
  CHECK(run_number(*i) == 11);
}

TEST_CASE("Event set spans multiple subruns")
{
  std::forward_list<mock::event> const data{{10, 1, 1}, {10, 1, 2}, {10, 2, 1}};
  auto i = data.begin();
  auto const e = data.end();
  increment_at_most(i, e, 5);
  CHECK(i != e);
  CHECK(event_number(*i) == 1);
  CHECK(subrun_number(*i) == 2);
}

TEST_CASE("Empty list")
{
  std::forward_list<mock::event> data;
  auto i = data.begin();
  auto const e = data.end();
  increment_at_most(i, e, 10);
  CHECK(i == data.begin());
}

TEST_CASE("increment matches size")
{
  std::forward_list<mock::event> const data{
    {10, 1, 1}, {10, 1, 2}, {10, 1, 3}, {10, 1, 4}, {10, 1, 5}};
  auto i = data.begin();
  auto const e = data.end();
  increment_at_most(i, e, 5);
  CHECK(i == data.end());
}

TEST_CASE("multiple increments")
{
  std::forward_list<mock::event> const data{
    {10, 1, 1}, {10, 1, 2}, {10, 1, 3}, {10, 1, 4}, {10, 1, 5}};
  auto i = data.begin();
  auto const e = data.end();
  increment_at_most(i, e, 3);
  CHECK(event_number(*i) == 4);
  increment_at_most(i, e, 3);
  CHECK(i == data.end());
}

TEST_CASE("small increment")
{
  std::forward_list<mock::event> const data{
    {10, 1, 1}, {10, 1, 2}, {10, 1, 3}, {10, 1, 4}, {10, 1, 5}};
  auto i = data.begin();
  auto const e = data.end();
  increment_at_most(i, e, 1);
  CHECK(event_number(*i) == 2);
  increment_at_most(i, e, 1);
  CHECK(event_number(*i) == 3);
  increment_at_most(i, e, 1);
  CHECK(event_number(*i) == 4);
  increment_at_most(i, e, 1);
  CHECK(event_number(*i) == 5);
  increment_at_most(i, e, 1);
  CHECK(i == data.end());
}

TEST_CASE("increment too big")
{
  std::forward_list<mock::event> const data{
    {10, 1, 1}, {10, 1, 2}, {10, 1, 3}, {10, 1, 4}, {10, 1, 5}};
  auto i = data.begin();
  auto const e = data.end();
  increment_at_most(i, e, 15);
  CHECK(i == data.end());
}
