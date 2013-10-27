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

#include "ChannelVolumeProxy.h"

#include <vtkMath.h>

using namespace EspINA;

//-----------------------------------------------------------------------------
ChannelVolumeProxy::ChannelVolumeProxy(FilterOutput *output)
: ChannelVolume(output)
{

}

//-----------------------------------------------------------------------------
bool ChannelVolumeProxy::setInternalData(ChannelRepresentationSPtr rhs)
{
  if (m_volumeRepresentation)
  {
    disconnect(m_volumeRepresentation.get(), SIGNAL(representationChanged()),
            this, SLOT(onProxyRepresentationChanged()));
  }

  m_volumeRepresentation = boost::dynamic_pointer_cast<ChannelVolume>(rhs);

  if (m_volumeRepresentation)
  {
    connect(m_volumeRepresentation.get(), SIGNAL(representationChanged()),
            this, SLOT(onProxyRepresentationChanged()));
  }

  emit representationChanged();

  return m_volumeRepresentation != NULL;
}

//-----------------------------------------------------------------------------
bool ChannelVolumeProxy::isValid() const
{
  bool res = false;

  if (m_volumeRepresentation)
    res = m_volumeRepresentation->isValid();

  return res;
}

//-----------------------------------------------------------------------------
void ChannelVolumeProxy::setVolume(itkVolumeType::Pointer volume, bool disconnect)
{

  if (m_volumeRepresentation)
    m_volumeRepresentation->setVolume(volume, disconnect);
}

//-----------------------------------------------------------------------------
itkVolumeType::IndexType ChannelVolumeProxy::index(Nm x, Nm y, Nm z)
{
  itkVolumeType::IndexType index;

  if (m_volumeRepresentation)
    index = m_volumeRepresentation->index(x, y, z);

  return index;
}

//-----------------------------------------------------------------------------
void ChannelVolumeProxy::extent(int out[6]) const
{
  if (m_volumeRepresentation)
    m_volumeRepresentation->extent(out);
}

//-----------------------------------------------------------------------------
void ChannelVolumeProxy::bounds(double out[6]) const
{
  if (m_volumeRepresentation)
    m_volumeRepresentation->bounds(out);
}

//-----------------------------------------------------------------------------
void ChannelVolumeProxy::spacing(double out[3]) const
{
  if (m_volumeRepresentation)
    m_volumeRepresentation->spacing(out);
}

//-----------------------------------------------------------------------------
itkVolumeType::SpacingType ChannelVolumeProxy::spacing() const
{
  itkVolumeType::SpacingType res;

  if (m_volumeRepresentation)
    res = m_volumeRepresentation->spacing();

  return res;
}

//-----------------------------------------------------------------------------
EspinaRegion ChannelVolumeProxy::espinaRegion(Nm bounds[6]) const
{
  EspinaRegion res;

  if (m_volumeRepresentation)
    res = m_volumeRepresentation->espinaRegion(bounds);

  return res;
}

//-----------------------------------------------------------------------------
EspinaRegion ChannelVolumeProxy::espinaRegion() const
{
  EspinaRegion res;

  if (m_volumeRepresentation)
    res = m_volumeRepresentation->espinaRegion();

  return res;
}

//-----------------------------------------------------------------------------
VolumeRepresentation::VolumeRegion ChannelVolumeProxy::volumeRegion() const
{
  VolumeRepresentation::VolumeRegion res;

  if (m_volumeRepresentation)
    res = m_volumeRepresentation->volumeRegion();

  return res;
}

//-----------------------------------------------------------------------------
VolumeRepresentation::VolumeRegion ChannelVolumeProxy::volumeRegion(const EspinaRegion &region) const
{
  VolumeRepresentation::VolumeRegion res;

  if (m_volumeRepresentation)
    res = m_volumeRepresentation->volumeRegion(region);

  return res;
}

//-----------------------------------------------------------------------------
itkVolumeIterator ChannelVolumeProxy::iterator()
{
  itkVolumeIterator it;

  if (m_volumeRepresentation)
    it = m_volumeRepresentation->iterator();

  return it;
}

//-----------------------------------------------------------------------------
itkVolumeIterator ChannelVolumeProxy::iterator(const EspinaRegion &region)
{
  itkVolumeIterator it;

  if (m_volumeRepresentation)
    it = m_volumeRepresentation->iterator(region);

  return it;
}

//-----------------------------------------------------------------------------
itkVolumeConstIterator ChannelVolumeProxy::constIterator()
{
  itkVolumeConstIterator it;

  if (m_volumeRepresentation)
    it = m_volumeRepresentation->constIterator();

  return it;
}

//-----------------------------------------------------------------------------
itkVolumeConstIterator ChannelVolumeProxy::constIterator(const EspinaRegion &region)
{
  itkVolumeConstIterator it;

  if (m_volumeRepresentation)
    it = m_volumeRepresentation->constIterator(region);

  return it;
}

//-----------------------------------------------------------------------------
itkVolumeType::Pointer ChannelVolumeProxy::toITK()
{
  itkVolumeType::Pointer res;

  if (m_volumeRepresentation)
    res = m_volumeRepresentation->toITK();

  return res;
}

//-----------------------------------------------------------------------------
const itkVolumeType::Pointer ChannelVolumeProxy::toITK() const
{
  itkVolumeType::Pointer res;

  if (m_volumeRepresentation)
    res =  m_volumeRepresentation->toITK();

  return res;
}

//-----------------------------------------------------------------------------
vtkAlgorithmOutput *ChannelVolumeProxy::toVTK()
{
  vtkAlgorithmOutput *res = NULL;

  if (m_volumeRepresentation)
    res = m_volumeRepresentation->toVTK();

  return res;
}

//-----------------------------------------------------------------------------
const vtkAlgorithmOutput *ChannelVolumeProxy::toVTK() const
{
  const vtkAlgorithmOutput *res = NULL;

  if (m_volumeRepresentation)
    res = m_volumeRepresentation->toVTK();

  return res;
}

//-----------------------------------------------------------------------------
void ChannelVolumeProxy::onProxyRepresentationChanged()
{
  emit representationChanged();
}
