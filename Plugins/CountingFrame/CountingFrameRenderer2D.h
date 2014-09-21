/*

 Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

 This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

class vtkCountingFrameSliceWidget;

namespace ESPINA
{
  namespace CF
  {
    class CountingFramePlugin_EXPORT CountingFrameRenderer2D
    : public Renderer
    {
      Q_OBJECT
      public:
      	/** brief CountingFrameRenderer2D class constructor.
      	 * \param[in] cfManager, CountingFrameManager reference.
      	 *
      	 */
        explicit CountingFrameRenderer2D(CountingFrameManager &cfManager);

        /** brief CountingFrameRenderer2D class virtual destructor.
         *
         */
        virtual ~CountingFrameRenderer2D();

        /** brief Implements Renderer::icon() const.
         *
         */
        virtual const QIcon icon() const
        { return QIcon(":/apply.svg");}

        /** brief Implements Renderer::name() const.
         *
         */
        virtual const QString name() const
        { return tr("Counting Frame 2D");}

        /** brief Implements Renderer::tooltip() const.
         *
         */
        virtual const QString tooltip() const
        { return tr("Stereological Counting Frame");}

        /** brief Implements Renderer::clone() const.
         *
         */
        virtual RendererSPtr clone() const;

        /** brief Implements Renderer::renderType() const.
         *
         */
        virtual RendererTypes renderType() const
        { return RendererType::RENDERER_VIEW2D;}

        /** brief Implements Renderer::numberOfRenderedItems() const.
         *
         */
        virtual int numberOfRenderedItems() const
        { return m_insertedCFs.size();}

        /** brief Implements Renderer::numberOfvtkActors() const.
         *
         */
        virtual unsigned int numberOfvtkActors() const;

        /** brief Overrides Renderer::setView().
         *
         */
        virtual void setView(RenderView *view) override;

        /** brief Implements Renderer::type().
         *
         */
        virtual Type type() const
        { return Type::Other; }

      protected:
        /** brief Implements Renderer::hode().
         *
         */
        virtual void hide();

        /** brief Implements Renderer::show().
         *
         */
        virtual void show();

      private slots:
				/** brief Helper method to update internal data when a CF is created.
				 *
				 */
        void onCountingFrameCreated(CountingFrame *cf);

  			/** brief Helper method to update internal data when a CF is removed.
  			 *
  			 */
        void onCountingFrameDeleted(CountingFrame *cf);

      private:
        CountingFrameManager &m_cfManager;
        QMap<CountingFrame *, vtkCountingFrameSliceWidget *> m_insertedCFs;
    };
  } // namespace CF
} // namespace ESPINA

#endif // ESPINA_COUNTING_FRAME_2D_RENDERER_H
