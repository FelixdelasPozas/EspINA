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

// EspINA
#include "GUI/Renderers/Renderer.h"
#include <Core/EspinaTypes.h>
#include "GUI/ViewManager.h"

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

  class CrosshairRenderer
  : public IRenderer
  {
  public:
    explicit CrosshairRenderer(QObject* parent = 0);
    virtual ~CrosshairRenderer();

    virtual const QIcon icon() const {return QIcon(":/espina/show_planes.svg");}
    virtual const QString name() const {return "Crosshairs";}
    virtual const QString tooltip() const {return "Sample's Crosshairs";}

    virtual void addRepresentation(PickableItemPtr seg, GraphicalRepresentationSPtr rep);
    virtual void removeRepresentation(GraphicalRepresentationSPtr rep);
    virtual bool hasRepresentation(GraphicalRepresentationSPtr rep);
    virtual bool managesRepresentation(GraphicalRepresentationSPtr rep);

    virtual void hide();
    virtual void show();
    virtual unsigned int getNumberOfvtkActors();

    void setCrosshairColors(double axialColor[3], double coronalColor[3], double sagittalColor[3]);
    void setCrosshair(Nm point[3]);
    void setPlanePosition(PlaneType plane, Nm dist);

    virtual IRendererSPtr clone()           { return IRendererSPtr(new CrosshairRenderer()); }
    virtual int itemsBeenRendered()         { return m_representations.size(); };

    virtual RendererType getRenderType() { return RendererType(RENDERER_VOLUMEVIEW); }
    virtual RenderabledItems getRenderableItemsType() { return RenderabledItems(EspINA::CHANNEL); };

    virtual ViewManager::Selection pick(int x,
                                        int y,
                                        vtkSmartPointer<vtkRenderer> renderer,
                                        RenderabledItems itemType = RenderabledItems(),
                                        bool repeat = false);
    virtual void getPickCoordinates(double *point);

  private:
    vtkSmartPointer<vtkPropPicker>                   m_picker;
  };

} // namespace EspINA

#endif // CROSSHAIRRENDERER_H
