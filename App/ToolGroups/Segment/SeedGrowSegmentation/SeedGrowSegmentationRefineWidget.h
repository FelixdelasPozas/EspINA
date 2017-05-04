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

#ifndef ESPINA_SEED_GROW_SEGMENTATION_HISTORY_WIDGET_H
#define ESPINA_SEED_GROW_SEGMENTATION_HISTORY_WIDGET_H

// Qt
#include <QWidget>

// ESPINA
#include <Filters/SeedGrowSegmentationFilter.h>
#include <GUI/Model/ModelAdapter.h>
#include <Support/Context.h>
#include <ToolGroups/Restrict/RestrictToolGroup.h>

class QUndoStack;

namespace ESPINA
{
  class ROISettings;

  namespace Ui
  {
    class SeedGrowSegmentationRefineWidget;
  }

  class SeedGrowSegmentationRefineWidget
  : public QWidget
  , private Support::WithContext
  {
      Q_OBJECT
    public:
      /** \brief SeedGrowSegmentationRefineWidget class constructor.
       * \param[in] segmentation input segmentation.
       * \param[in] context application context.
       * \param[in] parent QWidget parent of this one.
       *
       */
      explicit SeedGrowSegmentationRefineWidget(SegmentationAdapterPtr segmentation,
                                                Support::Context      &context,
                                                QWidget               *parent = nullptr);

      /** \brief SeedGrowSegmentationRefineWidget class virtual destructor.
       *
       */
      virtual ~SeedGrowSegmentationRefineWidget();

    public slots:
      /** \brief Sets the GUI filter threshold when the filter's changes.
       * \param[in] lower filter's lower threshold.
       * \param[in] upper filter's upper threshold.
       *
       */
      void onFilterThresholdModified(int lower, int upper);

      /** \brief Sets the GUI closing radius when the filter's changes.
       * \param[in] value filter's radius value.
       *
       */
      void onFilterRadiusModified(int value);

      /** \brief Sets the GUI roi when the filter's changes.
       * \param[in] roi filter's roi.
       *
       */
      void onFilterroiModified(ROISPtr roi);

    private slots:
      /** \brief Updates the GUI when the GUI threshold changes.
       * \param[in] value siimetric threshold value.
       *
       */
      void onThresholdChanged(int value);

      /** \brief Updates the GUI when the GUI application of the closing algorithm changes.
       * \param[in] value true to enable and false otherwise.
       *
       */
      void onApplyClosingChanged(bool value);

      /** \brief Updates the GUI when the GUI radius changes.
       * \param[in] value radius value.
       *
       */
      void onClosingRadiusChanged(int value);

      /** \brief Updates the GUI when the filter's roi changes.
       *
       */
      void onROIChanged();

      /** \brief Discards ROI modifications.
       *
       */
      void onDiscardROIModifications();

      /** \brief Executes the filter with the new parameters.
       *
       */
      void modifyFilter();

    private:
      /** \brief Builds and returns the title of the widget.
       *
       */
      QString dialogTitle() const;

    private:
      SegmentationAdapterPtr                m_segmentation; /** input segmentation.                     */
      Ui::SeedGrowSegmentationRefineWidget *m_gui;          /** widget's GUI class, chessire cat style. */
      SeedGrowSegmentationFilterSPtr        m_filter;       /** segmentation's filter.                  */
      RestrictToolGroupSPtr                 m_roiTools;     /** roi toolgroup.                          */

      static QMutex s_mutex;
      static bool s_exists;
  };

  /** \class DiscardROIModificationsCommand
   * \brief QUndoCommand to undo the modifications in the ROI of a filter.
   *
   */
  class DiscardROIModificationsCommand
  : public QUndoCommand
  {
    public:
      /** \brief DiscardROIModificationsCommand class constructor.
       * \param[in] roiTools roi toolgroup.
       * \param[in] filter SGS filter owner of the roi.
       * \param[in] parent pointer of the QUndoCommand parent of this one.
       *
       */
      explicit DiscardROIModificationsCommand(RestrictToolGroupSPtr          roiTools,
                                              SeedGrowSegmentationFilterSPtr filter,
                                              QUndoCommand                  *parent = nullptr);

      /** \brief DiscardROIModificationsCommand class virtual destructor.
       *
       */
      virtual ~DiscardROIModificationsCommand() {};

      virtual void redo() override;
      virtual void undo() override;

    private:
      /** \brief Swaps the filter ROI and the modified ROI.
       *
       */
      void swapCurrentROI();

      RestrictToolGroupSPtr m_roiTools; /** roi tool group. */
      ROISPtr               m_ROI;      /** ROI             */
  };

  /** \class SGSFilterModification
   * \brief QUndoCommand to undo the modifications of a SGS filter.
   *
   */
  class SGSFilterModification
  : public QUndoCommand
  {
    public:
      /** \brief SGSFilterModification class constructor.
       * \param[in] roi to be applied to limit the seed grow algorithm
       * \param[in] threshold symmetric threshold to be used to determine connectivity respect from the gray level value of the seed voxel
       * \param[in] closeRadius value of the radius to be applied to the closing post-proccessing. If this value is 0 no post-proccessing will be executed
       * \param[in] parent the undo command which will trigger this one.
       *
       */
      SGSFilterModification(SegmentationAdapterPtr segmentation,
                            ROISPtr                roi,
                            int                    threshold,
                            int                    closeRadius,
                            QUndoCommand          *parent = nullptr);

      /** \brief SGSFilterModification class virtual destructor.
       *
       */
      virtual ~SGSFilterModification() {};

      virtual void redo() override;
      virtual void undo() override;

    private:
      /** \brief Updates the filter with the new parameters.
       *
       */
      void update();

      /** \brief Invalidates the segmentation's representations.
       *
       */
      void invalidateRepresentations();

      SegmentationAdapterPtr         m_segmentation; /** input segmentation */
      SeedGrowSegmentationFilterSPtr m_filter;       /** segmentation's filter. */

      ROISPtr m_ROI;              /** filter's ROI                           */
      ROISPtr m_oldROI;           /** filter's previous ROI                  */
      int     m_threshold;        /** filter's simmetric threshold.          */
      int     m_oldThreshold;     /** filter's previous simmetric threshold. */
      int     m_closingRadius;    /** filter's close radius.                 */
      int     m_oldClosingRadius; /** filter's previous close radius.        */

      Bounds                 m_oldBounds;      /** volume's bounds previous to modification. */
      itkVolumeType::Pointer m_oldVolume;      /** volume previous to modification. */
      BoundsList             m_editedRegions;  /** filter's edited regions previous to modification. */
  };

} // namespace ESPINA

#endif // ESPINA_SEED_GROW_SEGMENTATION_HISTORY_WIDGET_H
