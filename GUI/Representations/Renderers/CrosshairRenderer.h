/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef CROSSHAIRRENDERER_H
#define CROSSHAIRRENDERER_H

#include "EspinaGUI_Export.h"

// EspINA
#include "Renderer.h"
#include <GUI/Representations/CrosshairRepresentation.h>
#include <Core/EspinaTypes.h>

// Qt
#include <QMap>

class vtkImageActor;
class vtkActor;
class vtkPolyData;
class vtkMatrix4x4;
class vtkLookupTable;
class vtkImageShiftScale;
class vtkPropPicker;

namespace EspINA
{
  class ViewManager;

  class EspinaGUI_EXPORT CrosshairRenderer
  : public Renderer
  {
  public:
    explicit CrosshairRenderer(QObject* parent = 0);
    virtual ~CrosshairRenderer();

    virtual const QIcon icon()      const {return QIcon(":/espina/show_planes.svg");}
    virtual const QString name()    const {return "Crosshairs";}
    virtual const QString tooltip() const {return "Sample's Crosshairs";}

    virtual void addRepresentation(ViewItemAdapterPtr item, RepresentationSPtr rep);
    virtual void removeRepresentation(RepresentationSPtr rep);
    virtual bool hasRepresentation(RepresentationSPtr rep);
    virtual bool managesRepresentation(RepresentationSPtr rep);

    virtual void hide();
    virtual void show();

    virtual RendererSPtr clone() { return RendererSPtr(new CrosshairRenderer()); }

    virtual unsigned int numberOfvtkActors();

    virtual RenderableItems renderableItems() { return RenderableItems(EspINA::CHANNEL); };

    virtual RendererTypes renderType() { return RendererTypes(RENDERER_VIEW3D); }

    virtual bool canRender(ItemAdapterPtr item)
    { return (item->type() == ItemAdapter::Type::CHANNEL); }

    virtual int numberOfRenderedItems()         { return m_representations.size(); };

    virtual SelectableView::Selection pick(int x,
                                           int y,
                                           Nm z,
                                           vtkSmartPointer<vtkRenderer> renderer,
                                           RenderableItems itemType = RenderableItems(),
                                           bool repeat = false);

    virtual NmVector3 pickCoordinates() const;

    void setCrosshairColors(double axialColor[3], double coronalColor[3], double sagittalColor[3]);
    void setCrosshair(NmVector3 point);
    void setPlanePosition(Plane plane, Nm dist);

  private:
    vtkSmartPointer<vtkPropPicker> m_picker;
  };

} // namespace EspINA

#endif // CROSSHAIRRENDERER_H
