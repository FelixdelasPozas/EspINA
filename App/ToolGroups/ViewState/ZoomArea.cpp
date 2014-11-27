/*

    Copyright (C) 2014 Felix de las Pozas Alvarez <@>

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
#include "ZoomArea.h"
#include <GUI/View/Widgets/Zoom/ZoomSelectionWidget.h>

// Qt
#include <QPixmap>
#include <QAction>

using namespace ESPINA;

//----------------------------------------------------------------------------
ZoomArea::ZoomArea(ViewManagerSPtr viewManager)
: m_enabled{false}
, m_viewManager{viewManager}
, m_zoomArea   {new QAction(QIcon(":/espina/zoom_selection.png"), tr("Define Zoom Area"),this)}
, m_widget     {EspinaWidgetSPtr(ZoomSelectionWidget::New())}
, m_zoomHandler{std::dynamic_pointer_cast<EventHandler>(m_widget)}
{
  m_zoomArea->setCheckable(true);

  connect(m_zoomArea, SIGNAL(triggered(bool)),
          this,       SLOT(activateTool(bool)));

  connect(m_zoomHandler.get(), SIGNAL(eventHandlerInUse(bool)),
          this,                SLOT(activateTool(bool)));
}

//----------------------------------------------------------------------------
ZoomArea::~ZoomArea()
{
  m_widget->setEnabled(false);
}

//----------------------------------------------------------------------------
void ZoomArea::setEnabled(bool value)
{
  m_enabled = value;
}

//----------------------------------------------------------------------------
bool ZoomArea::enabled() const
{
  return m_enabled;
}

//----------------------------------------------------------------------------
QList<QAction *> ZoomArea::actions() const
{
  QList<QAction *> actions;

  actions << m_zoomArea;

  return actions;
}

//----------------------------------------------------------------------------
void ZoomArea::abortOperation()
{
  if (m_zoomArea->isChecked())
  {
    m_zoomArea->setChecked(false);

    activateTool(false);
  }
}

//----------------------------------------------------------------------------
void ZoomArea::activateTool(bool value)
{
  if (value)
  {
    m_viewManager->setEventHandler(m_zoomHandler);
    m_viewManager->setSelectionEnabled(false);
    m_viewManager->addWidget(m_widget);
    m_widget->setEnabled(true);
  }
  else
  {
    m_widget->setEnabled(false);
    m_viewManager->removeWidget(m_widget);
    m_viewManager->unsetEventHandler(m_zoomHandler);
    m_viewManager->setSelectionEnabled(true);
  }

  m_zoomArea->blockSignals(true);
  m_zoomArea->setChecked(value);
  m_zoomArea->blockSignals(false);

}
