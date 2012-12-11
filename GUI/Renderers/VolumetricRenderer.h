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

#include <vtkVolume.h>
#include <vtkSmartPointer.h>

#include <Core/Model/HierarchyItem.h>
#include "GUI/Renderers/Renderer.h"

#include <QMap>

class ViewManager;
class ModelItem;
class Segmentation;
class vtkVolumeProperty;

class VolumetricRenderer
: public Renderer
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

  virtual const QIcon icon() const {return QIcon(":/espina/voxel.png");}
  virtual const QString name() const {return "Volumetric";}
  virtual const QString tooltip() const {return "Segmentation's Volumes";}

  virtual bool addItem(ModelItem* item);
  virtual bool updateItem(ModelItem* item);
  virtual bool removeItem(ModelItem* item);

  virtual void hide();
  virtual void show();

  virtual Renderer* clone() {return new VolumetricRenderer(m_viewManager);}

  virtual bool isASegmentationRenderer() { return true; };
private:
  // helper methods
  void createHierarchyProperties(Segmentation *);
  bool updateHierarchyProperties(Segmentation *);

  ViewManager *m_viewManager;
  QMap<ModelItem *, Representation> m_segmentations;
};

#endif // VOLUMETRICRENDERER_H
