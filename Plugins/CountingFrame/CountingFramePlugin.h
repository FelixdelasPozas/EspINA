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

// ESPINA
#include <Support/Plugin.h>
#include <GUI/Model/ModelAdapter.h>

// Plugin
#include "CountingFrameManager.h"
#include "ColorEngines/ColorEngine.h"

namespace ESPINA
{
  class Panel;

  namespace CF
  {
    class CountingFramePlugin_EXPORT CountingFramePlugin
    : public Support::AppPlugin
    {
        Q_INTERFACES(ESPINA::Core::CorePlugin ESPINA::Support::AppPlugin)
        Q_OBJECT
        Q_PLUGIN_METADATA(IID "es.upm.cesvima.ESPINA.Core.Plugin/1.0" FILE "plugin.json")
        Q_PLUGIN_METADATA(IID "es.upm.cesvima.ESPINA.Plugin/2.0" FILE "plugin.json")

      public:
        /** \brief CountingFramePlugin class constructor.
         *
         */
        explicit CountingFramePlugin();

        /** \brief CountingFramePlugin class virtual destructor.
         *
         */
        virtual ~CountingFramePlugin()
        {};

        virtual const QString name() const
        { return tr("Counting Frame Plugin"); }

        virtual const QString description() const
        { return tr("Stereological inclusion methods to count segmentations per volume."); }

        virtual const QString organization() const
        { return tr("Universidad Politécnica de Madrid."); }

        virtual const QString maintainer() const
        { return tr("felix.delaspozas@ctb.upm.es"); }

        virtual void init(Support::Context &context);

        virtual Core::StackExtensionFactorySList channelExtensionFactories() const;

        virtual Core::SegmentationExtensionFactorySList segmentationExtensionFactories() const;

        virtual Support::ColorEngineSwitchSList colorEngines() const;

        virtual RepresentationFactorySList representationFactories() const;

        virtual QList<Support::CategorizedTool> tools() const;

      public slots:
        virtual void onAnalysisClosed();

      private:
        Support::Context                      *m_context;                        /** application context.                           */
        SchedulerSPtr                          m_scheduler;                      /** application task scheduler.                    */
        QUndoStack                            *m_undoStack;                      /** application undo stack.                        */
        std::shared_ptr<CountingFrameManager>  m_manager;                        /** counting frame manager object.                 */
        Panel                                 *m_dockWidget;                     /** counting frame panel.                          */
        CountingFrameColorEngineSPtr           m_colorEngine;                    /** counting frame color engine.                   */
        RepresentationFactorySPtr              m_representationFactory;          /** counting frame representation factory.         */
        Core::SegmentationExtensionFactorySPtr m_segmentationExtensionFactory;   /** counting frame segmentation extension factory. */
        Core::StackExtensionFactorySPtr        m_channelExtensionFactory;        /** counting frame channel extension factory.      */
    };
  }
} // namespace ESPINA

#endif // ESPINA_COUNTING_FRAME_PLUGIN_H
