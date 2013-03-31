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
void ContourSource::draw(OutputId oId,
                         vtkPolyData *contour,
                         Nm slice, PlaneType plane,
                         itkVolumeType::PixelType value,
                         bool emitSignal)
{
  Q_ASSERT(0 == oId);
  if (m_outputs.isEmpty())
    createOutput(0, EspinaRegion(contour->GetBounds()), m_param.spacing());

  Filter::draw(oId, contour, slice, plane, value, emitSignal);

  m_executed = true;
}

//-----------------------------------------------------------------------------
bool ContourSource::needUpdate(OutputId oId) const
{
  return Filter::needUpdate(oId);
}
