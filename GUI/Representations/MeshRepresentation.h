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

#ifndef ESPINA_MESH_REPRESENTATION_H
#define ESPINA_MESH_REPRESENTATION_H

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include "MeshRepresentationBase.h"

namespace ESPINA
{
  class TransparencySelectionHighlighter;

  class EspinaGUI_EXPORT MeshRepresentation
  : public MeshRepresentationBase
  {
    Q_OBJECT
    public:
      static const Representation::Type TYPE;

    public:
      explicit MeshRepresentation(MeshDataSPtr data,
                                  RenderView *view);
      virtual ~MeshRepresentation() {};

      virtual RepresentationSettings *settingsWidget();

      virtual void updateRepresentation();

      void onCrosshairChanged(const NmVector3 &point) {};

      virtual bool crosshairDependent() const
      { return false; }

      virtual bool needUpdate()
      { return m_lastUpdatedTime != m_data->lastModified(); }

  protected:
      virtual RepresentationSPtr cloneImplementation(View3D *view);

    private:
      void initializePipeline();
  };

  using MeshRepresentationSPtr  = std::shared_ptr<MeshRepresentation>;
  using MeshRepresentationSList = QList<MeshRepresentationSPtr>;
} // namespace ESPINA

#endif // ESPINA_MESH_REPRESENTATION_H
