/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2012  <copyright holder> <email>
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef APPOSITIONSURFACERENDERER_H
#define APPOSITIONSURFACERENDERER_H

// EspINA
#include <GUI/Renderers/Renderer.h>

// Qt
#include <QMap>

class QColor;

namespace EspINA
{
  class AppositionSurface;

  class AppositionSurfaceRenderer
  : public IRenderer
  {
    struct Representation;
    struct State;

  public:
    explicit AppositionSurfaceRenderer(QColor color, AppositionSurface *plugin);
    virtual ~AppositionSurfaceRenderer();

    virtual const QIcon   icon()    const { return QIcon(":/AppSurface.svg"); }
    virtual const QString name()    const { return tr("Apposition Surface");}
    virtual const QString tooltip() const { return tr("Segmentation's Apposition Surface");}

    virtual bool addItem   (ModelItemPtr item);
    virtual bool updateItem(ModelItemPtr item);
    virtual bool removeItem(ModelItemPtr item);

    virtual void hide();
    virtual void show();

    virtual void clean();
    virtual IRendererSPtr clone();

    // updates the color of the actors
    void SetColor(QColor);

    // get number of vtkActors added to vtkRendered from this Renderer
    virtual unsigned int getNumberOfvtkActors() { return m_representations.size(); }

  private:
    QColor m_color;
    AppositionSurface* m_plugin;
    static QMap<ModelItemPtr, Representation*> m_representations;
    QMap<ModelItemPtr , State *> m_state;
  };
}

#endif // APPOSITIONSURFACERENDERER_H
