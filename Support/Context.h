/*
 * Copyright 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef ESPINA_SUPPORT_CONTEXT_H
#define ESPINA_SUPPORT_CONTEXT_H

// ESPINA
#include <GUI/Types.h>
#include <Support/Types.h>
#include "Support/ROIAccumulator.h"
#include <GUI/View/ViewState.h>
#include <GUI/View/Selection.h>
#include <GUI/Model/ModelAdapter.h>

// Qt
#include <QUndoStack>

class QMainWindow;
class QUndoStack;

namespace ESPINA
{
  namespace Support
  {
    /** \class Context
     * \brief Class for grouping several connected classes that are usually passed as parameters.
     */
    class Context
    {
      public:
        /** \brief Context class constructor.
         * \param[in] mainWindow application main window.
         */
        explicit Context(QMainWindow *mainWindow, bool *minimizedStatus);

        Context(Context &context) = delete;

        ~Context();

        SchedulerSPtr              scheduler() const;
        ModelAdapterSPtr           model() const;
        ROIAccumulatorSPtr         roiProvider();
        GUI::View::ViewState      &viewState();
        GUI::ColorEngines::ColorEngineSPtr colorEngine() const;
        QUndoStack *               undoStack();
        RepresentationFactorySList &availableRepresentations();
        ModelFactorySPtr           factory() const;

        void addColorEngine(GUI::ColorEngines::ColorEngineSPtr engine);

        void addPanel(Panel *panel);

        bool isMinimized() const;

      private:
        using ViewState   = GUI::View::ViewState;

        ViewState            m_viewState;
        ModelAdapterSPtr     m_model;
        ROIAccumulatorSPtr   m_activeROI;
        SchedulerSPtr        m_scheduler;
        QUndoStack           m_undoStack;
        RepresentationFactorySList m_availableRepresentations;
        ModelFactorySPtr     m_factory;
        GUI::ColorEngines::MultiColorEngineSPtr m_colorEngine;

        bool *m_minimizedStatus;
        QMainWindow *m_mainWindow;
    };

    GUI::View::SelectionSPtr getSelection(Context &context);

    ChannelAdapterPtr getActiveChannel(Context &context);

    SegmentationAdapterList getSelectedSegmentations(Context &context);

    class WithContext
    {
    public:
      explicit WithContext(Context &context);

      Context &getContext() const;

      ChannelAdapterPtr getActiveChannel() const;

      GUI::View::SelectionSPtr getSelection() const;

      SegmentationAdapterList getSelectedSegmentations() const;

      GUI::View::ViewState &getViewState() const;

      ModelFactorySPtr getFactory() const;

      SchedulerSPtr getScheduler() const;

      ModelAdapterSPtr getModel() const;

      QUndoStack *getUndoStack() const;

    private:
      Context &m_context;
    };
  }
}

#endif // ESPINA_SUPPORT_CONTEXT_H
