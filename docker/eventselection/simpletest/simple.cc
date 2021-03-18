#include "../disable_depman.hpp"


int main() {
  
    DisableDepManAll();

    caf::SRProxy p(0, "");
    auto val = ana::kNoCut(&p);

    return 1;
  
}
