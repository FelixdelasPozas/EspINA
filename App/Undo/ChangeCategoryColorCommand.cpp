/*

 Copyright (C) 2015 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

 This file is part of ESPINA.

 ESPINA is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 
 */

// ESPINA
#include <Core/Utils/ListUtils.hxx>
#include <Undo/ChangeCategoryColorCommand.h>
#include <GUI/ColorEngines/IntensitySelectionHighlighter.h>
#include <GUI/View/ViewState.h>

using namespace ESPINA;
using namespace ESPINA::GUI::ColorEngines;
using namespace ESPINA::Core::Utils;
using namespace ESPINA::GUI::View;

//------------------------------------------------------------------------
ChangeCategoryColorCommand::ChangeCategoryColorCommand(ModelAdapterSPtr      model,
                                                       GUI::View::ViewState &viewState,
                                                       CategoryAdapterPtr    category,
                                                       Hue                   hueValue)
: m_model    {model}
, m_viewState(viewState)
, m_category {category}
, m_hueValue {hueValue}
{
}

//------------------------------------------------------------------------
void ChangeCategoryColorCommand::redo()
{
  auto actualHue = m_category->color().hue();
  auto color = selectedColor(m_hueValue);
  m_hueValue = actualHue;
  m_category->setData(color, Qt::DecorationRole);

  invalidateDependentSegmentations();
}

//------------------------------------------------------------------------
void ChangeCategoryColorCommand::undo()
{
  redo();
}

//------------------------------------------------------------------------
void ChangeCategoryColorCommand::invalidateDependentSegmentations() const
{
  SegmentationAdapterList segmentations;

  for (auto segmentation : m_model->segmentations())
  {
    if (segmentation->category().get() == m_category)
    {
      segmentations << segmentation.get();
    }
  }

  auto itemList = toList<ViewItemAdapter, SegmentationAdapter>(segmentations);
  m_viewState.invalidateRepresentations(itemList);
}
