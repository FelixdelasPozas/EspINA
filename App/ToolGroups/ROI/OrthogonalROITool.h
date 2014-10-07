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
#ifndef ESPINA_ORTHOGONAL_ROI_H
#define ESPINA_ORTHOGONAL_ROI_H

// ESPINA
#include <Support/Widgets/Tool.h>
#include <Support/ViewManager.h>
#include <GUI/Model/ModelAdapter.h>
#include <GUI/Selectors/Selector.h>
#include <GUI/Selectors/PixelSelector.h>

// Qt
#include <QUndoCommand>

class QAction;
namespace ESPINA
{
  class RectangularRegion;
  class RectangularRegionSliceSelector;
  class ROISettings;
  class ROIToolsGroup;

  class OrthogonalROITool
  : public Tool
  {
    Q_OBJECT
  public:
    enum Mode { EDITION, COMMITED };

  public:
    /** \brief OrthogonalROITool class constructor.
     * \param[in] model, model adapter smart pointer.
     * \param[in] viewManager, view manager smart pointer.
     * \param[in] undoStack, QUndoStack object raw pointer.
     * \param[in] toolGroup, ROIToolsGroup raw pointer that contains ROI accumulator.
     */
    explicit OrthogonalROITool(ROISettings     *settings,
                               ModelAdapterSPtr model,
                               ViewManagerSPtr  viewManager,
                               QUndoStack      *undoStack,
                               ROIToolsGroup   *toolgroup);

    /** \brief OrthogonalROITool class virtual destructor.
     *
     */
    virtual ~OrthogonalROITool();

    /** \brief Implements Tool::setEnabled(bool).
     *
     */
    virtual void setEnabled(bool value);

    /** \brief Implements Tool::enabled().
     *
     */
    virtual bool enabled() const;

    /** \brief Implements Tool::actions().
     *
     */
    virtual QList<QAction *> actions() const;

    /** \brief Returns true if the Orthogonal widget has been placed
     *
     */
    bool isDefined() const;

    /** \brief Removes the current widget.
     *
     */
    void cancelWidget();

    /** \brief Set the active ROI to the given values
     *
     *   \param[in] bounds of the recttangular region
     *   \param[in] spacing of the voxels where it was defined
     *   \param[in] origin of the reference frame to be adjusted
     *   \param[in] mode select how the ROI contour are drawn, continous for COMMITED and discontinous for EDITION
     */
    void setROI(const Bounds& bounds, const NmVector3& spacing, const NmVector3& origin, const Mode& mode = EDITION);

  signals:
    void roiDefined();

  protected slots:
    /** \brief Modifies the tool and activates/deactivates the event handler for this tool.
     * \param[in] value true to activate tool and eventhandler, false to deactivate event handler.
     */
    void activateEventHandler(bool value);

    /** \brief Activates/Deactivates the tool and commits the current ROI if deactivated.
     * \param[in] value true to activate the tool.
     */
    void activateTool(bool value);

    /** \brief Defines a new ROI based on the selection.
     * \param[in] selection selection containing the active channel and selected voxel.
     *
     */
    void defineROI(Selector::Selection selection);

    /** \brief Modifies the application ROI with the current ROI of the tool.
     *
     */
    void commitROI();

  private:
    void enableSelector(bool value);

  private:
    ModelAdapterSPtr m_model;
    ViewManagerSPtr  m_viewManager;
    QUndoStack      *m_undoStack;
    ROIToolsGroup   *m_toolGroup;

    QAction         *m_applyROI;
    bool             m_enabled;

    PixelSelectorSPtr m_selector;
    EspinaWidgetSPtr  m_widget;
    NmVector3         m_spacing;
    NmVector3         m_origin;
    RectangularRegionSliceSelector *m_sliceSelector;
    ROISettings                    *m_settings;
  };

  using OrthogonalROIToolSPtr = std::shared_ptr<OrthogonalROITool>;

} // namespace ESPINA

#endif // ESPINA_ORTHOGONAL_ROI_H
