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

#ifndef ESPINA_MANUAL_PIPELINE_SOURCES_H
#define ESPINA_MANUAL_PIPELINE_SOURCES_H
#include "PipelineSources.h"

namespace ESPINA
{
  class ManualPipelineSources
  : public PipelineSources
  {
  public:
    explicit ManualPipelineSources(GUI::View::RepresentationInvalidator &invalidator);

    void addSource(ViewItemAdapterList sources, TimeStamp t);

    void removeSource(ViewItemAdapterList sources, TimeStamp t);

    void updateRepresentation(ViewItemAdapterList sources, TimeStamp t);
  };
}

#endif // ESPINA_MANUALPIPELINESOURCES_H
