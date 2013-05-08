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

// EspINA
#include "CrosshairRenderer.h"
#include <Core/Model/Channel.h>

// VTK
#include <vtkSmartPointer.h>
#include <vtkImageReslice.h>
#include <vtkImageMapToColors.h>
#include <vtkLookupTable.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkLine.h>
#include <vtkCellArray.h>
#include <vtkImageActor.h>
#include <vtkActor.h>
#include <vtkPolyData.h>
#include <vtkMatrix4x4.h>
#include <vtkLookupTable.h>
#include <vtkImageShiftScale.h>
#include <vtkPropPicker.h>

using namespace EspINA;

//-----------------------------------------------------------------------------
CrosshairRenderer::CrosshairRenderer(QObject* parent)
: IRenderer(parent)
, m_picker(vtkSmartPointer<vtkPropPicker>::New())
{
  m_picker->PickFromListOn();
}

//-----------------------------------------------------------------------------
bool CrosshairRenderer::addItem(ModelItemPtr item)
{
  if (EspINA::CHANNEL != item->type())
    return false;

  ChannelPtr channel = channelPtr(item);

  // duplicated item? addItem again
  if (m_channels.contains(item))
  {
    QMap<ModelItemPtr, Representation>::iterator it = m_channels.find(item);

    if ((*it).visible)
    {
      m_renderer->RemoveActor((*it).axial);
      m_renderer->RemoveActor((*it).coronal);
      m_renderer->RemoveActor((*it).sagittal);
      m_renderer->RemoveActor((*it).axialBorder);
      m_renderer->RemoveActor((*it).coronalBorder);
      m_renderer->RemoveActor((*it).sagittalBorder);
      m_picker->DeletePickList((*it).axial);
      m_picker->DeletePickList((*it).coronal);
      m_picker->DeletePickList((*it).sagittal);
    }

    (*it).axial->Delete();
    (*it).coronal->Delete();
    (*it).sagittal->Delete();
    (*it).matAxial->Delete();
    (*it).matCoronal->Delete();
    (*it).matSagittal->Delete();
    (*it).axialBorder->Delete();
    (*it).coronalBorder->Delete();
    (*it).sagittalBorder->Delete();
    (*it).axialScaler->Delete();
    (*it).coronalScaler->Delete();
    (*it).sagittalScaler->Delete();

    m_channels.remove(item);
  }

  itk::Matrix<double,3,3> direction = channel->volume()->toITK()->GetDirection();

  // copy direction matrix to reslice matrices
  double axialMat[16] = { direction(0,0), direction(0,1), direction(0,2), 0,
                          direction(1,0), direction(1,1), direction(1,2), 0,
                          direction(2,0), direction(2,1), direction(2,2), 0,
                          0, 0, 0, 1 };

  double coronalMat[16] = { direction(0,0), direction(0,1), direction(0,2), 0,
                            direction(2,0), direction(2,1), direction(2,2), 0,
                            direction(1,0), direction(1,1), direction(1,2), 0,
                            0, 0, 0, 1 };

  double sagittalMat[16] = { direction(2,0), direction(2,1), direction(2,2), 0,
                             direction(0,0), direction(0,1), direction(0,2), 0,
                             direction(1,0), direction(1,1), direction(1,2), 0,
                             0, 0, 0, 1 };

  m_channels[channel].matAxial = vtkMatrix4x4::New();
  m_channels[channel].matAxial->DeepCopy(axialMat);

  m_channels[channel].matCoronal = vtkMatrix4x4::New();
  m_channels[channel].matCoronal->DeepCopy(coronalMat);

  m_channels[channel].matSagittal = vtkMatrix4x4::New();
  m_channels[channel].matSagittal->DeepCopy(sagittalMat);

  vtkAlgorithmOutput *vtkVolume = channel->volume()->toVTK();

  vtkSmartPointer<vtkImageReslice> axialReslice = vtkSmartPointer<vtkImageReslice>::New();
  axialReslice->SetOptimization(true);
  axialReslice->BorderOn();
  axialReslice->SetInputConnection(vtkVolume);
  axialReslice->SetOutputDimensionality(2);
  axialReslice->SetResliceAxes(m_channels[channel].matAxial);

  vtkSmartPointer<vtkImageReslice> coronalReslice = vtkSmartPointer<vtkImageReslice>::New();
  coronalReslice->SetOptimization(true);
  coronalReslice->BorderOn();
  coronalReslice->SetInputConnection(vtkVolume);
  coronalReslice->SetOutputDimensionality(2);
  coronalReslice->SetResliceAxes(m_channels[channel].matCoronal);

  vtkSmartPointer<vtkImageReslice> sagittalReslice = vtkSmartPointer<vtkImageReslice>::New();
  sagittalReslice->SetOptimization(true);
  sagittalReslice->BorderOn();
  sagittalReslice->SetInputConnection(vtkVolume);
  sagittalReslice->SetOutputDimensionality(2);
  sagittalReslice->SetResliceAxes(m_channels[channel].matSagittal);

  // if hue is -1 then use 0 saturation to make a grayscale image
  double hue = channel->hue();
  double sat = hue >= 0?1.0:0.0;

  m_channels[channel].lut = vtkLookupTable::New();
  m_channels[channel].lut->Allocate();
  m_channels[channel].lut->SetTableRange(0,255);
  m_channels[channel].lut->SetHueRange(hue, hue);
  m_channels[channel].lut->SetSaturationRange(0.0, sat);
  m_channels[channel].lut->SetValueRange(0.0, 1.0);
  m_channels[channel].lut->SetAlphaRange(1.0,1.0);
  m_channels[channel].lut->SetNumberOfColors(256);
  m_channels[channel].lut->SetRampToLinear();
  m_channels[channel].lut->Build();
  channel->volume()->bounds(m_channels[channel].bounds);

  m_channels[channel].contrast = channel->contrast();
  m_channels[channel].brightness = channel->brightness();

  m_channels[channel].axialScaler = vtkImageShiftScale::New();
  m_channels[channel].axialScaler->SetInputConnection(axialReslice->GetOutputPort());
  m_channels[channel].axialScaler->SetShift(static_cast<int>(channel->brightness()*255));
  m_channels[channel].axialScaler->SetScale(channel->contrast());
  m_channels[channel].axialScaler->SetClampOverflow(true);
  m_channels[channel].axialScaler->SetOutputScalarType(axialReslice->GetOutput()->GetScalarType());
  m_channels[channel].axialScaler->Update();

  vtkSmartPointer<vtkImageMapToColors> axialImagemap = vtkSmartPointer<vtkImageMapToColors>::New();
  axialImagemap->SetLookupTable(m_channels[channel].lut);
  axialImagemap->SetOutputFormatToRGBA();
  axialImagemap->SetInputConnection(m_channels[channel].axialScaler->GetOutputPort());

  m_channels[channel].coronalScaler = vtkImageShiftScale::New();
  m_channels[channel].coronalScaler->SetInputConnection(coronalReslice->GetOutputPort());
  m_channels[channel].coronalScaler->SetShift(static_cast<int>(channel->brightness()*255));
  m_channels[channel].coronalScaler->SetScale(channel->contrast());
  m_channels[channel].coronalScaler->SetClampOverflow(true);
  m_channels[channel].coronalScaler->SetOutputScalarType(coronalReslice->GetOutput()->GetScalarType());
  m_channels[channel].coronalScaler->Update();

  vtkSmartPointer<vtkImageMapToColors> coronalImagemap = vtkSmartPointer<vtkImageMapToColors>::New();
  coronalImagemap->SetLookupTable(m_channels[channel].lut);
  coronalImagemap->SetOutputFormatToRGBA();
  coronalImagemap->SetInputConnection(m_channels[channel].coronalScaler->GetOutputPort());

  m_channels[channel].sagittalScaler = vtkImageShiftScale::New();
  m_channels[channel].sagittalScaler->SetInputConnection(sagittalReslice->GetOutputPort());
  m_channels[channel].sagittalScaler->SetShift(static_cast<int>(channel->brightness()*255));
  m_channels[channel].sagittalScaler->SetScale(channel->contrast());
  m_channels[channel].sagittalScaler->SetClampOverflow(true);
  m_channels[channel].sagittalScaler->SetOutputScalarType(sagittalReslice->GetOutput()->GetScalarType());
  m_channels[channel].sagittalScaler->Update();

  vtkSmartPointer<vtkImageMapToColors> sagittalImagemap = vtkSmartPointer<vtkImageMapToColors>::New();
  sagittalImagemap->SetLookupTable(m_channels[channel].lut);
  sagittalImagemap->SetOutputFormatToRGBA();
  sagittalImagemap->SetInputConnection(m_channels[channel].sagittalScaler->GetOutputPort());

  m_channels[channel].axial = vtkImageActor::New();
  m_channels[channel].axial->SetInput(axialImagemap->GetOutput());
  m_channels[channel].axial->SetInterpolate(false);

  m_channels[channel].coronal = vtkImageActor::New();
  m_channels[channel].coronal->SetInput(coronalImagemap->GetOutput());
  m_channels[channel].coronal->SetInterpolate(false);

  m_channels[channel].sagittal = vtkImageActor::New();
  m_channels[channel].sagittal->SetInput(sagittalImagemap->GetOutput());
  m_channels[channel].sagittal->SetInterpolate(false);

  // rotate actors
  double center[3], origin[3];
  memcpy(origin, m_channels[channel].coronal->GetOrigin(), 3*sizeof(double));
  memcpy(center, m_channels[channel].coronal->GetCenter(), 3*sizeof(double));
  m_channels[channel].coronal->SetOrigin(center);
  m_channels[channel].coronal->RotateX(90);
  m_channels[channel].coronal->SetOrigin(origin);

  memcpy(origin, m_channels[channel].sagittal->GetOrigin(), 3*sizeof(double));
  memcpy(center, m_channels[channel].sagittal->GetCenter(), 3*sizeof(double));
  m_channels[channel].sagittal->SetOrigin(center);
  m_channels[channel].sagittal->RotateX(90);
  m_channels[channel].sagittal->RotateY(90);
  m_channels[channel].sagittal->SetOrigin(origin);

  m_channels[channel].selected = channel->isSelected();
  m_channels[channel].visible = false;
  memset(m_channels[channel].point, 0, 3*sizeof(double));
  m_channels[channel].color.setHsvF(channel->hue(), 1.0, 1.0);

  double bounds[6];
  channel->volume()->bounds(bounds);
  double ap0[3] = { bounds[0], bounds[2], bounds[4] };
  double ap1[3] = { bounds[0], bounds[3], bounds[4] };
  double ap2[3] = { bounds[1], bounds[3], bounds[4] };
  double ap3[3] = { bounds[1], bounds[2], bounds[4] };

  // Create a vtkPoints object and store the points in it
  vtkSmartPointer < vtkPoints > axialPoints = vtkSmartPointer<vtkPoints>::New();
  axialPoints->InsertNextPoint(ap0);
  axialPoints->InsertNextPoint(ap1);
  axialPoints->InsertNextPoint(ap2);
  axialPoints->InsertNextPoint(ap3);
  axialPoints->InsertNextPoint(ap0);

  // Create a cell array to store the lines in and add the lines to it
  vtkSmartPointer < vtkCellArray > axialLines = vtkSmartPointer<vtkCellArray>::New();

  for (unsigned int i = 0; i < 4; i++)
  {
      vtkSmartPointer < vtkLine > line = vtkSmartPointer<vtkLine>::New();
      line->GetPointIds()->SetId(0, i);
      line->GetPointIds()->SetId(1, i + 1);
      axialLines->InsertNextCell(line);
  }

  // Add the points & lines to the dataset
  m_channels[channel].axialSquare = vtkPolyData::New();
  m_channels[channel].axialSquare->Reset();
  m_channels[channel].axialSquare->SetPoints(axialPoints);
  m_channels[channel].axialSquare->SetLines(axialLines);
  m_channels[channel].axialSquare->Update();

  double cp0[3] = { bounds[0], bounds[2], bounds[4] };
  double cp1[3] = { bounds[0], bounds[2], bounds[5] };
  double cp2[3] = { bounds[1], bounds[2], bounds[5] };
  double cp3[3] = { bounds[1], bounds[2], bounds[4] };

  // Create a vtkPoints object and store the points in it
  vtkSmartPointer < vtkPoints > coronalPoints = vtkSmartPointer<vtkPoints>::New();
  coronalPoints->InsertNextPoint(cp0);
  coronalPoints->InsertNextPoint(cp1);
  coronalPoints->InsertNextPoint(cp2);
  coronalPoints->InsertNextPoint(cp3);
  coronalPoints->InsertNextPoint(cp0);

  // Create a cell array to store the lines in and add the lines to it
  vtkSmartPointer < vtkCellArray > coronalLines = vtkSmartPointer<vtkCellArray>::New();

  for (unsigned int i = 0; i < 4; i++)
  {
      vtkSmartPointer < vtkLine > line = vtkSmartPointer<vtkLine>::New();
      line->GetPointIds()->SetId(0, i);
      line->GetPointIds()->SetId(1, i + 1);
      coronalLines->InsertNextCell(line);
  }

  m_channels[channel].coronalSquare = vtkPolyData::New();
  m_channels[channel].coronalSquare->Reset();
  m_channels[channel].coronalSquare->SetPoints(coronalPoints);
  m_channels[channel].coronalSquare->SetLines(coronalLines);
  m_channels[channel].coronalSquare->Update();

  double sp0[3] =  { bounds[0], bounds[2], bounds[4] };
  double sp1[3] =  { bounds[0], bounds[2], bounds[5] };
  double sp2[3] =  { bounds[0], bounds[3], bounds[5] };
  double sp3[3] =  { bounds[0], bounds[3], bounds[4] };

  // Create a vtkPoints object and store the points in it
  vtkSmartPointer<vtkPoints> sagittalPoints = vtkSmartPointer<vtkPoints>::New();
  sagittalPoints->InsertNextPoint(sp0);
  sagittalPoints->InsertNextPoint(sp1);
  sagittalPoints->InsertNextPoint(sp2);
  sagittalPoints->InsertNextPoint(sp3);
  sagittalPoints->InsertNextPoint(sp0);

  // Create a cell array to store the lines in and add the lines to it
  vtkSmartPointer<vtkCellArray> sagittalLines = vtkSmartPointer<vtkCellArray>::New();

  for (unsigned int i = 0; i < 4; i++)
  {
    vtkSmartPointer<vtkLine> line = vtkSmartPointer<vtkLine>::New();
    line->GetPointIds()->SetId(0, i);
    line->GetPointIds()->SetId(1, i + 1);
    sagittalLines->InsertNextCell(line);
  }

  m_channels[channel].sagittalSquare = vtkPolyData::New();
  m_channels[channel].sagittalSquare->Reset();
  m_channels[channel].sagittalSquare->SetPoints(sagittalPoints);
  m_channels[channel].sagittalSquare->SetLines(sagittalLines);
  m_channels[channel].sagittalSquare->Update();

  vtkSmartPointer<vtkPolyDataMapper> axialSquareMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  axialSquareMapper->SetInput(m_channels[channel].axialSquare);
  vtkSmartPointer<vtkPolyDataMapper> coronalSquareMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  coronalSquareMapper->SetInput(m_channels[channel].coronalSquare);
  vtkSmartPointer<vtkPolyDataMapper> sagittalSquareMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  sagittalSquareMapper->SetInput(m_channels[channel].sagittalSquare);

  // these colors have been defined in DefaultEspinaView.cpp for the 2D crosshair
  double cyan[3] = { 0, 1, 1 };
  double blue[3] = { 0, 0, 1 };
  double magenta[3] = { 1, 0, 1 };

  m_channels[channel].axialBorder = vtkActor::New();
  m_channels[channel].axialBorder->SetMapper(axialSquareMapper);
  m_channels[channel].axialBorder->GetProperty()->SetColor(cyan);
  m_channels[channel].axialBorder->GetProperty()->SetPointSize(2);
  m_channels[channel].axialBorder->GetProperty()->SetLineWidth(1);
  m_channels[channel].axialBorder->SetPickable(false);

  m_channels[channel].coronalBorder = vtkActor::New();
  m_channels[channel].coronalBorder->SetMapper(coronalSquareMapper);
  m_channels[channel].coronalBorder->GetProperty()->SetColor(blue);
  m_channels[channel].coronalBorder->GetProperty()->SetPointSize(2);
  m_channels[channel].coronalBorder->GetProperty()->SetLineWidth(1);
  m_channels[channel].coronalBorder->SetPickable(false);

  m_channels[channel].sagittalBorder = vtkActor::New();
  m_channels[channel].sagittalBorder->SetMapper(sagittalSquareMapper);
  m_channels[channel].sagittalBorder->GetProperty()->SetColor(magenta);
  m_channels[channel].sagittalBorder->GetProperty()->SetPointSize(2);
  m_channels[channel].sagittalBorder->GetProperty()->SetLineWidth(1);
  m_channels[channel].sagittalBorder->SetPickable(false);

  if (m_enable)
  {
    m_channels[channel].visible = true;
    m_renderer->AddActor(m_channels[channel].axial);
    m_renderer->AddActor(m_channels[channel].coronal);
    m_renderer->AddActor(m_channels[channel].sagittal);
    m_picker->AddPickList(m_channels[channel].axial);
    m_picker->AddPickList(m_channels[channel].coronal);
    m_picker->AddPickList(m_channels[channel].sagittal);
  }

  return true;
}


