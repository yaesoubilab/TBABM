#include "../../include/TBABM/TBTypes.h"

TBHandlers CreateTBHandlers(function<void(int)> death) {
  return {
    death
  };
}


TBQueryHandlers CreateTBQueryHandlers(function<int(Time)> Age,
    function<bool(void)> Alive,
    function<double(Time)> CD4Count,
    function<HIVStatus(void)> GetHIVStatus,
    function<bool(void)> ARTStatus,
    function<double(Time)> GlobalTBPrevalence,
    function<shared_p<TB>(void)> Lifetime)
{
  if (!Age || !Alive || \
      !CD4Count || !GetHIVStatus || !ARTStatus || \
      !GlobalTBPrevalence || !Lifetime) {
    printf("Error: >= 1 argument to CreateTBHandlers contained empty std::function\n");
    exit(1);
  }

  return {
    std::move(Age),
    std::move(Alive),
    std::move(CD4Count),
    std::move(GetHIVStatus),
    std::move(ARTStatus),
    std::move(GlobalTBPrevalence),
    std::move(Lifetime)
  };
}
