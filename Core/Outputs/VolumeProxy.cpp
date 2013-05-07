/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "VolumeProxy.h"
#include <vtkMath.h>

using namespace EspINA;

//-----------------------------------------------------------------------------
VolumeProxy::VolumeProxy(FilterOutput *output)
: SegmentationVolume(output)
{

}

//-----------------------------------------------------------------------------
bool VolumeProxy::setInternalData(SegmentationRepresentationSPtr rhs)
{
  m_volumeRepresentation = boost::dynamic_pointer_cast<SegmentationVolume>(rhs);

  return m_volumeRepresentation != NULL;
}

//-----------------------------------------------------------------------------
void VolumeProxy::draw(vtkImplicitFunction *brush,
                       const Nm bounds[6],
                       itkVolumeType::PixelType value,
                       bool emitSignal)
{
  if (m_volumeRepresentation)
    m_volumeRepresentation->draw(brush, bounds, value, emitSignal);
}

//-----------------------------------------------------------------------------
void VolumeProxy::draw(itkVolumeType::IndexType index,
                       itkVolumeType::PixelType value,
                       bool emitSignal)
{
  if (m_volumeRepresentation)
    m_volumeRepresentation->draw(index, value, emitSignal);
}

//-----------------------------------------------------------------------------
void VolumeProxy::draw(Nm x, Nm y, Nm z,
                       itkVolumeType::PixelType value,
                       bool emitSignal)
{
  if (m_volumeRepresentation)
    m_volumeRepresentation->draw(x, y, z, value, emitSignal);
}

//-----------------------------------------------------------------------------
void VolumeProxy::draw(vtkPolyData *contour,
                       Nm slice, PlaneType plane,
                       itkVolumeType::PixelType value,
                       bool emitSignal)
{
  if (m_volumeRepresentation)
    m_volumeRepresentation->draw(contour, slice, plane, value, emitSignal);
}

//-----------------------------------------------------------------------------
void VolumeProxy::draw(itkVolumeType::Pointer volume,
                       bool emitSignal)
{
  if (m_volumeRepresentation)
    m_volumeRepresentation->draw(volume, emitSignal);
}

//-----------------------------------------------------------------------------
void VolumeProxy::fill(itkVolumeType::PixelType value, bool emitSignal)
{
  if (m_volumeRepresentation)
    m_volumeRepresentation->fill(value, emitSignal);
}

//-----------------------------------------------------------------------------
void VolumeProxy::fill(const EspinaRegion &region, itkVolumeType::PixelType value, bool emitSignal)
{
  if (m_volumeRepresentation)
    m_volumeRepresentation->fill(region, value, emitSignal);
}

//-----------------------------------------------------------------------------
bool VolumeProxy::dumpSnapshot(const QString &prefix, Snapshot &snapshot) const
{
  bool dumped = false;

  if (m_volumeRepresentation)
    dumped = m_volumeRepresentation->dumpSnapshot(prefix, snapshot);

  return dumped;
}

// //-----------------------------------------------------------------------------
// bool VolumeProxy::fetchSnapshot(Filter *filter, const QString &prefix)
// {
//   bool fetched = false;
// 
//   if (m_volumeRepresentation)
//     fetched = m_volumeRepresentation->fetchSnapshot(filter, prefix);
// 
//   return fetched;
// }

//-----------------------------------------------------------------------------
bool VolumeProxy::isEdited() const
{
  bool res = false;

  if (m_volumeRepresentation)
    res = m_volumeRepresentation->isEdited();

  return res;
}

//-----------------------------------------------------------------------------
bool VolumeProxy::isValid() const
{
  bool res = false;

  if (m_volumeRepresentation)
    res = m_volumeRepresentation->isValid();

  return res;
}

//-----------------------------------------------------------------------------
void VolumeProxy::addEditedRegion(const EspinaRegion &region)
{
  if (m_volumeRepresentation)
    m_volumeRepresentation->addEditedRegion(region);
}

//-----------------------------------------------------------------------------
void VolumeProxy::clearEditedRegions()
{
  if (m_volumeRepresentation)
    m_volumeRepresentation->clearEditedRegions();
}

//-----------------------------------------------------------------------------
void VolumeProxy::commitEditedRegions(bool withData) const
{
  if (m_volumeRepresentation)
    m_volumeRepresentation->commitEditedRegions(withData);
}

//-----------------------------------------------------------------------------
void VolumeProxy::restoreEditedRegion(Filter *filter, const EspinaRegion &region, const QString &prefix)
{
  if (m_volumeRepresentation)
    m_volumeRepresentation->restoreEditedRegion(filter, region, prefix);
}

//-----------------------------------------------------------------------------
QList<EspinaRegion> VolumeProxy::editedRegions() const
{
  QList<EspinaRegion> res;

  if (m_volumeRepresentation)
    res = m_volumeRepresentation->editedRegions();

  return res;
}

//-----------------------------------------------------------------------------
void VolumeProxy::setEditedRegions(QList<EspinaRegion> regions)
{
  if (m_volumeRepresentation)
    m_volumeRepresentation->setEditedRegions(regions);
}

//-----------------------------------------------------------------------------
void VolumeProxy::setVolume(itkVolumeType::Pointer volume, bool disconnect)
{

  if (m_volumeRepresentation)
    m_volumeRepresentation->setVolume(volume, disconnect);
}

//-----------------------------------------------------------------------------
itkVolumeType::IndexType VolumeProxy::index(Nm x, Nm y, Nm z)
{
  itkVolumeType::IndexType index;

  if (m_volumeRepresentation)
    index = m_volumeRepresentation->index(x, y, z);

  return index;
}