//-----------------------------------------------------------------------------
bool CrosshairRenderer::updateItem(ModelItemPtr item, bool forced)
{
  if (!m_enable && !forced)
    return false;

  if (EspINA::CHANNEL != item->type())
    return false;

  bool updated = false;
  ChannelPtr channel = channelPtr(item);
  Q_ASSERT(m_channels.contains(channel));
  Representation &rep = m_channels[channel];

  if (channel->isVisible())
  {
    if (!rep.visible)
    {
      m_renderer->AddActor(rep.axial);
      m_renderer->AddActor(rep.coronal);
      m_renderer->AddActor(rep.sagittal);
      m_renderer->AddActor(rep.axialBorder);
      m_renderer->AddActor(rep.coronalBorder);
      m_renderer->AddActor(rep.sagittalBorder);
      m_picker->AddPickList(rep.axial);
      m_picker->AddPickList(rep.coronal);
      m_picker->AddPickList(rep.sagittal);
      rep.visible = true;
      updated = true;
    }
  }
  else
  {
    // return avoiding updated to the VTK pipelines
    if (rep.visible)
    {
      m_renderer->RemoveActor(rep.axial);
      m_renderer->RemoveActor(rep.coronal);
      m_renderer->RemoveActor(rep.sagittal);
      m_renderer->RemoveActor(rep.axialBorder);
      m_renderer->RemoveActor(rep.coronalBorder);
      m_renderer->RemoveActor(rep.sagittalBorder);
      m_picker->DeletePickList(rep.axial);
      m_picker->DeletePickList(rep.axial);
      m_picker->DeletePickList(rep.axial);
      rep.visible = false;
      return true;
    }
    return false;
  }


  if (((channel->hue() != -1) && ((rep.color.hueF() != channel->hue()) || (rep.color.saturation() != 1.0))) ||
     (((channel->hue() == -1) && ((rep.color.hue() != 0) || (rep.color.saturation() != 0)))))
  {
    // if hue is -1 then use 0 saturation to make a grayscale image
    double hue = (channel->hue() == -1) ? 0 : channel->hue();
    double sat = channel->hue() >= 0 ? 1.0 : 0.0;
    rep.color.setHsvF(hue, sat, 1.0);

    rep.lut->Allocate();
    rep.lut->SetTableRange(0,255);
    rep.lut->SetHueRange(hue, hue);
    rep.lut->SetSaturationRange(0.0, sat);
    rep.lut->SetValueRange(0.0, 1.0);
    rep.lut->SetAlphaRange(1.0,1.0);
    rep.lut->SetNumberOfColors(256);
    rep.lut->Build();
    rep.lut->Modified();

    updated = true;
  }

  if (rep.brightness != channel->brightness() || rep.contrast != channel->contrast())
  {
    rep.axialScaler->SetShift(static_cast<int>(channel->brightness()*255));
    rep.axialScaler->SetScale(channel->contrast());
    rep.axialScaler->Update();
    rep.coronalScaler->SetShift(static_cast<int>(channel->brightness()*255));
    rep.coronalScaler->SetScale(channel->contrast());
    rep.coronalScaler->Update();
    rep.sagittalScaler->SetShift(static_cast<int>(channel->brightness()*255));
    rep.sagittalScaler->SetScale(channel->contrast());
    rep.sagittalScaler->Update();

    rep.contrast = channel->contrast();
    rep.brightness = channel->brightness();

    updated = true;
  }

  return updated;
}

