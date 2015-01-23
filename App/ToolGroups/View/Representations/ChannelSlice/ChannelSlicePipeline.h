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

#ifndef ESPINA_CHANNELSLICEPIPELINE_H
#define ESPINA_CHANNELSLICEPIPELINE_H

#include <GUI/Model/ViewItemAdapter.h>
#include <GUI/Representations/ChannelRepresentationSettingsEditor.h>
#include <Core/Analysis/Data/VolumetricData.hxx>
#include <Core/Analysis/Data/VolumetricDataUtils.hxx>
#include "App/ToolGroups/View/Representations/RepresentationSettings.h"

// VTK
#include <vtkSmartPointer.h>
#include <vtkImageReslice.h>
#include <vtkImageMapToColors.h>
#include <vtkImageShiftScale.h>
#include <vtkImageActor.h>
#include <vtkImageMapper3D.h>
#include <vtkImageData.h>
#include <vtkLookupTable.h>

namespace ESPINA
{
  using namespace Representations;

  template<Plane T>
  class ChannelSlicePipeline
  : public RepresentationPipeline
  {
  public:
    explicit ChannelSlicePipeline(ViewItemAdapterPtr item);

    virtual bool pick(const NmVector3 &point, vtkProp *actor);

    virtual void update();

    virtual QList<Actor> getActors();

    void applySettings(const Settings &settings);

  private:
    void initPipeline();

    void updateBrightness();

    void updateContrast();

    void updateStain();

  private:
    static Plane s_plane;

    ChannelRepresentationSettingsEditor m_channelSettings;

    int m_planeIndex;

    vtkSmartPointer<vtkImageMapToColors> m_mapToColors;
    vtkSmartPointer<vtkImageShiftScale>  m_shiftScaleFilter;
    vtkSmartPointer<vtkImageActor>       m_actor;
    vtkSmartPointer<vtkLookupTable>      m_lut;

    QList<Actor> m_actors;
  };

#include "ChannelSlicePipeline.cpp"
}

#endif // ESPINA_CHANNELSLICEPIPELINE_H
