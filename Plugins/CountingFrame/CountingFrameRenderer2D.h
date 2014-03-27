/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef ESPINA_COUNTING_FRAME_2D_RENDERER_H
#define ESPINA_COUNTING_FRAME_2D_RENDERER_H

// Plugin
#include "CountingFramePlugin_Export.h"
#include "CountingFrames/CountingFrame.h"
#include "CountingFrameManager.h"
#include <GUI/Representations/Renderers/Renderer.h>

// Qt
#include <QList>

namespace EspINA
{
  class SliceWidget;

  namespace CF
  {
    class CountingFramePlugin_EXPORT CountingFrameRenderer2D
    : public Renderer
    {
      Q_OBJECT
      public:
        explicit CountingFrameRenderer2D(CountingFrameManager &cfManager);
        virtual ~CountingFrameRenderer2D();

        virtual const QIcon icon() const
        { return QIcon(":/apply.svg");}

        virtual const QString name() const
        { return tr("Counting Frame 2D");}

        virtual const QString tooltip() const
        { return tr("Stereological Counting Frame");}

        virtual RendererSPtr clone() const;

        virtual RendererTypes renderType() const
        { return RendererType::RENDERER_VIEW2D;}

        virtual int numberOfRenderedItems() const
        { return m_insertedCFs.size();}

        virtual unsigned int numberOfvtkActors() const;

        virtual void setView(RenderView *view);

      protected:
        virtual void hide();
        virtual void show();

      private slots:
        void onCountingFrameCreated(CountingFrame *cf);
        void onCountingFrameDeleted(CountingFrame *cf);

      private:
        CountingFrameManager &m_cfManager;
        QList<CountingFrame *> m_insertedCFs;
    };
  } // namespace CF
} // namespace EspINA

#endif // ESPINA_COUNTING_FRAME_2D_RENDERER_H
