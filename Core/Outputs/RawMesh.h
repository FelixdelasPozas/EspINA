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

#ifndef ESPINA_RAWMESH_H
#define ESPINA_RAWMESH_H

#include <Core/Outputs/MeshType.h>
#include <vtkSmartPointer.h>

namespace EspINA
{
  class RawMesh 
  : public MeshRepresentation
  {
  public:
    explicit RawMesh(FilterOutput *output=NULL);
    explicit RawMesh(vtkSmartPointer<vtkPolyData> mesh,
                     itkVolumeType::SpacingType spacing,
                     FilterOutput *output = NULL);

    virtual bool dumpSnapshot(const QString &prefix, Snapshot &snapshot) const;
    virtual bool fetchSnapshot(Filter *filter, const QString &prefix);
    virtual bool isValid() const;
    virtual bool setInternalData(SegmentationRepresentationSPtr rhs);
    virtual bool isEdited() const;
    virtual void clearEditedRegions();
    virtual void commitEditedRegions(bool withData) const;
    virtual void restoreEditedRegions(const QDir &cacheDir, const QString &outputId);

    virtual vtkAlgorithmOutput *mesh();

  private:
    vtkSmartPointer<vtkPolyData> m_mesh;
  };

  typedef boost::shared_ptr<RawMesh> RawMeshSPtr;

  RawMeshSPtr rawMesh(OutputSPtr output);
}

#endif // ESPINA_RAWMESH_H
