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

#include "PipelineSources.h"

#include <GUI/Model/ModelAdapter.h>

namespace ESPINA
{

  /** \class PipelineSourcesFilter contains all view items used as source
   *         of pipeline representations
   *
   */
  class PipelineSourcesFilter
  : public PipelineSources
  {
    Q_OBJECT

  public:
    explicit PipelineSourcesFilter(ModelAdapterSPtr model, const ItemAdapter::Type type, GUI::View::ViewState &viewState);

    virtual ~PipelineSourcesFilter();

    void setSelectedSources(ViewItemAdapterSList sources);

  private slots:
    void onSourcesAdded  (ViewItemAdapterSList sources);
    void onSourcesRemoved(ViewItemAdapterSList sources);
    void onReset();

  private:
    ViewItemAdapterList filter(ViewItemAdapterSList sources);

    inline bool acceptedType(ViewItemAdapterSPtr source) const;

    inline bool acceptedSource(ViewItemAdapterSPtr source) const;

  private:
    ModelAdapterSPtr    m_model;
    ItemAdapter::Type   m_type;

    ViewItemAdapterList m_selectedSources;
  };
}

#endif // ESPINA_PIPELINE_SOURCES_FILTER_H