//-----------------------------------------------------------------------------
void VolumeProxy::extent(int out[6]) const
{
  if (m_volumeRepresentation)
    m_volumeRepresentation->extent(out);
}

//-----------------------------------------------------------------------------
void VolumeProxy::bounds(double out[6]) const
{
  if (m_volumeRepresentation)
    m_volumeRepresentation->bounds(out);
}

//-----------------------------------------------------------------------------
void VolumeProxy::spacing(double out[3]) const
{
  if (m_volumeRepresentation)
    m_volumeRepresentation->spacing(out);
}

//-----------------------------------------------------------------------------
itkVolumeType::SpacingType VolumeProxy::spacing() const
{
  itkVolumeType::SpacingType res;

  if (m_volumeRepresentation)
    res = m_volumeRepresentation->spacing();

  return res;
}

//-----------------------------------------------------------------------------
EspinaRegion VolumeProxy::espinaRegion() const
{
  EspinaRegion res;

  if (m_volumeRepresentation)
    res = m_volumeRepresentation->espinaRegion();

  return res;
}

//-----------------------------------------------------------------------------
VolumeRepresentation::VolumeRegion VolumeProxy::volumeRegion() const
{
  VolumeRepresentation::VolumeRegion res;

  if (m_volumeRepresentation)
    res = m_volumeRepresentation->volumeRegion();

  return res;
}

//-----------------------------------------------------------------------------
VolumeRepresentation::VolumeRegion VolumeProxy::volumeRegion(const EspinaRegion &region) const
{
  VolumeRepresentation::VolumeRegion res;

  if (m_volumeRepresentation)
    res = m_volumeRepresentation->volumeRegion(region);

  return res;
}

//-----------------------------------------------------------------------------
itkVolumeIterator VolumeProxy::iterator()
{
  itkVolumeIterator it;

  if (m_volumeRepresentation)
    it = m_volumeRepresentation->iterator();

  return it;
}

//-----------------------------------------------------------------------------
itkVolumeIterator VolumeProxy::iterator(const EspinaRegion &region)
{
  itkVolumeIterator it;

  if (m_volumeRepresentation)
    it = m_volumeRepresentation->iterator(region);

  return it;
}

//-----------------------------------------------------------------------------
itkVolumeConstIterator VolumeProxy::constIterator()
{
  itkVolumeConstIterator it;

  if (m_volumeRepresentation)
    it = m_volumeRepresentation->constIterator();

  return it;
}

//-----------------------------------------------------------------------------
itkVolumeConstIterator VolumeProxy::constIterator(const EspinaRegion &region)
{
  itkVolumeConstIterator it;

  if (m_volumeRepresentation)
    it = m_volumeRepresentation->constIterator(region);

  return it;
}

//-----------------------------------------------------------------------------
itkVolumeType::Pointer VolumeProxy::toITK()
{
  itkVolumeType::Pointer res;

  if (m_volumeRepresentation)
    res = m_volumeRepresentation->toITK();

  return res;
}

//-----------------------------------------------------------------------------
const itkVolumeType::Pointer VolumeProxy::toITK() const
{
  itkVolumeType::Pointer res;

  if (m_volumeRepresentation)
    res =  m_volumeRepresentation->toITK();

  return res;
}

//-----------------------------------------------------------------------------
vtkAlgorithmOutput *VolumeProxy::toVTK()
{
  vtkAlgorithmOutput *res = NULL;

  if (m_volumeRepresentation)
    res = m_volumeRepresentation->toVTK();

  return res;
}

//-----------------------------------------------------------------------------
const vtkAlgorithmOutput *VolumeProxy::toVTK() const
{
  const vtkAlgorithmOutput *res = NULL;

  if (m_volumeRepresentation)
    res = m_volumeRepresentation->toVTK();

  return res;
}

//-----------------------------------------------------------------------------
itkVolumeType::Pointer VolumeProxy::cloneVolume() const
{
  itkVolumeType::Pointer res;

  if (m_volumeRepresentation)
    res = m_volumeRepresentation->cloneVolume();

  return res;
}

//-----------------------------------------------------------------------------
itkVolumeType::Pointer VolumeProxy::cloneVolume(const EspinaRegion &region) const
{
  itkVolumeType::Pointer res;

  if (m_volumeRepresentation)
    res = m_volumeRepresentation->cloneVolume(region);

  return res;
}

//-----------------------------------------------------------------------------
itkVolumeType::Pointer VolumeProxy::cloneVolume(const VolumeRepresentation::VolumeRegion &region) const
{
  itkVolumeType::Pointer res;

  if (m_volumeRepresentation)
    res = m_volumeRepresentation->cloneVolume(region);

  return res;
}

//-----------------------------------------------------------------------------
void VolumeProxy::markAsModified(bool emitSignal)
{
  if (m_volumeRepresentation)
    m_volumeRepresentation->markAsModified(emitSignal);
}

//-----------------------------------------------------------------------------
bool VolumeProxy::fitToContent()  throw(itk::ExceptionObject)
{
  bool res = false;

  if (m_volumeRepresentation)
    res = m_volumeRepresentation->fitToContent();

  return res;
}

//-----------------------------------------------------------------------------
bool VolumeProxy::collision(SegmentationVolumeSPtr segmentation)
{
  bool res = false;

  if (m_volumeRepresentation)
    res = m_volumeRepresentation->collision(segmentation);

  return res;
}