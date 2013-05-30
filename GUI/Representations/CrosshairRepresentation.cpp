/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2013 Félix de las Pozas Álvarez <felixdelaspozas@gmail.com>

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
#include "CrosshairRepresentation.h"
#include "GUI/QtWidget/VolumeView.h"

// VTK
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
#include <vtkImageData.h>

// Qt
#include <QColor>

using namespace EspINA;

//------------------------------------------------------------------------
CrosshairRepresentation::CrosshairRepresentation(ChannelVolumeSPtr data,
                                                 VolumeView       *view)
: ChannelGraphicalRepresentation(view)
, m_data(data)
, m_tiling(false)
{
}

//-----------------------------------------------------------------------------
void CrosshairRepresentation::setBrightness(double value)
{
  ChannelGraphicalRepresentation::setBrightness(value);

  m_axialScaler->SetShift(static_cast<int>(m_brightness*255));
  m_coronalScaler->SetShift(static_cast<int>(m_brightness*255));
  m_sagittalScaler->SetShift(static_cast<int>(m_brightness*255));
}

//-----------------------------------------------------------------------------
void CrosshairRepresentation::setContrast(double value)
{
  ChannelGraphicalRepresentation::setContrast(value);

  m_axialScaler->SetScale(m_contrast);
  m_coronalScaler->SetScale(m_contrast);
  m_sagittalScaler->SetScale(m_contrast);
}

//-----------------------------------------------------------------------------
void CrosshairRepresentation::setColor(const QColor &color)
{
  GraphicalRepresentation::setColor(color);

  m_lut->SetHueRange(color.hueF(), color.hueF());
  m_lut->SetSaturationRange(0.0, color.saturationF());
  m_lut->Build();
}

//-----------------------------------------------------------------------------
void CrosshairRepresentation::setOpacity(double value)
{
  ChannelGraphicalRepresentation::setOpacity(value);

  m_axial->SetOpacity(m_opacity);
  m_coronal->SetOpacity(m_opacity);
  m_sagittal->SetOpacity(m_opacity);
  m_axialBorder->GetProperty()->SetOpacity(m_opacity);
  m_coronalBorder->GetProperty()->SetOpacity(m_opacity);
  m_sagittalBorder->GetProperty()->SetOpacity(m_opacity);
}

//-----------------------------------------------------------------------------
bool CrosshairRepresentation::isInside(Nm *point)
{
  return ((m_bounds[0] <= point[0]) &&
          (m_bounds[1] >= point[0]) &&
          (m_bounds[2] <= point[1]) &&
          (m_bounds[3] >= point[1]) &&
          (m_bounds[4] <= point[2]) &&
          (m_bounds[5] >= point[2]));
}

//-----------------------------------------------------------------------------
bool CrosshairRepresentation::hasActor(vtkProp *actor) const
{
  return (m_axial.GetPointer()          == actor ||
          m_coronal.GetPointer()        == actor ||
          m_sagittal.GetPointer()       == actor ||
          m_axialBorder.GetPointer()    == actor ||
          m_coronalBorder.GetPointer()  == actor ||
          m_sagittalBorder.GetPointer() == actor);
}

//-----------------------------------------------------------------------------
void CrosshairRepresentation::updateRepresentation()
{
  m_matAxial->Modified();
  m_matCoronal->Modified();
  m_matSagittal->Modified();

  m_axialReslice->Update();
  m_coronalReslice->Update();
  m_sagittalReslice->Update();

  m_axialScaler->Update();
  m_coronalScaler->Update();
  m_sagittalScaler->Update();

  m_axialSquare->Update();
  m_coronalSquare->Update();
  m_sagittalSquare->Update();

  m_axial->Update();
  m_coronal->Update();
  m_sagittal->Update();

  m_axialBorder->Modified();
  m_coronalBorder->Modified();
  m_sagittalBorder->Modified();
}

