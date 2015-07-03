#ifndef ESPINA_GUI_TYPES_H_
#define ESPINA_GUI_TYPES_H_

#include <memory>

namespace ESPINA
{
  namespace GUI
  {
    namespace ColorEngines
    {
      class PropertyColorEngine;
    }

    namespace Widgets
    {
      class ProgressAction;
      class PixelValueSelector;
      class CategorySelector;
      class NumericalInput;
    }

    namespace Representations
    {
      namespace Managers
      {
        class TemporalPrototypes;
        using TemporalPrototypesSPtr = std::shared_ptr<TemporalPrototypes>;
      }
    }

   namespace Utils
   {
     class ColorRange;
     class RangeHSV;
   }
  }
}

#endif // ESPINA_GUI_TYPES_H_
