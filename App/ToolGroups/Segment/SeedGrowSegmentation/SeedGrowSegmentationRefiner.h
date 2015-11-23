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

#ifndef ESPINA_SEED_GROW_SEGMENTATION_HISTORY_H
#define ESPINA_SEED_GROW_SEGMENTATION_HISTORY_H

// ESPINA
#include <Support/FilterRefiner.h>
#include <Filters/SeedGrowSegmentationFilter.h>
#include <ToolGroups/Restrict/RestrictToolGroup.h>

namespace ESPINA
{
  class ROISettings;

  class SeedGrowSegmentationRefiner
  : public FilterRefiner
  {
      Q_OBJECT
    public:
      /** \brief SeedGrowSegmentationRefiner class constructor.
       *
       */
      SeedGrowSegmentationRefiner();

      /** \brief SeedGrowSegmentationRefiner class virtual destructor.
       *
       */
      virtual ~SeedGrowSegmentationRefiner();

      virtual QWidget* createWidget(SegmentationAdapterPtr segmentation, Support::Context& context);

      /** \brief Returns the roi refinement tools used by the widgets.
       *
       */
      static RestrictToolGroupSPtr tools(Support::Context &context);

    signals:
      void thresholdChanged(SegmentationAdapterPtr , int);
      void applyClosingChanged(SegmentationAdapterPtr, bool);
      void closingRadiusChanged(SegmentationAdapterPtr, int);

    private slots:
      /**
       *  Decrease widget count and hides ROI if no widgets are visible
       */
      void onWidgetDestroyed(QObject *widget);

    private:
      struct RefineWidget
      {
        /** \brief RefineWidget struct constructor.
         * \param[in] segmentation input segmentation.
         * \param[in] context application context.
         *
         */
        RefineWidget(SegmentationAdapterPtr segmentation, Support::Context &context);

        /** \brief RefineWidget struct destructor.
         *
         */
        ~RefineWidget();

        int                   Count;    /** contains the widgets in effect. */
        RestrictToolGroupSPtr RoiTools; /** ROI refinement tools */
      };

      QMap<SegmentationAdapterPtr, RefineWidget *> m_refineWidgets;       /** refinement widgets. */
      QMap<QObject *, SegmentationAdapterPtr>      m_widgetSegmentation;  /** refinement widget-segmentation map. */
  };

} // namespace ESPINA

#endif // ESPINA_SEED_GROW_SEGMENTATION_HISTORY_H
