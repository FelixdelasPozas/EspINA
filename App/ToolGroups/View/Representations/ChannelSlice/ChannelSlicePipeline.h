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

#ifndef ESPINA_CHANNEL_SLICE_PIPELINE_H
#define ESPINA_CHANNEL_SLICE_PIPELINE_H

#include <GUI/Model/ViewItemAdapter.h>
#include <GUI/Representations/ChannelPipeline.h>
#include <Core/Analysis/Data/VolumetricData.hxx>
#include <Core/Analysis/Data/VolumetricDataUtils.hxx>
#include <Core/Analysis/Output.h>

// VTK
#include <vtkSmartPointer.h>
#include <vtkImageReslice.h>
#include <vtkImageMapToColors.h>
#include <vtkImageShiftScale.h>
#include <vtkImageActor.h>
#include <vtkImageMapper3D.h>
#include <vtkImageData.h>
#include <vtkLookupTable.h>

#include <chrono>

namespace ESPINA
{
  template<Plane T>
  class ChannelSlicePipeline
  : public RepresentationPipeline
  {
  public:
    explicit ChannelSlicePipeline();

    virtual RepresentationState representationState(const ViewItemAdapter     *item,
                                                    const RepresentationState &settings) override;


    virtual RepresentationPipeline::ActorList createActors(const ViewItemAdapter     *item,
                                                           const RepresentationState &state) override;

    virtual bool pick( ViewItemAdapter *item, const NmVector3 &point) const;

  private:
    static Plane s_plane;
  };

#include "ChannelSlicePipeline.cpp"
}

#endif // ESPINA_CHANNEL_SLICE_PIPELINE_H
