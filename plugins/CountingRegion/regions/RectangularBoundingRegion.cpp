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
#include <vtkSMPropertyHelper.h>
#include <extensions/CountingRegionSampleExtension.h>
#include <pq3DWidgetInterface.h>
#include <pqApplicationCore.h>
#include <pqInterfaceTracker.h>

//-----------------------------------------------------------------------------
RectangularBoundingRegion::RectangularBoundingRegion(CountingRegionSampleExtension *sampleExt,
						     double borders[6],
						     double inclusion[3],
						     double exclusion[3])
: BoundingRegion(sampleExt, inclusion, exclusion)
{
  // Configuration of Bounding Region interface
  pqFilter::Arguments regionArgs;
  QString margin = QString("%1,%2,%3,%4,%5,%6").arg((int)borders[0]).arg((int)borders[1])
                                               .arg((int)borders[2]).arg((int)borders[3])
				               .arg((int)borders[4]).arg((int)borders[5]);

  regionArgs << pqFilter::Argument("Margin",pqFilter::Argument::DOUBLEVECT, margin);
  QString inclusionArg = QString("%1,%2,%3").arg(left()).arg(top()).arg(upper());
  regionArgs << pqFilter::Argument("InclusionOffset", pqFilter::Argument::DOUBLEVECT, inclusionArg);
  QString exclusionArg = QString("%1,%2,%3").arg(right()).arg(bottom()).arg(lower());
  regionArgs << pqFilter::Argument("ExclusionOffset", pqFilter::Argument::DOUBLEVECT, exclusionArg);

  CachedObjectBuilder *cob = CachedObjectBuilder::instance();
  m_boundingRegion = cob->createFilter("filters","RectangularBoundingRegion", regionArgs);

  Q_ASSERT(m_boundingRegion);
}

//-----------------------------------------------------------------------------
RectangularBoundingRegion::~RectangularBoundingRegion()
{
  m_sampleExt->removeRegion(this);
  foreach(pq3DWidget *widget, m_widgets)
  {
    widget->deselect();
    widget->deleteLater();
  }
  m_widgets.clear();

  CachedObjectBuilder *cob = CachedObjectBuilder::instance();
  if (m_boundingRegion)
    cob->removeFilter(m_boundingRegion);
}

//-----------------------------------------------------------------------------
QVariant RectangularBoundingRegion::data(int role) const
{
  if (role == Qt::DisplayRole)
  {
    QString repName = QString(tr("Rectangular (%1,%2,%3,%4,%5,%6)"))
      .arg(left()).arg(top()).arg(upper())
      .arg(right()).arg(bottom()).arg(lower());
    return repName;
  }

  return BoundingRegion::data(role);
}

//-----------------------------------------------------------------------------
pq3DWidget* RectangularBoundingRegion::createWidget()
{
  pq3DWidget *widget = createWidget("RectangularBoundingVolume");
  Q_ASSERT(widget);
  // By default ParaView doesn't "Apply" the changes to the widget. So we set
  // up a slot to "Apply" when the interaction ends.
  QObject::connect(widget, SIGNAL(widgetEndInteraction()),
		   widget, SLOT(accept()));
  QObject::connect(widget, SIGNAL(widgetEndInteraction()),
		   this, SLOT(resetWidgets()));
  
  m_widgets << widget;

  return widget;
}

//-----------------------------------------------------------------------------
SliceWidget *RectangularBoundingRegion::createSliceWidget(vtkPVSliceView::VIEW_PLANE plane)
{
  pq3DWidget *widget = createWidget("RectangularBoundingRegion");
  Q_ASSERT(widget);
  // By default ParaView doesn't "Apply" the changes to the widget. So we set
  // up a slot to "Apply" when the interaction ends.
  QObject::connect(widget, SIGNAL(widgetEndInteraction()),
		   widget, SLOT(accept()));
  QObject::connect(widget, SIGNAL(widgetEndInteraction()),
		   this, SLOT(resetWidgets()));

  vtkAbstractWidget *aw = widget->getWidgetProxy()->GetWidget();
  vtkRectangularBoundingRegionWidget *regionWidget =
    dynamic_cast<vtkRectangularBoundingRegionWidget*>(aw);
  Q_ASSERT(regionWidget);
  regionWidget->SetPlane(plane);

  m_widgets << widget;

  return new SliceWidget(widget);
}

//-----------------------------------------------------------------------------
void RectangularBoundingRegion::setBounds(double bounds[6])
{
  Q_ASSERT(false);
}

//-----------------------------------------------------------------------------
void RectangularBoundingRegion::bounds(double bounds[6])
{
  Q_ASSERT(false);
}

//-----------------------------------------------------------------------------
void RectangularBoundingRegion::setEnabled(bool enable)
{
  Q_ASSERT(false);
}

//TODO: Widget related methods seem to be common in both regions
//-----------------------------------------------------------------------------
void RectangularBoundingRegion::resetWidgets()
{
  foreach(pq3DWidget *widget, m_widgets)
    widget->reset();

  vtkSMProxy *proxy = m_boundingRegion->pipelineSource()->getProxy();
  vtkSMPropertyHelper(proxy, "InclusionOffset").Get(m_inclusion, 3);
  vtkSMPropertyHelper(proxy, "ExclusionOffset").Get(m_exclusion, 3);
  emitDataChanged();
  emit modified(this);
}

//-----------------------------------------------------------------------------
pq3DWidget* RectangularBoundingRegion::createWidget(QString name)
{
  vtkSMProxy *proxy = m_boundingRegion->pipelineSource()->getProxy();
//   vtkPVXMLElement *hints = proxy->GetHints();
//   Q_ASSERT(hints->GetNumberOfNestedElements() == 1);
//   vtkPVXMLElement* element = hints->GetNestedElement(0);
  QList<pq3DWidgetInterface *> interfaces =
  pqApplicationCore::instance()->interfaceTracker()->interfaces<pq3DWidgetInterface*>();
  pq3DWidget *widget = 0;

  // Create the widget from plugins.
  foreach (pq3DWidgetInterface* iface, interfaces)
  {
    widget = iface->newWidget(name, proxy, proxy);
    if (widget)
    {
//       widget->setHints(element);
      return widget;
    }
  }
  return NULL;
}