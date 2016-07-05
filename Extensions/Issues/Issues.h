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

#ifndef ESPINA_EXTENSIONS_ISSUES_H
#define ESPINA_EXTENSIONS_ISSUES_H

#include "Extensions/EspinaExtensions_Export.h"

// Qt
#include <QString>
#include <QList>

// C++
#include <memory>

namespace ESPINA
{
  namespace Extensions
  {
    /** \class Issue
     * \brief Contains issue information.
     *
     */
    class EspinaExtensions_EXPORT Issue
    {
    public:
      /** \brief Enumeration of issue severity.
       *
       */
      enum class Severity : std::int8_t
      {
        NONE        = 0,
        INFORMATION = 1,
        WARNING     = 2,
        CRITICAL    = 3
      };

      static QString INFORMATION_TAG;
      static QString WARNING_TAG;
      static QString CRITICAL_TAG;

      /** \brief Issue class empty constructor.
       *
       * NOTE: Required by qRegisterMetatype.
       *
       */
      explicit Issue()
      : m_severity(Severity::CRITICAL)
      {};

      /** \brief Issue class constructor.
       *
       */
      explicit Issue(const QString &itemName, const Severity severity, const QString &description, const QString &suggestion = QString())
      : m_itemName(itemName)
      , m_severity(severity)
      , m_description(description)
      , m_suggestion(suggestion)
      {};

      /** \brief Returns the name of the item that has the problem.
       *
       */
      QString displayName() const;

      /** \brief Returns the severity level of the problem.
       *
       */
      Severity severity() const;

      /** \brief Returns a textual description of the problem.
       *
       */
      QString description() const;

      /** \brief Returns a suggestion on how to fix the problem.
       *
       */
      QString suggestion() const;

    private:
      QString m_itemName;    /** Element that has the issue.            */
      Severity m_severity;   /** Severity of the issue.                 */
      QString m_description; /** Description of the issue.              */
      QString m_suggestion;  /** Suggestion of a solution to the issue. */

    };

    using IssueSPtr = std::shared_ptr<Issue>;
    using IssueList = QList<IssueSPtr>;
  }
}

#endif // ESPINA_EXTENSIONS_ISSUES_H
