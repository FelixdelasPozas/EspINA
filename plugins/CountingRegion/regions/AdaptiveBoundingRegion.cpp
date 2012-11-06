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


#include "AdaptiveBoundingRegion.h"

#include <common/model/Channel.h>
#include "vtkBoundingRegionSliceWidget.h"
#include <common/extensions/Margins/MarginsChannelExtension.h>
#include <common/widgets/EspinaInteractorAdapter.h>
#include <extensions/CountingRegionChannelExtension.h>

#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include "vtkBoundingRegion3DWidget.h"

class AdaptiveRegionWidget
: public SliceWidget
{
public:
  explicit AdaptiveRegionWidget(vtkBoundingRegionSliceWidget *widget)
  : SliceWidget(widget)
  , m_slicedWidget(widget)
  {}

  virtual void setSlice(Nm pos, PlaneType plane)
  {
    m_slicedWidget->SetSlice(pos);
    SliceWidget::setSlice(pos, plane);
  }
private:
  vtkBoundingRegionSliceWidget *m_slicedWidget;
};

//-----------------------------------------------------------------------------
const QString AdaptiveBoundingRegion::ID = "AdaptiveBoundingRegion";

//-----------------------------------------------------------------------------
AdaptiveBoundingRegion::AdaptiveBoundingRegion(CountingRegionChannelExtension *channelExt,
                                               Nm inclusion[3],
                                               Nm exclusion[3],
                                               ViewManager *vm)
: BoundingRegion(channelExt, inclusion, exclusion, vm)
, m_channel(channelExt->channel())
{
  updateBoundingRegion();
}

//-----------------------------------------------------------------------------
AdaptiveBoundingRegion::~AdaptiveBoundingRegion()
{
  m_channelExt->removeRegion(this);
  foreach(vtkAbstractWidget *w, m_widgets2D)
  {
    w->EnabledOn();
    w->Delete();
  }
  foreach(vtkAbstractWidget *w, m_widgets3D)
  {
    w->EnabledOn();
    w->Delete();
  }
  m_widgets2D.clear();
  m_widgets3D.clear();
}

//-----------------------------------------------------------------------------
QVariant AdaptiveBoundingRegion::data(int role) const
{
  if (role == Qt::DisplayRole)
    return tr("%1 - Adaptive Region").arg(m_channelExt->channel()->data().toString());

  return BoundingRegion::data(role);
}

//-----------------------------------------------------------------------------
QString AdaptiveBoundingRegion::serialize() const
{
  return QString("%1=%2,%3,%4,%5,%6,%7")
         .arg(ID)
         .arg(left(),0,'f',2).arg(top(),0,'f',2).arg(upper(),0,'f',2)
         .arg(right(),0,'f',2).arg(bottom(),0,'f',2).arg(lower(),0,'f',2);
}

//-----------------------------------------------------------------------------
vtkAbstractWidget* AdaptiveBoundingRegion::createWidget()
{
  BoundingRegion3DWidgetAdapter *wa = new BoundingRegion3DWidgetAdapter();
  Q_ASSERT(wa);
  wa->SetBoundingRegion(m_boundingRegion, m_inclusion, m_exclusion);

  m_widgets3D << wa;

  return wa;
}

//-----------------------------------------------------------------------------
void AdaptiveBoundingRegion::deleteWidget(vtkAbstractWidget* widget)
{
  widget->Off();
  widget->RemoveAllObservers();

  BoundingRegion3DWidgetAdapter *brwa3D = dynamic_cast<BoundingRegion3DWidgetAdapter *>(widget);
  if (brwa3D)
    m_widgets3D.removeAll(brwa3D);
  else
  {
    BoundingRegion2DWidgetAdapter *brwa2D = dynamic_cast<BoundingRegion2DWidgetAdapter *>(widget);
    if (brwa2D)
      m_widgets2D.removeAll(brwa2D);
    else
      Q_ASSERT(false);
  }

  widget->Delete();
}

//-----------------------------------------------------------------------------
SliceWidget* AdaptiveBoundingRegion::createSliceWidget(PlaneType plane)
{
  Channel *channel = m_channelExt->channel();
  double spacing[3];
  channel->spacing(spacing);

  BoundingRegion2DWidgetAdapter *wa = new BoundingRegion2DWidgetAdapter();
  Q_ASSERT(wa);
  wa->AddObserver(vtkCommand::EndInteractionEvent, this);
  wa->SetPlane(plane);
  wa->SetSlicingStep(spacing);
  wa->SetBoundingRegion(m_representation, m_inclusion, m_exclusion);

  m_widgets2D << wa;

  return new AdaptiveRegionWidget(wa);
}

//-----------------------------------------------------------------------------
bool AdaptiveBoundingRegion::processEvent(vtkRenderWindowInteractor* iren,
                                          long unsigned int event)
{
  foreach(BoundingRegion2DWidgetAdapter *wa, m_widgets2D)
  {
    if (wa->GetInteractor() == iren)
      return wa->ProcessEventsHandler(event);
  }
  foreach(BoundingRegion3DWidgetAdapter *wa, m_widgets3D)
  {
    if (wa->GetInteractor() == iren)
      return wa->ProcessEventsHandler(event);
  }

  return false;
}


//-----------------------------------------------------------------------------
void AdaptiveBoundingRegion::setEnabled(bool enable)
{
  Q_ASSERT(false);
}

