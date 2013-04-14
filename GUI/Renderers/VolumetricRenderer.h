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

// VTK
#include <vtkVolume.h>
#include <vtkSmartPointer.h>
#include <vtkVolumePicker.h>

// EspINA
#include <Core/Model/HierarchyItem.h>
#include "GUI/Renderers/Renderer.h"

// Qt
#include <QMap>

class vtkVolumeProperty;
namespace EspINA
{
  class ViewManager;

  class VolumetricRenderer
  : public IRenderer
  {
    struct Representation
    {
      vtkVolume *volume;
      bool visible;
      bool selected;
      QColor color;
      bool overridden;
      HierarchyItem::HierarchyRenderingType renderingType;
      vtkSmartPointer<vtkVolumeProperty> actorPropertyBackup;
    };

  public:
    explicit VolumetricRenderer(ViewManager *vm, QObject* parent = 0);
    virtual ~VolumetricRenderer() {};

    virtual const QIcon icon() const {return QIcon(":/espina/voxel.png");}
    virtual const QString name() const {return "Volumetric";}
    virtual const QString tooltip() const {return "Segmentation's Volumes";}

    virtual bool addItem   (ModelItemPtr item);
    virtual bool updateItem(ModelItemPtr item, bool forced = false);
    virtual bool removeItem(ModelItemPtr item);

    virtual void hide();
    virtual void show();
    virtual unsigned int getNumberOfvtkActors(){return 0;}

    virtual IRendererSPtr clone() {return IRendererSPtr(new VolumetricRenderer(m_viewManager));}

    virtual RenderedItems getRendererType() { return RenderedItems(IRenderer::SEGMENTATION); }
    virtual int itemsBeenRendered() { return m_segmentations.size(); }

    // to pick items been rendered
    virtual ViewManager::Selection pick(int x, int y, bool repeat);
    virtual void getPickCoordinates(double *point);

  private:
    // helper methods
    void createHierarchyProperties(SegmentationPtr seg);
    bool updateHierarchyProperties(SegmentationPtr seg);

    ViewManager *m_viewManager;
    QMap<ModelItemPtr, Representation> m_segmentations;
    vtkSmartPointer<vtkVolumePicker> m_picker;
  };

} // namespace EspINA

#endif // VOLUMETRICRENDERER_H