//-----------------------------------------------------------------------------
bool CrosshairRenderer::removeItem(ModelItemPtr item)
{
  if (EspINA::CHANNEL != item->type())
    return false;

  ChannelPtr channel = channelPtr(item);
  Q_ASSERT(m_channels.contains(channel));

  QMap<ModelItemPtr, Representation>::iterator it = m_channels.find(item);
  if ((*it).visible)
  {
    m_renderer->RemoveActor((*it).axial);
    m_renderer->RemoveActor((*it).coronal);
    m_renderer->RemoveActor((*it).sagittal);
    m_renderer->RemoveActor((*it).axialBorder);
    m_renderer->RemoveActor((*it).coronalBorder);
    m_renderer->RemoveActor((*it).sagittalBorder);
    m_picker->DeletePickList((*it).axial);
    m_picker->DeletePickList((*it).coronal);
    m_picker->DeletePickList((*it).sagittal);
  }

  (*it).axial->Delete();
  (*it).coronal->Delete();
  (*it).sagittal->Delete();
  (*it).matAxial->Delete();
  (*it).matCoronal->Delete();
  (*it).matSagittal->Delete();
  (*it).axialBorder->Delete();
  (*it).coronalBorder->Delete();
  (*it).sagittalBorder->Delete();
  (*it).axialSquare->Delete();
  (*it).coronalSquare->Delete();
  (*it).sagittalSquare->Delete();
  (*it).axialScaler->Delete();
  (*it).coronalScaler->Delete();
  (*it).sagittalScaler->Delete();

  m_channels.remove(channel);

  return true;
}


