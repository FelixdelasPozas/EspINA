/*

 Copyright (C) 2017 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef GUI_REPRESENTATIONS_PIPELINES_SEGMENTATIONSKELETONPIPELINEBASE_H_
#define GUI_REPRESENTATIONS_PIPELINES_SEGMENTATIONSKELETONPIPELINEBASE_H_

#include <GUI/EspinaGUI_Export.h>

// ESPINA
#include <Core/Utils/Vector3.hxx>
#include <GUI/ColorEngines/IntensitySelectionHighlighter.h>
#include <GUI/Representations/RepresentationPipeline.h>
#include <GUI/Representations/RepresentationState.h>
#include <GUI/Types.h>

// VTK
#include <vtkSmartPointer.h>

class vtkActor;

namespace ESPINA
{
  namespace GUI
  {
    namespace Representations
    {
      /** \class SegmentationSkeletonPipelineBase
       * \brief Base class for skeleton representations.
       *
       */
      class EspinaGUI_EXPORT SegmentationSkeletonPipelineBase
      : public RepresentationPipeline
      {
        public:
          /** \brief SegmentationSkeletonPipelineBase class constructor.
           * \param[in] id Representation id.
           * \param[in] colorEngine representation color engine.
           *
           */
          explicit SegmentationSkeletonPipelineBase(const QString &id, GUI::ColorEngines::ColorEngineSPtr colorEngine);

          /** \brief SegmentationSkeletonPipelineBase class virtual destructor.
           *
           */
          virtual ~SegmentationSkeletonPipelineBase()
          {};

          virtual RepresentationPipeline::ActorList createActors(ConstViewItemAdapterPtr    item,
                                                                 const RepresentationState &state) = 0;

          virtual  void updateColors(RepresentationPipeline::ActorList &actors,
                                     ConstViewItemAdapterPtr            item,
                                     const RepresentationState         &state) override;

          virtual bool pick(ConstViewItemAdapterPtr item, const NmVector3 &point) const override;

          virtual RepresentationState representationState(ConstViewItemAdapterPtr    item,
                                                          const RepresentationState &settings) override;

          /** \brief Helper to make stippled line actors in the new OpenGL 2 backend as the GetProperty() old methods doesn't work.
           * \param[in] actor Pointer of actor to modify.
           * \param[in] pattern Line pattern.
           * \param[in] repeat Repeat specifies how many times each bit in the pattern is to be repeated.
           *
           */
          static void stippledLine(vtkSmartPointer<vtkActor> &actor, int pattern = 0xF0F0, int repeat = 2);
  
        protected:
          GUI::ColorEngines::ColorEngineSPtr m_colorEngine; /** color engine for skeleton representations. */

          static GUI::ColorEngines::IntensitySelectionHighlighter s_highlighter; /** highlighter for skeleton representations. */
      };

    } // namespace Representations
  } // namespace Support
} // namespace ESPINA

#endif // GUI_REPRESENTATIONS_PIPELINES_SEGMENTATIONSKELETONPIPELINEBASE_H_
