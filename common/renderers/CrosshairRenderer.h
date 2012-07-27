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


#ifndef CROSSHAIRRENDERER_H
#define CROSSHAIRRENDERER_H

#include "common/pluginInterfaces/Renderer.h"
#include "common/EspinaTypes.h"

#include <QMap>
#include <vtkImageActor.h>
#include <vtkActor.h>
#include <vtkPolyData.h>
#include <vtkMatrix4x4.h>
#include <vtkLookupTable.h>

class ModelItem;

class CrosshairRenderer
: public Renderer
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
  };

public:
  virtual const QIcon icon() const {return QIcon(":/espina/show_planes.svg");}
  virtual const QString name() const {return "Crosshairs";}
  virtual const QString tooltip() const {return "Sample's Crosshairs";}

  virtual bool addItem(ModelItem* item);
  virtual bool updateItem(ModelItem* item);
  virtual bool removeItem(ModelItem* item);

  virtual void hide();
  virtual void show();
  virtual unsigned int getNumberOfvtkActors();
  void setCrosshairColors(double axialColor[3], double coronalColor[3], double sagittalColor[3]);
  void setCrosshair(Nm point[3]);
  void setPlanePosition(PlaneType plane, Nm dist);

  virtual Renderer* clone() {return new CrosshairRenderer();}

private:
  QMap<ModelItem *, Representation> m_channels;
};

#endif // CROSSHAIRRENDERER_H
