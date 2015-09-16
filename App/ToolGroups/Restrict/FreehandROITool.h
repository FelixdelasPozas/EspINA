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

    void setColor(const QColor &color);

    virtual void restoreSettings(std::shared_ptr<QSettings> settings) override final;

    virtual void saveSettings(std::shared_ptr<QSettings> settings) override final;

  signals:
    void roiDefined(BinaryMaskSPtr<unsigned char> mask);

  private slots:
    void onPainterChanged(MaskPainterSPtr painter);

    void updateReferenceItem(ChannelAdapterPtr channel);

    void cancelROI();

    void ROIChanged(ROISPtr roi);

  private:
    void configureDrawingTools();

  private:
    using DrawingTool = GUI::Widgets::DrawingWidget;

    QUndoStack        *m_undoStack;
    RestrictToolGroup *m_toolGroup;

    ChannelAdapterPtr m_referenceItem;

    DrawingTool m_drawingWidget;
  };

  using FreehandROIToolSPtr = std::shared_ptr<FreehandROITool>;

} // namespace ESPINA

#endif // ESPINA_BRUSH_ROI_H
