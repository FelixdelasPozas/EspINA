#ifndef ESPINA_GUI_TYPES_H_
#define ESPINA_GUI_TYPES_H_

#include <memory>

namespace ESPINA
{
  namespace GUI
  {
    namespace Widgets
    {
      class ProgressAction;
    }

    namespace Representations
    {
      namespace Managers
      {
        class TemporalPrototypes;
        using TemporalPrototypesSPtr = std::shared_ptr<TemporalPrototypes>;
      }
    }
  }
}

#endif // ESPINA_GUI_TYPES_H_