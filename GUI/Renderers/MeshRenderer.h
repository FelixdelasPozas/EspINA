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

#include "GUI/Renderers/Renderer.h"
#include <Core/Model/HierarchyItem.h>

#include <QMap>
#include <vtkActor.h>
#include <vtkSmartPointer.h>

class vtkImageConstantPad;
class vtkProperty;
class vtkDecimatePro;

namespace EspINA
{
class ViewManager;

class MeshRenderer
: public IRenderer
{
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
    // the beginning of the pipeline, needed to check if the mesh
    // of the segmentation has changed
    vtkSmartPointer<vtkDecimatePro> decimate;
  };
public:
  explicit MeshRenderer(ViewManager *vm, QObject* parent = 0);

  virtual const QIcon icon() const      {return QIcon(":/espina/mesh.png");}
  virtual const QString name() const    {return "Mesh";}
  virtual const QString tooltip() const {return "Segmentation's Meshes";}

  virtual bool addItem   (ModelItemPtr item);
  virtual bool updateItem(ModelItemPtr item);
  virtual bool removeItem(ModelItemPtr item);

  virtual void hide();
  virtual void show();
  virtual unsigned int getNumberOfvtkActors();

  virtual void clean() {Q_ASSERT(false);}
  virtual IRendererSPtr clone() {return IRendererSPtr(new MeshRenderer(m_viewManager));}

  virtual bool isASegmentationRenderer() { return true; };

protected:
  ViewManager *m_viewManager;

private:
  void createHierarchyProperties(SegmentationPtr seg);
  bool updateHierarchyProperties(SegmentationPtr seg);

  QMap<ModelItemPtr, Representation> m_segmentations;
};

} // namespace EspINA

#endif // MESHRENDERER_H
