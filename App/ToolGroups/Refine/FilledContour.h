/*
 
 Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

#ifndef ESPINA_FILLED_CONTOUR_H
#define ESPINA_FILLED_CONTOUR_H

// ESPINA
#include <Core/EspinaTypes.h>
#include <Core/Utils/vtkVoxelContour2D.h>
#include <Support/Widgets/Tool.h>
#include <GUI/Model/ModelAdapter.h>
#include <GUI/Selectors/BrushSelector.h>

class QUndoStack;
class vtkPolyData;

namespace ESPINA
{
  class ContourSelector;
  class ViewManager;
  class ContourWidget;

  class FilledContour
  : public Tool
  {
    Q_OBJECT
    public:
      /** \brief FilledContour class constructor.
       * \param[in] model ModelAdapter smart pointer.
       * \param[in] undoStack application QUndoStack object raw pointer.
       * \param[in] viewManager application view manager smart pointer.
       */
      explicit FilledContour(ModelAdapterSPtr model, QUndoStack *undoStack, ViewManagerSPtr viewManager);

      /** \brief FilledContour class virtual destructor.
       *
       */
      virtual ~FilledContour();

      virtual QList<QAction *> actions() const;

      virtual void abortOperation();

      /** \brief Sets the contour of the widget.
       * \param[in] contour ContourData object.
       *
       */
      void setContour(ContourWidget::ContourData contour);

      /** \brief Returs the contour of the ContourWidget.
       *
       */
      ContourWidget::ContourData getContour();

    signals:
      void changeMode(DrawSelector::BrushMode);
      void stopDrawing();
      void startDrawing();

    public slots:
      /** \brief Helper method to create and undo command in response to a user interaction in the widget.
       *
       */
      void createUndoCommand();

    protected slots:
      /** \brief Helper method to create a volume from the given contour list.
       * \param[in] contours list of ContourWidget's contours.
       *
       */
      void rasterize(ContourWidget::ContourList contours);

    private:
      ModelAdapterSPtr m_model;
      QUndoStack      *m_undoStack;
      ViewManagerSPtr  m_viewManager;

      ContourSelector *m_picker;
      bool m_enabled;
      bool m_inUse;

      ContourWidget *m_contourWidget;

      FilterSPtr m_currentSource;
      SegmentationSPtr m_currentSeg;

      bool m_widgetHasContour;
      vtkPolyData *m_lastContour;
  };

  using FilledContourPtr  = FilledContour *;
  using FilledContourSPtr = std::shared_ptr<FilledContour>;

} // namespace ESPINA

#endif // ESPINA_FILLED_CONTOUR_H
