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
#include "RepresentationEmptySettings.h"
#include <GUI/View/View3D.h>

// VTK
#include <vtkImageReslice.h>
#include <vtkImageMapToColors.h>
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
#include <vtkImageMapper3D.h>
#include <vtkImageMapToColors.h>

// Qt
#include <QColor>
#include <QDebug>

using namespace EspINA;

const Representation::Type CrosshairRepresentation::TYPE = "Crosshair";

//------------------------------------------------------------------------
CrosshairRepresentation::CrosshairRepresentation(VolumeSPtr data, RenderView *view)
: Representation(view)
, m_data(data)
, m_axialExporter(nullptr)
, m_coronalExporter(nullptr)
, m_sagittalExporter(nullptr)
, m_axial(nullptr)
, m_coronal(nullptr)
, m_sagittal(nullptr)
, m_axialImageMapToColors(nullptr)
, m_coronalImageMapToColors(nullptr)
, m_sagittalImageMapToColors(nullptr)
, m_axialBorder(nullptr)
, m_coronalBorder(nullptr)
, m_sagittalBorder(nullptr)
, m_axialSquare(nullptr)
, m_coronalSquare(nullptr)
, m_sagittalSquare(nullptr)
, m_lut(nullptr)
, m_axialScaler(nullptr)
, m_coronalScaler(nullptr)
, m_sagittalScaler(nullptr)
, m_tiling(false)
{
}

//-----------------------------------------------------------------------------
void CrosshairRepresentation::setBrightness(double value)
{
  Representation::setBrightness(value);

  if (m_axial != nullptr)
  {
    m_axialScaler->SetShift(static_cast<int>(m_brightness * 255));
    m_coronalScaler->SetShift(static_cast<int>(m_brightness * 255));
    m_sagittalScaler->SetShift(static_cast<int>(m_brightness * 255));
  }
}

//-----------------------------------------------------------------------------
void CrosshairRepresentation::setContrast(double value)
{
  Representation::setContrast(value);

  if (m_axial != nullptr)
  {
    m_axialScaler->SetScale(m_contrast);
    m_coronalScaler->SetScale(m_contrast);
    m_sagittalScaler->SetScale(m_contrast);
  }
}

//-----------------------------------------------------------------------------
RepresentationSettings *CrosshairRepresentation::settingsWidget()
{
  return new RepresentationEmptySettings();
}

//-----------------------------------------------------------------------------
void CrosshairRepresentation::setColor(const QColor &color)
{
  Representation::setColor(color);

  if (m_axial != nullptr)
  {
    m_lut->SetHueRange(color.hueF(), color.hueF());
    m_lut->SetSaturationRange(0, color.saturationF());
    m_lut->Build();
  }
}

//-----------------------------------------------------------------------------
void CrosshairRepresentation::setOpacity(double value)
{
  Representation::setOpacity(value);

  if (m_axial != nullptr)
  {
    m_axial->SetOpacity(m_opacity);
    m_coronal->SetOpacity(m_opacity);
    m_sagittal->SetOpacity(m_opacity);

    m_axialBorder->GetProperty()->SetOpacity(m_opacity);
    m_coronalBorder->GetProperty()->SetOpacity(m_opacity);
    m_sagittalBorder->GetProperty()->SetOpacity(m_opacity);
  }
}

//-----------------------------------------------------------------------------
bool CrosshairRepresentation::isInside(const NmVector3 &point) const
{
  if (m_axial == nullptr)
    m_bounds = m_data->bounds();

  return ((m_bounds[0] <= point[0]) && (m_bounds[1] >= point[0]) && (m_bounds[2] <= point[1])
      && (m_bounds[3] >= point[1]) && (m_bounds[4] <= point[2]) && (m_bounds[5] >= point[2]));
}

//-----------------------------------------------------------------------------
bool CrosshairRepresentation::hasActor(vtkProp *actor) const
{
  if (m_axial == nullptr)
    return false;

  return (m_axial.GetPointer() == actor || m_coronal.GetPointer() == actor ||
          m_sagittal.GetPointer() == actor || m_axialBorder.GetPointer() == actor ||
          m_coronalBorder.GetPointer() == actor || m_sagittalBorder.GetPointer() == actor);
}

