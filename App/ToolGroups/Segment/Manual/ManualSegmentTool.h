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

using namespace ESPINA::GUI::View;

namespace ESPINA
{
  class ManualSegmentTool
  : public Support::Widgets::ProgressTool
  {
    Q_OBJECT

    enum class Mode
    {
      SINGLE_STROKE,
      MULTI_STROKE
    };

    class ManualFilterFactory
    : public FilterFactory
    {
      virtual FilterSPtr createFilter(InputSList inputs, const Filter::Type& filter, SchedulerSPtr scheduler) const override;

      virtual FilterTypeList providedFilters() const override;

    private:
      mutable DataFactorySPtr m_dataFactory;
    };

  public:
    /** \brief ManualSegmentTool class constructor.
     * \param[in] context to be used for this tool
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

  signals:
    void voxelsDeleted(ViewItemAdapterPtr item);

  public slots:
    void onSelectionChanged();

  private:
    void initMultiStrokeWidgets();

    void setInitialStroke();

    void setMultiStroke();

    void createSegmentation(BinaryMaskSPtr<unsigned char> mask);

    void modifySegmentation(BinaryMaskSPtr<unsigned char> mask);

    bool isCreationMode() const;

    SegmentationAdapterSPtr referenceSegmentation() const;

  private slots:
    void onStrokeStarted(BrushPainter *painter, RenderView *view);

    void onMaskCreated(BinaryMaskSPtr<unsigned char> mask);

    void onCategoryChange(CategoryAdapterSPtr category);

    void onPainterChanged(MaskPainterSPtr painter);

    void onToolToggled(bool toggled);

    void onStrokeModeToggled(bool toggled);

    void startNextSegmentation();

    void onEventHandlerActivated(bool inUse);

    void onVoxelDeletion(ViewItemAdapterPtr item);

  protected:
    ModelAdapterSPtr  m_model;
    ModelFactorySPtr  m_factory;
    GUI::ColorEngines::ColorEngineSPtr   m_colorEngine;
    FilterFactorySPtr m_filterFactory;

    using DrawingTool = GUI::Widgets::DrawingWidget;

    DrawingTool        m_drawingWidget;
    QPushButton       *m_multiStroke;
    Mode               m_mode;
    bool               m_createSegmentation;
    ViewItemAdapterPtr m_referenceItem;

    MaskPainterSPtr m_currentPainter;

    bool                      m_validStroke;
    SliceEditionPipelineSPtr  m_temporalPipeline;
  };
} // namespace ESPINA

#endif // ESPINA_MANUAL_SEGMENT_TOOL_H_
