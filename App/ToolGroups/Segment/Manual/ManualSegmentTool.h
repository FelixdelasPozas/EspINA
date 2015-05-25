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
#include <Support/Widgets/Tool.h>
#include <Support/Context.h>
#include <Core/Factory/FilterFactory.h>
#include <GUI/Model/ModelAdapter.h>
#include <GUI/EventHandlers/Brush.h>
#include <GUI/EventHandlers/StrokePainter.h>
#include <GUI/View/Selection.h>
#include <GUI/Widgets/DrawingWidget.h>
#include <ToolGroups/Refine/SliceEditionPipeline.h>

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
      virtual FilterSPtr createFilter(InputSList inputs, const Filter::Type& filter, SchedulerSPtr scheduler) const throw (Unknown_Filter_Exception);

      virtual FilterTypeList providedFilters() const;

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

    virtual void abortOperation();

  signals:
    void voxelsDeleted(ViewItemAdapterPtr item);

  public slots:
    void onSelectionChanged();

  private:
    virtual void onToolEnabled(bool enabled);

    void initMultiStrokeWidgets();

    void setInitialStroke();

    void setMultiStroke();

    void createSegmentation(BinaryMaskSPtr<unsigned char> mask);

    void modifySegmentation(BinaryMaskSPtr<unsigned char> mask);

    bool isCreationMode() const;

    SegmentationAdapterSPtr referenceSegmentation() const;

    ChannelAdapterPtr activeChannel() const;

  private slots:
    void onStrokeStarted(BrushPainter *painter, RenderView *view);

    void onMaskCreated(BinaryMaskSPtr<unsigned char> mask);

    void onCategoryChange(CategoryAdapterSPtr category);

    void onToolToggled(bool toggled);

    void onStrokeModeToggled(bool toggled);

  protected:
    ModelAdapterSPtr  m_model;
    ModelFactorySPtr  m_factory;
    QUndoStack       *m_undoStack;
    ColorEngineSPtr   m_colorEngine;
    SelectionSPtr     m_selection;
    FilterFactorySPtr m_filterFactory;
    Support::Context &m_context;

    using DrawingTool = GUI::Widgets::DrawingWidget;

    DrawingTool        m_drawingWidget;
    Mode               m_mode;
    bool               m_createSegmentation;
    ViewItemAdapterPtr m_referenceItem;

    bool                      m_validStroke;
    SliceEditionPipelineSPtr  m_temporalPipeline;
  };
} // namespace ESPINA

#endif // ESPINA_MANUAL_SEGMENT_TOOL_H_
