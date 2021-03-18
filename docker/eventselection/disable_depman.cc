#include "CAFAna/Core/Cut.h"
#include "CAFAna/Core/DepMan.h"
#include "StandardRecord/Proxy/SRProxy.h"

#include "disable_depman.hpp"

template <class T>
void
DisableDepMan()
{
  ana::DepMan<T>::Instance().Disable();
}

void
DisableDepManAll()
{
  DisableDepMan<ana::GenericCut<caf::SRNeutrinoProxy>>();
  DisableDepMan<ana::GenericCut<caf::SRSpillProxy>>();
  DisableDepMan<ana::GenericCut<caf::SRProxy>>();

  DisableDepMan<ana::GenericVar<caf::SRProxy>>();
  DisableDepMan<ana::GenericVar<caf::SRNeutrinoProxy>>();
  DisableDepMan<ana::GenericVar<caf::SRSpillProxy>>();
}
