/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2013 Félix de las Pozas Álvarez <felixdelaspozas@gmail.com>

 This program is free software: you can redistribute it and/or modify
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

// EspINA
#include "Undo/ContourUndoCommand.h"
#include "GUI/ViewManager.h"

namespace EspINA
{
  //-----------------------------------------------------------------------------
  ContourUndoCommand::ContourUndoCommand(SegmentationSPtr seg,
                                         vtkPolyData* contour,
                                         Nm pos,
                                         PlaneType plane,
                                         itkVolumeType::PixelType value,
                                         ViewManager *vm)
  : m_segmentation(seg)
  , m_contour(contour)
  , m_plane(plane)
  , m_pos(pos)
  , m_value(value)
  , m_viewManager(vm)
  , m_needReduction(false)
  {
    contour->GetBounds(m_contourBounds);
  }
  
  //-----------------------------------------------------------------------------
  ContourUndoCommand::~ContourUndoCommand()
  {
  }

  //-----------------------------------------------------------------------------
  void ContourUndoCommand::redo()
  {
    if (m_newVolume.IsNotNull())
    {
      m_segmentation->filter()->draw(0, m_newVolume);
    }
    else
    {
      EspinaRegion contourRegion(m_contourBounds);
      EspinaVolume::Pointer volume = m_segmentation->volume();

      m_prevRegions = m_segmentation->filter()->output(0).editedRegions;

      if (!contourRegion.isInside(volume->espinaRegion()))
      {
        if (contourRegion.intersect(volume->espinaRegion()))
          m_prevVolume = volume->cloneVolume(contourRegion.intersection(volume->espinaRegion()));

        m_needReduction = true;
      }
      else
        m_prevVolume = volume->cloneVolume(contourRegion);

      m_segmentation->filter()->draw(0, m_contour, m_pos, m_plane, m_value, true);

      m_newVolume = m_segmentation->volume()->cloneVolume(contourRegion);
    }

    m_viewManager->updateSegmentationRepresentations(m_segmentation.data());
  }
  
  //-----------------------------------------------------------------------------
  void ContourUndoCommand::undo()
  {
    Q_ASSERT(m_newVolume.IsNotNull() && m_prevVolume.IsNotNull());

    EspinaRegion contourRegion(m_contourBounds);
    m_segmentation->filter()->fill(0, contourRegion, SEG_BG_VALUE, false);

    m_segmentation->filter()->draw(0, m_prevVolume, !m_needReduction);

    if (m_needReduction)
      m_segmentation->volume()->fitToContent();

    // Restore previous edited regions
    m_segmentation->filter()->output(0).editedRegions = m_prevRegions;

    m_viewManager->updateSegmentationRepresentations(m_segmentation.data());
  }

} /* namespace EspINA */
