/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "AppositionSurfaceExtension.h"
#include <Filter/AppositionSurfaceFilter.h>

// EspINA
#include <Core/Model/Segmentation.h>
#include <Core/EspinaSettings.h>

// Qt
#include <QDebug>
#include <boost/concept_check.hpp>

using namespace EspINA;

///-----------------------------------------------------------------------
/// APPOSITION SURFACE EXTENSION-
///-----------------------------------------------------------------------
/// Information Provided:
/// - Apposition Surface Area
/// - Apposition Surface Perimeter
/// - Apposition Surface Tortuosity
/// - Synapse from which the Apposition Surface was obtained

const ModelItem::ExtId   AppositionSurfaceExtension::ID
= "AppositionSurfaceExtension";

const Segmentation::InfoTag AppositionSurfaceExtension::AREA
= "AS Area";
const Segmentation::InfoTag AppositionSurfaceExtension::PERIMETER
= "AS Perimeter";
const Segmentation::InfoTag AppositionSurfaceExtension::TORTUOSITY
= "AS Tortuosity";

const Segmentation::InfoTag AppositionSurfaceExtension::SYNAPSE
= "AS Synapse";

const double UNDEFINED = -1.;

QMap<SegmentationPtr, AppositionSurfaceExtension::CacheEntry> AppositionSurfaceExtension::s_cache;


//------------------------------------------------------------------------
AppositionSurfaceExtension::CacheEntry::CacheEntry()
: Area      (UNDEFINED)
, Perimeter (UNDEFINED)
, Tortuosity(UNDEFINED)
{
}

//------------------------------------------------------------------------
AppositionSurfaceExtension::AppositionSurfaceExtension()
{
}

//------------------------------------------------------------------------
AppositionSurfaceExtension::~AppositionSurfaceExtension()
{
}

//------------------------------------------------------------------------
ModelItem::ExtId AppositionSurfaceExtension::id()
{
  return ID;
}

//------------------------------------------------------------------------
Segmentation::InfoTagList AppositionSurfaceExtension::availableInformations() const
{
  Segmentation::InfoTagList tags;

  tags << AREA << PERIMETER << TORTUOSITY << SYNAPSE;

  return tags;
}

//------------------------------------------------------------------------
QVariant AppositionSurfaceExtension::information(const Segmentation::InfoTag &tag)
{
  if (m_seg->taxonomy()->name() != tr("AS"))
    return QVariant();

  AppositionSurfaceFilter *filter = dynamic_cast<AppositionSurfaceFilter *>(m_seg->filter().data());

  if (AREA == tag)
    return filter->getArea();
  if (PERIMETER == tag)
    return filter->getPerimeter();
  if (TORTUOSITY == tag)
    return filter->getTortuosity();
  if (SYNAPSE == tag)
    return filter->getOriginSegmentation();

  qWarning() << ID << ":"  << tag << " is not provided";
  Q_ASSERT(false);
  return QVariant();
}

//------------------------------------------------------------------------
void AppositionSurfaceExtension::initialize(ModelItem::Arguments args)
{
  //m_init = true;
}

//------------------------------------------------------------------------
Segmentation::InformationExtension AppositionSurfaceExtension::clone()
{
  return new AppositionSurfaceExtension();
}
