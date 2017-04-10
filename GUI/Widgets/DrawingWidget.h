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

#include <GUI/EspinaGUI_Export.h>

// ESPINA
#include <GUI/EventHandlers/BrushPainter.h>
#include <GUI/EventHandlers/ContourPainter.h>
#include <GUI/Model/ModelAdapter.h>
#include <GUI/Types.h>
#include "NumericalInput.h"
#include "CategorySelector.h"

// Qt
#include <QAction>
#include <QMap>

class QPushButton;
class QHBoxLayout;
class QSettings;
class QWidgetAction;

namespace ESPINA
{
  namespace GUI
  {
    namespace Widgets
    {
      class CategorySelector;
      class NumericalInput;

      class EspinaGUI_EXPORT DrawingWidget
      : public QWidget
      {
        Q_OBJECT

      public:
        explicit DrawingWidget(View::ViewState &viewState, ModelAdapterSPtr model, QWidget *parent = 0);

        virtual ~DrawingWidget();

        MaskPainterSPtr painter() const;

        void setDrawingColor(const QColor &color);

        void setDrawingBorderColor(const QColor &color);

        void setBrushImage(const QImage &image);

        void clearBrushImage();

        CategoryAdapterSPtr selectedCategory();

        void setMaskProperties(const NmVector3 &spacing, const NmVector3 &origin=NmVector3());

        /** \brief Shows/hides category controls.
         * \param[in] value true for show.
         *
         */
        void showCategoryControls(bool value);

        /** \brief Shows/hides radius controls.
         * \param[in] value true for show.
         *
         */
        void showRadiusControls(bool value);

        /** \brief Shows/hides opacity controls.
         * \param[in] value true for show.
         *
         */
        void showOpacityControls(bool value);

        /** \brief Shows/hides eraser controls.
         * \param[in] value true for show.
         *
         */
        void showEraserControls(bool value);

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

        void setCategory(CategoryAdapterSPtr category);

        /** \brief Disables eraser tool
         *
         */
        void setCanErase(bool value);

        void abortOperation();

        void stopDrawing();

        void setEnabled(bool value);

        bool enabled()
        { return m_enabled; }

        /** \brief Restores the internal values from the settings object.
         * \param[in] settings.
         *
         */
        void restoreSettings(std::shared_ptr<QSettings> settings);

        /** \brief Saves the internal values to the settings object.
         * \param[in] settings.
         *
         */
        void saveSettings(std::shared_ptr<QSettings> settings);

        /** \brief Sets if the widget must manage the brushes actors.
         * \param[in] value true to let the widget manage the actors and false otherwise.
         *
         */
        void setManageActors(bool value);

      signals:
        void strokeStarted(BrushPainter *painter, RenderView *view);

        void maskPainted(BinaryMaskSPtr<unsigned char> mask);

        void categoryChanged(CategoryAdapterSPtr category);

        void painterChanged(MaskPainterSPtr painter);

      private slots:
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

        /** \brief Updates the tool when the category selector changes.
         * \param[in] unused unused value.
         */
        void onCategoryChange(CategoryAdapterSPtr unused);

        /** \brief Updates the UI when the drawing mode gets changed.
         *
         */
        void onDrawingModeChange(DrawingMode mode);

        /** \brief Helper method to update UI.
         *
         */
        void updateVisibleControls();

        void onButtonClicked(bool value);

        void onEventHandlerInUse(bool value);

      private:
        void initPainters();

        void initCategoryWidget();

        void initEraseWidget();

        void initRadiusWidget();

        void initOpacityWidget();

        void addWidget(QWidget *widget);

        QPushButton *registerPainter(const QString  &icon,
                                     const QString  &description,
                                     MaskPainterSPtr painter);

        QPushButton *registerBrush(const QString   &icon,
                                   const QString   &description,
                                   BrushPainterSPtr painter);

        bool displayBrushControls() const;

        bool displayContourControls() const;

        void changePainter(MaskPainterSPtr painter);

      private:
        View::ViewState &m_viewState;

        BrushPainterSPtr   m_circularPainter;
        BrushPainterSPtr   m_sphericalPainter;
        ContourPainterSPtr m_contourPainter;
        MaskPainterSPtr    m_currentPainter;

        CategorySelector m_categorySelector;
        NumericalInput   m_radiusWidget;
        NumericalInput   m_opacityWidget;
        QPushButton      *m_eraserWidget;

        QPushButton *m_circularPainterAction;
        QPushButton *m_sphericalPainterAction;
        QPushButton *m_contourPainterAction;

        QMap<QPushButton *, MaskPainterSPtr> m_painters;

        bool m_showCategoryControls;
        bool m_showRadiusControls;
        bool m_showOpacityControls;
        bool m_showEraserControls;

        bool m_enabled;

        // Shared painter widgets needs to retain it's values.
        int m_brushRadius;
        int m_contourDistance;
        int m_opacity;

        using TemporalPrototypesSPtr = Representations::Managers::TemporalPrototypesSPtr;

        TemporalPrototypesSPtr m_contourWidgetfactory;
      };
    }
  }
}

#endif // ESPINA_DRAWING_WIDGET_H
