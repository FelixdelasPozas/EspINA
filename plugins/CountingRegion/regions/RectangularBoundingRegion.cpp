/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

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


#include "regions/RectangularBoundingRegion.h"

#include <common/cache/CachedObjectBuilder.h>
#include <pqPipelineSource.h>
#include <pq3DWidget.h>
#include <vtkRectangularBoundingRegionWidget.h>
#include <vtkSMNewWidgetRepresentationProxy.h>

//-----------------------------------------------------------------------------
RectangularBoundingRegion::RectangularBoundingRegion(int left, int top, int upper, int right, int bottom, int lower)
: BoundingRegion(left, top, upper, right, bottom, lower)
{
  // Configuration of Bounding Region interface
  pqFilter::Arguments regionArgs;
  regionArgs << pqFilter::Argument("Input",pqFilter::Argument::INPUT,"");
  regionArgs << pqFilter::Argument("Extent",pqFilter::Argument::INTVECT, QString("0,400,0,400,0,110"));
  regionArgs << pqFilter::Argument("Spacing",pqFilter::Argument::DOUBLEVECT, QString("10,10,10"));
  QString inclusion = QString("%1,%2,%3").arg(left).arg(top).arg(upper);
  regionArgs << pqFilter::Argument("Inclusion", pqFilter::Argument::INTVECT, inclusion);
  QString exclusion = QString("%1,%2,%3").arg(right).arg(bottom).arg(lower);
  regionArgs << pqFilter::Argument("Exclusion", pqFilter::Argument::INTVECT, exclusion);

  CachedObjectBuilder *cob = CachedObjectBuilder::instance();
  m_boundingRegion = cob->createFilter("filters","RectangularBoundingRegion", regionArgs);

  Q_ASSERT(m_boundingRegion);
}

//-----------------------------------------------------------------------------
RectangularBoundingRegion::~RectangularBoundingRegion()
{
  foreach(pq3DWidget *widget, m_widgets)
  {
    widget->deselect();
    widget->deleteLater();
  }
  m_widgets.clear();
}

//-----------------------------------------------------------------------------
QVariant RectangularBoundingRegion::data(int role) const
{
  if (role == Qt::DisplayRole)
  {
    QString repName = QString("Rectangular Region (%1,%2,%3,%4,%5,%6)")
      .arg(m_inclusion[0]).arg(m_inclusion[1]).arg(m_inclusion[2])
      .arg(m_exclusion[0]).arg(m_exclusion[1]).arg(m_exclusion[2]);
    return repName;
  }

  return QStandardItem::data(role);
}

//-----------------------------------------------------------------------------
pq3DWidget* RectangularBoundingRegion::createWidget(vtkPVSliceView::VIEW_PLANE plane)
{
  vtkSMProxy *proxy = m_boundingRegion->pipelineSource()->getProxy();
  QList<pq3DWidget *> widgets =  pq3DWidget::createWidgets(proxy, proxy);

  Q_ASSERT(widgets.size() == 1);
  // By default ParaView doesn't "Apply" the changes to the widget. So we set
  // up a slot to "Apply" when the interaction ends.
  QObject::connect(widgets[0], SIGNAL(widgetEndInteraction()),
		   widgets[0], SLOT(accept()));
  
  vtkRectangularBoundingRegionWidget *regionwidget = dynamic_cast<vtkRectangularBoundingRegionWidget*>(widgets[0]->getWidgetProxy()->GetWidget());
  Q_ASSERT(regionwidget);
  regionwidget->SetViewType(plane);
  m_widgets << widgets;

  return widgets[0];
}

//-----------------------------------------------------------------------------
void RectangularBoundingRegion::setBounds(double bounds[6])
{

}

//-----------------------------------------------------------------------------
void RectangularBoundingRegion::setEnabled(bool enable)
{

}
