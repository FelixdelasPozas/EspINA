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

#ifndef ESPINA_MANUAL_EDITION_TOOL_H_
#define ESPINA_MANUAL_EDITION_TOOL_H_

// ESPINA
#include <Support/Widgets/Tool.h>
#include <Support/Context.h>
#include <Core/Factory/FilterFactory.h>
#include <GUI/Model/ModelAdapter.h>
#include <GUI/EventHandlers/Brush.h>
#include <GUI/EventHandlers/StrokePainter.h>
#include <GUI/View/Selection.h>
#include <GUI/Widgets/DrawingWidget.h>
#include "SliceEditionPipeline.h"

class QUndoStack;

using namespace ESPINA::GUI::View;

namespace ESPINA
{
  class ManualEditionTool
  : public Tool
  {
    Q_OBJECT

    enum class Mode
    {
      CREATION,
      EDITION
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
    /** \brief ManualEditionTool class constructor.
     * \param[in] context to be used for this tool
     *
     */
    ManualEditionTool(Support::Context &context);

    /** \brief ManualEditionTool class virtual destructor.
     *
     */
    virtual ~ManualEditionTool();

    virtual QList<QAction *> actions() const;

    virtual void abortOperation();

  signals:
    void voxelsDeleted(ViewItemAdapterPtr item);

  public slots:
    /** \brief Updates the reference item for the tool.
     *
     */
    void updateReferenceItem() const;

  private:
    virtual void onToolEnabled(bool enabled);

    void createSegmentation(BinaryMaskSPtr<unsigned char> mask);

    void modifySegmentation(BinaryMaskSPtr<unsigned char> mask);

    bool isCreationMode() const;

    SegmentationAdapterSPtr referenceSegmentation() const;

    ChannelAdapterPtr activeChannel() const;

  private slots:
    void onStrokeStarted(BrushPainter *painter, RenderView *view);

    void onMaskCreated(BinaryMaskSPtr<unsigned char> mask);

    void onCategoryChange(CategoryAdapterSPtr category);

  protected:
    ModelAdapterSPtr  m_model;
    ModelFactorySPtr  m_factory;
    QUndoStack       *m_undoStack;
    ColorEngineSPtr   m_colorEngine;
    SelectionSPtr     m_selection;
    FilterFactorySPtr m_filterFactory;
    Support::Context &m_context;

    // mutable needed by updateReferenceItem() const
    mutable DrawingWidget      m_drawingWidget;
    mutable Mode               m_mode;
    mutable ViewItemAdapterPtr m_referenceItem;

    bool                      m_validStroke;
    SliceEditionPipelineSPtr  m_temporalPipeline;
  };

  using ManualEditionToolPtr  = ManualEditionTool *;
  using ManualEditionToolSPtr = std::shared_ptr<ManualEditionTool>;

} // namespace ESPINA

#endif // ESPINA_MANUAL_EDITION_TOOL_H_
