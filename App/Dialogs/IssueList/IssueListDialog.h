/*

 Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef ESPINA_ISSUE_LIST_DIALOG_H_
#define ESPINA_ISSUE_LIST_DIALOG_H_

// Qt
#include <QDialog>
#include "ui_IssueListDialog.h"
#include <Extensions/Issues/Issues.h>

// C++
#include <cstdint>

namespace ESPINA
{
  class IssueListDialog
  : public QDialog
  , public Ui::IssueListDialog
  {
    public:
      /** \brief IssueListDialog class constructor.
       * \param[in] issuesList, list of problem descriptions as Issue structs.
       *
       */
      explicit IssueListDialog(Extensions::IssueList issuesList);

      /** \brief IssueListDialog class virtual destructor.
       *
       */
      virtual ~IssueListDialog();
  };

  class IssueTableWidgetItem
  : public QTableWidgetItem
  {
    public:
      /** \brief IssueTableWidgetItem class constructor.
       * \param[in] text, data of the item.
       *
       */
      IssueTableWidgetItem(const QString &text)
      : QTableWidgetItem(text)
      {}

      /** \brief IssueTableWidget class virtual destructor.
       *
       */
      virtual ~IssueTableWidgetItem()
      {};

      /** \brief less-than operator for QTableWidgetItem sorting.
       *
       */
      virtual bool operator<(const QTableWidgetItem & other) const override
      {
        auto ownData = this->data(Qt::DisplayRole).toString();
        auto otherData = other.data(Qt::DisplayRole).toString();

        if(ownData.length() < otherData.length())
          return true;

        auto ownStrings = ownData.split(" ");
        auto otherStrings = otherData.split(" ");

        if(ownStrings[1].toInt() < otherStrings[1].toInt())
          return true;

        return false;
      }
  };

} // namespace ESPINA

#endif // ESPINA_ISSUE_LIST_DIALOG_H_
