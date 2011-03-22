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

#include "EspinaPlugin.h"

#include <QMap>

class Sample;
class pqPipelineSource;

class RegionRenderer : public IViewWidget
{
  Q_OBJECT
public:
  RegionRenderer(QMap<Sample *, QList<pqPipelineSource *> > &regions, QWidget* parent = 0);
  
  virtual IViewWidget* clone();
  
public slots:
  virtual void updateState(bool checked);
  virtual void renderInView(pqView* view);
  
private:
  QMap<Sample *, QList<pqPipelineSource *> > &m_regions;
};

#endif // REGIONRENDERER_H
