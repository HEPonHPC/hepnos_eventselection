#include "mocks.hpp"

bool
ana::good_event(ana::StandardRecord const&)
{
  return false;
};

std::vector<ana::StandardRecord>
ana::create_records(hepnos::Event const&)
{
  return {};
};
