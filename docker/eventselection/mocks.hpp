#ifndef MOCK_HPP
#define MOCK_HPP

#include <hepnos/Event.hpp>
#include <vector>

namespace ana {
  struct StandardRecord {
    std::uint32_t slice;
  };
  std::vector<StandardRecord> create_records(hepnos::Event const&);
  bool good_event(StandardRecord const&);
}

#endif
