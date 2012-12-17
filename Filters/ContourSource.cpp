/*
 * ContourSource.cpp
 *
 *  Created on: Sep 27, 2012
 *      Author: Félix de las Pozas Álvarez
 */

#include "ContourSource.h"

#include <Core/Model/EspinaFactory.h>

#include <itkImageRegionIteratorWithIndex.h>
#include <vtkCellArray.h>
#include <vtkPoints.h>

#include <QDebug>
using namespace EspINA;

const QString ContourSource::TYPE = "EditorToolBar::ContourSource";

typedef ModelItem::ArgumentId ArgumentId;
const ArgumentId ContourSource::SPACING = "SPACING";

//-----------------------------------------------------------------------------
ContourSource::ContourSource(Filter::NamedInputs inputs,
                               ModelItem::Arguments args)
: SegmentationFilter(inputs, args)
, m_param(m_args)
{
  Q_ASSERT(inputs.isEmpty());
}

//-----------------------------------------------------------------------------
ContourSource::~ContourSource()
{
  QMap<Nm, vtkPolyData*>::iterator it = m_contourMap[AXIAL].begin();
  while (it != m_contourMap[AXIAL].end())
  {
    it.value()->Delete();
    it = m_contourMap[AXIAL].erase(it);
  }
  m_contourMap[AXIAL].clear();

  it = m_contourMap[CORONAL].begin();
  while (it != m_contourMap[CORONAL].end())
  {
    it.value()->Delete();
    it = m_contourMap[CORONAL].erase(it);
  }
  m_contourMap[CORONAL].clear();

  it = m_contourMap[SAGITTAL].begin();
  while (it != m_contourMap[SAGITTAL].end())
  {
    it.value()->Delete();
    it = m_contourMap[SAGITTAL].erase(it);
  }
  m_contourMap[SAGITTAL].clear();
  m_contourMap.clear();

}

//-----------------------------------------------------------------------------
void ContourSource::draw(OutputId oId,
                         vtkPolyData *contour,
                         Nm slice, PlaneType plane,
                         itkVolumeType::PixelType value)
{
  double bounds[6] = { 0,0,0,0,0,0 };

  Q_ASSERT(0 == oId);
  if (m_outputs.isEmpty())
  {
    // need to create a dummy image to register the filter/action, we'll change it later.
    createOutput(0, EspinaRegion(bounds), m_param.spacing());
  }
  else
  {
    EspinaVolume::Pointer volume = m_outputs[oId].volume;
    contour->ComputeBounds();
    contour->GetBounds(bounds);
    volume->expandToFitRegion(EspinaRegion(bounds));

    vtkPolyData *newContour = this->TransformContour(plane, contour);
    m_contourMap[plane].insert(slice, newContour);
    Filter::draw(oId, newContour, slice, plane);
  }
}

//-----------------------------------------------------------------------------
QVariant ContourSource::data(int role) const
{
  if (role == Qt::DisplayRole)
    return TYPE;
  else
    return QVariant();
}

//-----------------------------------------------------------------------------
bool ContourSource::needUpdate() const
{
  return Filter::needUpdate();
}

//-----------------------------------------------------------------------------
void ContourSource::signalAsModified()
{
  emit modified(this);
}

vtkPolyData* ContourSource::TransformContour(PlaneType plane, vtkPolyData* contour)
{
  double pos[3];

  int count = contour->GetPoints()->GetNumberOfPoints();
  vtkPolyData *rotatedContour = vtkPolyData::New();

  vtkPoints *points = vtkPoints::New();
  vtkCellArray *lines = vtkCellArray::New();
  vtkIdType index = 0;

  points->SetNumberOfPoints(count);
  vtkIdType numLines = count + 1;

  if (numLines > 0)
  {
    vtkIdType *lineIndices = new vtkIdType[numLines];

    for (int i = 0; i < count; i++)
    {
      double temporal;
      contour->GetPoint(i, pos);
      switch (plane)
      {
        case AXIAL:
          pos[2] -= 0.5 * m_param.spacing()[2];
          break;
        case CORONAL:
          temporal = pos[1];
          pos[1] = pos[2];
          pos[2] = temporal - 0.5 * m_param.spacing()[1];
          break;
        case SAGITTAL:
          temporal = pos[0];
          pos[0] = pos[1];
          pos[1] = pos[2];
          pos[2] = temporal - 0.5 * m_param.spacing()[0];
          break;
        default:
          Q_ASSERT(false);
          break;
      }
      points->InsertPoint(index, pos);
      lineIndices[index] = index;
      index++;
    }

    lineIndices[index] = 0;

    lines->InsertNextCell(numLines, lineIndices);
    delete[] lineIndices;
  }

  rotatedContour->SetPoints(points);
  rotatedContour->SetLines(lines);

  points->Delete();
  lines->Delete();

  rotatedContour->Update();
  return rotatedContour;
}