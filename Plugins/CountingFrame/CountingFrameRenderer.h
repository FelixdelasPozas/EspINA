/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>
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


#ifndef FRAMERENDERER_H
#define FRAMERENDERER_H

#include <QMap>
#include <GUI/Renderers/Renderer.h>

class vtkAbstractWidget;

namespace EspINA
{

  class CountingFrame;
  class CountingFramePanel;

  class CountingFrameRenderer
  : public IRenderer
  {
    Q_OBJECT
  public:
    explicit CountingFrameRenderer(CountingFramePanel *plugin);
    virtual ~CountingFrameRenderer();

    virtual const QIcon icon() const
    { return QIcon(":/apply.svg"); }
    virtual const QString name() const
    { return tr("Counting Frame");}
    virtual const QString tooltip() const
    { return tr("Stereological Counting Frame");}

    virtual void hide();
    virtual void show();
    virtual unsigned int getNumberOfvtkActors();

    virtual bool addItem   (ModelItemPtr item){return false;}
    virtual bool updateItem(ModelItemPtr item){return false;}
    virtual bool removeItem(ModelItemPtr item){return false;}

    virtual void clean();
    virtual IRendererSPtr clone();

  public slots:
    void countingFrameCreated(CountingFrame *cf);
    void countingFrameDeleted(CountingFrame *cf);

  private:
    CountingFramePanel *m_plugin;
    QMap<CountingFrame *, vtkAbstractWidget *> m_widgets;
  };

} // namespace EspINA

#endif // FRAMERENDERER_H
