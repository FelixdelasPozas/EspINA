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

#include "CrosshairRenderer.h"

#include <QDebug>

#include "common/model/Channel.h"
#include "common/model/Representation.h"

#include <vtkSmartPointer.h>
#include <vtkImageReslice.h>
#include <vtkImageMapToColors.h>
#include <vtkLookupTable.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkLine.h>
#include <vtkCellArray.h>

//-----------------------------------------------------------------------------
bool CrosshairRenderer::addItem(ModelItem* item)
{
  if (ModelItem::CHANNEL != item->type())
    return false;

  Channel *channel = dynamic_cast<Channel *>(item);

  // duplicated item? addItem again
  if (m_channels.contains(item))
  {
    QMap<ModelItem *, Representation>::iterator it = m_channels.find(item);

    if ((*it).visible)
    {
      m_renderer->RemoveActor((*it).axial);
      m_renderer->RemoveActor((*it).coronal);
      m_renderer->RemoveActor((*it).sagittal);
      m_renderer->RemoveActor((*it).axialBorder);
      m_renderer->RemoveActor((*it).coronalBorder);
      m_renderer->RemoveActor((*it).sagittalBorder);
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

    m_channels.remove(item);
  }

  itk::Matrix<double,3,3> direction = channel->volume()->GetDirection();

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

  vtkSmartPointer<vtkImageReslice> axialReslice = vtkSmartPointer<vtkImageReslice>::New();
  axialReslice->SetOptimization(true);
  axialReslice->BorderOn();
  axialReslice->SetInputConnection(channel->image());
  axialReslice->SetOutputDimensionality(2);
  axialReslice->SetResliceAxes(m_channels[channel].matAxial);

  vtkSmartPointer<vtkImageReslice> coronalReslice = vtkSmartPointer<vtkImageReslice>::New();
  coronalReslice->SetOptimization(true);
  coronalReslice->BorderOn();
  coronalReslice->SetInputConnection(channel->image());
  coronalReslice->SetOutputDimensionality(2);
  coronalReslice->SetResliceAxes(m_channels[channel].matCoronal);

  vtkSmartPointer<vtkImageReslice> sagittalReslice = vtkSmartPointer<vtkImageReslice>::New();
  sagittalReslice->SetOptimization(true);
  sagittalReslice->BorderOn();
  sagittalReslice->SetInputConnection(channel->image());
  sagittalReslice->SetOutputDimensionality(2);
  sagittalReslice->SetResliceAxes(m_channels[channel].matSagittal);

  // if given hue is -1 then just use 0 and make a grayscale image
  double color = channel->color();
  if (-1 == channel->color())
    color = 0;

  m_channels[channel].lut = vtkLookupTable::New();
  m_channels[channel].lut->Allocate();
  m_channels[channel].lut->SetTableRange(0,255);
  m_channels[channel].lut->SetValueRange(0.0, 1.0);
  m_channels[channel].lut->SetHueRange(color, color);
  m_channels[channel].lut->SetSaturationRange(0.0, 0.0);
  m_channels[channel].lut->SetAlphaRange(1.0,1.0);
  m_channels[channel].lut->SetNumberOfColors(256);
  m_channels[channel].lut->Build();
  channel->bounds(m_channels[channel].bounds);

  vtkSmartPointer<vtkImageMapToColors> axialImagemap = vtkSmartPointer<vtkImageMapToColors>::New();
  axialImagemap->SetLookupTable(m_channels[channel].lut);
  axialImagemap->SetOutputFormatToRGBA();
  axialImagemap->SetInputConnection(axialReslice->GetOutputPort());

  vtkSmartPointer<vtkImageMapToColors> coronalImagemap = vtkSmartPointer<vtkImageMapToColors>::New();
  coronalImagemap->SetLookupTable(m_channels[channel].lut);
  coronalImagemap->SetOutputFormatToRGBA();
  coronalImagemap->SetInputConnection(coronalReslice->GetOutputPort());

  vtkSmartPointer<vtkImageMapToColors> sagittalImagemap = vtkSmartPointer<vtkImageMapToColors>::New();
  sagittalImagemap->SetLookupTable(m_channels[channel].lut);
  sagittalImagemap->SetOutputFormatToRGBA();
  sagittalImagemap->SetInputConnection(sagittalReslice->GetOutputPort());


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
  origin[0] = m_channels[channel].coronal->GetOrigin()[0];
  origin[1] = m_channels[channel].coronal->GetOrigin()[1];
  origin[2] = m_channels[channel].coronal->GetOrigin()[2];
  center[0] = m_channels[channel].coronal->GetCenter()[0];
  center[1] = m_channels[channel].coronal->GetCenter()[1];
  center[2] = m_channels[channel].coronal->GetCenter()[2];
  m_channels[channel].coronal->SetOrigin(center);
  m_channels[channel].coronal->RotateX(90);
  m_channels[channel].coronal->SetOrigin(origin);

  origin[0] = m_channels[channel].sagittal->GetOrigin()[0];
  origin[1] = m_channels[channel].sagittal->GetOrigin()[1];
  origin[2] = m_channels[channel].sagittal->GetOrigin()[2];
  center[0] = m_channels[channel].sagittal->GetCenter()[0];
  center[1] = m_channels[channel].sagittal->GetCenter()[1];
  center[2] = m_channels[channel].sagittal->GetCenter()[2];
  m_channels[channel].sagittal->SetOrigin(center);
  m_channels[channel].sagittal->RotateX(90);
  m_channels[channel].sagittal->RotateY(90);
  m_channels[channel].sagittal->SetOrigin(origin);

  m_channels[channel].selected = channel->isSelected();
  m_channels[channel].visible = false;
  m_channels[channel].point[0] = 0;
  m_channels[channel].point[1] = 0;
  m_channels[channel].point[2] = 0;
  m_channels[channel].color.setHsvF(channel->color(), 1.0, 1.0);

  double bounds[6];
  channel->bounds(bounds);
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
  m_channels[channel].axialBorder->GetProperty()->SetLineWidth(2);
  m_channels[channel].axialBorder->SetPickable(false);

  m_channels[channel].coronalBorder = vtkActor::New();
  m_channels[channel].coronalBorder->SetMapper(coronalSquareMapper);
  m_channels[channel].coronalBorder->GetProperty()->SetColor(blue);
  m_channels[channel].coronalBorder->GetProperty()->SetPointSize(2);
  m_channels[channel].coronalBorder->GetProperty()->SetLineWidth(2);
  m_channels[channel].coronalBorder->SetPickable(false);

  m_channels[channel].sagittalBorder = vtkActor::New();
  m_channels[channel].sagittalBorder->SetMapper(sagittalSquareMapper);
  m_channels[channel].sagittalBorder->GetProperty()->SetColor(magenta);
  m_channels[channel].sagittalBorder->GetProperty()->SetPointSize(2);
  m_channels[channel].sagittalBorder->GetProperty()->SetLineWidth(2);
  m_channels[channel].sagittalBorder->SetPickable(false);

  return true;
}


//-----------------------------------------------------------------------------
bool CrosshairRenderer::updateItem(ModelItem* item)
{

  if (ModelItem::CHANNEL != item->type())
    return false;

  bool updated = false;
  Channel *channel = dynamic_cast<Channel *>(item);
  Q_ASSERT(m_channels.contains(channel));
  Representation &rep = m_channels[channel];

  if (channel->isVisible() != rep.visible
      || channel->color() != rep.color.hueF())
  {
    double color = channel->color();
    if (-1 == channel->color())
      color = 0;

    // don't want to update the color table everytime
    if (channel->color() != rep.color.hueF())
    {
      rep.lut->Allocate();
      rep.lut->SetTableRange(0,255);
      rep.lut->SetHueRange(color, color);
      rep.lut->SetSaturationRange(0.0, 0.0);
      rep.lut->SetValueRange(0.0, 1.0);
      rep.lut->SetAlphaRange(1.0,1.0);
      rep.lut->SetNumberOfColors(256);
      rep.lut->Build();
      rep.lut->Modified();
    }
    rep.color.setHsvF(channel->color(), 1.0, 1.0);
    updated = true;

    // now handle visibility
    if (m_enable && channel->isVisible())
    {
      if (!rep.visible)
      {
        m_renderer->AddActor(rep.axial);
        m_renderer->AddActor(rep.coronal);
        m_renderer->AddActor(rep.sagittal);
        m_renderer->AddActor(rep.axialBorder);
        m_renderer->AddActor(rep.coronalBorder);
        m_renderer->AddActor(rep.sagittalBorder);
        rep.visible = true;
      }
    }
    else
    {
      if (rep.visible)
      {
        m_renderer->RemoveActor(rep.axial);
        m_renderer->RemoveActor(rep.coronal);
        m_renderer->RemoveActor(rep.sagittal);
        m_renderer->RemoveActor(rep.axialBorder);
        m_renderer->RemoveActor(rep.coronalBorder);
        m_renderer->RemoveActor(rep.sagittalBorder);
        rep.visible = false;
      }
    }
  }

  return updated;
}

//-----------------------------------------------------------------------------
bool CrosshairRenderer::removeItem(ModelItem* item)
{
  if (ModelItem::CHANNEL != item->type())
    return false;

  Channel *channel = dynamic_cast<Channel *>(item);
  Q_ASSERT(m_channels.contains(channel));

  QMap<ModelItem *, Representation>::iterator it = m_channels.find(item);
  if ((*it).visible)
  {
    m_renderer->RemoveActor((*it).axial);
    m_renderer->RemoveActor((*it).coronal);
    m_renderer->RemoveActor((*it).sagittal);
    m_renderer->RemoveActor((*it).axialBorder);
    m_renderer->RemoveActor((*it).coronalBorder);
    m_renderer->RemoveActor((*it).sagittalBorder);
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

  m_channels.remove(channel);

  return true;
}


//-----------------------------------------------------------------------------
void CrosshairRenderer::hide()
{
  if (!this->m_enable)
    return;

  QMap<ModelItem *, Representation>::iterator it;
  for (it = m_channels.begin(); it != m_channels.end(); it++)
    if ((*it).visible)
    {
      m_renderer->RemoveActor((*it).axial);
      m_renderer->RemoveActor((*it).coronal);
      m_renderer->RemoveActor((*it).sagittal);
      m_renderer->RemoveActor((*it).axialBorder);
      m_renderer->RemoveActor((*it).coronalBorder);
      m_renderer->RemoveActor((*it).sagittalBorder);
      (*it).visible = false;
    }

  emit renderRequested();
}

//-----------------------------------------------------------------------------
void CrosshairRenderer::show()
{
  if (this->m_enable)
    return;

  QMap<ModelItem *, Representation>::iterator it;
  for (it = m_channels.begin(); it != m_channels.end(); it++)
    if (!(*it).visible)
    {
      m_renderer->AddActor((*it).axial);
      m_renderer->AddActor((*it).coronal);
      m_renderer->AddActor((*it).sagittal);
      m_renderer->AddActor((*it).axialBorder);
      m_renderer->AddActor((*it).coronalBorder);
      m_renderer->AddActor((*it).sagittalBorder);
      (*it).visible = true;
    }

  emit renderRequested();
}

//-----------------------------------------------------------------------------
unsigned int CrosshairRenderer::getNumberOfvtkActors()
{
  unsigned int numActors = 0;

  QMap<ModelItem *, Representation>::iterator it;
  for (it = m_channels.begin(); it != m_channels.end(); it++)
    if ((*it).visible)
      numActors += 6;

  return numActors;
}

//-----------------------------------------------------------------------------
void CrosshairRenderer::setCrosshairColors(double aColor[3], double cColor[3], double sColor[3])
{
  QMap<ModelItem *, Representation>::iterator it;
  for (it = m_channels.begin(); it != m_channels.end(); it++)
  {
    (*it).axialBorder->GetProperty()->SetColor(aColor);
    (*it).coronalBorder->GetProperty()->SetColor(cColor);
    (*it).sagittalBorder->GetProperty()->SetColor(sColor);
  }
}

//-----------------------------------------------------------------------------
void CrosshairRenderer::setCrosshair(Nm point[3])
{
  QMap<ModelItem *, Representation>::iterator it;
  for (it = m_channels.begin(); it != m_channels.end(); it++)
  {
    if (point[0] != (*it).point[0])
    {
      (*it).point[0] = point[0];
      if ((point[0] < (*it).bounds[0]) || (point[0] > (*it).bounds[1]))
      {
        (*it).sagittal->SetVisibility(false);
        (*it).sagittalBorder->SetVisibility(false);
      }
      else
      {
        // sagittal changed
        double actorpos[3];
        (*it).matSagittal->SetElement(0, 3, point[0]);
        (*it).sagittal->GetPosition(actorpos);
        actorpos[0] = point[0];
        (*it).sagittal->SetPosition(actorpos);
        (*it).sagittalBorder->SetPosition(actorpos);
      }
    }

    if (point[1] != (*it).point[1])
    {
      (*it).point[1] = point[1];
      if ((point[1] < (*it).bounds[2]) || (point[1] > (*it).bounds[3]))
      {
        (*it).coronal->SetVisibility(false);
        (*it).coronalBorder->SetVisibility(false);
      }
      else
      {
        // coronal changed
        double actorpos[3];
        (*it).matCoronal->SetElement(1, 3, point[1]);
        (*it).coronal->GetPosition(actorpos);
        actorpos[1] = point[1];
        (*it).coronal->SetPosition(actorpos);
        (*it).coronalBorder->SetPosition(actorpos);
      }
    }

    if (point[2] != (*it).point[2])
    {
      (*it).point[2] = point[2];
      if ((point[2] < (*it).bounds[4]) || (point[2] > (*it).bounds[5]))
      {
        (*it).axial->SetVisibility(false);
        (*it).axialBorder->SetVisibility(false);
      }
      else
      {
        // axial changed
        double actorpos[3];
        (*it).matAxial->SetElement(2, 3, point[2]);
        (*it).axial->GetPosition(actorpos);
        actorpos[2] = point[2];
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

