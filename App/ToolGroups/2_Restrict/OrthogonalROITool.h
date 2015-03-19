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
#include <GUI/View/Widgets/OrthogonalRegion/OrthogonalRegionSliceSelector.h>

// Qt
#include <QUndoCommand>

class QAction;
namespace ESPINA
{
  class OrthogonalRegion;
  class OrthogonalRegionSliceSelector;
  class ROISettings;
  class RestrictToolGroup;

  class OrthogonalROITool
  : public Tool
  {
    Q_OBJECT
  public:
    enum class Mode { FIXED, RESIZABLE };

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
                               RestrictToolGroup   *toolgroup);

    /** \brief OrthogonalROITool class virtual destructor.
     *
     */
    virtual ~OrthogonalROITool();

    virtual QList<QAction *> actions() const;

    /** \brief Sets ROI to be resized by this tool
     *
     *  If ROI is null then resize action is disabled
     */
    void setROI(ROISPtr roi);

    ROISPtr currentROI() const
    { return m_roi; }

    void setVisible(bool visible);

    void setColor(const QColor &color);

  signals:
    void roiDefined(ROISPtr);

  private slots:
    /** \brief Activates/Deactivates the tool
     *  \param[in] value true to activate the tool
     *
     *  When the tool is active, it will display the two actions available.
     *  When the tool is deactivated, Orthogonal ROI widget interaction will be disabled.
     */
    void setActive(bool value);


    /** \brief Modifies the tool and activates/deactivates the event handler for this tool.
     * \param[in] value true to activate tool and eventhandler, false to deactivate event handler.
     */
    void setDefinitionMode(bool value);

    /** \brief Update GUI status to be in accordance with the event handler state
     * \param[in] active event handler status
     */
    void onEventHandlerChanged();



    /** \brief Sets the operation mode of the Orthogonal ROI
     *
     *   \param[in] resizable true value allows ROI modification using a widget,
     *                        false value only diplays the ROI on the views
     */
    void setResizable(bool resizable);

    /** \brief Defines a new ROI based on the selection.
     * \param[in] selection selection containing the active channel and selected voxel.
     *
     */
    void defineROI(Selector::Selection selection);


    void updateBounds(Bounds bounds);

    void updateOrthogonalRegion();

  private:
    virtual void onToolEnabled(bool enabled);

    /** \brief Creates the rectangular region widget for the current roi
     *
     */
    void createOrthogonalWidget();

    /** \brief Removes the current widget.
     *
     */
    void destroyOrthogonalWidget();

    /** \brief Changes Orthogonal ROI action buttons visibility
     *
     *  \param[in] visibliy when visibility is true, action buttons are displayed
     */
    void setActionVisibility(bool visiblity);

    bool isResizable() const
    { return Mode::RESIZABLE == m_mode; }

    void setResizeMode(const Mode mode)
    { return setResizable(Mode::RESIZABLE == mode); }

    void updateRepresentationColor();

  private:
    ModelAdapterSPtr m_model;
    ViewManagerSPtr  m_viewManager;
    QUndoStack      *m_undoStack;

    QAction         *m_activeTool;
    QAction         *m_resizeROI;
    QAction         *m_applyROI;

    bool             m_enabled;

    ROISPtr          m_roi;
    Mode             m_mode;
    QColor           m_color;

    EspinaWidgetSPtr   m_widget;
    OrthogonalRegion *m_orWidget;

    EventHandlerSPtr   m_resizeHandler;
    PixelSelectorSPtr  m_defineHandler;

    using OrthognalSelectorSPtr = std::shared_ptr<OrthogonalRegionSliceSelector>;

    OrthognalSelectorSPtr m_sliceSelector;
    ROISettings          *m_settings;
  };

  using OrthogonalROIToolSPtr = std::shared_ptr<OrthogonalROITool>;

} // namespace ESPINA

#endif // ESPINA_ORTHOGONAL_ROI_H
