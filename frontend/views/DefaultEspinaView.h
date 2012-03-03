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


#ifndef DEFAULTESPINAVIEW_H
#define DEFAULTESPINAVIEW_H

#include <common/gui/EspinaView.h>

// Forward-declaration
class SliceView;
class VolumeView;

class DefaultEspinaView
: public EspinaView
{
  Q_OBJECT
public:
  explicit DefaultEspinaView(QMainWindow* parent, const QString activity = QString());

  virtual void createViewMenu(QMenu* menu);
  virtual void restoreLayout();
  virtual void saveLayout();

  virtual void resetCamera();

  virtual QSize sizeHint() const;

  virtual void gridSize(double size[3]);
  virtual void setGridSize(double size[3]);
  virtual void addWidget(EspinaWidget* widget);

  virtual void setShowSegmentations(bool visibility);
  void setCenter(double x, double y, double z);

protected slots:
  virtual void rowsInserted(const QModelIndex& parent, int start, int end);
  virtual void rowsAboutToBeRemoved(const QModelIndex& parent, int start, int end);

  void setFitToSlices(bool fit);
  void setRulerVisibility(bool visible);


private:
  bool first;

  SliceView  *xyView, *yzView, *xzView;
  VolumeView *volView;
  QDockWidget *volDock, *yzDock, *xzDock;
  double m_gridSize[3];
};

#endif // DEFAULTESPINAVIEW_H
