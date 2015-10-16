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
    explicit PipelineSources(GUI::View::ViewState &viewState);

    virtual ~PipelineSources();

    ViewItemAdapterList sources(const ItemAdapter::Type &type) const;

    bool isEmpty() const;

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
    void insert(ViewItemAdapterList sources);

    bool contains(ViewItemAdapterPtr source) const;

    void remove(ViewItemAdapterList sources);

    GUI::Representations::FrameCSPtr createFrame() const;

  protected slots:
    void onRepresentationsInvalidated(ViewItemAdapterList items, const GUI::Representations::FrameCSPtr frame);

    void onRepresentationColorsInvalidated(ViewItemAdapterList items, const GUI::Representations::FrameCSPtr frame);

  protected:
    ViewItemAdapterList m_stacks;
    ViewItemAdapterList m_segmentations;

  private:
    GUI::View::ViewState &m_viewState;
  };
}

#endif // ESPINA_PIPELINE_SOURCES_H
