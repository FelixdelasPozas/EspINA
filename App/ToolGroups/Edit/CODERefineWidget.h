/*
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
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

#ifndef ESPINA_CODE_REFINE_WIDGET_H
#define ESPINA_CODE_REFINE_WIDGET_H

// Qt
#include <QWidget>

// ESPINA
#include <Filters/MorphologicalEditionFilter.h>
#include <GUI/Model/ModelAdapter.h>
#include <Support/Context.h>

class QUndoStack;

namespace ESPINA
{

  namespace Ui
  {
    class CODERefineWidget;
  }

  /** \class CODERefineWidget
   * \brief Refiner widget for CODE filters.
   *
   */
  class CODERefineWidget
  : public QWidget
  , private Support::WithContext
  {
      Q_OBJECT
    public:
      /** \brief CODERefineWidget class constructor.
       * \param[in] title widget title.
       * \param[in] segmentation input segmentation.
       * \param[in] context application context.
       * \param[in] parent QWidget parent of this one.
       *
       */
      explicit CODERefineWidget(const QString                 &title,
                                SegmentationAdapterPtr         segmentation,
                                Support::Context              &context,
                                QWidget                       *parent = nullptr);

      /** \brief CODERefineWidget class destructor.
       *
       */
      virtual ~CODERefineWidget();

    private slots:
      /** \brief Updates the GUI radius value when the filter radius changes.
       *
       */
      void onRadiusModified(int value);

      /** \brief Updates the filter.
       *
       */
      void refineFilter();

    private:
      Ui::CODERefineWidget          *m_gui;          /** widget's gui.                             */
      QString                        m_title;        /** widget's title, depends on the operation. */
      SegmentationAdapterPtr         m_segmentation; /** filter's segmentation.                    */
      MorphologicalEditionFilterSPtr m_filter;       /** morphological filter.                     */
  };

  /** \class CODEModification
   * \brief QUndoCommand to undo/redo modifications of CODE filters.
   *
   */
  class CODEModification
  : public QUndoCommand
  {
    public:
      /** \brief CODEModification class constructor.
       * \param[in] segmentation input segmentation.
       * \param[in] radius new radius value.
       * \param[in] parent pointer to QUndoCommand parent of this one.
       *
       */
      CODEModification(SegmentationAdapterPtr segmentation,
                       unsigned int           radius,
                       QUndoCommand          *parent = nullptr);

      /** \brief CODEModification class virtual destructor.
       *
       */
      virtual ~CODEModification()
      {};

      virtual void redo() override;
      virtual void undo() override;

    private:
      /** \brief Updates the filter.
       *
       */
      void update();

      /** \brief Invalidates the segmentation's representations after an update
       *
       */
      void invalidateRepresentations();

      SegmentationAdapterPtr         m_segmentation;  /** affected segmentation.                  */
      MorphologicalEditionFilterSPtr m_filter;        /** segmentation's filter.                  */
      unsigned int                   m_radius;        /** radius of the operation.                */
      unsigned int                   m_oldRadius;     /** filter's old radius value.              */
      Bounds                         m_oldBounds;     /** old segmentation's bounds value.        */
      itkVolumeType::Pointer         m_oldVolume;     /** old segmentation's volume.              */
      BoundsList                     m_editedRegions; /** old segmentation's edited regions list. */
  };

} // namespace ESPINA

#endif // ESPINA_CODE_REFINE_WIDGET_H
