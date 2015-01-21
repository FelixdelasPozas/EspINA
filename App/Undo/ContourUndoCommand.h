/*
 
 Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef ESPINA_CONTOUR_UNDOCOMMAND_H_
#define ESPINA_CONTOUR_UNDOCOMMAND_H_

// ESPINA
#include <Core/EspinaTypes.h>
#include <Core/Utils/Bounds.h>
#include <GUI/Model/SegmentationAdapter.h>
#include <GUI/Model/ModelAdapter.h>
#include <GUI/View/Widgets/Contour/ContourWidget.h>
#include <App/ToolGroups/Edition/ManualEditionTool.h>

// Qt
#include <QUndoStack>

class vtkPolyData;

namespace ESPINA
{
  class ViewManager;
  
  class ContourUndoCommand
  : public QUndoCommand
  {
    public:
      ContourUndoCommand(SegmentationAdapterPtr seg,
                         ViewManagerSPtr        vm,
                         ManualEditionToolSPtr  tool);

      virtual ~ContourUndoCommand();

      virtual void redo();
      virtual void undo();

      /** \brief Helper method to create the rasterized volume and assign values to internal variables.
       *  \param[in] contour contour data to be rasterized.
       */
      void rasterizeContour(ContourWidget::ContourData) const;

    private:
      SegmentationAdapterPtr         m_segmentation;
      ViewManagerSPtr                m_viewManager;
      ManualEditionToolSPtr          m_tool;
      bool                           m_createData;

      mutable BinaryMaskSPtr<unsigned char> m_volume;
      mutable bool                     m_rasterized;
      mutable itkVolumeType::PixelType m_value;

      mutable ContourWidget::ContourData m_contour;
  };

  class ContourAddSegmentation
  : public QUndoCommand
  {
  public:
    explicit ContourAddSegmentation(ChannelAdapterPtr     channel,
                                    CategoryAdapterSPtr   category,
                                    ModelAdapterSPtr      model,
                                    ModelFactorySPtr      factory,
                                    ViewManagerSPtr       vm,
                                    ManualEditionToolSPtr tool);
    virtual ~ContourAddSegmentation();

    virtual void redo();
    virtual void undo();

    /** \brief Helper method to create the rasterized volume and assigns values to internal variables.
     *  \param[in] contour contour data to be rasterized.
     */
    void rasterizeContour(ContourWidget::ContourData contour) const;

    /** \brief Returns the segmentation adapter created by the action.
     *
     */
    SegmentationAdapterSPtr getCreatedSegmentation() const;

  private:
    ModelAdapterSPtr                   m_model;
    ModelFactorySPtr                   m_factory;
    SampleAdapterSPtr                  m_sample;
    ChannelAdapterPtr                  m_channel;
    mutable SegmentationAdapterSPtr    m_segmentation;
    CategoryAdapterSPtr                m_category;
    ViewManagerSPtr                    m_viewManager;
    ManualEditionToolSPtr              m_tool;
    mutable bool                       m_rasterized;
    mutable ContourWidget::ContourData m_contour;
  };

} /* namespace ESPINA */
#endif // ESPINA_CONTOUR_UNDOCOMMAND_H_
