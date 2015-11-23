/*
 * Copyright 2015 <copyright holder> <email>
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

#ifndef ESPINA_PIPELINE_SOURCES_H
#define ESPINA_PIPELINE_SOURCES_H

#include <GUI/Model/ViewItemAdapter.h>

#include <GUI/Types.h>

namespace ESPINA
{
  class PipelineSources
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
