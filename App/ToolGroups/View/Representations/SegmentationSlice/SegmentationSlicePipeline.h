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

#ifndef ESPINA_SEGMENTATION_SLICE_PIPELINE_H
#define ESPINA_SEGMENTATION_SLICE_PIPELINE_H

#include <Core/Analysis/Data/VolumetricData.hxx>
#include <Core/Analysis/Data/VolumetricDataUtils.hxx>
#include <GUI/Representations/SegmentationPipeline.h>
#include "App/ToolGroups/View/Representations/RepresentationSettings.h"

#include <QColor>

// VTK
#include <vtkSmartPointer.h>
#include <vtkImageReslice.h>
#include <vtkImageMapToColors.h>
#include <vtkImageActor.h>
#include <vtkImageMapper3D.h>
#include <vtkImageData.h>

namespace ESPINA
{
  using namespace Representations;

  template<Plane T>
  class SegmentationSlicePipeline
  : public RepresentationPipeline
  {
  public:
    explicit SegmentationSlicePipeline(ViewItemAdapterPtr item);

    virtual bool pick(const NmVector3 &point, vtkProp *actor);

    virtual void update();

    virtual QList<Actor> getActors();

    void applySettings(const Settings &settings);

  private:
    void initPipeline();

    void updateColor();

  private:
    static Plane s_plane;

    SegmentationAdapterPtr m_segmentation;

    int m_planeIndex;

    vtkSmartPointer<vtkImageMapToColors> m_mapToColors;
    vtkSmartPointer<vtkImageActor>       m_actor;

    QList<Actor> m_actors;

    //static TransparencySelectionHighlighter *s_highlighter;
  };

#include "SegmentationSlicePipeline.cpp"
}

#endif // ESPINA_SEGMENTATION_SLICE_PIPELINE_H
