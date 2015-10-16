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
  class ModelSources
  : public PipelineSources
  {
    Q_OBJECT

  public:
    explicit ModelSources(ModelAdapterSPtr model, GUI::View::ViewState &viewState);

    virtual ~ModelSources();

  private slots:
    void onSourcesAdded  (ViewItemAdapterSList sources);
    void onSourcesRemoved(ViewItemAdapterSList sources);
    void onReset();

  private:
    ModelAdapterSPtr m_model;
  };
}

#endif // ESPINA_PIPELINE_SOURCES_FILTER_H
