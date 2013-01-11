/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  <copyright holder> <email>

    This program is free software: you can redistribute it and/or modify
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


#include "RectangularVOI.h"
#include <Toolbars/VOI/Settings.h>
#include <Settings/VOI/SettingsPanel.h>

#include <Core/Model/Channel.h>
#include <Core/Model/EspinaModel.h>
#include <Core/Model/EspinaFactory.h>
#include <GUI/ViewManager.h>
#include <GUI/vtkWidgets/RectangularRegion.h>
#include <GUI/vtkWidgets/RectangularRegionSliceSelector.h>

#include <vtkMath.h>

#include <QPixmap>
#include <boost/concept_check.hpp>

using namespace EspINA;

//-----------------------------------------------------------------------------
RectangularVOI::RectangularVOI(EspinaModel *model,
                               ViewManager *viewManager)
: m_model(model)
, m_viewManager(viewManager)
, m_inUse(false)
, m_enabled(true)
, m_widget (NULL)
, m_sliceSelector(NULL)
, m_settings     (new Settings())
, m_settingsPanel(new SettingsPanel(m_model, m_settings, m_viewManager))
{
  m_picker.setCursor(QCursor(QPixmap(":/espina/roi_go.svg").scaled(32,32)));
  m_picker.setMultiSelection(false);
  m_picker.setPickable(IPicker::CHANNEL);
  connect(&m_picker, SIGNAL(itemsPicked(IPicker::PickList)),
          this, SLOT(defineVOI(IPicker::PickList)));

  vtkMath::UninitializeBounds(m_bounds);
  m_model->factory()->registerSettingsPanel(m_settingsPanel.data());
}

//-----------------------------------------------------------------------------
RectangularVOI::~RectangularVOI()
{
//   qDebug() << "Destroy RVOI";
  delete m_settings;

  if (m_sliceSelector)
  {
    m_viewManager->removeSliceSelectors(m_sliceSelector);
    delete m_sliceSelector;
  }

  if (m_widget)
    delete m_widget;
}

//-----------------------------------------------------------------------------
QCursor RectangularVOI::cursor() const
{
  QCursor voiCursor(Qt::ArrowCursor);

  if (!m_widget)
    voiCursor= m_picker.cursor();

  return voiCursor;
}


//-----------------------------------------------------------------------------
bool RectangularVOI::filterEvent(QEvent* e, EspinaRenderView* view)
{
  bool eventHandled = false;

  if (m_enabled)
  {
    if (m_widget)
      eventHandled = m_widget->filterEvent(e,view);

    if (!eventHandled && !m_widget)
      eventHandled = m_picker.filterEvent(e, view);
  }

  return eventHandled;
}

//-----------------------------------------------------------------------------
void RectangularVOI::setInUse(bool value)
{
  if (m_inUse == value)
    return;

  m_inUse = value;

  if (m_inUse)
  {
    m_enabled = true;
  }
  else
  {
    if (m_widget)
    {
      m_viewManager->removeSliceSelectors(m_sliceSelector);
      delete m_sliceSelector;
      m_sliceSelector = NULL;

      m_viewManager->removeWidget(m_widget);
      delete m_widget;
      m_widget = NULL;

      m_viewManager->updateViews();
    }
    emit voiDeactivated();
  }
}

//-----------------------------------------------------------------------------
void RectangularVOI::setEnabled(bool enable)
{
  if (m_enabled == enable)
    return;

  m_enabled = enable;
  if (m_widget)
    m_widget->setEnabled(enable);
}

//-----------------------------------------------------------------------------
IVOI::Region RectangularVOI::region()
{
  if (m_widget)
    m_widget->bounds(m_bounds);

  return m_bounds;
}

//-----------------------------------------------------------------------------
void RectangularVOI::defineVOI(IPicker::PickList channels)
{
  if (channels.isEmpty())
    return;

  // Compute default bounds
  Q_ASSERT(channels.size() == 1); //Only one element is selected
  IPicker::PickedItem pickedItem = channels.first();

  Q_ASSERT(pickedItem.first->GetNumberOfPoints() == 1); //Only one pixel's selected
  double pos[3];
  pickedItem.first->GetPoint(0, pos);

  PickableItemPtr pItem = pickedItem.second;
  if (EspINA::CHANNEL != pItem->type())
    return;

  ChannelPtr pickedChannel = channelPtr(pItem);
  double spacing[3];
  pickedChannel->volume()->spacing(spacing);

  const Nm xRadius = vtkMath::Round(m_settings->xSize() / 2.0);
  const Nm yRadius = vtkMath::Round(m_settings->ySize() / 2.0);
  const Nm zRadius = vtkMath::Round(m_settings->zSize() / 2.0);

  Nm bounds[6] = {
     pos[0] - xRadius, pos[0] + xRadius,
     pos[1] - yRadius, pos[1] + yRadius,
     pos[2] - zRadius, pos[2] + zRadius};

  m_widget = new RectangularRegion(bounds, m_viewManager);
  Q_ASSERT(m_widget);
  m_widget->setResolution(spacing);
  m_widget->setRepresentationPattern(0xFFF0);
  m_viewManager->addWidget(m_widget);
  m_sliceSelector = new RectangularRegionSliceSelector(m_widget);
  m_sliceSelector->setLeftLabel ("From");
  m_sliceSelector->setRightLabel("To");
  m_viewManager->addSliceSelectors(m_sliceSelector, ViewManager::From|ViewManager::To);
  m_viewManager->updateViews();
}