//-----------------------------------------------------------------------------
void CrosshairRepresentation::updateRepresentation()
{
  if (m_axial == nullptr)
    return;

  int coordIndex = normalCoordinateIndex(Plane::XY);
  int reslicePoint = m_point[coordIndex];

  Bounds imageBounds = m_data->bounds();
  imageBounds[2*coordIndex] = imageBounds[(2*coordIndex)+1] = reslicePoint;
  imageBounds.setUpperInclusion(toAxis(coordIndex), true);

  m_axialExporter = ExporterType::New();
  m_axialExporter->ReleaseDataFlagOn();
  m_axialExporter->SetNumberOfThreads(1);
  m_axialExporter->SetInput(m_data->itkImage(imageBounds));
  m_axialExporter->Update();

  m_axialScaler->SetInputData(m_axialExporter->GetOutput());
  m_axialScaler->Update();

  m_axialImageMapToColors->SetInputConnection(m_axialScaler->GetOutputPort());
  m_axialImageMapToColors->Update();

  m_axial->GetMapper()->SetInputConnection(m_axialImageMapToColors->GetOutputPort());
  m_axial->SetDisplayExtent(m_axialExporter->GetOutput()->GetExtent());
  m_axial->Update();

  coordIndex = normalCoordinateIndex(Plane::XZ);
  reslicePoint = m_point[coordIndex];

  imageBounds = m_data->bounds();
  imageBounds[2*coordIndex] = imageBounds[(2*coordIndex)+1] = reslicePoint;
  imageBounds.setUpperInclusion(toAxis(coordIndex), true);

  m_coronalExporter = ExporterType::New();
  m_coronalExporter->ReleaseDataFlagOn();
  m_coronalExporter->SetNumberOfThreads(1);
  m_coronalExporter->SetInput(m_data->itkImage(imageBounds));
  m_coronalExporter->Update();

  m_coronalScaler->SetInputData(m_coronalExporter->GetOutput());
  m_coronalScaler->Update();

  m_coronalImageMapToColors->SetInputConnection(m_coronalScaler->GetOutputPort());
  m_coronalImageMapToColors->Update();

  m_coronal->GetMapper()->SetInputConnection(m_coronalImageMapToColors->GetOutputPort());
  m_coronal->SetDisplayExtent(m_coronalExporter->GetOutput()->GetExtent());
  m_coronal->Update();

  coordIndex = normalCoordinateIndex(Plane::YZ);
  reslicePoint = m_point[coordIndex];

  imageBounds = m_data->bounds();
  imageBounds[2*coordIndex] = imageBounds[(2*coordIndex)+1] = reslicePoint;
  imageBounds.setUpperInclusion(toAxis(coordIndex), true);

  m_sagittalExporter = ExporterType::New();
  m_sagittalExporter->ReleaseDataFlagOn();
  m_sagittalExporter->SetNumberOfThreads(1);
  m_sagittalExporter->SetInput(m_data->itkImage(imageBounds));
  m_sagittalExporter->Update();

  m_sagittalScaler->SetInputData(m_sagittalExporter->GetOutput());
  m_sagittalScaler->Update();

  m_sagittalImageMapToColors->SetInputConnection(m_sagittalScaler->GetOutputPort());
  m_sagittalImageMapToColors->Update();

  m_sagittal->GetMapper()->SetInputConnection(m_sagittalImageMapToColors->GetOutputPort());
  m_sagittal->SetDisplayExtent(m_sagittalExporter->GetOutput()->GetExtent());
  m_sagittal->Update();

  m_bounds = m_data->bounds();
  double ap0[3] = { m_bounds[0], m_bounds[2], m_bounds[4] };
  double ap1[3] = { m_bounds[0], m_bounds[3], m_bounds[4] };
  double ap2[3] = { m_bounds[1], m_bounds[3], m_bounds[4] };
  double ap3[3] = { m_bounds[1], m_bounds[2], m_bounds[4] };

  // Create a vtkPoints object and store the points in it
  auto axialPoints = vtkSmartPointer<vtkPoints>::New();
  axialPoints->InsertNextPoint(ap0);
  axialPoints->InsertNextPoint(ap1);
  axialPoints->InsertNextPoint(ap2);
  axialPoints->InsertNextPoint(ap3);
  axialPoints->InsertNextPoint(ap0);

  // Create a cell array to store the lines in and add the lines to it
  auto axialLines = vtkSmartPointer<vtkCellArray>::New();

  for (unsigned int i = 0; i < 4; i++)
  {
    auto line = vtkSmartPointer<vtkLine>::New();
    line->GetPointIds()->SetId(0, i);
    line->GetPointIds()->SetId(1, i + 1);
    axialLines->InsertNextCell(line);
  }

  m_axialSquare->Reset();
  m_axialSquare->SetPoints(axialPoints);
  m_axialSquare->SetLines(axialLines);
  m_axialSquare->Modified();

  double cp0[3] = { m_bounds[0], m_bounds[2], m_bounds[4] };
  double cp1[3] = { m_bounds[0], m_bounds[2], m_bounds[5] };
  double cp2[3] = { m_bounds[1], m_bounds[2], m_bounds[5] };
  double cp3[3] = { m_bounds[1], m_bounds[2], m_bounds[4] };

  // Create a vtkPoints object and store the points in it
  auto coronalPoints = vtkSmartPointer<vtkPoints>::New();
  coronalPoints->InsertNextPoint(cp0);
  coronalPoints->InsertNextPoint(cp1);
  coronalPoints->InsertNextPoint(cp2);
  coronalPoints->InsertNextPoint(cp3);
  coronalPoints->InsertNextPoint(cp0);

  // Create a cell array to store the lines in and add the lines to it
  auto coronalLines = vtkSmartPointer<vtkCellArray>::New();

  for (unsigned int i = 0; i < 4; i++)
  {
    auto line = vtkSmartPointer<vtkLine>::New();
    line->GetPointIds()->SetId(0, i);
    line->GetPointIds()->SetId(1, i + 1);
    coronalLines->InsertNextCell(line);
  }

  m_coronalSquare->Reset();
  m_coronalSquare->SetPoints(coronalPoints);
  m_coronalSquare->SetLines(coronalLines);
  m_coronalSquare->Modified();

  double sp0[3] = { m_bounds[0], m_bounds[2], m_bounds[4] };
  double sp1[3] = { m_bounds[0], m_bounds[2], m_bounds[5] };
  double sp2[3] = { m_bounds[0], m_bounds[3], m_bounds[5] };
  double sp3[3] = { m_bounds[0], m_bounds[3], m_bounds[4] };

  // Create a vtkPoints object and store the points in it
  auto sagittalPoints = vtkSmartPointer<vtkPoints>::New();
  sagittalPoints->InsertNextPoint(sp0);
  sagittalPoints->InsertNextPoint(sp1);
  sagittalPoints->InsertNextPoint(sp2);
  sagittalPoints->InsertNextPoint(sp3);
  sagittalPoints->InsertNextPoint(sp0);

  // Create a cell array to store the lines in and add the lines to it
  auto sagittalLines = vtkSmartPointer<vtkCellArray>::New();

  for (unsigned int i = 0; i < 4; i++)
  {
    auto line = vtkSmartPointer<vtkLine>::New();
    line->GetPointIds()->SetId(0, i);
    line->GetPointIds()->SetId(1, i + 1);
    sagittalLines->InsertNextCell(line);
  }

  m_sagittalSquare->Reset();
  m_sagittalSquare->SetPoints(sagittalPoints);
  m_sagittalSquare->SetLines(sagittalLines);
  m_sagittalSquare->Modified();

  m_axialBorder->Modified();
  m_coronalBorder->Modified();
  m_sagittalBorder->Modified();
}

