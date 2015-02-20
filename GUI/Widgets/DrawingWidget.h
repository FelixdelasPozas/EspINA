/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
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
 *
 */

#ifndef ESPINA_DRAWING_WIDGET_H
#define ESPINA_DRAWING_WIDGET_H

#include <GUI/EventHandlers/BrushPainter.h>
#include <GUI/Model/ModelAdapter.h>
#include <Support/ViewManager.h>

#include <QAction>
#include <QMap>

class ActionSelector;

namespace ESPINA
{
  class CategorySelector;
  class SliderAction;

  class DrawingWidget
  : public QObject
  {
    Q_OBJECT

  public:
    explicit DrawingWidget(ModelAdapterSPtr model, ViewManagerSPtr viewManager);

    virtual ~DrawingWidget();

    void setDrawingColor(const QColor &color);

    void setDrawingBorderColor(const QColor &color);

    void setBrushImage(const QImage &image);

    void clearBrushImage();

    CategoryAdapterSPtr selectedCategory() const;
    
    void setMaskProperties(const NmVector3 &spacing, const NmVector3 &origin=NmVector3());

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
    { m_circularPainterAction->setIcon(icon); }

    /** \brief Sets pencil 3d icon.
     * \param[in] icon QIcon object.
     *
     */
    void setPencil3DIcon(QIcon icon)
    { m_sphericalPainterAction->setIcon(icon); }

    /** \brief Sets pencil 2d text,
     * \param[in] text text for the control.
     *
     */
    void setPencil2DText(QString text)
    { m_circularPainterAction->setText(text); }

    /** \brief Sets pencil 3d text.
     * \param[in] text text for the control.
     *
     */
    void setPencil3DText(QString text)
    { m_sphericalPainterAction->setText(text); }

    void setCategory(CategoryAdapterSPtr category);

    /** \brief Disables eraser tool
     *
     */
    void setCanErase(bool value);

    QList<QAction *> actions() const;

    void stopDrawing();

  signals:
    void strokeStarted(BrushSPtr brush, RenderView *view);

    void maskPainted(BinaryMaskSPtr<unsigned char> mask);

  private slots:
    /** \brief Changes the selector for the operation.
     *
     */
    void changePainter(QAction *action);

    /** \brief Unsets the selector and disables the tool.
     *
     */
    void unsetPainter();

    /** \brief Changes the radius for the operation.
     * \param[in] value radius value.
     *
     */
    void changeRadius(int value);

    /** \brief Modifies the GUI when the radius changes.
     *
     */
    void radiusChanged(int);

    /** \brief Changes the opacity for the operation.
     * \param[in] value opacity slider value.
     */
    void changeOpacity(int value);

    /** \brief Modifies the GUI when the eraser mode changes.
     *
     */
    void setEraserMode(bool value);


    /** \brief Modifies the GUI when the drawing mode changes.
     * \param[in] value true if drawing.
     */
    void drawingModeChanged(bool value);

    /** \brief Updates the reference item if the selector is in use. Disables the selector otherwise.
     * \param[in] inUse true if selector is in use.
     */
    void selectorInUse(bool inUse);

    /** \brief Updates the tool when the category selector changes.
     * \param[in] unused unused value.
     */
    void categoryChanged(CategoryAdapterSPtr unused);



  private:
    void initPainters();

    QAction *registerPainter(const QIcon    &icon,
                             const QString  &description,
                             MaskPainterSPtr painter);

    QAction *registerBrush(const QIcon     &icon,
                           const QString   &description,
                           BrushPainterSPtr painter);

    /** \brief Helper method to hide/show the controls of the tool.
     *
     */
    void setControlVisibility(bool visible);

  private:
    ViewManagerSPtr m_viewManager;

    BrushPainterSPtr m_circularPainter;
    BrushPainterSPtr m_sphericalPainter;
    MaskPainterSPtr  m_contourPainter;
    MaskPainterSPtr  m_currentPainter;

    ActionSelector   *m_painterSelector;
    CategorySelector *m_categorySelector;
    QMap<QAction *, MaskPainterSPtr> m_painters;

    SliderAction *m_radiusWidget;
    SliderAction *m_opacityWidget;
    QAction      *m_eraserWidget;

    QAction *m_circularPainterAction;
    QAction *m_sphericalPainterAction;

    bool m_showCategoryControls;
    bool m_showRadiusControls;
    bool m_showOpacityControls;
    bool m_showEraserControls;
    bool m_hasEnteredEraserMode;
  };
}

#endif // ESPINA_DRAWING_WIDGET_H
