/*

    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef ESPINA_MANUAL_ROI_TOOL_H
#define ESPINA_MANUAL_ROI_TOOL_H

// ESPINA
#include <Support/Widgets/ProgressTool.h>
#include <Support/Context.h>
#include <GUI/Widgets/DrawingWidget.h>

// Qt
#include <QUndoStack>

namespace ESPINA
{
  class RestrictToolGroup;

  /** \class FreehandROITool
   * \brief Implements the manual edition of a ROI.
   *
   */
  class FreehandROITool
  : public Support::Widgets::ProgressTool
  {
    Q_OBJECT

  public:
    /** \brief FreehandROITool class constructor.
     * \param[in] context to be used for this tool
     * \param[in] toolGroup ROIToolsGroup raw pointer containing ROI accumulator.
     */
    explicit FreehandROITool(Support::Context  &context,
                           RestrictToolGroup *toolGroup);

    /** \brief FreehandROITool class virtual destructor.
     *
     */
    virtual ~FreehandROITool();

    virtual void abortOperation() override;

    /** \brief Sets the color of the brush.
     * \param[in] color brush color.
     *
     */
    void setColor(const QColor &color);

    virtual void restoreSettings(std::shared_ptr<QSettings> settings) override final;

    virtual void saveSettings(std::shared_ptr<QSettings> settings) override final;

  signals:
    void roiDefined(BinaryMaskSPtr<unsigned char> mask);

  private slots:
    /** \brief Updates the brush when the painter changes.
     *
     */
    void onPainterChanged(MaskPainterSPtr painter);

    /** \brief Updates the reference item for spacing and updates the drawing tool accordingly.
     *
     */
    void updateReferenceItem(ChannelAdapterPtr channel);

    /** \brief Updates the drawing widget when the ROI changes.
     *
     */
    void ROIChanged(ROISPtr roi);

  private:
    /** \brief Helper method that configures the default parameters of the drawing widget.
     *
     */
    void configureDrawingTools();

  private:
    using DrawingTool = GUI::Widgets::DrawingWidget;

    QUndoStack        *m_undoStack;     /** application undo-redo stack.           */
    RestrictToolGroup *m_toolGroup;     /** parent toolgroup this tool belongs to. */
    ChannelAdapterPtr  m_referenceItem; /** reference item for spacing.            */
    DrawingTool        m_drawingWidget; /** drawing widget for ROI.                */
  };

  using FreehandROIToolSPtr = std::shared_ptr<FreehandROITool>;

} // namespace ESPINA

#endif // ESPINA_BRUSH_ROI_H
