/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015 Jorge Peña Pastor <jpena@cesvima.upm.es>
 * Copyright (C) 2015 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ESPINA_GUI_TYPES_H_
#define ESPINA_GUI_TYPES_H_

#include <memory>
#include <QList>

namespace ESPINA
{
  class ItemAdapter;
  using ItemAdapterPtr      = ItemAdapter *;
  using ConstItemAdapterPtr = const ItemAdapter *;

  class CategoryAdapter;
  using CategoryAdapterPtr   = CategoryAdapter *;
  using CategoryAdapterSPtr  = std::shared_ptr<CategoryAdapter>;

  class ViewItemAdapter;
  using ViewItemAdapterPtr      = ViewItemAdapter *;
  using ConstViewItemAdapterPtr = const ViewItemAdapter *;

  class ChannelAdapter;
  using ChannelAdapterPtr      = ChannelAdapter *;
  using ConstChannelAdapterPtr = const ChannelAdapter *;
  using ChannelAdapterList     = QList<ChannelAdapterPtr>;
  using ChannelAdapterSPtr     = std::shared_ptr<ChannelAdapter>;
  using ChannelAdapterSList    = QList<ChannelAdapterSPtr>;

  class SegmentationAdapter;
  using SegmentationAdapterPtr      = SegmentationAdapter *;
  using ConstSegmentationAdapterPtr = const SegmentationAdapter *;
  using SegmentationAdapterCPtr     = const SegmentationAdapter *;
  using SegmentationAdapterSet      = QSet<SegmentationAdapterPtr>;
  using SegmentationAdapterList     = QList<SegmentationAdapterPtr>;
  using SegmentationAdapterSPtr     = std::shared_ptr<SegmentationAdapter>;
  using SegmentationAdapterSList    = QList<SegmentationAdapterSPtr>;

  namespace GUI
  {
    namespace ColorEngines
    {
      class ColorEngine;
      using ColorEngineSPtr  = std::shared_ptr<ColorEngine>;

      class MultiColorEngine;
      using MultiColorEngineSPtr = std::shared_ptr<MultiColorEngine>;

      class InformationColorEngine;
    }

    namespace Widgets
    {
      class ColorBar;
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

   namespace View
   {
     class ViewState;
     using ViewStateSPtr = std::shared_ptr<ViewState>;
   }
  }
}

#endif // ESPINA_GUI_TYPES_H_
