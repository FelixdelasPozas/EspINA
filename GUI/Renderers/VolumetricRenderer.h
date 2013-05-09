/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

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


#ifndef VOLUMETRICRENDERER_H
#define VOLUMETRICRENDERER_H

// EspINA
#include "GUI/Renderers/Renderer.h"
#include "GUI/Representations/VolumeRaycastRepresentation.h"

// VTK
#include <vtkVolumePicker.h>

namespace EspINA
{
  class VolumetricRenderer
  : public IRenderer
  {
  public:
    explicit VolumetricRenderer(QObject* parent = 0);

    virtual const QIcon icon() const {return QIcon(":/espina/voxel.png");}
    virtual const QString name() const {return "Volumetric";}
    virtual const QString tooltip() const {return "Segmentation's Volumes";}

    virtual void addRepresentation(GraphicalRepresentationSPtr rep);
    virtual void removeRepresentation(GraphicalRepresentationSPtr rep);
    virtual bool hasRepresentation(GraphicalRepresentationSPtr rep);
    virtual bool managesRepresentation(GraphicalRepresentationSPtr rep);

    virtual void hide();
    virtual void show();
    virtual unsigned int getNumberOfvtkActors() { return 0; }

    virtual IRendererSPtr clone() {return IRendererSPtr(new VolumetricRenderer());}

    virtual RenderedItems getRendererType() { return RenderedItems(IRenderer::SEGMENTATION); }
    virtual int itemsBeenRendered() { return m_representations.size(); }

    // to pick items been rendered
    virtual GraphicalRepresentationSList pick(int x, int y, bool repeat);
    virtual void getPickCoordinates(double *point);

  private:
    vtkSmartPointer<vtkVolumePicker> m_picker;
  };

} // namespace EspINA

#endif // VOLUMETRICRENDERER_H
