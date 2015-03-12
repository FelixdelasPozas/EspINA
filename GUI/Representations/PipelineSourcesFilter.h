/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ESPINA_PIPELINE_SOURCES_FILTER_H
#define ESPINA_PIPELINE_SOURCES_FILTER_H

#include <GUI/Model/ViewItemAdapter.h>
#include <GUI/Model/ModelAdapter.h>

namespace ESPINA
{

  /** \class PipelineSourcesFilter contains all view items used as source
   *         of pipeline representations
   *
   */
  class PipelineSourcesFilter
  : public QObject
  {
    Q_OBJECT

  public:
    explicit PipelineSourcesFilter(ModelAdapterSPtr model, const ItemAdapter::Type type);

    virtual ~PipelineSourcesFilter();

    ViewItemAdapterList sources() const;

    bool isEmpty() const
    { return m_sources.isEmpty(); }

    int size() const
    { return m_sources.size(); }

  signals:
    void sourcesAdded  (ViewItemAdapterList sources, TimeStamp t);
    void sourcesRemoved(ViewItemAdapterList sources, TimeStamp t);
    void representationsModified(ViewItemAdapterList sources, TimeStamp t);
    void updateTimeStamp(TimeStamp t);

  private slots:
    void onSourcesAdded  (ViewItemAdapterSList sources,  TimeStamp t);
    void onSourcesRemoved(ViewItemAdapterSList sources,  TimeStamp t);
    void onRepresentationModified(ViewItemAdapterSList sources, TimeStamp t);
    void onReset(TimeStamp t);

  private:
    inline void insert(ViewItemAdapterList sources);

    inline bool contains(ViewItemAdapterPtr source) const;

    inline void remove(ViewItemAdapterList sources);

    ViewItemAdapterList filter(ViewItemAdapterSList sources);

    inline bool acceptedType(ViewItemAdapterSPtr source) const;

    inline bool acceptedSource(ViewItemAdapterSPtr source) const;

  private:
    ModelAdapterSPtr    m_model;
    ItemAdapter::Type   m_type;

    ViewItemAdapterList m_sources;
    ViewItemAdapterList m_selectedSources;
  };
}

#endif // ESPINA_PIPELINE_SOURCES_FILTER_H
