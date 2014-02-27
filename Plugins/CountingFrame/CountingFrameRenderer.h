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
#ifndef ESPINA_COUNTING_FRAME_RENDERER_H
#define ESPINA_COUNTING_FRAME_RENDERER_H

#include "CountingFramePlugin_Export.h"
#include <GUI/Representations/Renderers/Renderer.h>

#include "CountingFrameManager.h"

#include <QMap>

class vtkAbstractWidget;

namespace EspINA
{
  namespace CF {

    class CountingFramePlugin_EXPORT CountingFrameRenderer
    : public Renderer
    {
      Q_OBJECT
    public:
      explicit CountingFrameRenderer(CountingFrameManager &cfManager);
      virtual ~CountingFrameRenderer();

      virtual const QIcon icon() const
      { return QIcon(":/apply.svg"); }

      virtual const QString name() const
      { return tr("Counting Frame");}

      virtual const QString tooltip() const
      { return tr("Stereological Counting Frame");}

      virtual RendererSPtr clone();

      virtual RendererTypes renderType()
      { return RendererType::RENDERER_VIEW3D; }

      virtual int numberOfRenderedItems()
      { return m_widgets.size(); }

      virtual unsigned int numberOfvtkActors();

      public slots:
        void visibilityChanged();

    protected:
      virtual void hide();
      virtual void show();

    private slots:
      void onCountingFrameCreated(CountingFrame *cf);
      void onCountingFrameDeleted(CountingFrame *cf);

    private:
      CountingFrameManager &m_cfManager;
      QMap<CountingFrame *, vtkAbstractWidget *> m_widgets;
    };
  } // namespace CF
} // namespace EspINA

#endif // ESPINA_COUNTING_FRAME_RENDERER_H
