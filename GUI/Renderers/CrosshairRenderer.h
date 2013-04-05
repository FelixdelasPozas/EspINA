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
    struct Representation
    {
      vtkImageActor *axial;
      vtkImageActor *coronal;
      vtkImageActor *sagittal;
      vtkActor *axialBorder;
      vtkActor *coronalBorder;
      vtkActor *sagittalBorder;
      vtkPolyData *axialSquare;
      vtkPolyData *coronalSquare;
      vtkPolyData *sagittalSquare;
      vtkMatrix4x4 *matAxial;
      vtkMatrix4x4 *matCoronal;
      vtkMatrix4x4 *matSagittal;
      vtkLookupTable *lut;
      double bounds[6];
      bool visible;
      bool selected;
      QColor color;
      Nm point[3];
      vtkImageShiftScale *axialScaler;
      vtkImageShiftScale *coronalScaler;
      vtkImageShiftScale *sagittalScaler;
      double contrast;
      double brightness;
    };

  public:
    explicit CrosshairRenderer(ViewManager *vm, QObject* parent = 0);

    virtual const QIcon icon() const {return QIcon(":/espina/show_planes.svg");}
    virtual const QString name() const {return "Crosshairs";}
    virtual const QString tooltip() const {return "Sample's Crosshairs";}

    virtual bool addItem   (ModelItemPtr item);
    virtual bool updateItem(ModelItemPtr item, bool forced = false);
    virtual bool removeItem(ModelItemPtr item);

    virtual void hide();
    virtual void show();
    virtual unsigned int getNumberOfvtkActors();
    void setCrosshairColors(double axialColor[3], double coronalColor[3], double sagittalColor[3]);
    void setCrosshair(Nm point[3]);
    void setPlanePosition(PlaneType plane, Nm dist);

    virtual IRendererSPtr clone() {return IRendererSPtr(new CrosshairRenderer(m_viewManager));}

    virtual void clean() {Q_ASSERT(false);}

    virtual int itemsBeenRendered() { return m_channels.size(); };

    virtual RenderedItems getRendererType() { return RenderedItems(IRenderer::CHANNEL); };

    virtual ViewManager::Selection pick(int x, int y, bool repeat);
    virtual void getPickCoordinates(double *point);

  private:
    ViewManager *m_viewManager;
    QMap<ModelItemPtr, Representation> m_channels;
    vtkSmartPointer<vtkPropPicker>     m_picker;
  };

} // namespace EspINA

#endif // CROSSHAIRRENDERER_H