//-----------------------------------------------------------------------------
void CrosshairRepresentation::initializePipeline()
{
  m_lut = vtkSmartPointer<vtkLookupTable>::New();
  m_lut->Allocate();
  m_lut->SetVectorModeToRGBColors();
  m_lut->SetTableRange(0, 255);
  m_lut->SetHueRange(m_color.hueF(), m_color.hueF());
  m_lut->SetSaturationRange(0.0, m_color.saturationF());
  m_lut->SetValueRange(0.0, 1.0);
  m_lut->SetAlphaRange(1.0, 1.0);
  m_lut->SetNumberOfColors(256);
  m_lut->SetRampToLinear();
  m_lut->Build();

  int coordIndex = normalCoordinateIndex(Plane::XY);
  int reslicePoint = m_crosshair[coordIndex];

  Bounds imageBounds = m_data->bounds();
  imageBounds[2*coordIndex] = imageBounds[(2*coordIndex)+1] = reslicePoint;
  imageBounds.setUpperInclusion(toAxis(coordIndex), true);

  m_axialExporter = ExporterType::New();
  m_axialExporter->ReleaseDataFlagOn();
  m_axialExporter->SetNumberOfThreads(1);
  m_axialExporter->SetInput(m_data->itkImage(imageBounds));
  m_axialExporter->Update();

  m_axialScaler = vtkSmartPointer<vtkImageShiftScale>::New();
  m_axialScaler->SetInputData(m_axialExporter->GetOutput());
  m_axialScaler->SetShift(static_cast<int>(m_brightness * 255));
  m_axialScaler->SetScale(m_contrast);
  m_axialScaler->SetClampOverflow(true);
  m_axialScaler->SetOutputScalarType(m_axialExporter->GetOutput()->GetScalarType());
  m_axialScaler->Update();

  m_axialImageMapToColors = vtkSmartPointer<vtkImageMapToColors>::New();
  m_axialImageMapToColors->SetInputConnection(m_axialScaler->GetOutputPort());
  m_axialImageMapToColors->SetLookupTable(m_lut);
  m_axialImageMapToColors->SetNumberOfThreads(1);
  m_axialImageMapToColors->Update();

  m_axial = vtkSmartPointer<vtkImageActor>::New();
  m_axial->SetInterpolate(false);
  m_axial->GetMapper()->BorderOn();
  m_axial->GetMapper()->SetInputConnection(m_axialImageMapToColors->GetOutputPort());
  m_axial->SetDisplayExtent(m_axialImageMapToColors->GetOutput()->GetExtent());
  m_axial->SetVisibility(isVisible());
  m_axial->Update();

  coordIndex = normalCoordinateIndex(Plane::XZ);
  reslicePoint = m_crosshair[coordIndex];

  imageBounds = m_data->bounds();
  imageBounds[2 * coordIndex] = imageBounds[(2 * coordIndex) + 1] = reslicePoint;
  imageBounds.setUpperInclusion(toAxis(coordIndex), true);

  m_coronalExporter = ExporterType::New();
  m_coronalExporter->ReleaseDataFlagOn();
  m_coronalExporter->SetNumberOfThreads(1);
  m_coronalExporter->SetInput(m_data->itkImage(imageBounds));
  m_coronalExporter->Update();

  m_coronalScaler = vtkSmartPointer<vtkImageShiftScale>::New();
  m_coronalScaler->SetInputData(m_coronalExporter->GetOutput());
  m_coronalScaler->SetShift(static_cast<int>(m_brightness * 255));
  m_coronalScaler->SetScale(m_contrast);
  m_coronalScaler->SetClampOverflow(true);
  m_coronalScaler->SetOutputScalarType(m_coronalExporter->GetOutput()->GetScalarType());
  m_coronalScaler->Update();

  m_coronalImageMapToColors = vtkSmartPointer<vtkImageMapToColors>::New();
  m_coronalImageMapToColors->SetInputConnection(m_coronalScaler->GetOutputPort());
  m_coronalImageMapToColors->SetLookupTable(m_lut);
  m_coronalImageMapToColors->SetNumberOfThreads(1);
  m_coronalImageMapToColors->Update();

  m_coronal = vtkSmartPointer<vtkImageActor>::New();
  m_coronal->SetInterpolate(false);
  m_coronal->GetMapper()->BorderOn();
  m_coronal->GetMapper()->SetInputConnection(m_coronalImageMapToColors->GetOutputPort());
  m_coronal->SetDisplayExtent(m_coronalImageMapToColors->GetOutput()->GetExtent());
  m_coronal->SetVisibility(isVisible());
  m_coronal->Update();

  coordIndex = normalCoordinateIndex(Plane::YZ);
  reslicePoint = m_crosshair[coordIndex];

  imageBounds = m_data->bounds();
  imageBounds[2 * coordIndex] = imageBounds[(2 * coordIndex) + 1] = reslicePoint;

  m_sagittalExporter = ExporterType::New();
  m_sagittalExporter->ReleaseDataFlagOn();
  m_sagittalExporter->SetNumberOfThreads(1);
  m_sagittalExporter->SetInput(m_data->itkImage(imageBounds));
  m_sagittalExporter->UpdateLargestPossibleRegion();

  m_sagittalScaler = vtkSmartPointer<vtkImageShiftScale>::New();
  m_sagittalScaler->SetInputData(m_sagittalExporter->GetOutput());
  m_sagittalScaler->SetShift(static_cast<int>(m_brightness * 255));
  m_sagittalScaler->SetScale(m_contrast);
  m_sagittalScaler->SetClampOverflow(true);
  m_sagittalScaler->SetOutputScalarType(m_sagittalExporter->GetOutput()->GetScalarType());
  m_sagittalScaler->Update();

  m_sagittalImageMapToColors = vtkSmartPointer<vtkImageMapToColors>::New();
  m_sagittalImageMapToColors->SetInputConnection(m_sagittalScaler->GetOutputPort());
  m_sagittalImageMapToColors->SetLookupTable(m_lut);
  m_sagittalImageMapToColors->SetNumberOfThreads(1);
  m_sagittalImageMapToColors->Update();

  m_sagittal = vtkSmartPointer<vtkImageActor>::New();
  m_sagittal->SetInterpolate(false);
  m_sagittal->GetMapper()->BorderOn();
  m_sagittal->GetMapper()->SetInputConnection(m_sagittalImageMapToColors->GetOutputPort());
  m_sagittal->SetDisplayExtent(m_sagittalImageMapToColors->GetOutput()->GetExtent());
  m_sagittal->SetVisibility(isVisible());
  m_sagittal->Update();

  m_bounds = m_data->bounds();
  double ap0[3] = { m_bounds[0], m_bounds[2], m_bounds[4] };
  double ap1[3] = { m_bounds[0], m_bounds[3], m_bounds[4] };
  double ap2[3] = { m_bounds[1], m_bounds[3], m_bounds[4] };
  double ap3[3] = { m_bounds[1], m_bounds[2], m_bounds[4] };

  // Create a vtkPoints object and store the points in it
  auto axialPoints = vtkSmartPointer<vtkPoints>::New();
  axialPoints->InsertNextPoint(ap0);
  axialPoints->InsertNextPoint(ap1);
  axialPoints->InsertNextPoint(ap2);
  axialPoints->InsertNextPoint(ap3);
  axialPoints->InsertNextPoint(ap0);

  // Create a cell array to store the lines in and add the lines to it
  auto axialLines = vtkSmartPointer<vtkCellArray>::New();

  for (unsigned int i = 0; i < 4; i++)
  {
    auto line = vtkSmartPointer<vtkLine>::New();
    line->GetPointIds()->SetId(0, i);
    line->GetPointIds()->SetId(1, i + 1);
    axialLines->InsertNextCell(line);
  }

  // Add the points & lines to the dataset
  m_axialSquare = vtkSmartPointer<vtkPolyData>::New();
  m_axialSquare->Reset();
  m_axialSquare->SetPoints(axialPoints);
  m_axialSquare->SetLines(axialLines);
  m_axialSquare->Modified();

  double cp0[3] = { m_bounds[0], m_bounds[2], m_bounds[4] };
  double cp1[3] = { m_bounds[0], m_bounds[2], m_bounds[5] };
  double cp2[3] = { m_bounds[1], m_bounds[2], m_bounds[5] };
  double cp3[3] = { m_bounds[1], m_bounds[2], m_bounds[4] };

  // Create a vtkPoints object and store the points in it
  auto coronalPoints = vtkSmartPointer<vtkPoints>::New();
  coronalPoints->InsertNextPoint(cp0);
  coronalPoints->InsertNextPoint(cp1);
  coronalPoints->InsertNextPoint(cp2);
  coronalPoints->InsertNextPoint(cp3);
  coronalPoints->InsertNextPoint(cp0);

  // Create a cell array to store the lines in and add the lines to it
  auto coronalLines = vtkSmartPointer<vtkCellArray>::New();

  for (unsigned int i = 0; i < 4; i++)
  {
    auto line = vtkSmartPointer<vtkLine>::New();
    line->GetPointIds()->SetId(0, i);
    line->GetPointIds()->SetId(1, i + 1);
    coronalLines->InsertNextCell(line);
  }

  m_coronalSquare = vtkSmartPointer<vtkPolyData>::New();
  m_coronalSquare->Reset();
  m_coronalSquare->SetPoints(coronalPoints);
  m_coronalSquare->SetLines(coronalLines);
  m_coronalSquare->Modified();

  double sp0[3] = { m_bounds[0], m_bounds[2], m_bounds[4] };
  double sp1[3] = { m_bounds[0], m_bounds[2], m_bounds[5] };
  double sp2[3] = { m_bounds[0], m_bounds[3], m_bounds[5] };
  double sp3[3] = { m_bounds[0], m_bounds[3], m_bounds[4] };

  // Create a vtkPoints object and store the points in it
  auto sagittalPoints = vtkSmartPointer<vtkPoints>::New();
  sagittalPoints->InsertNextPoint(sp0);
  sagittalPoints->InsertNextPoint(sp1);
  sagittalPoints->InsertNextPoint(sp2);
  sagittalPoints->InsertNextPoint(sp3);
  sagittalPoints->InsertNextPoint(sp0);

  // Create a cell array to store the lines in and add the lines to it
  auto sagittalLines = vtkSmartPointer<vtkCellArray>::New();

  for (unsigned int i = 0; i < 4; i++)
  {
    auto line = vtkSmartPointer<vtkLine>::New();
    line->GetPointIds()->SetId(0, i);
    line->GetPointIds()->SetId(1, i + 1);
    sagittalLines->InsertNextCell(line);
  }

  m_sagittalSquare = vtkSmartPointer<vtkPolyData>::New();
  m_sagittalSquare->Reset();
  m_sagittalSquare->SetPoints(sagittalPoints);
  m_sagittalSquare->SetLines(sagittalLines);
  m_sagittalSquare->Modified();

  auto axialSquareMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  axialSquareMapper->SetInputData(m_axialSquare);
  auto coronalSquareMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  coronalSquareMapper->SetInputData(m_coronalSquare);
  auto sagittalSquareMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  sagittalSquareMapper->SetInputData(m_sagittalSquare);

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
}

