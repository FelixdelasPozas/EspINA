/*

 Copyright (C) 2014 Jorge Peña Pastor <jpena@cesvima.upm.es>

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

#ifndef ESPINA_MANUAL_PIPELINE_SOURCES_H
#define ESPINA_MANUAL_PIPELINE_SOURCES_H

#include <GUI/EspinaGUI_Export.h>

// ESPINA
#include <GUI/Representations/PipelineSources.h>

namespace ESPINA
{
  /** \class ManualPipelineSources
   * \brief PipelineSources subclass that allows manually inserting or deleting sources.
   *
   */
  class EspinaGUI_EXPORT ManualPipelineSources
  : public PipelineSources
  {
    public:
      /** \brief ManualPipelineSources class constructor.
       * \param[in] viewState application view state.
       *
       */
      explicit ManualPipelineSources(GUI::View::ViewState &viewState);

      /** \brief ManualPipelineSources class virtual destructor.
       *
       */
      virtual ~ManualPipelineSources()
      {};

      /** \brief Adds source items to the group.
       * \param[in] sources view items list.
       * \parma[in] unused.
       *
       */
      void addSource(ViewItemAdapterList sources, const GUI::Representations::FrameCSPtr unused);

      /** \brief Removes source items from the group.
       * \param[in] sources view items list.
       * \parma[in] unused.
       *
       */
      void removeSource(ViewItemAdapterList sources, const GUI::Representations::FrameCSPtr unused);

      /** \brief Signals the need to update the representations of the given list of items.
       * \param[in] sources view items list.
       * \param[in] frame update representations frame object.
       *
       */
      void updateRepresentation(ViewItemAdapterList sources, const GUI::Representations::FrameCSPtr frame);
  };
}

#endif // ESPINA_MANUALPIPELINESOURCES_H
