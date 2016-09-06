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

#include <GUI/EspinaGUI_Export.h>

// ESPINA
#include "PipelineSources.h"
#include <GUI/Model/ModelAdapter.h>

namespace ESPINA
{

  /** \class ModelSources
   * \brief Contains all view items used as source of pipeline representations.
   *
   */
  class EspinaGUI_EXPORT ModelSources
  : public PipelineSources
  {
      Q_OBJECT
    public:
      /** \brief ModelSources class constructor.
       * \param[in] model model adapter.
       * \param[in] viewState application view state.
       *
       */
      explicit ModelSources(ModelAdapterSPtr model, GUI::View::ViewState &viewState);

      /** \brief ModelSources class virtual destructor.
       *
       */
      virtual ~ModelSources();

    private slots:
      /** \brief Inserts the sources into the list when added to the model.
       * \param[in] sources view item list.
       *
       */
      void onSourcesAdded  (ViewItemAdapterSList sources);

      /** \brief Removes the sources from the list when removed from the model.
       * \param[in] sources view item list.
       *
       */
      void onSourcesRemoved(ViewItemAdapterSList sources);

      /** \brief Clears the list of sources when the model is resetted.
       *
       */
      void onReset();

    private:
      ModelAdapterSPtr m_model; /** model adapter to monitor. */
  };
}

#endif // ESPINA_PIPELINE_SOURCES_FILTER_H
