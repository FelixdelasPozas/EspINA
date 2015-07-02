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

#include <Support/SupportTypes.h>

#include "Support/ROIAccumulator.h"
#include <GUI/View/ViewState.h>
#include <GUI/View/Selection.h>
#include <GUI/Model/ModelAdapter.h>
#include <GUI/ColorEngines/MultiColorEngine.h>
#include <QUndoStack>

class QMainWindow;
class QUndoStack;

namespace ESPINA
{

class DockWidget;
  namespace Support
  {
    class Context
    {
    public:
      explicit Context(QMainWindow *mainWindow);

      Context(Context &context) = delete;

      ~Context();

      SchedulerSPtr              scheduler() const;
      ModelAdapterSPtr           model() const;
      ROIAccumulatorSPtr         roiProvider();
      GUI::View::ViewState      &viewState();
      MultiColorEngineSPtr       colorEngine() const;
      QUndoStack *               undoStack();
      RepresentationFactorySList &availableRepresentations();
      ModelFactorySPtr           factory() const;

      Timer &timer();
      GUI::View::RepresentationInvalidator &representationInvalidator();

      void addPanel(DockWidget *panel);

    private:
      using Invalidator = GUI::View::RepresentationInvalidator;
      using ViewState   = GUI::View::ViewState;

      Timer                m_timer;
      Invalidator          m_invalidator;
      ViewState            m_viewState;
      ModelAdapterSPtr     m_model;
      ROIAccumulatorSPtr   m_activeROI;
      SchedulerSPtr        m_scheduler;
      QUndoStack           m_undoStack;
      RepresentationFactorySList m_availableRepresentations;
      ModelFactorySPtr     m_factory;
      MultiColorEngineSPtr m_colorEngine;

      QMainWindow *m_mainWindow;
    };

    GUI::View::SelectionSPtr getSelection(Context &context);

    ChannelAdapterPtr getActiveChannel(Context &context);

    SegmentationAdapterList getSelectedSegmentations(Context &context);

    class WithContext
    {
    public:
      explicit WithContext(Context &context);

      Context &context();

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
