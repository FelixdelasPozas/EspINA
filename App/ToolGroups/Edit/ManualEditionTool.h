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
#include <Support/Widgets/EditTool.h>
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
  : public Support::Widgets::EditTool
  {
    Q_OBJECT

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

    virtual void abortOperation() override;

    virtual void restoreSettings(std::shared_ptr<QSettings> settings) override final;

    virtual void saveSettings(std::shared_ptr<QSettings> settings) override final;

    void updateReferenceItem(SegmentationAdapterPtr segmentation) const;

  signals:
    void voxelsDeleted(ViewItemAdapterPtr item);

  private:
    void modifySegmentation(BinaryMaskSPtr<unsigned char> mask);

    SegmentationAdapterSPtr referenceSegmentation() const;

    virtual bool acceptsNInputs(int n) const override
    { return n == 1; }

    virtual bool acceptsSelection(SegmentationAdapterList segmentations) override;

  private slots:
    void onStrokeStarted(BrushPainter *painter, RenderView *view);

    void onMaskCreated(BinaryMaskSPtr<unsigned char> mask);

    void onPainterChanged(MaskPainterSPtr painter);

    /** \brief Updates the signal connection and manages selected item.
     *
     */
    void onEventHandlerActivated(bool inUse);

  private:
    using DrawingTool = GUI::Widgets::DrawingWidget;

    // mutable needed by updateReferenceItem() const
    mutable DrawingTool        m_drawingWidget;
    mutable ViewItemAdapterPtr m_referenceItem;

    MaskPainterSPtr m_currentHandler;
    bool                      m_validStroke;
    SliceEditionPipelineSPtr  m_temporalPipeline;
  };

  using ManualEditionToolPtr  = ManualEditionTool *;
  using ManualEditionToolSPtr = std::shared_ptr<ManualEditionTool>;

} // namespace ESPINA

#endif // ESPINA_MANUAL_EDITION_TOOL_H_