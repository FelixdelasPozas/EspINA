/*

 Copyright (C) 2018 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef GUI_DIALOGS_COLORENGINERANGEDEFINITIONDIALOG_COLORENGINERANGEDEFINITIONDIALOG_H_
#define GUI_DIALOGS_COLORENGINERANGEDEFINITIONDIALOG_COLORENGINERANGEDEFINITIONDIALOG_H_

#include "EspinaGUI_Export.h"

// ESPINA
#include <ui_ColorEngineRangeDefinitionDialog.h>
#include <GUI/Dialogs/DefaultDialogs.h>
#include <GUI/Utils/ColorRange.h>


// Qt
#include <QDialog>

namespace ESPINA
{
  class HueSelector;

  namespace GUI
  {
    /** \class ColorEngineRangeDefinitionDialog
     * \brief Information color engine color range definition dialog.
     *
     */
    class EspinaGUI_EXPORT ColorEngineRangeDefinitionDialog
    : public QDialog
    , private Ui::ColorEngineRangeDefinitionDialog
    {
        Q_OBJECT
      public:
        /** \class Position
         * \brief Widget position on the view.
         *
         */
        enum class Position: char { TOP_RIGHT = 0, TOP_LEFT, BOTTOM_RIGHT, BOTTOM_LEFT };

        /** \class Orientation
         * \brief Widget orientation on the view.
         *
         */
        enum class Orientation: char { HORIZONTAL = 0, VERTICAL };

        /** \class TextPosition
         * \brief Widget title position regarding the scalar bar.
         *
         */
        enum class TextPosition: char { PRECEDE = 0, SUCCEED };

        /** \brief ColorEngineRangeDefinitionDialog class constructor.
         * \param[in] parent Raw pointer of the widget parent of this one.
         * \param[in] flags QDialog window flags.
         *
         */
        explicit ColorEngineRangeDefinitionDialog(QWidget *parent = DefaultDialogs::defaultParentWidget(),
                                                  Qt::WindowFlags flags = Qt::WindowFlags());

        /** \brief ColorEngineRangeDefinitionDialog class virtual destructor.
         *
         */
        virtual ~ColorEngineRangeDefinitionDialog()
        {};

        /** \brief Returns the hue value of minimum color.
         *
         */
        const int minimum() const;

        /** \brief Returns the hue value of maximum color.
         *
         */
        const int maximum() const;

        /** \brief Returns true if the user has selected to show the range in the views and false otherwise.
         *
         */
        const bool showRangeInViews() const;

        /** \brief Sets the minimum and maximum colors of the range.
         * \param[in] hueMinimum Hue value of minimum color.
         * \param[in] hueMaximum Hue value of maximum color.
         *
         */
        void setRangeColors(const int hueMinimum, const int hueMaximum);

        /** \brief Sets the state of 'show in views' checkbox.
         * \param[in] value True to check and false otherwise.
         *
         */
        void setShowRangeInViews(const bool value);

        /** \brief Sets the widget position on the view.
         * \param[in] position Position enum value to int.
         *
         */
        void setWidgetPosition(const Position position);

        /** \brief Returns the widget position as a Position enum.
         *
         */
        const Position getWidgetPosition() const;

        /** \brief Sets the widget orientation on the view.
         * \param[in] orientation Orientation enum value.
         *
         */
        void setWidgetOrientation(const Orientation orientation);

        /** \brief Returns the widget orientation on the view as a Orientation enum value.
         *
         */
        const Orientation getWidgetOrientation() const;

        /** \brief Sets the title position as a TextPosition enum value.
         * \param[in] position TextPosition enum value.
         *
         */
        void setTitlePosition(const TextPosition position);

        /** \brief Returns the title text position on the widget as a TextPosition enum value.
         *
         */
        const TextPosition getTitlePosition() const;

        /** \brief Sets the number of labels of the widget.
         * \param[in] labelsNum unsigned integer.
         *
         */
        void setNumberOfLabels(const int labelsNum);

        /** \brief Returns the number of labels of the widget.
         *
         */
        const int getNumberOfLabels() const;

        /** \brief Sets the relative width of the widget.
         * \param[in] width Double in [0,1]
         *
         */
        void setWidthRatio(const double width);

        /** \brief Returns the widget width ratio.
         *
         */
        const double getWidthRatio() const;

        /** \brief Sets the relative height of the widget.
         * \param[in] height Double in [0,1].
         *
         */
        void setHeightRatio(const double height);

        /** \brief Returns the widget height ratio.
         *
         */
        const double getHeightRatio() const;

        /** \brief Sets the ratio of the bar relative to widget size.
         * \param[in] ratio Double in [0,1].
         */
        void setBarRatio(const double ratio);

        /** \brief Returns the ratio of the bar relative to widget size.
         *
         */
        const double getBarRatio() const;

        /** \brief Sets the number of decimals to show of the data.
         * \param[in] decimals unsigned integer.
         *
         */
        void setNumberOfDecimals(const int decimals);

        /** \brief Returns the number of decimals of the data to show.
         *
         */
        const int getNumberOfDecimals() const;

      signals:
        void widgetsEnabled(int value);
        void widgetsPropertiesModified();

      private slots:
        /** \brief Updates the UI with a new selected color for the custom range.
         * \param[in] value HUE of new color.
         *
         */
        void onColorModified(int value);

        /** \brief Modifies the UI depending if the widgets are enabled or not.
         *
         */
        void onWidgetsEnabled(int value);

        /** \brief Swaps height and width values when orientation changes.
         * \param[in] value New index of orientation combo box.
         *
         */
        void onOrientationChanged(int value);

      private:
        /** \brief Helper method to fill the color ranges and other widgtes in the UI.
         *
         */
        void createWidgets();

        /** \brief Helper method to connect the UI widgets signals to its slots.
         *
         */
        void connectSignals();

        HueSelector *m_fromSelector; /** from color hue selector for custom range. */
        HueSelector *m_toSelector;   /** to color hue selector for custom range.   */
    };
  
  } // namespace GUI
} // namespace ESPINA

#endif // GUI_DIALOGS_COLORENGINERANGEDEFINITIONDIALOG_COLORENGINERANGEDEFINITIONDIALOG_H_
