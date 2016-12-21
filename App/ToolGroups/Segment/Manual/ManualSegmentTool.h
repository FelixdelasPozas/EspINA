/*
 *
 * Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>
 *
 * This file is part of ESPINA.
 *
 *    ESPINA is free software: you can redistribute it and/or modify
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
 */

#ifndef ESPINA_MANUAL_SEGMENT_TOOL_H_
#define ESPINA_MANUAL_SEGMENT_TOOL_H_

// ESPINA
#include <Support/Widgets/ProgressTool.h>
#include <Support/Context.h>
#include <Core/Factory/FilterFactory.h>
#include <GUI/Model/ModelAdapter.h>
#include <GUI/EventHandlers/Brush.h>
#include <GUI/EventHandlers/StrokePainter.h>
#include <GUI/View/Selection.h>
#include <GUI/Widgets/DrawingWidget.h>
#include <ToolGroups/Edit/SliceEditionPipeline.h>

class QUndoStack;

namespace ESPINA
{
  /** \class ManualFilterFactory
   * \brief Factory for SourceFilter filters.
   */
  class ManualFilterFactory
  : public FilterFactory
  {
    public:
      static const Filter::Type SOURCE_FILTER;
      static const Filter::Type SOURCE_FILTER_V4;

      virtual FilterSPtr createFilter(InputSList inputs, const Filter::Type& filter, SchedulerSPtr scheduler) const override;

      virtual FilterTypeList providedFilters() const override;

    private:
      mutable DataFactorySPtr m_dataFactory; /** data factory for this provider. */
  };

  /** \class ManualSegmentTool
   * \brief Tool for manual segmentation filters.
   *
   */
  class ManualSegmentTool
  : public Support::Widgets::ProgressTool
  {
      Q_OBJECT

      enum class Mode
      {
        SINGLE_STROKE,
        MULTI_STROKE
      };

    public:
      /** \brief ManualSegmentTool class constructor.
       * \param[in] context application context.
       *
       */
      ManualSegmentTool(Support::Context &context);

      /** \brief ManualSegmentTool class virtual destructor.
       *
       */
      virtual ~ManualSegmentTool();

      virtual void abortOperation() override;

      virtual void restoreSettings(std::shared_ptr<QSettings> settings) override final;

      virtual void saveSettings(std::shared_ptr<QSettings> settings) override final;

    public slots:
      /** \brief Updates GUI and the tool when the current selection changes.
       *
       */
      void onSelectionChanged();

    private:
      /** \brief Initializes multi stroke option widgets.
       *
       */
      void initMultiStrokeWidgets();

      /** \brief Updates the tool before the first user stroke.
       *
       */
      void setInitialStroke();

      /** \brief Updates the tool when the multi stroke option changes state.
       *
       */
      void setMultiStroke();

      /** \brief Helper method to create a segmentation.
       * \param[in] mask segmentation volume mask.
       *
       */
      void createSegmentation(BinaryMaskSPtr<unsigned char> mask);

      /** \brief Helper to modify the volume of a segmentation.
       * \param[in] mask modification volume mask.
       *
       */
      void modifySegmentation(BinaryMaskSPtr<unsigned char> mask);

      bool isCreationMode() const;

      SegmentationAdapterSPtr referenceSegmentation() const;

    private slots:
      /** \brief Updates the GUI when a stroke is started.
       * \param[in] painter painter doing the stroke.
       * \param[in] view view to show the stroke.
       *
       */
      void onStrokeStarted(BrushPainter *painter, RenderView *view);

      /** \brief Creates or modifies a segmentation with the mask's volume.
       * \param[in] mask volume mask.
       *
       */
      void onMaskCreated(BinaryMaskSPtr<unsigned char> mask);

      /** \brief Updates the tool and the GUI when the edition category changes.
       * \param[in] category new selected category.
       *
       */
      void onCategoryChange(CategoryAdapterSPtr category);

      /** \brief
       *
       */
      void onPainterChanged(MaskPainterSPtr painter);

      /** \brief Updates the tool when enabling/disabling it.
       * \param[in] toggled true if checked and false otherwise.
       *
       */
      void onToolToggled(bool toggled);

      /** \brief Updates the tool when the stroke mode changes state.
       * \param[in] toggled true if checked and false otherwise.
       *
       */
      void onStrokeModeToggled(bool toggled);

      /** \brief Updates the tool when the user clicks on the next segmentation button.
       *
       */
      void startNextSegmentation();

      /** \brief Updates the tool and GUI when a new event handler changes state.
       * \param[in] inUse true if the handler is active and false otherwise.
       *
       */
      void onEventHandlerActivated(bool inUse);

      /** \brief Checks for complete deletion of the given item.
       * \param[in] item view item to check.
       *
       */
      void onVoxelDeletion(ViewItemAdapterPtr item);

    private:
      /** \brief Removes the temporal representation for the reference item.
       *
       */
      void clearTemporalPipeline() const;

    protected:
      ModelAdapterSPtr                   m_model;         /** current analysis model. */
      ModelFactorySPtr                   m_factory;       /** current analysis factory. */
      GUI::ColorEngines::ColorEngineSPtr m_colorEngine;   /** application color engine for coloring. */
      FilterFactorySPtr                  m_filterFactory; /** tool's filter factory. */

      using DrawingTool = GUI::Widgets::DrawingWidget;

      DrawingTool        m_drawingWidget;      /** drawing widget. */
      QPushButton       *m_multiStroke;        /** multi stroke button. */
      Mode               m_mode;               /** edition mode: paint/erase. */
      bool               m_createSegmentation; /** true to create a segmentation when a mask object is received. */
      ViewItemAdapterPtr m_referenceItem;      /** item used to take spacing reference. */

      MaskPainterSPtr m_currentPainter; /** current painter. */

      bool                              m_validStroke;      /** true if the last stroke is a valid one and false otherwise. */
      mutable SliceEditionPipelineSPtr  m_temporalPipeline; /** temporal pipeline to show while the user paints. */
  };
} // namespace ESPINA

#endif // ESPINA_MANUAL_SEGMENT_TOOL_H_
