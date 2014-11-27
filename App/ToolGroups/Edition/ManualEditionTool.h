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
#include <GUI/Selectors/CircularBrushSelector.h>
#include <GUI/Selectors/SphericalBrushSelector.h>
#include <GUI/Model/ModelAdapter.h>
#include <GUI/Widgets/ActionSelector.h>
#include <GUI/Selectors/Selector.h>
#include <GUI/Selectors/BrushSelector.h>
#include <GUI/Widgets/CategorySelector.h>
#include <Support/Widgets/Tool.h>
#include <Support/ViewManager.h>

class QAction;

namespace ESPINA
{
  class SliderAction;

  class ManualEditionTool
  : public Tool
  {
    Q_OBJECT
  public:
    /** \brief ManualEditionTool class constructor.
     * \param[in] model model adapter smart pointer.
     * \param[in] viewManager view manager smart pointer.
     *
     */
    ManualEditionTool(ModelAdapterSPtr model,
                      ViewManagerSPtr  viewManager);

    /** \brief ManualEditionTool class virtual destructor.
     *
     */
    virtual ~ManualEditionTool();

    /** \brief Implements Tool::setEnabled().
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

    /** \brief Aborts current operation.
     *
     */
    virtual void abortOperation();

    /** \brief Shows/hides category controls.
     * \param[in] value true for show.
     *
     */
    void showCategoryControls(bool value)
    { m_showCategoryControls = value; }

    /** \brief Shows/hides radius controls.
     * \param[in] value true for show.
     *
     */
    void showRadiusControls(bool value)
    { m_showRadiusControls = value; }

    /** \brief Shows/hides opacity controls.
     * \param[in] value true for show.
     *
     */
    void showOpacityControls(bool value)
    { m_showOpacityControls = value; }

    /** \brief Shows/hides eraser controls.
     * \param[in] value true for show.
     *
     */
    void showEraserControls(bool value)
    { m_showEraserControls = value; }

    /** \brief Returns true if the category controls are visible, false otherwise.
     *
     */
    bool categoryControlsVisibility()
    { return m_showCategoryControls; }

    /** \brief Returns true if the radius controls are visible, false otherwise.
     *
     */
    bool radiusControlsVisibility()
    { return m_showRadiusControls; }

    /** \brief Returns true if the opacity controls are visible, false otherwise.
     *
     */
    bool opacityControlsVisibility()
    { return m_showOpacityControls; }

    /** \brief Returns true if the eraser controls are visible, false otherwise.
     *
     */
    bool eraserControlsVisibility()
    { return m_showEraserControls; }

    /** \brief Sets pencil 2d icon.
     * \param[in] icon QIcon object.
     *
     */
    void setPencil2DIcon(QIcon icon)
    { m_discTool->setIcon(icon); }

    /** \brief Sets pencil 3d icon.
     * \param[in] icon QIcon object.
     *
     */
    void setPencil3DIcon(QIcon icon)
    { m_sphereTool->setIcon(icon); }

    /** \brief Sets pencil 2d text,
     * \param[in] text text for the control.
     *
     */
    void setPencil2DText(QString text)
    { m_discTool->setText(text); }

    /** \brief Sets pencil 3d text.
     * \param[in] text text for the control.
     *
     */
    void setPencil3DText(QString text)
    { m_sphereTool->setText(text); }

  signals:
    void brushModeChanged(BrushSelector::BrushMode);
    void stopDrawing(ViewItemAdapterPtr item, bool eraseModeEntered);
    void stroke(CategoryAdapterSPtr, BinaryMaskSPtr<unsigned char>);

  public slots:
    /** \brief Emits a stroke signal with the mask and the selected category.
     * \param[in] selection list of SelectionItem <mask, neuroitemadapter> pairs.
     *
     */
    virtual void drawStroke(Selector::Selection selection);

    /** \brief Modifies the GUI when the radius changes.
     *
     */
    virtual void radiusChanged(int);

    /** \brief Modifies the GUI when the drawing mode changes.
     * \param[in] value true if drawing.
     */
    virtual void drawingModeChanged(bool value);

    /** \brief Updates the reference item for the tool.
     *
     */
    virtual void updateReferenceItem();

  protected slots:
    /** \brief Changes the selector for the operation.
     *
     */
    virtual void changeSelector(QAction *);

    /** \brief Changes the radius for the operation.
     * \param[in] value radius value.
     *
     */
    virtual void changeRadius(int value);

    /** \brief Changes the opacity for the operation.
     * \param[in] value opacity slider value.
     */
    virtual void changeOpacity(int value);

    /** \brief Updates the reference item if the selector is in use. Disables the selector otherwise.
     * \param[in] inUse true if selector is in use.
     */
    virtual void selectorInUse(bool inUse);

    /** \brief Unsets the selector and disables the tool.
     *
     */
    virtual void unsetSelector();

    /** \brief Updates the tool when the category selector changes.
     * \param[in] unused unused value.
     */
    virtual void categoryChanged(CategoryAdapterSPtr unused);

  private:
    /** \brief Helper method to hide/show the controls of the tool.
     *
     */
    void setControlVisibility(bool visible);

    /** \brief Returns the category of the current item.
     *
     */
    CategoryAdapterSPtr currentReferenceCategory();

  private slots:
    /** \brief Modifies the GUI when the eraser mode changes.
     *
     */
    void setEraserMode(bool value);

  protected:
    ModelAdapterSPtr m_model;
    ViewManagerSPtr  m_viewManager;

    BrushSelectorSPtr m_circularBrushSelector;
    BrushSelectorSPtr m_sphericalBrushSelector;
    BrushSelectorSPtr m_currentSelector;

    ActionSelector   *m_drawToolSelector;
    CategorySelector *m_categorySelector;
    QMap<QAction *, SelectorSPtr> m_drawTools;

    SliderAction *m_radiusWidget;
    SliderAction *m_opacityWidget;
    QAction      *m_eraserWidget;

    QAction *m_discTool;
    QAction *m_sphereTool;

    bool m_showCategoryControls;
    bool m_showRadiusControls;
    bool m_showOpacityControls;
    bool m_showEraserControls;

    bool m_enabled;
    bool m_hasEnteredEraserMode;
  };

  using ManualEditionToolPtr = ManualEditionTool *;
  using ManualEditionToolSPtr = std::shared_ptr<ManualEditionTool>;

} // namespace ESPINA

#endif // ESPINA_MANUAL_EDITION_TOOL_H_