//-----------------------------------------------------------------------------
void CrosshairRenderer::hide()
{
  if (!this->m_enable)
    return;

  m_enable = false;
  QMap<ModelItemPtr, Representation>::iterator it;
  for (it = m_channels.begin(); it != m_channels.end(); ++it)
    if ((*it).visible)
    {
      m_renderer->RemoveActor((*it).axial);
      m_renderer->RemoveActor((*it).coronal);
      m_renderer->RemoveActor((*it).sagittal);
      m_renderer->RemoveActor((*it).axialBorder);
      m_renderer->RemoveActor((*it).coronalBorder);
      m_renderer->RemoveActor((*it).sagittalBorder);
      m_picker->DeletePickList((*it).axial);
      m_picker->DeletePickList((*it).coronal);
      m_picker->DeletePickList((*it).sagittal);
      (*it).visible = false;
    }

  emit renderRequested();
}

//-----------------------------------------------------------------------------
void CrosshairRenderer::show()
{
  if (this->m_enable)
    return;

  m_enable = true;
  QMap<ModelItemPtr, Representation>::iterator it;
  for (it = m_channels.begin(); it != m_channels.end(); ++it)
    if (!(*it).visible)
      updateItem(it.key(), true);

  emit renderRequested();
}

//-----------------------------------------------------------------------------
unsigned int CrosshairRenderer::getNumberOfvtkActors()
{
  unsigned int numActors = 0;

  QMap<ModelItemPtr, Representation>::iterator it;
  for (it = m_channels.begin(); it != m_channels.end(); ++it)
    if ((*it).visible)
      numActors += 6;

  return numActors;
}