#include <QDebug>

//-----------------------------------------------------------------------------
void AdaptiveBoundingRegion::updateBoundingRegionImplementation()
{
  double spacing[3];
  m_channel->spacing(spacing);
  int extent[6];
  m_channel->extent(extent);

  m_inclusionVolume = 0;
  m_totalVolume = 0;

  ModelItemExtension *ext = m_channel->extension(MarginsChannelExtension::ID);
  Q_ASSERT(ext);
  MarginsChannelExtension *marginsExt = dynamic_cast<MarginsChannelExtension *>(ext);
  Q_ASSERT(marginsExt);

  vtkSmartPointer<vtkPolyData> margins = marginsExt->margins();
  Q_ASSERT(margins.GetPointer());

  int inSliceOffset = upperOffset() / spacing[2];
  int exSliceOffset = lowerOffset() / spacing[2];

  int upperSlice = extent[4] + inSliceOffset;
  upperSlice = std::max(upperSlice, extent[4]);
  upperSlice = std::min(upperSlice, extent[5]);

  int lowerSlice = extent[5] + exSliceOffset;
  lowerSlice = std::max(lowerSlice, extent[4]);
  lowerSlice = std::min(lowerSlice, extent[5]);

  // upper and lower refer to Espina's orientation
  Q_ASSERT(upperSlice <= lowerSlice);

  m_boundingRegion = vtkSmartPointer<vtkPolyData>::New();
  m_representation = margins;

  vtkSmartPointer<vtkPoints> regionVertex = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkCellArray> faces = vtkSmartPointer<vtkCellArray>::New();
  vtkSmartPointer<vtkIntArray> faceData = vtkSmartPointer<vtkIntArray>::New();

  for (int slice = upperSlice; slice <= lowerSlice; slice++)
  {
    vtkIdType cell[4];
    vtkIdType lastCell[4];

    double LB[3], LT[3], RT[3], RB[3];

    margins->GetPoint(4*slice+0, LB);
    roundToSlice(LB[0], leftOffset());
    roundToSlice(LB[1], bottomOffset());
    roundToSlice(LB[2], 0);
    cell[0] = regionVertex->InsertNextPoint(LB);

    margins->GetPoint(4*slice+1, LT);
    roundToSlice(LT[0], leftOffset());
    roundToSlice(LT[1], topOffset());
    roundToSlice(LT[2], 0);
    cell[1] = regionVertex->InsertNextPoint(LT);

    margins->GetPoint(4*slice+2, RT);
    roundToSlice(RT[0], rightOffset());
    roundToSlice(RT[1], topOffset());
    roundToSlice(RT[2], 0);
    cell[2] = regionVertex->InsertNextPoint(RT);

    margins->GetPoint(4*slice+3, RB);
    roundToSlice(RB[0], rightOffset());
    roundToSlice(RB[1], bottomOffset());
    roundToSlice(RB[2], 0);
    cell[3] = regionVertex->InsertNextPoint(RB);
    if (slice == upperSlice)
    {
      // Upper Inclusion Face
      faces->InsertNextCell(4, cell);
      faceData->InsertNextValue(INCLUSION_FACE);
    } else if (slice == lowerSlice)
    {
      // Lower Inclusion Face
      faces->InsertNextCell(4, cell);
      faceData->InsertNextValue(EXCLUSION_FACE);
    } else
    {
      // Create lateral faces

      // Left Inclusion Face
      vtkIdType left[4];
      left[0] = lastCell[0];
      left[1] = lastCell[1];
      left[2] = cell[1];
      left[3] = cell[0];
      faces->InsertNextCell(4, left);
      faceData->InsertNextValue(INCLUSION_FACE);

      // Right Exclusion Face
      vtkIdType right[4];
      right[0] = lastCell[2];
      right[1] = lastCell[3];
      right[2] = cell[3];
      right[3] = cell[2];
      faces->InsertNextCell(4, right);
      faceData->InsertNextValue(EXCLUSION_FACE);

      // Top Inclusion Face
      vtkIdType top[4];
      top[0] = lastCell[1];
      top[1] = lastCell[2];
      top[2] = cell[2];
      top[3] = cell[1];
      faces->InsertNextCell(4, top);
      faceData->InsertNextValue(INCLUSION_FACE);

      // Bottom Exclusion Face
      vtkIdType bottom[4];
      bottom[0] = lastCell[3];
      bottom[1] = lastCell[0];
      bottom[2] = cell[0];
      bottom[3] = cell[3];
      faces->InsertNextCell(4, bottom);
      faceData->InsertNextValue(EXCLUSION_FACE);
    }
    memcpy(lastCell,cell,4*sizeof(vtkIdType));

    // Update Volumes
    if (slice != lowerSlice)
    {
      m_totalVolume += ((RT[0] - LT[0] + 1)*(LB[1] - LT[1] + 1))*spacing[2];
      m_inclusionVolume += (((RT[0] + rightOffset())  - (LT[0] + leftOffset()))*
                           ((LB[1] + bottomOffset()) - (LT[1] + topOffset())))*
                           spacing[2];
    }
  }

  m_boundingRegion->SetPoints(regionVertex);
  m_boundingRegion->SetPolys(faces);
  vtkCellData *data = m_boundingRegion->GetCellData();
  data->SetScalars(faceData);
  data->GetScalars()->SetName("Type");
}