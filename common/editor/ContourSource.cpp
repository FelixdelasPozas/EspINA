/*
 * ContourSource.cpp
 *
 *  Created on: Sep 27, 2012
 *      Author: Félix de las Pozas Álvarez
 */

#include "ContourSource.h"
#include "ContourSourceInspector.h"

#include <EspinaRegions.h>
#include <model/EspinaFactory.h>

#include <itkImageRegionIteratorWithIndex.h>
#include <vtkCellArray.h>
#include <vtkPoints.h>

#include <QDebug>
const QString ContourSource::TYPE = "EditorToolBar::ContourSource";

typedef ModelItem::ArgumentId ArgumentId;
const ArgumentId ContourSource::SPACING = "SPACING";

//-----------------------------------------------------------------------------
ContourSource::ContourSource(Filter::NamedInputs inputs,
                               ModelItem::Arguments args)
: Filter(inputs, args)
, m_param(m_args)
, ImageInitializedByFilter(false)
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
void ContourSource::draw(OutputNumber i, vtkPolyData *contour, Nm slice, PlaneType plane,
    EspinaVolume::PixelType value)
{
  double bounds[6] = { 0,0,0,0,0,0 };

  Q_ASSERT(0 == i);
  if (m_outputs[i].IsNull())
  {
    // need to create a dummy image to register the filter/action, we'll change it later.
    ImageInitializedByFilter = true;
    EspinaVolume::Pointer img = EspinaVolume::New();
    EspinaVolume::RegionType buffer = BoundsToRegion(bounds, m_param.spacing());
    img->SetRegions(buffer);
    img->SetSpacing(m_param.spacing());
    img->Allocate();
    img->FillBuffer(0);
    m_outputs[i] = img;
  }
  else
  {
    contour->ComputeBounds();
    contour->GetBounds(bounds);
    if (true == ImageInitializedByFilter)
    {
      ImageInitializedByFilter = false;
      EspinaVolume::RegionType buffer = BoundsToRegion(bounds, m_param.spacing());
      m_outputs[i]->SetRegions(buffer);
      m_outputs[i]->SetSpacing(m_param.spacing());
      m_outputs[i]->Allocate();
      m_outputs[i]->FillBuffer(0);
    }
    else
    {
      EspinaVolume::SpacingType spacing = m_outputs[i]->GetSpacing();
      EspinaVolume::RegionType contourRegion = BoundsToRegion(bounds, spacing);
      m_outputs[i] = addRegionToVolume(m_outputs[i], contourRegion);
    }

    vtkPolyData *newContour = this->TransformContour(plane, contour);
    m_contourMap[plane].insert(slice, newContour);
    Filter::draw(i, newContour, slice, plane);
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
  return m_outputs[0].IsNull();
}

//-----------------------------------------------------------------------------
void ContourSource::signalAsModified()
{
  emit modified(this);
}

vtkPolyData* ContourSource::TransformContour(PlaneType plane, vtkPolyData* contour)
{
  double spacing[3], pos[3], temporal;
  //BUG 2012-10-04: Coger spacing del canal seccionado, en lugar del activo
  //SelectionManager::instance()->activeChannel()->spacing(spacing);
  Q_ASSERT(false);
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
      contour->GetPoint(i, pos);
      switch (plane)
      {
        case AXIAL:
//          pos[2] -= 0.5 * spacing[2];
          break;
        case CORONAL:
          temporal = pos[1];
          pos[1] = pos[2];
          pos[2] = temporal; // - 0.5 * spacing[1];
          break;
        case SAGITTAL:
          temporal = pos[0];
          pos[0] = pos[1];
          pos[1] = pos[2];
          pos[2] = temporal;// - 0.5 * spacing[0];
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

QWidget* ContourSource::createConfigurationWidget()
{
  return new ContourSource::ContourSourceInspector(this);
}