//-----------------------------------------------------------------------------
void CrosshairRenderer::setCrosshairColors(double aColor[3], double cColor[3], double sColor[3])
{
  QMap<ModelItemPtr, Representation>::iterator it;
  for (it = m_channels.begin(); it != m_channels.end(); ++it)
  {
    (*it).axialBorder->GetProperty()->SetColor(aColor);
    (*it).coronalBorder->GetProperty()->SetColor(cColor);
    (*it).sagittalBorder->GetProperty()->SetColor(sColor);
  }
}

//-----------------------------------------------------------------------------
void CrosshairRenderer::setCrosshair(Nm point[3])
{
  QMap<ModelItemPtr, Representation>::iterator it;
  for (it = m_channels.begin(); it != m_channels.end(); ++it)
  {
    if (point[0] != (*it).point[0])
    {
      (*it).point[0] = point[0];
      if (((point[0] < (*it).bounds[0]) || (point[0] > (*it).bounds[1])) && (m_channels.size() > 1))
      {
        (*it).sagittal->SetVisibility(false);
        (*it).sagittalBorder->SetVisibility(false);
      }
      else
      {
        // if not tiling
        if (point[0] < (*it).bounds[0])
          (*it).point[0] = (*it).bounds[0];

        if (point[0] > (*it).bounds[1])
          (*it).point[0] = (*it).bounds[1];

        // sagittal changed
        if ((*it).sagittal->GetVisibility() == false)
        {
          (*it).sagittal->SetVisibility(true);
          (*it).sagittalBorder->SetVisibility(true);
        }

        double actorpos[3];
        (*it).matSagittal->SetElement(0, 3, (*it).point[0]);
        (*it).sagittal->GetPosition(actorpos);
        actorpos[0] = (*it).point[0];
        (*it).sagittal->SetPosition(actorpos);
        (*it).sagittalBorder->SetPosition(actorpos);
      }
    }

    if (point[1] != (*it).point[1])
    {
      (*it).point[1] = point[1];
      if (((point[1] < (*it).bounds[2]) || (point[1] > (*it).bounds[3])) && (m_channels.size() > 1))
      {
        (*it).coronal->SetVisibility(false);
        (*it).coronalBorder->SetVisibility(false);
      }
      else
      {
        // if not tiling
        if (point[1] < (*it).bounds[2])
          (*it).point[1] = (*it).bounds[0];

        if (point[1] > (*it).bounds[3])
          (*it).point[1] = (*it).bounds[1];

        // coronal changed
        if ((*it).coronal->GetVisibility() == false)
        {
          (*it).coronal->SetVisibility(true);
          (*it).coronalBorder->SetVisibility(true);
        }

        double actorpos[3];
        (*it).matCoronal->SetElement(1, 3, (*it).point[1]);
        (*it).coronal->GetPosition(actorpos);
        actorpos[1] = (*it).point[1];
        (*it).coronal->SetPosition(actorpos);
        (*it).coronalBorder->SetPosition(actorpos);
      }
    }

    if (point[2] != (*it).point[2])
    {
      (*it).point[2] = point[2];
      if (((point[2] < (*it).bounds[4]) || (point[2] > (*it).bounds[5])) && (m_channels.size() > 1))
      {
        (*it).axial->SetVisibility(false);
        (*it).axialBorder->SetVisibility(false);
      }
      else
      {
        // if not tiling
        if (point[2] < (*it).bounds[4])
          (*it).point[2] = (*it).bounds[4];

        if (point[2] > (*it).bounds[5])
          (*it).point[2] = (*it).bounds[5];

        // axial changed
        if ((*it).axial->GetVisibility() == false)
        {
          (*it).axial->SetVisibility(true);
          (*it).axialBorder->SetVisibility(true);
        }

        double actorpos[3];
        (*it).matAxial->SetElement(2, 3, (*it).point[2]);
        (*it).axial->GetPosition(actorpos);
        actorpos[2] = (*it).point[2];
        (*it).axial->SetPosition(actorpos);
        (*it).axialBorder->SetPosition(actorpos);
      }
    }
  }
}

