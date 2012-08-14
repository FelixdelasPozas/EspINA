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


#include "MarginsChannelExtension.h"

//#include "MarginsSegmentationExtension.h"

#include "common/model/Channel.h"
#include "common/model/Segmentation.h"

#include <QApplication>
#include <QDebug>
#include <QMessageBox>
#include "MarginDetector.h"
#include "MarginsSegmentationExtension.h"

#include <vtkDistancePolyDataFilter.h>
#include <vtkPlaneSource.h>
#include <vtkPointData.h>

typedef ModelItem::ArgumentId ArgumentId;

const ModelItemExtension::ExtId MarginsChannelExtension::ID = "MarginsExtension";

const ModelItemExtension::InfoTag MarginsChannelExtension::LEFT_MARGIN   = "Left Margin";
const ModelItemExtension::InfoTag MarginsChannelExtension::TOP_MARGIN    = "Top Margin";
const ModelItemExtension::InfoTag MarginsChannelExtension::UPPER_MARGIN  = "Upper Margin";
const ModelItemExtension::InfoTag MarginsChannelExtension::RIGHT_MARGIN  = "Right Margin";
const ModelItemExtension::InfoTag MarginsChannelExtension::BOTTOM_MARGIN = "Bottom Margin";
const ModelItemExtension::InfoTag MarginsChannelExtension::LOWER_MARGIN  = "Lower Margin";

const ArgumentId MarginsChannelExtension::MARGINTYPE = ArgumentId("MarginType", ArgumentId::VARIABLE);

//-----------------------------------------------------------------------------
MarginsChannelExtension::MarginsChannelExtension()
: m_useExtentMargins(true)
{
//   m_availableInformations << LEFT_MARGIN;
//   m_availableInformations << TOP_MARGIN;
//   m_availableInformations << UPPER_MARGIN;
//   m_availableInformations << RIGHT_MARGIN;
//   m_availableInformations << BOTTOM_MARGIN;
//   m_availableInformations << LOWER_MARGIN;
}

//-----------------------------------------------------------------------------
MarginsChannelExtension::~MarginsChannelExtension()
{

}

//-----------------------------------------------------------------------------
ModelItemExtension::ExtId MarginsChannelExtension::id()
{
  return ID;
}

//-----------------------------------------------------------------------------
void MarginsChannelExtension::initialize(Channel* channel, ModelItem::Arguments args)
{
  if (m_init && m_channel == channel)
    return;

  m_channel = channel;
//   qDebug() << args;

  bool computeMargin = false;
  if (args.contains(MARGINTYPE))
  {
    computeMargin = args[MARGINTYPE] == "Yes";
  } else
  {
    QMessageBox msgBox;
    msgBox.setWindowTitle("Margins Channel Extension");
    msgBox.setText(tr("Compute %1's margins").arg(channel->data().toString()));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    computeMargin = msgBox.exec() == QMessageBox::Yes;
    args[MARGINTYPE] = computeMargin?"Yes":"No";
  }

  if (computeMargin)
  {
    m_borders = vtkSmartPointer<vtkPolyData>::New();
    QApplication::setOverrideCursor(Qt::WaitCursor);
    MarginDetector *marginDetector = new MarginDetector(this);
    connect(marginDetector, SIGNAL(finished()),
	    marginDetector, SLOT(deleteLater()));
    marginDetector->start();
    QApplication::restoreOverrideCursor();
  }

  m_init = true;
  m_useExtentMargins = !computeMargin;
  m_args = args;
}

//-----------------------------------------------------------------------------
QString MarginsChannelExtension::serialize() const
{
  return m_args.serialize();
}

//-----------------------------------------------------------------------------
QVariant MarginsChannelExtension::information(ModelItemExtension::InfoTag tag) const
{
  qWarning() << ID << ":"  << tag << " is not provided";
  Q_ASSERT(false);
  return QVariant();
}

//-----------------------------------------------------------------------------
ChannelExtension* MarginsChannelExtension::clone()
{
  return new MarginsChannelExtension();
}

//-----------------------------------------------------------------------------
void
MarginsChannelExtension::computeMarginDistance(Segmentation* seg)
{
  ModelItemExtension *ext = seg->extension(ID);
  Q_ASSERT(ext);
  MarginsSegmentationExtension *marginExt = dynamic_cast<MarginsSegmentationExtension *>(ext);
  Q_ASSERT(marginExt);
  Nm distance[6], smargins[6];
  VolumeBounds(seg->itkVolume(), smargins);
  if (m_useExtentMargins)
  {
    double cmargins[6];
    m_channel->bounds(cmargins);
    for (int i = 0; i < 6; i++)
      distance[i] = fabs(smargins[i] - cmargins[i]);
  }
  else
  {
    m_borderMutex.lock();

    /////
    vtkPoints *borderPoints = m_borders->GetPoints();

    vtkSmartPointer<vtkPlaneSource> borderPlane = vtkSmartPointer<vtkPlaneSource>::New();
    borderPlane->SetXResolution(100);
    borderPlane->SetYResolution(100);
    borderPlane->SetNormal(0,1,0);
    borderPlane->SetCenter(0,0,0);

    vtkSmartPointer<vtkDistancePolyDataFilter> distanceFilter = vtkSmartPointer<vtkDistancePolyDataFilter>::New();
    distanceFilter->SetInputConnection( 0, seg->mesh() );
    distanceFilter->SetInputConnection( 1, borderPlane->GetOutputPort(0));
    distanceFilter->Update();

    double minimumDistance = distanceFilter->GetSecondDistanceOutput()->GetPointData()->GetScalars()->GetRange()[0];

    /////
    Q_ASSERT(m_borders->GetNumberOfPoints() > 0);
    for (int i = 0; i < 6; i++)
      distance[i] = smargins[i];
    m_borderMutex.unlock();
  }
  marginExt->setMargins(distance);
}

//-----------------------------------------------------------------------------
vtkSmartPointer< vtkPolyData > MarginsChannelExtension::margins()
{
  return m_borders;
}
