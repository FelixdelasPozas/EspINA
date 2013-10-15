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

#ifndef CHANNELVOLUMEPROXY_H
#define CHANNELVOLUMEPROXY_H

#include "EspinaCore_Export.h"

#include <Core/OutputRepresentations/VolumeRepresentation.h>

namespace EspINA
{
  class EspinaCore_EXPORT ChannelVolumeProxy
  : public ChannelVolume
  {
    Q_OBJECT
  public:
    explicit ChannelVolumeProxy(FilterOutput *output = NULL);

    virtual ~ChannelVolumeProxy(){}

    virtual bool setInternalData(ChannelRepresentationSPtr rhs);

    virtual void setVolume(itkVolumeType::Pointer volume, bool disconnect=false);

    virtual bool isValid() const;

    /// Volume's voxel's index at given spatial position
    /// It doesn't check whether the index is valid or not
    virtual itkVolumeType::IndexType index(Nm x, Nm y, Nm z);

    /// Get the vtk-equivalent extent defining the volume
    virtual void extent(int out[6]) const;

    /// Get the vtk-equivalent bounds defining the volume
    virtual void bounds(double out[6]) const;
    ///
    virtual void spacing(double out[3]) const;

    virtual itkVolumeType::SpacingType spacing() const;

    /// Return the smallest valid espina region  which contains bounds
    virtual EspinaRegion espinaRegion(Nm bounds[6]) const;

    /// Equivalent to bounds method
    virtual EspinaRegion espinaRegion() const;

    /// Largest possible region
    virtual VolumeRegion volumeRegion() const;

    /// Volume's region equivalent to the normalized region
    virtual VolumeRegion volumeRegion(const EspinaRegion &region) const; 

    virtual itkVolumeIterator iterator();
    virtual itkVolumeIterator iterator(const EspinaRegion &region);

    virtual itkVolumeConstIterator constIterator();
    virtual itkVolumeConstIterator constIterator(const EspinaRegion &region);

    virtual itkVolumeType::Pointer toITK();
    virtual const itkVolumeType::Pointer toITK() const;

    virtual vtkAlgorithmOutput *toVTK();
    virtual const vtkAlgorithmOutput *toVTK() const;

  private slots:
    void onProxyRepresentationChanged();

  private:
    ChannelVolumeSPtr  m_volumeRepresentation;
  };

  typedef ChannelVolumeProxy                  * ChannelVolumeProxyPtr;
  typedef boost::shared_ptr<ChannelVolumeProxy> ChannelVolumeProxySPtr;
}

#endif // CHANNELVOLUMEPROXY_H
