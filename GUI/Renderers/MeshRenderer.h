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

#ifndef MESHRENDERER_H
#define MESHRENDERER_H

// EspINA
#include "GUI/Renderers/Renderer.h"
#include <Core/Model/HierarchyItem.h>

// Qt
#include <QMap>

// VTK
#include <vtkSmartPointer.h>

class vtkImageConstantPad;
class vtkProperty;
class vtkPolyDataMapper;
class vtkActor;
class vtkPropPicker;

namespace EspINA
{
  class ViewManager;

  class MeshRenderer
  : public IRenderer
  {
    Q_OBJECT
    public:
      explicit MeshRenderer(ViewManager *vm, QObject* parent = 0);

      virtual const QIcon icon()      const { return QIcon(":/espina/mesh.png"); }
      virtual const QString name()    const { return "Mesh"; }
      virtual const QString tooltip() const { return "Segmentation's Meshes"; }

      virtual bool addItem(ModelItemPtr item);
      virtual bool removeItem(ModelItemPtr item);

      virtual void hide();
      virtual void show();
      virtual unsigned int getNumberOfvtkActors();

      virtual void clean()          { Q_ASSERT(false); }
      virtual IRendererSPtr clone() { return IRendererSPtr(new MeshRenderer(m_viewManager)); }

      virtual RenderedItems getRendererType() { return RenderedItems(IRenderer::SEGMENTATION); };
      virtual int itemsBeenRendered()        { return m_segmentations.size(); }

      virtual ViewManager::Selection pick(int x, int y, bool repeat);
      virtual void getPickCoordinates(double *point);

    public slots:
      virtual bool updateItem(ModelItemPtr item, bool forced = false);

    protected:
      virtual void createHierarchyProperties(SegmentationPtr seg);
      virtual bool updateHierarchyProperties(SegmentationPtr seg);

      ViewManager *m_viewManager;

      struct Representation
      {
          vtkActor *actor;
          bool visible;
          bool selected;
          QColor color;
          int extent[6];
          bool overridden;
          HierarchyItem::HierarchyRenderingType renderingType;
          vtkSmartPointer<vtkProperty> actorPropertyBackup;
      };

      QMap<ModelItemPtr, Representation> m_segmentations;
      vtkSmartPointer<vtkPropPicker> m_picker;

    private:
      // the beginning of the pipeline, needed to check if the mesh
      // of the segmentation has changed
      QMap<ModelItemPtr, vtkSmartPointer<vtkPolyDataMapper> > m_mappers;
  };

} // namespace EspINA

#endif // MESHRENDERER_H
