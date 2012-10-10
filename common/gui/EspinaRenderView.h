/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  <copyright holder> <email>

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


#ifndef ESPINARENDERVIEW_H
#define ESPINARENDERVIEW_H

#include <QWidget>
#include "common/gui/IEspinaView.h"

#include "common/EspinaTypes.h"
#include "common/selection/SelectionHandler.h"

class Channel;
class Segmentation;
class EspinaWidget;
class vtkProp;
class vtkRenderWindow;

class EspinaRenderView
: public QWidget
, public IEspinaView
{
  Q_OBJECT
public:
  explicit EspinaRenderView(QWidget* parent = 0);
  virtual ~EspinaRenderView();

  virtual void addChannel(Channel *channel) = 0;
  virtual void removeChannel(Channel *channel) = 0;
  virtual bool updateChannel(Channel *channel) = 0;

  virtual void addSegmentation(Segmentation *seg) = 0;
  virtual void removeSegmentation(Segmentation *seg) = 0;
  virtual bool updateSegmentation(Segmentation *seg) = 0;


  virtual void addWidget(EspinaWidget *widget) = 0;
  virtual void removeWidget(EspinaWidget *widget) = 0;

  virtual void addPreview(vtkProp *) = 0;
  virtual void removePreview(vtkProp *) = 0;

  virtual void setCursor(const QCursor& cursor) = 0;

  virtual void eventPosition(int &x, int &y) = 0;
  virtual IPicker::PickList pick(IPicker::PickableItems filter,
                                 IPicker::DisplayRegionList regions) = 0;

  virtual vtkRenderWindow *renderWindow() = 0;

  virtual void updateView() = 0;
  virtual void resetCamera() = 0;

  const Nm *sceneBounds() const {return m_sceneBounds;}
  const Nm *sceneResolution() const {return m_sceneResolution;}

protected slots:
  virtual void updateSceneBounds();

protected:
  void addChannelBounds(Channel *channel);
  void removeChannelBounds(Channel *channel);
  double suggestedChannelOpacity();

  void resetSceneBounds();


protected:
  QList<Channel *> m_channels;

  Nm m_sceneBounds[6];
  Nm m_sceneResolution[3];// Min distance between 2 voxels in each axis
};

#endif // ESPINARENDERVIEW_H