//-----------------------------------------------------------------------------
void CrosshairRepresentation::updatePipelineConnections()
{
  if (m_axialReslice->GetInputConnection(0,0) != m_data->toVTK())
  {
    itk::Matrix<double,3,3> direction = m_data->toITK()->GetDirection();

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

    m_matAxial = vtkSmartPointer<vtkMatrix4x4>::New();
    m_matAxial->DeepCopy(axialMat);

    m_matCoronal = vtkSmartPointer<vtkMatrix4x4>::New();
    m_matCoronal->DeepCopy(coronalMat);

    m_matSagittal = vtkSmartPointer<vtkMatrix4x4>::New();
    m_matSagittal->DeepCopy(sagittalMat);

    m_axialReslice->SetInputConnection(m_data->toVTK());
    m_axialReslice->SetResliceAxes(m_matAxial);
    m_axialReslice->Update();

    m_coronalReslice->SetInputConnection(m_data->toVTK());
    m_coronalReslice->SetResliceAxes(m_matAxial);
    m_coronalReslice->Update();

    m_sagittalReslice->SetInputConnection(m_data->toVTK());
    m_sagittalReslice->SetResliceAxes(m_matAxial);
    m_sagittalReslice->Update();

    m_axialScaler->SetInputConnection(m_axialReslice->GetOutputPort());
    m_axialScaler->SetOutputScalarType(m_axialReslice->GetOutput()->GetScalarType());
    m_axialScaler->Update();

    m_coronalScaler->SetInputConnection(m_axialReslice->GetOutputPort());
    m_coronalScaler->SetOutputScalarType(m_axialReslice->GetOutput()->GetScalarType());
    m_coronalScaler->Update();

    m_sagittalScaler->SetInputConnection(m_axialReslice->GetOutputPort());
    m_sagittalScaler->SetOutputScalarType(m_axialReslice->GetOutput()->GetScalarType());
    m_sagittalScaler->Update();

    m_data->bounds(m_bounds);
    double ap0[3] = { m_bounds[0], m_bounds[2], m_bounds[4] };
    double ap1[3] = { m_bounds[0], m_bounds[3], m_bounds[4] };
    double ap2[3] = { m_bounds[1], m_bounds[3], m_bounds[4] };
    double ap3[3] = { m_bounds[1], m_bounds[2], m_bounds[4] };

    // Create a vtkPoints object and store the points in it
    vtkSmartPointer<vtkPoints> axialPoints = vtkSmartPointer<vtkPoints>::New();
    axialPoints->InsertNextPoint(ap0);
    axialPoints->InsertNextPoint(ap1);
    axialPoints->InsertNextPoint(ap2);
    axialPoints->InsertNextPoint(ap3);
    axialPoints->InsertNextPoint(ap0);

    // Create a cell array to store the lines in and add the lines to it
    vtkSmartPointer<vtkCellArray> axialLines = vtkSmartPointer<vtkCellArray>::New();

    for (unsigned int i = 0; i < 4; i++)
    {
        vtkSmartPointer < vtkLine > line = vtkSmartPointer<vtkLine>::New();
        line->GetPointIds()->SetId(0, i);
        line->GetPointIds()->SetId(1, i + 1);
        axialLines->InsertNextCell(line);
    }

    m_axialSquare->Reset();
    m_axialSquare->SetPoints(axialPoints);
    m_axialSquare->SetLines(axialLines);
    m_axialSquare->Update();

    double cp0[3] = { m_bounds[0], m_bounds[2], m_bounds[4] };
    double cp1[3] = { m_bounds[0], m_bounds[2], m_bounds[5] };
    double cp2[3] = { m_bounds[1], m_bounds[2], m_bounds[5] };
    double cp3[3] = { m_bounds[1], m_bounds[2], m_bounds[4] };

    // Create a vtkPoints object and store the points in it
    vtkSmartPointer<vtkPoints> coronalPoints = vtkSmartPointer<vtkPoints>::New();
    coronalPoints->InsertNextPoint(cp0);
    coronalPoints->InsertNextPoint(cp1);
    coronalPoints->InsertNextPoint(cp2);
    coronalPoints->InsertNextPoint(cp3);
    coronalPoints->InsertNextPoint(cp0);

    // Create a cell array to store the lines in and add the lines to it
    vtkSmartPointer<vtkCellArray> coronalLines = vtkSmartPointer<vtkCellArray>::New();

    for (unsigned int i = 0; i < 4; i++)
    {
        vtkSmartPointer < vtkLine > line = vtkSmartPointer<vtkLine>::New();
        line->GetPointIds()->SetId(0, i);
        line->GetPointIds()->SetId(1, i + 1);
        coronalLines->InsertNextCell(line);
    }

    m_coronalSquare->Reset();
    m_coronalSquare->SetPoints(coronalPoints);
    m_coronalSquare->SetLines(coronalLines);
    m_coronalSquare->Update();

    double sp0[3] =  { m_bounds[0], m_bounds[2], m_bounds[4] };
    double sp1[3] =  { m_bounds[0], m_bounds[2], m_bounds[5] };
    double sp2[3] =  { m_bounds[0], m_bounds[3], m_bounds[5] };
    double sp3[3] =  { m_bounds[0], m_bounds[3], m_bounds[4] };

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

    m_sagittalSquare->Reset();
    m_sagittalSquare->SetPoints(sagittalPoints);
    m_sagittalSquare->SetLines(sagittalLines);
    m_sagittalSquare->Update();
  }
}

