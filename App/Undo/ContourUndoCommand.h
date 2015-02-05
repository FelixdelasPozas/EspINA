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
#include <GUI/View/Widgets/Contour/ContourWidget.h>

// Qt
#include <QUndoStack>

namespace ESPINA
{
  class ManualEditionTool;
  /** \class ContourModificationUndoCommand.
   *
   */
  class ContourModificationUndoCommand
  : public QUndoCommand
  {
    public:
      /** \brief ContourModificationUndoCommand class constructor.
       * \param[in] previousContour previous contour data.
       * \param[in] actualContour actual contour data.
       * \param[in] tool contour edition tool.
       *
       */
      explicit ContourModificationUndoCommand(ContourWidget::ContourData  previousContour,
                                              ContourWidget::ContourData  actualContour,
                                              ManualEditionTool          *tool);

      virtual void redo() override final;
      virtual void undo() override final;

    private:
      ContourWidget::ContourData  m_previousContour;
      ContourWidget::ContourData  m_actualContour;
      ManualEditionTool          *m_tool;
  };

  /** \class ContourRasterizeUndoCommand.
   *
   */
  class ContourRasterizeUndoCommand
  : public QUndoCommand
  {
    public:
      /** \brief ContourRasterizeUndoCommand class constructor.
       * \param[in] segmentation segmentation to modify.
       * \param[in] contourVolume rasterized volume of the contour.
       * \param[in] contour actual un-rasterized contour.
       * \param[in] tool contour edition tool.
       *
       */
      explicit ContourRasterizeUndoCommand(SegmentationAdapterPtr        segmentation,
                                           BinaryMaskSPtr<unsigned char> contourVolume,
                                           ContourWidget::ContourData    contour,
                                           ManualEditionTool            *tool);

      virtual void redo() override final;
      virtual void undo() override final;

    private:
      SegmentationAdapterPtr         m_segmentation;
      BinaryMaskSPtr<unsigned char>  m_contourVolume;
      ContourWidget::ContourData     m_contour;
      ManualEditionTool             *m_tool;
      bool                           m_createData;
  };

} /* namespace ESPINA */
#endif // ESPINA_CONTOUR_UNDOCOMMAND_H_