//-----------------------------------------------------------------------------
QList<vtkProp*> CrosshairRepresentation::getActors()
{
  QList<vtkProp*> list;

  if (m_axial == nullptr)
  {
    initializePipeline();
    NmVector3 point = m_point;
    m_point[0]--;
    m_point[1]--;
    m_point[2]--;
    setCrosshair(point);
  }

  list << m_axial << m_coronal << m_sagittal;
  list << m_axialBorder << m_coronalBorder << m_sagittalBorder;

  return list;
}

//-----------------------------------------------------------------------------
void CrosshairRepresentation::setCrosshairColors(double aColor[3], double cColor[3], double sColor[3])
{
  if (m_axial != nullptr)
  {
    m_axialBorder->GetProperty()->SetColor(aColor);
    m_coronalBorder->GetProperty()->SetColor(cColor);
    m_sagittalBorder->GetProperty()->SetColor(sColor);
  }
}

//-----------------------------------------------------------------------------
void CrosshairRepresentation::setCrosshair(NmVector3 point)
{
  if (m_axial == nullptr)
  {
    m_point = point;
    return;
  }

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
      m_sagittal->GetPosition(actorpos);
      actorpos[0] = m_point[0];
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
      m_coronal->GetPosition(actorpos);
      actorpos[1] = m_point[1];
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
      m_axial->GetPosition(actorpos);
      actorpos[2] = m_point[2];
      m_axialBorder->SetPosition(actorpos);
    }
  }
  updateRepresentation();
}

//-----------------------------------------------------------------------------
void CrosshairRepresentation::setPlanePosition(Plane plane, Nm dist)
{
  if (m_point[normalCoordinateIndex(plane)] == dist)
    return;

  NmVector3 point = m_point;
  point[normalCoordinateIndex(plane)] = dist;
  setCrosshair(point);
}

//-----------------------------------------------------------------------------
RepresentationSPtr CrosshairRepresentation::cloneImplementation(View3D *view)
{
  auto representation = new CrosshairRepresentation(m_data, view);
  representation->setView(view);

  return RepresentationSPtr(representation);
}

//-----------------------------------------------------------------------------
void CrosshairRepresentation::updateVisibility(bool visible)
{
  if (m_axial)
  {
    m_axial->SetVisibility(visible);
    m_coronal->SetVisibility(visible);
    m_sagittal->SetVisibility(visible);
    m_axialBorder->SetVisibility(visible);
    m_coronalBorder->SetVisibility(visible);
    m_sagittalBorder->SetVisibility(visible);
  }
}