//-----------------------------------------------------------------------------
void CrosshairRepresentation::initializePipeline(EspinaRenderView *view)
{
  connect(m_data.get(), SIGNAL(representationChanged()),
          this, SLOT(updatePipelineConnections()));

  itk::Matrix<double,3,3> direction = m_data->toITK()->GetDirection();

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

  m_matAxial = vtkSmartPointer<vtkMatrix4x4>::New();
  m_matAxial->DeepCopy(axialMat);

  m_matCoronal = vtkSmartPointer<vtkMatrix4x4>::New();
  m_matCoronal->DeepCopy(coronalMat);

  m_matSagittal = vtkSmartPointer<vtkMatrix4x4>::New();
  m_matSagittal->DeepCopy(sagittalMat);

  m_axialReslice = vtkSmartPointer<vtkImageReslice>::New();
  m_axialReslice->SetOptimization(true);
  m_axialReslice->BorderOn();
  m_axialReslice->SetInputConnection(m_data->toVTK());
  m_axialReslice->SetOutputDimensionality(2);
  m_axialReslice->SetResliceAxes(m_matAxial);

  m_coronalReslice = vtkSmartPointer<vtkImageReslice>::New();
  m_coronalReslice->SetOptimization(true);
  m_coronalReslice->BorderOn();
  m_coronalReslice->SetInputConnection(m_data->toVTK());
  m_coronalReslice->SetOutputDimensionality(2);
  m_coronalReslice->SetResliceAxes(m_matCoronal);

  m_sagittalReslice = vtkSmartPointer<vtkImageReslice>::New();
  m_sagittalReslice->SetOptimization(true);
  m_sagittalReslice->BorderOn();
  m_sagittalReslice->SetInputConnection(m_data->toVTK());
  m_sagittalReslice->SetOutputDimensionality(2);
  m_sagittalReslice->SetResliceAxes(m_matSagittal);

  // if hue is -1 then use 0 saturation to make a grayscale image
  double hue = m_color.hueF();
  double sat = hue >= 0?1.0:0.0;

  m_lut = vtkSmartPointer<vtkLookupTable>::New();
  m_lut->Allocate();
  m_lut->SetTableRange(0,255);
  m_lut->SetHueRange(hue, hue);
  m_lut->SetSaturationRange(0.0, sat);
  m_lut->SetValueRange(0.0, 1.0);
  m_lut->SetAlphaRange(1.0,1.0);
  m_lut->SetNumberOfColors(256);
  m_lut->SetRampToLinear();
  m_lut->Build();

  m_axialScaler = vtkSmartPointer<vtkImageShiftScale>::New();
  m_axialScaler->SetInputConnection(m_axialReslice->GetOutputPort());
  m_axialScaler->SetShift(static_cast<int>(m_brightness*255));
  m_axialScaler->SetScale(m_contrast);
  m_axialScaler->SetClampOverflow(true);
  m_axialScaler->SetOutputScalarType(m_axialReslice->GetOutput()->GetScalarType());
  m_axialScaler->Update();

  vtkSmartPointer<vtkImageMapToColors> axialImagemap = vtkSmartPointer<vtkImageMapToColors>::New();
  axialImagemap->SetLookupTable(m_lut);
  axialImagemap->SetOutputFormatToRGBA();
  axialImagemap->SetInputConnection(m_axialScaler->GetOutputPort());

  m_coronalScaler = vtkSmartPointer<vtkImageShiftScale>::New();
  m_coronalScaler->SetInputConnection(m_coronalReslice->GetOutputPort());
  m_coronalScaler->SetShift(static_cast<int>(m_brightness*255));
  m_coronalScaler->SetScale(m_contrast);
  m_coronalScaler->SetClampOverflow(true);
  m_coronalScaler->SetOutputScalarType(m_coronalReslice->GetOutput()->GetScalarType());
  m_coronalScaler->Update();

  vtkSmartPointer<vtkImageMapToColors> coronalImagemap = vtkSmartPointer<vtkImageMapToColors>::New();
  coronalImagemap->SetLookupTable(m_lut);
  coronalImagemap->SetOutputFormatToRGBA();
  coronalImagemap->SetInputConnection(m_coronalScaler->GetOutputPort());

  m_sagittalScaler = vtkSmartPointer<vtkImageShiftScale>::New();
  m_sagittalScaler->SetInputConnection(m_sagittalReslice->GetOutputPort());
  m_sagittalScaler->SetShift(static_cast<int>(m_brightness*255));
  m_sagittalScaler->SetScale(m_contrast);
  m_sagittalScaler->SetClampOverflow(true);
  m_sagittalScaler->SetOutputScalarType(m_sagittalReslice->GetOutput()->GetScalarType());
  m_sagittalScaler->Update();

  vtkSmartPointer<vtkImageMapToColors> sagittalImagemap = vtkSmartPointer<vtkImageMapToColors>::New();
  sagittalImagemap->SetLookupTable(m_lut);
  sagittalImagemap->SetOutputFormatToRGBA();
  sagittalImagemap->SetInputConnection(m_sagittalScaler->GetOutputPort());

  m_axial = vtkSmartPointer<vtkImageActor>::New();
  m_axial->SetInput(axialImagemap->GetOutput());
  m_axial->SetInterpolate(false);

  m_coronal = vtkSmartPointer<vtkImageActor>::New();
  m_coronal->SetInput(coronalImagemap->GetOutput());
  m_coronal->SetInterpolate(false);

  m_sagittal = vtkSmartPointer<vtkImageActor>::New();
  m_sagittal->SetInput(sagittalImagemap->GetOutput());
  m_sagittal->SetInterpolate(false);

  // rotate actors
  double center[3], origin[3];
  memcpy(origin, m_coronal->GetOrigin(), 3*sizeof(double));
  memcpy(center, m_coronal->GetCenter(), 3*sizeof(double));
  m_coronal->SetOrigin(center);
  m_coronal->RotateX(90);
  m_coronal->SetOrigin(origin);

  memcpy(origin, m_sagittal->GetOrigin(), 3*sizeof(double));
  memcpy(center, m_sagittal->GetCenter(), 3*sizeof(double));
  m_sagittal->SetOrigin(center);
  m_sagittal->RotateX(90);
  m_sagittal->RotateY(90);
  m_sagittal->SetOrigin(origin);

  memset(m_point, 0, 3*sizeof(double));

  m_data->bounds(m_bounds);
  double ap0[3] = { m_bounds[0], m_bounds[2], m_bounds[4] };
  double ap1[3] = { m_bounds[0], m_bounds[3], m_bounds[4] };
  double ap2[3] = { m_bounds[1], m_bounds[3], m_bounds[4] };
  double ap3[3] = { m_bounds[1], m_bounds[2], m_bounds[4] };

  // Create a vtkPoints object and store the points in it
  vtkSmartPointer<vtkPoints> axialPoints = vtkSmartPointer<vtkPoints>::New();
  axialPoints->InsertNextPoint(ap0);
  axialPoints->InsertNextPoint(ap1);
  axialPoints->InsertNextPoint(ap2);
  axialPoints->InsertNextPoint(ap3);
  axialPoints->InsertNextPoint(ap0);

  // Create a cell array to store the lines in and add the lines to it
  vtkSmartPointer<vtkCellArray> axialLines = vtkSmartPointer<vtkCellArray>::New();

  for (unsigned int i = 0; i < 4; i++)
  {
      vtkSmartPointer < vtkLine > line = vtkSmartPointer<vtkLine>::New();
      line->GetPointIds()->SetId(0, i);
      line->GetPointIds()->SetId(1, i + 1);
      axialLines->InsertNextCell(line);
  }

  // Add the points & lines to the dataset
  m_axialSquare = vtkSmartPointer<vtkPolyData>::New();
  m_axialSquare->Reset();
  m_axialSquare->SetPoints(axialPoints);
  m_axialSquare->SetLines(axialLines);
  m_axialSquare->Update();

  double cp0[3] = { m_bounds[0], m_bounds[2], m_bounds[4] };
  double cp1[3] = { m_bounds[0], m_bounds[2], m_bounds[5] };
  double cp2[3] = { m_bounds[1], m_bounds[2], m_bounds[5] };
  double cp3[3] = { m_bounds[1], m_bounds[2], m_bounds[4] };

  // Create a vtkPoints object and store the points in it
  vtkSmartPointer<vtkPoints> coronalPoints = vtkSmartPointer<vtkPoints>::New();
  coronalPoints->InsertNextPoint(cp0);
  coronalPoints->InsertNextPoint(cp1);
  coronalPoints->InsertNextPoint(cp2);
  coronalPoints->InsertNextPoint(cp3);
  coronalPoints->InsertNextPoint(cp0);

  // Create a cell array to store the lines in and add the lines to it
  vtkSmartPointer<vtkCellArray> coronalLines = vtkSmartPointer<vtkCellArray>::New();

  for (unsigned int i = 0; i < 4; i++)
  {
      vtkSmartPointer < vtkLine > line = vtkSmartPointer<vtkLine>::New();
      line->GetPointIds()->SetId(0, i);
      line->GetPointIds()->SetId(1, i + 1);
      coronalLines->InsertNextCell(line);
  }

  m_coronalSquare = vtkSmartPointer<vtkPolyData>::New();
  m_coronalSquare->Reset();
  m_coronalSquare->SetPoints(coronalPoints);
  m_coronalSquare->SetLines(coronalLines);
  m_coronalSquare->Update();

  double sp0[3] =  { m_bounds[0], m_bounds[2], m_bounds[4] };
  double sp1[3] =  { m_bounds[0], m_bounds[2], m_bounds[5] };
  double sp2[3] =  { m_bounds[0], m_bounds[3], m_bounds[5] };
  double sp3[3] =  { m_bounds[0], m_bounds[3], m_bounds[4] };

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

  m_sagittalSquare = vtkSmartPointer<vtkPolyData>::New();
  m_sagittalSquare->Reset();
  m_sagittalSquare->SetPoints(sagittalPoints);
  m_sagittalSquare->SetLines(sagittalLines);
  m_sagittalSquare->Update();

  vtkSmartPointer<vtkPolyDataMapper> axialSquareMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  axialSquareMapper->SetInput(m_axialSquare);
  vtkSmartPointer<vtkPolyDataMapper> coronalSquareMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  coronalSquareMapper->SetInput(m_coronalSquare);
  vtkSmartPointer<vtkPolyDataMapper> sagittalSquareMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  sagittalSquareMapper->SetInput(m_sagittalSquare);

  // these colors have been defined in DefaultEspinaView.cpp for the 2D crosshair
  double cyan[3] = { 0, 1, 1 };
  double blue[3] = { 0, 0, 1 };
  double magenta[3] = { 1, 0, 1 };

  m_axialBorder = vtkSmartPointer<vtkActor>::New();
  m_axialBorder->SetMapper(axialSquareMapper);
  m_axialBorder->GetProperty()->SetColor(cyan);
  m_axialBorder->GetProperty()->SetPointSize(2);
  m_axialBorder->GetProperty()->SetLineWidth(1);
  m_axialBorder->SetPickable(false);

  m_coronalBorder = vtkSmartPointer<vtkActor>::New();
  m_coronalBorder->SetMapper(coronalSquareMapper);
  m_coronalBorder->GetProperty()->SetColor(blue);
  m_coronalBorder->GetProperty()->SetPointSize(2);
  m_coronalBorder->GetProperty()->SetLineWidth(1);
  m_coronalBorder->SetPickable(false);

  m_sagittalBorder = vtkSmartPointer<vtkActor>::New();
  m_sagittalBorder->SetMapper(sagittalSquareMapper);
  m_sagittalBorder->GetProperty()->SetColor(magenta);
  m_sagittalBorder->GetProperty()->SetPointSize(2);
  m_sagittalBorder->GetProperty()->SetLineWidth(1);
  m_sagittalBorder->SetPickable(false);

  m_view = view;
}

