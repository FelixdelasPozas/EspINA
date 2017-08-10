/*

 Copyright (C) 2017 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef APP_DIALOGS_SKELETONSTROKEDEFINITION_STROKEDEFINITIONDIALOG_H_
#define APP_DIALOGS_SKELETONSTROKEDEFINITION_STROKEDEFINITIONDIALOG_H_

// ESPINA
#include <Core/Analysis/Data/SkeletonDataUtils.h>
#include <GUI/Dialogs/DefaultDialogs.h>
#include <GUI/Types.h>

// Qt
#include <QDialog>
#include <ui_StrokeDefinitionDialog.h>

class QCloseEvent;

namespace ESPINA
{
  /** \class StrokeDefinitionDialog
   * \brief Implements a dialog to define a stroke and its properties.
   *
   */
  class StrokeDefinitionDialog
  : public QDialog
  , private Ui::StrokeDefinitionDialog
  {
      Q_OBJECT
    public:
      /** \brief StrokeDefinitionDialog class constructor.
       * \param[inout] strokes skeleton strokes information reference.
       * \param[in] category category of the strokes.
       * \param[in] parent raw pointer of the QWidget parent of this one.
       * \param[in] flags QDialog window flags.
       *
       */
      explicit StrokeDefinitionDialog(Core::SkeletonStrokes &strokes, const CategoryAdapterSPtr category, QWidget *parent = GUI::DefaultDialogs::defaultParentWidget(), Qt::WindowFlags flags = Qt::WindowFlags());

      /** \brief StrokeDefinitionDialog class virtual destructor.
       *
       */
      virtual ~StrokeDefinitionDialog()
      {}

    protected:
      virtual void closeEvent(QCloseEvent *event) override;

    private slots:
      /** \brief Adds a new stroke to the list.
       *
       */
      void onAddButtonPressed();

      /** \brief Removes the current stroke from the list.
       *
       */
      void onRemoveButtonPressed();

      /** \brief Updates the stroke properties when the user selects a new stroke definition.
       * \param[in] row current selected stroke index.
       *
       */
      void onStrokeChanged(int row);

      /** \brief Updates the hue value in the UI and in the stroke definition.
       * \param[in] hueValue new hue value.
       *
       */
      void onHueChanged(int hueValue);

      /** \brief Updates the item text in the UI and in the stroke definition.
       * \param[in] text new text.
       *
       */
      void onTextChanged(const QString &text);

      /** \brief Updates the line type in the UI and in the stroke definition.
       * \param[in] index index of the new line type.
       *
       */
      void onTypeChanged(int index);

      /** \brief Updates the validity of the measure in the stroke definition.
       *
       */
      void onValidCheckChanged();

      /** \brief Enables/Disables the properties fields depending on the value.
       * \param[in] true to enable and false to disable.
       *
       */
      void enableProperties(bool value);

      /** \brief Set/Unsets the color to the category color.
       * \param[in] unused
       *
       */
      void onCategoryColorChecked(int unused);

    private:
      /** \brief Connects the UI signals with the slots.
       *
       */
      void connectSignals();

      /** \brief Updates the list widget with the stroke definitions.
       *
       */
      void updateStrokeList();

      /** \brief Updates the stroke properties values.
       *
       */
      void updateStrokeProperties();

      Core::SkeletonStrokes &m_strokes;       /** stroke definitions map.                                       */
      const int              m_categoryColor; /** hue value of the color of the category the strokes belong to. */
  };

} // namespace ESPINA

#endif // APP_DIALOGS_SKELETONSTROKEDEFINITION_STROKEDEFINITIONDIALOG_H_
