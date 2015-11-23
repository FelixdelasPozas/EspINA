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

  class CODERefineWidget
  : public QWidget
  , private Support::WithContext
  {
      Q_OBJECT
    public:
      /** \brief CODERefineWidget class constructor.
       * \param[in] title widget title.
       * \param[in] segmentation input segmentation.
       * \param[in] filter segmentation's filter.
       * \param[in] context application context.
       *
       */
      explicit CODERefineWidget(const QString                 &title,
                                SegmentationAdapterPtr         segmentation,
                                Support::Context              &context);

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
      Ui::CODERefineWidget *m_gui;

      QString m_title;
      SegmentationAdapterPtr m_segmentation;
      MorphologicalEditionFilterSPtr m_filter;
  };

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

      virtual ~CODEModification() {};

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

      SegmentationAdapterPtr         m_segmentation;
      MorphologicalEditionFilterSPtr m_filter;
      unsigned int                   m_radius;

      unsigned int           m_oldRadius;
      Bounds                 m_oldBounds;
      itkVolumeType::Pointer m_oldVolume;
      BoundsList             m_editedRegions;
  };

} // namespace ESPINA

#endif // ESPINA_CODE_REFINE_WIDGET_H
