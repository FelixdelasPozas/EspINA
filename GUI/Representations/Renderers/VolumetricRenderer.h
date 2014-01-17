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

#ifndef ESPINA_VOLUMETRIC_RENDERER_H
#define ESPINA_VOLUMETRIC_RENDERER_H

#include "EspinaGUI_Export.h"

// EspINA
#include <GUI/Representations/Renderers/Renderer.h>
#include <GUI/Representations/VolumetricRepresentation.h>
#include <GUI/View/RenderView.h>
#include <Core/EspinaTypes.h>
#include <Support/ViewManager.h>

// VTK
#include <vtkVolumePicker.h>
#include <vtkVolume.h>

// Qt
#include <QApplication>


namespace EspINA
{
  template<class T>
  class EspinaGUI_EXPORT VolumetricRenderer
  : public Renderer
  {
  public:
    explicit VolumetricRenderer(QObject* parent = 0);
    virtual ~VolumetricRenderer();

    const QIcon icon()      const {return QIcon(":/espina/voxel.png");}
    const QString name()    const {return "Volumetric";}
    const QString tooltip() const {return "Segmentation's Volumes";}

    void addRepresentation(ViewItemAdapterPtr item, RepresentationSPtr rep);
    void removeRepresentation(RepresentationSPtr rep);
    bool hasRepresentation(RepresentationSPtr rep);
    bool managesRepresentation(RepresentationSPtr rep);

    void hide();
    void show();

    RendererSPtr clone() {return RendererSPtr(new VolumetricRenderer());}

    unsigned int numberOfvtkActors() { return 0; }

    RenderableItems renderableItems() { return RenderableItems(EspINA::SEGMENTATION); }

    RendererTypes renderType() { return RendererTypes(RENDERER_VIEW3D); }

    bool canRender(ItemAdapterPtr item)
    { return (item->type() == ItemAdapter::Type::SEGMENTATION); }

    int numberOfRenderedItems() { return m_representations.size(); }

    // to pick items been rendered
    ViewItemAdapterList pick(int x, int y, Nm z,
                             vtkSmartPointer<vtkRenderer> renderer,
                             RenderableItems itemType = RenderableItems(),
                             bool repeat = false);

    NmVector3 pickCoordinates() const;

  protected:
    vtkSmartPointer<vtkVolumePicker> m_picker;
  };


} // namespace EspINA

#include "VolumetricRenderer.cpp"

#endif // ESPINA_VOLUMETRIC_RENDERER_H
