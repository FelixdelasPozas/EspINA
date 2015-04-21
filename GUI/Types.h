#ifndef ESPINA_GUI_TYPES_H_
#define ESPINA_GUI_TYPES_H_

#include <memory>

namespace ESPINA
{
  namespace GUI
  {
    namespace View
    {
      namespace Widgets
      {
        class WidgetFactory;
        using WidgetFactorySPtr = std::shared_ptr<WidgetFactory>;
      }
    }
  }
}

#endif // ESPINA_GUI_TYPES_H_