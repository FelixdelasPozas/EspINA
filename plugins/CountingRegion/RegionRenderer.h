/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>

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


#ifndef REGIONRENDERER_H
#define REGIONRENDERER_H

#include <QMap>
#include <common/pluginInterfaces/Renderer.h>

class BoundingRegion;
class CountingRegion;
class Sample;
class vtkAbstractWidget;

class RegionRenderer
: public Renderer
{
  Q_OBJECT
public:
  explicit RegionRenderer(CountingRegion *plugin);
  virtual ~RegionRenderer();

  virtual const QIcon icon() const
  { return QIcon(":/apply.svg"); }
  virtual const QString name() const
  { return tr("Counting Region");}
  virtual const QString tooltip() const
  { return tr("Counting Region's Borders");}

  virtual void hide();
  virtual void show();

  virtual Renderer* clone();

public slots:
  void regionCreated(BoundingRegion *region);
  void regionRemoved(BoundingRegion *region);

private:
  CountingRegion *m_plugin;
  QMap<BoundingRegion *, vtkAbstractWidget *> m_widgets;
};

#endif // REGIONRENDERER_H
