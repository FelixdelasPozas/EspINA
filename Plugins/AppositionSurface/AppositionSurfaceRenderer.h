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


#ifndef APPOSITIONPLANERENDERER_H
#define APPOSITIONPLANERENDERER_H

#include <common/gui/Renderer.h>

#include <QMap>

class ModelItem;

class AppositionSurfaceRenderer
: public Renderer
{
  struct Representation;
  struct State;

public:
  virtual ~AppositionSurfaceRenderer(){}

  virtual const QIcon icon() const
  { return QIcon(":/AppSurface.svg"); }
  virtual const QString name() const
  { return tr("Apposition Surface");}
  virtual const QString tooltip() const
  { return tr("Segmentation's Apposition Surface");}

  virtual bool addItem(ModelItem* item);
  virtual bool updateItem(ModelItem* item);
  virtual bool removeItem(ModelItem* item);

  virtual void hide();
  virtual void show();

  virtual Renderer* clone();

private:
  static QMap<ModelItem *, Representation*> m_representations;
  QMap<ModelItem *, State *> m_state;
};

#endif // APPOSITIONPLANERENDERER_H