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

#ifndef ESPINA_MESH_REPRESENTATION_BASE_H
#define ESPINA_MESH_REPRESENTATION_BASE_H

#include "GUI/EspinaGUI_Export.h"

// EspINA
#include "Representation.h"
#include <Core/Analysis/Data/MeshData.h>
#include <GUI/View/View3D.h>

// VTK
#include <vtkSmartPointer.h>

class vtkPolyDataMapper;
class vtkActor;

namespace EspINA
{
  class TransparencySelectionHighlighter;

  class EspinaGUI_EXPORT MeshRepresentationBase
  : public Representation
  {
    Q_OBJECT
    public:
      explicit MeshRepresentationBase(MeshDataSPtr data,
                                      RenderView *view);
      virtual ~MeshRepresentationBase() {};

      virtual void setColor(const QColor &color);

      virtual void setHighlighted(bool highlighted);

      virtual bool isInside(const NmVector3 &point) const;

      virtual RenderableView canRenderOnView() const
      { return Representation::RENDERABLEVIEW_VOLUME; }

      virtual bool hasActor(vtkProp *actor) const;

      virtual void updateRepresentation() = 0;

      virtual QList<vtkProp *> getActors();

  protected:
      virtual RepresentationSPtr cloneImplementation(View2D *view)
      { return RepresentationSPtr(); }

      virtual RepresentationSPtr cloneImplementation(View3D *view) = 0;

    virtual void updateVisibility(bool visible);
    virtual void setView(RenderView *view) { m_view = view; };

    private:
      virtual void initializePipeline() = 0;

    protected:
      MeshDataSPtr m_data;
      vtkSmartPointer<vtkPolyDataMapper> m_mapper;
      vtkSmartPointer<vtkActor>          m_actor;

      static TransparencySelectionHighlighter *s_highlighter;
  };

  // one shouldn't address objects of this class directly but use the subclasses,
  // however this is here for convenience.
  using MeshRepresentationBaseSPtr  = std::shared_ptr<MeshRepresentationBase>;
  using MeshRepresentationBaseSList = QList<MeshRepresentationBaseSPtr>;

} // namespace EspINA
#endif // ESPINA_MESH_REPRESENTATION_BASE_H