//-----------------------------------------------------------------------------
void CrosshairRenderer::setPlanePosition(PlaneType plane, Nm dist)
{
  QMap<ModelItem *, Representation>::iterator it;
  if (m_channels.empty())
    return;

  Nm point[3] = { m_channels.begin()->point[0], m_channels.begin()->point[1], m_channels.begin()->point[2] };

  switch(plane)
  {
  case AXIAL:
    point[2] = dist;
    break;
  case CORONAL:
    point[1] = dist;
    break;
  case SAGITTAL:
    point[0] = dist;
    break;
  default:
    break;
  }

  setCrosshair(point);
}

//-----------------------------------------------------------------------------
GraphicalRepresentationSList CrosshairRenderer::pick(int x, int y, bool repeat)
{
  GraphicalRepresentationSList selection;
  QList<vtkProp*> removedProps;

  if (m_renderer)
  {
    while (m_picker->Pick(x,y,0, m_renderer))
    {
      vtkProp *pickedProp = m_picker->GetViewProp();
      Q_ASSERT(pickedProp);


      m_picker->GetPickList()->RemoveItem(pickedProp);
      removedProps << pickedProp;

      foreach(GraphicalRepresentationSPtr rep, m_representations)
        if (rep->hasActor(pickedProp))
        {
          selection << rep;
          break;
        }

      if (!repeat)
        break;
    }
  }

  foreach(vtkProp *actor, removedProps)
    m_picker->GetPickList()->AddItem(actor);

  return selection;}

//-----------------------------------------------------------------------------
void CrosshairRenderer::getPickCoordinates(double *point)
{
  m_picker->GetPickPosition(point);
}
