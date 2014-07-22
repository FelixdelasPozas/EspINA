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
#ifndef ESPINA_BRUSH_ROI_H
#define ESPINA_BRUSH_ROI_H

// ESPINA
#include <Support/Tool.h>
#include <Support/ViewManager.h>
#include <GUI/Model/ModelAdapter.h>
#include <App/ToolGroups/Edition/ManualEditionTool.h>

// Qt
#include <QUndoStack>

class QAction;
namespace ESPINA
{
  class ROIToolsGroup;

  // Volume Of Interest Toolbar
  class ManualROITool
  : public ManualEditionTool
  {
    Q_OBJECT
  public:
    /* \brief ManualROITool class constructor.
     * \param[in] model       Analysis model adapter.
     * \param[in] viewManager Application view manager.
     * \param[in] undoStack   Application qt undo stack pointer.
     * \param[in] toolGroup   ROIToolsGroup pointer containing ROI accumulator.
     */
    explicit ManualROITool(ModelAdapterSPtr model,
                           ViewManagerSPtr  viewManager,
                           QUndoStack      *undoStack,
                           ROIToolsGroup   *toolGroup);

    /* \brief ManualROITool class virtual destructor.
     *
     */
    virtual ~ManualROITool();

  protected slots:
    /* \brief Implements ManualEditionTool::drawingModeChanged(bool) slot.
     *
     */
    void drawingModeChanged(bool);

    /* \brief Implements ManualEditionTool::changeSelector(QAction *) slot.
     *
     */
    void changeSelector(QAction *selectorAction);

    /* \brief Implements ManualEditionTool::selectorInUse(bool) slot.
     *
     */
    void selectorInUse(bool value);

    /* \brief Implements ManualEditionTool::drawStroke(Selector::Selection) slot.
     *
     */
    void drawStroke(Selector::Selection);

    /* \brief Updates the selector parameters based on application selected items.
     *
     */
    void updateReferenceItem(SelectionSPtr selection);

    /* \brief Aborts current tool operation.
     *
     */
    void cancelROI();

    /* \brief Updates the selectors parameters based on ROI existence or not.
     *
     */
    void ROIChanged();

  private:
    void setControlVisibility(bool value);

  private:
    QUndoStack    *m_undoStack;
    ROIToolsGroup *m_toolGroup;
  };

  using ManualROIToolSPtr = std::shared_ptr<ManualROITool>;

} // namespace ESPINA

#endif // ESPINA_BRUSH_ROI_H
