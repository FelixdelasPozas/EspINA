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

#ifndef MESHPROXY_H
#define MESHPROXY_H

#include "MeshType.h"

namespace EspINA
{
  class MeshProxy
  : public MeshType
  {
  public:
    explicit MeshProxy(FilterOutput *output = NULL);
    virtual ~MeshProxy() {}

    virtual bool setInternalData(SegmentationRepresentationSPtr rhs);

    virtual bool dumpSnapshot(const QString &prefix, Snapshot &snapshot) const;

    virtual bool isValid() const;

    virtual bool isEdited() const;

    virtual void clearEditedRegions();

    virtual void commitEditedRegions(bool withData) const;

    virtual void restoreEditedRegions(const QDir &cacheDir, const QString &outputId);

    virtual vtkAlgorithmOutput *mesh() const;

  protected:
    MeshTypeSPtr m_meshRepresentation;
  };

  typedef MeshProxy                  * MeshProxyPtr;
  typedef boost::shared_ptr<MeshProxy> MeshProxySPtr;
}

#endif // MESHPROXY_H
