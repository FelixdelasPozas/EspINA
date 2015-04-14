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
#include <GUI/View/RepresentationInvalidator.h>

namespace ESPINA
{
  class PipelineSources
  : public QObject
  {
    Q_OBJECT

  public:
    explicit PipelineSources(GUI::View::RepresentationInvalidator &invalidator);

    virtual ~PipelineSources();

    ViewItemAdapterList sources() const;

    bool isEmpty() const
    { return m_sources.isEmpty(); }

    int size() const
    { return m_sources.size(); }

    GUI::View::RepresentationInvalidator &invalidator();

  signals:
    void sourcesAdded  (ViewItemAdapterList sources, TimeStamp t);
    void sourcesRemoved(ViewItemAdapterList sources, TimeStamp t);
    void representationsModified(ViewItemAdapterList sources, TimeStamp t);
    void updateTimeStamp(TimeStamp t);

  protected:
    void insert(ViewItemAdapterList sources);

    bool contains(ViewItemAdapterPtr source) const;

    void remove(ViewItemAdapterList sources);

    TimeStamp timeStamp() const;

  private slots:
    void onRepresentationInvalidated(ViewItemAdapterSList items, TimeStamp t);

  protected:
    ViewItemAdapterList m_sources;

  private:
    GUI::View::RepresentationInvalidator &m_representationInvalidator;
  };
}

#endif // ESPINA_PIPELINE_SOURCES_H
