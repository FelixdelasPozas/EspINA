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

#ifndef ESPINA_COUNTING_FRAME_PLUGIN_H
#define ESPINA_COUNTING_FRAME_PLUGIN_H

#include "CountingFramePlugin_Export.h"

#include <Support/Plugin.h>

#include "CountingFrameManager.h"
#include "ColorEngines/ColorEngine.h"
#include <GUI/Model/ModelAdapter.h>

namespace ESPINA
{

class Panel;
  namespace CF
  {
    class CountingFramePlugin_EXPORT CountingFramePlugin
    : public Support::Plugin
    {
      Q_OBJECT
      Q_INTERFACES(ESPINA::Support::Plugin)

    public:
      /** \brief CountingFramePlugin class constructor.
       *
       */
      explicit CountingFramePlugin();

      /** \brief CountingFramePlugin class virtual destructor.
       *
       */
      virtual ~CountingFramePlugin();

      virtual void init(Support::Context &context);

      virtual ChannelExtensionFactorySList channelExtensionFactories() const;

      virtual SegmentationExtensionFactorySList segmentationExtensionFactories() const;

      virtual Support::ColorEngineSwitchSList colorEngines() const;

      virtual RepresentationFactorySList representationFactories() const;

      virtual QList<Support::CategorizedTool> tools() const;

    public slots:
      virtual void onAnalysisClosed();

    private:
      SchedulerSPtr                         m_scheduler;                      /** application task scheduler.                    */
      QUndoStack                           *m_undoStack;                      /** application undo stack.                        */
      Support::Context                     *m_context;                        /** application context.                           */

      std::shared_ptr<CountingFrameManager> m_manager;                        /** counting frame manager object.                 */
      Panel                                *m_dockWidget;                     /** counting frame panel.                          */
      CountingFrameColorEngineSPtr          m_colorEngine;                    /** counting frame color engine.                   */
      RepresentationFactorySPtr             m_representationFactory;          /** counting frame representation factory.         */
      ChannelExtensionFactorySPtr           m_channelExtensionFactory;        /** counting frame channel extension factory.      */
      SegmentationExtensionFactorySPtr      m_segmentationExtensionFactory;   /** counting frame segmentation extension factory. */
    };
  }
} // namespace ESPINA

#endif // ESPINA_COUNTING_FRAME_PLUGIN_H
