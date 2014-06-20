/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2014 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

 This program is free software: you can redistribute it and/or modify
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

#ifndef ESPINA_PROBLEM_LIST_DIALOG_H_
#define ESPINA_PROBLEM_LIST_DIALOG_H_

// Qt
#include <QDialog>
#include "ui_ProblemListDialog.h"

namespace EspINA
{
  /* \brief Enumeration of problem severity.
   *
   */
  enum class Severity : char { CRITICAL = 0, WARNING = 1, INFORMATION = 2 };

  /* \brief Struct that contains problem description
   *
   */
  struct Problem
  {
      QString element;
      Severity severity;
      QString message;
      QString suggestion;

      Problem(QString inputElement, Severity inputSeverity, QString inputMessage, QString inputSuggestion)
      : element(inputElement), severity(inputSeverity), message(inputMessage), suggestion(inputSuggestion) {};

      // required by qRegisterMetaType
      Problem(): element(QString()), severity(Severity::CRITICAL), message(QString()), suggestion(QString()) {};
  };


  using ProblemList = QList<struct Problem>;
  
  class ProblemListDialog
  : public QDialog
  , public Ui::ProblemListDialog
  {
    public:
      /* \brief ProblemListDialog class constructor.
       * \param[in] problems, list of problem descriptions as Problem structs.
       *
       */
      explicit ProblemListDialog(ProblemList problemsList);
      virtual ~ProblemListDialog();

    private:
  };

} // namespace EspINA

#endif // ESPINA_PROBLEM_LIST_DIALOG_H_