//-----------------------------------------------------------------------------
QList<vtkProp*> CrosshairRepresentation::getActors()
{
  QList<vtkProp*> list;
  list << m_axial << m_coronal << m_sagittal;
  list << m_axialBorder << m_coronalBorder << m_sagittalBorder;

  return list;
}

//-----------------------------------------------------------------------------
void CrosshairRepresentation::setCrosshairColors(double aColor[3], double cColor[3], double sColor[3])
{
  m_axialBorder->GetProperty()->SetColor(aColor);
  m_coronalBorder->GetProperty()->SetColor(cColor);
  m_sagittalBorder->GetProperty()->SetColor(sColor);
}

//-----------------------------------------------------------------------------
void CrosshairRepresentation::setCrosshair(Nm point[3])
{
  if (point[0] != m_point[0])
  {
    m_point[0] = point[0];
    if (((point[0] < m_bounds[0]) || (point[0] > m_bounds[1])) && m_tiling)
    {
      m_sagittal->SetVisibility(false);
      m_sagittalBorder->SetVisibility(false);
    }
    else
    {
      // if not tiling
      if (point[0] < m_bounds[0])
        m_point[0] = m_bounds[0];

      if (point[0] > m_bounds[1])
        m_point[0] = m_bounds[1];

      // sagittal changed
      if (m_sagittal->GetVisibility() == false)
      {
        m_sagittal->SetVisibility(true);
        m_sagittalBorder->SetVisibility(true);
      }

      double actorpos[3];
      m_matSagittal->SetElement(0, 3, m_point[0]);
      m_sagittal->GetPosition(actorpos);
      actorpos[0] = m_point[0];
      m_sagittal->SetPosition(actorpos);
      m_sagittalBorder->SetPosition(actorpos);
    }
  }

  if (point[1] != m_point[1])
  {
    m_point[1] = point[1];
    if (((point[1] < m_bounds[2]) || (point[1] > m_bounds[3])) && m_tiling)
    {
      m_coronal->SetVisibility(false);
      m_coronalBorder->SetVisibility(false);
    }
    else
    {
      // if not tiling
      if (point[1] < m_bounds[2])
        m_point[1] = m_bounds[0];

      if (point[1] > m_bounds[3])
        m_point[1] = m_bounds[1];

      // coronal changed
      if (m_coronal->GetVisibility() == false)
      {
        m_coronal->SetVisibility(true);
        m_coronalBorder->SetVisibility(true);
      }

      double actorpos[3];
      m_matCoronal->SetElement(1, 3, m_point[1]);
      m_coronal->GetPosition(actorpos);
      actorpos[1] = m_point[1];
      m_coronal->SetPosition(actorpos);
      m_coronalBorder->SetPosition(actorpos);
    }
  }

  if (point[2] != m_point[2])
  {
    m_point[2] = point[2];
    if (((point[2] < m_bounds[4]) || (point[2] > m_bounds[5])) && m_tiling)
    {
      m_axial->SetVisibility(false);
      m_axialBorder->SetVisibility(false);
    }
    else
    {
      // if not tiling
      if (point[2] < m_bounds[4])
        m_point[2] = m_bounds[4];

      if (point[2] > m_bounds[5])
        m_point[2] = m_bounds[5];

      // axial changed
      if (m_axial->GetVisibility() == false)
      {
        m_axial->SetVisibility(true);
        m_axialBorder->SetVisibility(true);
      }

      double actorpos[3];
      m_matAxial->SetElement(2, 3, m_point[2]);
      m_axial->GetPosition(actorpos);
      actorpos[2] = m_point[2];
      m_axial->SetPosition(actorpos);
      m_axialBorder->SetPosition(actorpos);
    }
  }
}

//-----------------------------------------------------------------------------
void CrosshairRepresentation::setPlanePosition(PlaneType plane, Nm dist)
{
  Nm point[3];
  memcpy(point, m_point, 3*sizeof(Nm));

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
GraphicalRepresentationSPtr CrosshairRepresentation::cloneImplementation(VolumeView *view)
{
  CrosshairRepresentation *representation = new CrosshairRepresentation(m_data, view);

  representation->initializePipeline(view);

  return GraphicalRepresentationSPtr(representation);
}

//-----------------------------------------------------------------------------
void CrosshairRepresentation::updateVisibility(bool visible)
{
  if (m_axial)
  {
    m_axial         ->SetVisibility(visible);
    m_coronal       ->SetVisibility(visible);
    m_sagittal      ->SetVisibility(visible);
    m_axialBorder   ->SetVisibility(visible);
    m_coronalBorder ->SetVisibility(visible);
    m_sagittalBorder->SetVisibility(visible);
  }
}