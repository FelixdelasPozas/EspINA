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


typedef ModelItem::ArgumentId ArgumentId;
const ArgumentId ContourSource::SPACING = "SPACING";

//-----------------------------------------------------------------------------
ContourSource::ContourSource(NamedInputs inputs,
                             Arguments   args,
                             FilterType  type)
: SegmentationFilter(inputs, args, type)
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
                         itkVolumeType::PixelType value,
                         bool emitSignal)
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
    m_contourMap[plane].insert(slice, contour);
    Filter::draw(oId, contour, slice, plane, value, emitSignal);
  }

  m_executed = true;
}

//-----------------------------------------------------------------------------
bool ContourSource::needUpdate() const
{
  return Filter::needUpdate();
}
