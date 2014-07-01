/*
 *    
 *    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

#ifndef ESPINA_COUNTING_FRAME_3D_RENDERER_H
#define ESPINA_COUNTING_FRAME_3D_RENDERER_H

// Plugin
#include "CountingFramePlugin_Export.h"
#include <GUI/Representations/Renderers/Renderer.h>
#include "CountingFrameManager.h"

// Qt
#include <QMap>

class vtkCountingFrame3DWidget;

namespace EspINA
{
  namespace CF
  {

    class CountingFramePlugin_EXPORT CountingFrameRenderer3D
    : public Renderer
    {
      Q_OBJECT
    public:
      explicit CountingFrameRenderer3D(CountingFrameManager &cfManager);
      virtual ~CountingFrameRenderer3D();

      virtual const QIcon icon() const
      { return QIcon(":/apply.svg"); }

      virtual const QString name() const
      { return tr("Counting Frame 3D");}

      virtual const QString tooltip() const
      { return tr("Stereological Counting Frame");}

      virtual RendererSPtr clone() const;

      virtual RendererTypes renderType() const
      { return RendererType::RENDERER_VIEW3D; }

      virtual int numberOfRenderedItems() const
      { return m_widgets.size(); }

      virtual unsigned int numberOfvtkActors() const;

      virtual void setView(RenderView *view);

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
      QMap<CountingFrame *, vtkCountingFrame3DWidget *> m_widgets;
    };
  } // namespace CF
} // namespace EspINA

#endif // ESPINA_COUNTING_FRAME_3D_RENDERER_H
