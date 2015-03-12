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

#ifndef ESPINA_PIPELINE_SOURCES_H
#define ESPINA_PIPELINE_SOURCES_H

#include <GUI/Model/ViewItemAdapter.h>

namespace ESPINA
{

  /** \class PipelineSources contains all view items used as source
   *         of pipeline representations
   *
   */
  class PipelineSources
  : public QObject
  {
    Q_OBJECT

  public:
    virtual ~PipelineSources();

    /** \brief Adds source view item to the pipeline sources
     * \param[in] source to be added
     */
    void addSource(ViewItemAdapterPtr source);

    /** \brief Notifies the source has been modified
     * \param[in] source to be updated
     */
    void onSourceUpdated(ViewItemAdapterPtr source);

    /** \brief Removes source view item from the pipeline sources
     * \param[in] source to be removed
     */
    void removeSource(ViewItemAdapterPtr source);

    /** \brief Removes all source view items from the pipeline sources
     *
     */
    void clear();

    ViewItemAdapterList sources() const;

    bool isEmpty() const
    { return m_sources.isEmpty(); }

  signals:
    void sourcesAdded  (ViewItemAdapterList);
    void sourcesUpdated(ViewItemAdapterList);
    void sourcesRemoved(ViewItemAdapterList);

  private:
    void insert(ViewItemAdapterPtr source);

    bool contains(ViewItemAdapterPtr source) const
    { return m_sources.contains(source); }

    void remove(ViewItemAdapterPtr source);

    ViewItemAdapterList createList(ViewItemAdapterPtr item) const;

  private:
    ViewItemAdapterList m_sources;
  };
}

#endif // ESPINA_PIPELINE_SOURCES_H
