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

    virtual void setEnabled(bool value) override;

    virtual void abortOperation() override;

    virtual void restoreSettings(std::shared_ptr<QSettings> settings) override final;

    virtual void saveSettings(std::shared_ptr<QSettings> settings) override final;

    void updateReferenceItem(SegmentationAdapterPtr segmentation) const;

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

    /** \brief Updates the item being edited or disables the tool on invalid selection.
     * \param[in] segmentations current selection's segmentations.
     *
     */
    void onSelectionChanged(SegmentationAdapterList segmentations);

  private:
    void onVoxelDeletion(ViewItemAdapterPtr item);

    /** \brief Removes the temporal representation for the reference item.
     *
     */
    void clearTemporalPipeline() const;

    using DrawingTool = GUI::Widgets::DrawingWidget;

    // mutable needed by updateReferenceItem() const
    mutable DrawingTool        m_drawingWidget; /** drawing widget.                                */
    mutable ViewItemAdapterPtr m_referenceItem; /** current item being edited or the origin stack. */

    MaskPainterSPtr                  m_currentHandler;   /** current event handler.                                              */
    bool                             m_validStroke;      /** true if the last stroke is valid, false otherwise.                  */
    mutable SliceEditionPipelineSPtr m_temporalPipeline; /** temporal pipeline to generate representations for the current item. */
  };

  using ManualEditionToolPtr  = ManualEditionTool *;
  using ManualEditionToolSPtr = std::shared_ptr<ManualEditionTool>;

} // namespace ESPINA

#endif // ESPINA_MANUAL_EDITION_TOOL_H_
