/*
 
 Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

 This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ESPINA_SMOOTHED_MESH_REPRESENTATION_H_
#define ESPINA_SMOOTHED_MESH_REPRESENTATION_H_

#include "GUI/EspinaGUI_Export.h"

// EspINA
#include "MeshRepresentation.h"

class vtkDecimatePro;

namespace EspINA
{
  class EspinaGUI_EXPORT SmoothedMeshRepresentation
  : public MeshRepresentationBase
  {
    Q_OBJECT
  public:
    static const Representation::Type TYPE;

  public:
    explicit SmoothedMeshRepresentation(MeshDataSPtr mesh,
                                        RenderView *view);
    virtual ~SmoothedMeshRepresentation() {};

    virtual RepresentationSettings *settingsWidget();

    virtual void updateRepresentation();

    virtual bool crosshairDependent() const
    { return false; }

    virtual bool needUpdate()
    { return m_lastUpdatedTime != m_data->lastModified(); }

  protected:
    virtual RepresentationSPtr cloneImplementation(View3D *view);

  private:
    void setView(RenderView *view) { m_view = view; };
    void initializePipeline();

  private:
    vtkSmartPointer<vtkDecimatePro> m_decimate;
  };

  using SmoothedMeshRepresentationSPtr  = std::shared_ptr<SmoothedMeshRepresentation>;
  using SmoothedMeshRepresentationSList = QList<SmoothedMeshRepresentationSPtr>;

} /* namespace EspINA */
#endif /* ESPINA_SMOOTHED_MESH_REPRESENTATION_H_ */
