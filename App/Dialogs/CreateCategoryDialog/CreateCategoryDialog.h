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

#ifndef APP_DIALOGS_CREATECATEGORYDIALOG_CREATECATEGORYDIALOG_H_
#define APP_DIALOGS_CREATECATEGORYDIALOG_CREATECATEGORYDIALOG_H_

// ESPINA
#include "ui_CreateCategoryDialog.h"
#include <Core/Utils/Vector3.hxx>
#include <GUI/Widgets/HueSelector.h>

// Qt
#include <QDialog>

namespace ESPINA
{
  /** \class CreateCategoryDialog
   * \brief Dialog to create a category or sub-category.
   *
   */
  class CreateCategoryDialog
  : public QDialog
  , private Ui::CreateCategoryDialog
  {
      Q_OBJECT
    public:
      /** \brief CreateCategory class constructor.
       *
       */
      explicit CreateCategoryDialog();

      /** \brief CreateCategoryDialog class virtual destructor.
       *
       */
      virtual ~CreateCategoryDialog()
      {};

      /** \brief Sets the text of the operation field.
       * \param[in] text Description of the operation.
       *
       */
      void setOperationText(const QString &text);

      /** \brief Sets the name of the category to create.
       * \param[in] name Category name.
       *
       */
      void setCategoryName(const QString &name);

      /** \brief Sets a color in the color selection widget.
       * \param[in] color QColor reference.
       *
       */
      void setColor(const QColor &color);

      /** \brief Sets the default ROI values.
       * \param[in] values ROI values in X,Y,Z as a vector.
       *
       */
      void setROI(const Vector3<long long> &values);

      /** \brief Returns the contents of the category name field.
       *
       */
      const QString categoryName() const;

      /** \brief Returns the selected category color.
       *
       */
      const QColor categoryColor() const;

      /** \brief Returns the category ROI values as a vector.
       *
       */
      const Vector3<long long> ROI() const;

    private slots:
      /** \brief Updates the hue value and the spinbox value.
       * \param[in] h Hue value.
       * \param[in] s Saturation value.
       * \param[in] v Brightness value.
       *
       */
      void onSelectorValueChanged(int h, int s, int v);

      /** \brief Updates the hue value and the selector value.
       *
       */
      void onSpinboxValueChanged(int h);

    private:
      HueSelector *m_selector; /** hue selector widget. */
  };
} // ESPINA

#endif // APP_DIALOGS_CREATECATEGORYDIALOG_CREATECATEGORYDIALOG_H_
