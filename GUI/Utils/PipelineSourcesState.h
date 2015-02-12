/*
 Copyright (C) 2015  Felix de las Pozas Alvarez

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

#ifndef ESPINA_PIPELINE_SOURCES_STATE_H_
#define ESPINA_PIPELINE_SOURCES_STATE_H_

// ESPINA
#include <GUI/Model/ViewItemAdapter.h>
#include <GUI/Utils/Timer.h>

// Qt
#include <QObject>
#include <QList>

namespace ESPINA
{
  class PipelineSources;

  class PipelineSourcesState
  : public QObject
  {
    Q_OBJECT

    public:
      /** \brief PipelineSourcesState class constructor.
       * \param[in] timer state timer object.
       *
       */
      explicit PipelineSourcesState(TimerSPtr timer);

      /** \brief PipelineSourcesState class virtual destructor.
       *
       */
      virtual ~PipelineSourcesState()
      {};

      /** \brief Adds a source to the state.
       * \param[in] source PipelineSources object to add to the state.
       *
       */
      void addSource(PipelineSources *source);

      /** \brief Removes a source from the state.
       * \param[in] source PipelineSources object to remove from the state.
       *
       */
      void removeSource(PipelineSources *source);

    signals:
      void sourceAdded(ViewItemAdapterPtr item, PipelineSources* source, TimeStamp time);
      void sourceUpdated(ViewItemAdapterPtr item, PipelineSources* source, TimeStamp time);
      void sourceRemoved(ViewItemAdapterPtr item, PipelineSources* source, TimeStamp time);

    public slots:
      void onSourceAdded(ViewItemAdapterPtr item) const;
      void onSourceUpdated(ViewItemAdapterPtr item) const;
      void onSourceRemoved(ViewItemAdapterPtr item) const;

    private:
      TimerSPtr                m_timer;
      QList<PipelineSources *> m_sources;
  };

  using PipelineSourcesStatePtr  = PipelineSourcesState *;
  using PipelineSourcesStateSPtr = std::shared_ptr<PipelineSourcesState>;

} /* namespace ESPINA */

#endif // ESPINA_PIPELINE_SOURCES_STATE_H_
