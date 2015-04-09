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
#include <GUI/ColorEngines/ColorEngine.h>

class QUndoStack;

namespace ESPINA
{
  namespace Support
  {
    class Context
    {
    public:
      explicit Context();

      ~Context();

      SchedulerSPtr              scheduler() const;
      ModelAdapterSPtr           model() const;
      ROIAccumulatorSPtr         activeROI() const;
      GUI::View::ViewState      &viewState();
      SelectionSPtr              selection() const;
      ColorEngineSPtr            colorEngine() const;
      QUndoStack *               undoStack() const;
      RepresentationFactorySList &availableRepresentations();
      ModelFactorySPtr           factory() const;

      Timer &timer();
      GUI::View::RepresentationInvalidator &representationInvalidator();

      ChannelAdapterPtr ActiveChannel;//Move to selection

    private:
      Timer                m_timer;
      GUI::View::RepresentationInvalidator m_invalidathor;
      GUI::View::ViewState m_viewState;
      ModelAdapterSPtr     m_model;
      ROIAccumulatorSPtr   m_activeROI;
      SchedulerSPtr        m_scheduler;
      SelectionSPtr        m_selection;
      QUndoStack *         m_undoStack;
      RepresentationFactorySList m_availableRepresentations;
      ModelFactorySPtr   m_factory;
      ColorEngineSPtr    m_colorEngine; //TODO: Decide how to deal with ColorEngines (probably split ColorEngineMenu into ColorEngine and Menu)
    };
  }
}

#endif // ESPINA_SUPPORT_CONTEXT_H
