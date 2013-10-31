/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2013 Félix de las Pozas Álvarez <felixdelaspozas@gmail.com>

 This program is free software: you can redistribute it and/or modify
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

#include "EspinaGUI_Export.h"

// EspINA
#include "MeshRepresentationBase.h"

namespace EspINA
{
  class TransparencySelectionHighlighter;
  class VolumeView;

  class EspinaGUI_EXPORT MeshRepresentation
  : public MeshRepresentationBase
  {
    Q_OBJECT
    public:
      explicit MeshRepresentation(MeshRepresentationSPtr data,
                                  EspinaRenderView *view);
      virtual ~MeshRepresentation() {};

      virtual GraphicalRepresentationSettings *settingsWidget();

      virtual void updateRepresentation();

  protected:
      virtual GraphicalRepresentationSPtr cloneImplementation(VolumeView *view);

    private slots:
      void updatePipelineConnections();

    private:
      void initializePipeline();
  };

  typedef boost::shared_ptr<SimpleMeshRepresentation> SimpleMeshRepresentationSPtr;
  typedef QList<SimpleMeshRepresentationSPtr> MeshRepresentationSList;

} // namespace EspINA

#endif // ESPINA_MESH_REPRESENTATION_H
