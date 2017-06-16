/*

 Copyright (C) 2017 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
#include <Core/Analysis/Data/SkeletonData.h>
#include <GUI/Representations/Managers/TemporalManager.h>
#include <GUI/View/Widgets/Skeleton/SkeletonWidget2D.h>
#include <GUI/Widgets/Styles.h>
#include <App/ToolGroups/Edit/SkeletonEditionTool.h>

using namespace ESPINA;
using namespace ESPINA::GUI::Widgets::Styles;
using namespace ESPINA::GUI::Representations::Managers;
using namespace ESPINA::GUI::View::Widgets::Skeleton;

//--------------------------------------------------------------------
SkeletonEditionTool::SkeletonEditionTool(Support::Context& context)
: EditTool("SkeletonEditionTool", ":/espina/tubular.svg", tr("Manual modification of skeletons."), context)
, m_init  {false}
, m_item  {nullptr}
{
  initEventHandler();
  initRepresentationFactory();

  setCheckable(true);
  setExclusive(true);

  connect(m_eventHandler.get(), SIGNAL(eventHandlerInUse(bool)),
          this                , SLOT(initTool(bool)));

  initParametersWidgets();
}

//--------------------------------------------------------------------
SkeletonEditionTool::~SkeletonEditionTool()
{
  if(m_item != nullptr) initTool(false);
}

//--------------------------------------------------------------------
void SkeletonEditionTool::initTool(bool value)
{
}

//--------------------------------------------------------------------
void SkeletonEditionTool::onSegmentationsRemoved(ViewItemAdapterSList segmentations)
{
}

//--------------------------------------------------------------------
void SkeletonEditionTool::onResolutionChanged()
{
}

//--------------------------------------------------------------------
void SkeletonEditionTool::onModelReset()
{
}

//--------------------------------------------------------------------
void SkeletonEditionTool::onWidgetCloned(GUI::Representations::Managers::TemporalRepresentation2DSPtr clone)
{
}

//--------------------------------------------------------------------
void SkeletonEditionTool::onSkeletonModified(vtkSmartPointer<vtkPolyData> polydata)
{
}

//--------------------------------------------------------------------
bool SkeletonEditionTool::acceptsNInputs(int n) const
{
  return (n == 1);
}

//--------------------------------------------------------------------
bool SkeletonEditionTool::acceptsSelection(SegmentationAdapterList segmentations)
{
  bool hasRequiredData = true;

  for(auto segmentation : segmentations)
  {
    hasRequiredData &= hasSkeletonData(segmentation->output());
  }

  return hasRequiredData;
}

//--------------------------------------------------------------------
void SkeletonEditionTool::initRepresentationFactory()
{
  auto representation2D = std::make_shared<SkeletonWidget2D>(m_eventHandler);

  connect(representation2D.get(), SIGNAL(cloned(GUI::Representations::Managers::TemporalRepresentation2DSPtr)),
          this,                   SLOT(onWidgetCloned(GUI::Representations::Managers::TemporalRepresentation2DSPtr)));

  m_factory = std::make_shared<TemporalPrototypes>(representation2D, TemporalRepresentation3DSPtr(), id());
}

//--------------------------------------------------------------------
void SkeletonEditionTool::initParametersWidgets()
{
  m_eraseButton = createToolButton(":/espina/eraser.png", tr("Erase skeleton nodes."));
  m_eraseButton->setCheckable(true);
  m_eraseButton->setChecked(false);

  connect(m_eraseButton, SIGNAL(clicked(bool)), this, SLOT(onEraseButtonClicked(bool)));
  connect(this, SIGNAL(toggled(bool)), m_eraseButton, SLOT(setVisible(bool)));

  addSettingsWidget(m_eraseButton);

  m_moveButton = createToolButton(":/espina/tubular_move.svg", tr("Translate skeleton nodes."));
  m_moveButton->setCheckable(true);
  m_moveButton->setChecked(false);

  connect(m_moveButton, SIGNAL(clicked(bool)), this, SLOT(onMoveButtonClicked(bool)));
  connect(this, SIGNAL(toggled(bool)), m_moveButton, SLOT(setVisible(bool)));

  addSettingsWidget(m_moveButton);
}

//--------------------------------------------------------------------
void SkeletonEditionTool::onEraseButtonClicked(bool value)
{
}

//--------------------------------------------------------------------
void SkeletonEditionTool::onMoveButtonClicked(bool value)
{
}

//--------------------------------------------------------------------
void SkeletonEditionTool::initEventHandler()
{
  m_eventHandler = std::make_shared<SkeletonEventHandler>();
  m_eventHandler->setInterpolation(true);
  m_eventHandler->setCursor(Qt::CrossCursor);

  setEventHandler(m_eventHandler);
}
