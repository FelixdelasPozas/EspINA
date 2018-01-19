/*

 Copyright (C) 2014 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

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

#ifndef APP_UNDO_MODIFY_SKELETON_COMMAND_H_
#define APP_UNDO_MODIFY_SKELETON_COMMAND_H_

// ESPINA
#include <Core/Utils/Bounds.h>
#include <GUI/Types.h>

// VTK
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>

// Qt
#include <QUndoStack>

class vtkPolyData;

namespace ESPINA
{
  /** \class ModifySkeletonCommand
   * \brief Undo/redo class for skeleton data modifications.
   */
  class ModifySkeletonCommand
  : public QUndoCommand
  {
    public:
      /** \brief ModifySkeletonCommand class constructor.
       * \param[in] output output smart pointer with the skeleton data to be modified.
       * \param[in] sleleton SkeletonData smart pointer of the new skeleton.
       * \param[in] connections list of segmentation connections by this segmentation.
       *
       */
      explicit ModifySkeletonCommand(SegmentationAdapterSPtr segmentation, vtkSmartPointer<vtkPolyData> skeleton, ConnectionList connections = ConnectionList());

      /** \brief ModifySkeletonCommand class virtual destructor.
       *
       */
      virtual ~ModifySkeletonCommand();

      void undo() override;
      void redo() override;

    private:
      SegmentationAdapterSPtr      m_segmentation;
      vtkSmartPointer<vtkPolyData> m_newSkeleton;
      vtkSmartPointer<vtkPolyData> m_oldSkeleton;
      BoundsList                   m_editedRegions;
      ConnectionList               m_connections;
      ConnectionList               m_oldConnections;
  };

} // namespace ESPINA

#endif // APP_UNDO_MODIFY_SKELETON_COMMAND_H_
