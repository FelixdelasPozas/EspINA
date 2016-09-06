/*

 Copyright (C) 2015 Jorge Peña Pastor <jpena@cesvima.upm.es>

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

#ifndef ESPINA_PIPELINE_SOURCES_H
#define ESPINA_PIPELINE_SOURCES_H

#include <GUI/EspinaGUI_Export.h>

// ESPINA
#include <GUI/Model/ViewItemAdapter.h>
#include <GUI/Types.h>

namespace ESPINA
{
  class EspinaGUI_EXPORT PipelineSources
  : public QObject
  {
    Q_OBJECT

  public:
    /** \brief PipelineSources class constructor.
     * \param[in] viewState view state.
     *
     */
    explicit PipelineSources(GUI::View::ViewState &viewState);

    /** \brief PipelineSources class virtual destructor.
     *
     */
    virtual ~PipelineSources();

    /** \brief Returs the sources of the type managed by the class.
     * \param[in] type type of the sources to return.
     *
     */
    ViewItemAdapterList sources(const ItemAdapter::Type &type) const;

    /** \brief Returns true if the sources are empty.
     *
     */
    bool isEmpty() const;

    /** \brief Returns the number of managed sources.
     *
     */
    int size() const;

  signals:
    void stacksAdded  (ViewItemAdapterList sources, const GUI::Representations::FrameCSPtr frame);
    void stacksRemoved(ViewItemAdapterList sources, const GUI::Representations::FrameCSPtr frame);
    void stacksInvalidated(ViewItemAdapterList sources, const GUI::Representations::FrameCSPtr frame);
    void stackColorsInvalidated(ViewItemAdapterList sources, const GUI::Representations::FrameCSPtr frame);

    void segmentationsAdded  (ViewItemAdapterList sources, const GUI::Representations::FrameCSPtr frame);
    void segmentationsRemoved(ViewItemAdapterList sources, const GUI::Representations::FrameCSPtr frame);
    void segmentationsInvalidated(ViewItemAdapterList sources, const GUI::Representations::FrameCSPtr frame);
    void segmentationColorsInvalidated(ViewItemAdapterList sources, const GUI::Representations::FrameCSPtr frame);

  protected:
    /** \brief Inserts a list of items.
     * \param[in] sources items to insert.
     *
     */
    void insert(ViewItemAdapterList sources);

    /** \brief Returns true if the class contains the item.
     * \param[in] source item.
     *
     */
    bool contains(ViewItemAdapterPtr source) const;

    /** \brief Removes a list of items.
     * \param[in] sources items to remove.
     *
     */
    void remove(ViewItemAdapterList sources);

  protected slots:
    /** \brief Emits the invalidation signal for the source items contained in this class.
     * \param[in] items invalidated items.
     * \param[in] frame invalidation frame.
     *
     */
    void onRepresentationsInvalidated(ViewItemAdapterList items, const GUI::Representations::FrameCSPtr frame);

    /** \brief Emits the color invalidation signal for the source items contained in this class.
     * \param[in] items invalidated items.
     * \param[in] frame invalidation frame.
     *
     */
    void onRepresentationColorsInvalidated(ViewItemAdapterList items, const GUI::Representations::FrameCSPtr frame);

  protected:
    ViewItemAdapterList m_stacks;        /** stack items. */
    ViewItemAdapterList m_segmentations; /** segmentation items. */

  private:
    GUI::View::ViewState &m_viewState;   /** view state to obtain the frames. */
  };
}

#endif // ESPINA_PIPELINE_SOURCES_H
