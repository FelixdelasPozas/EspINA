/*
 * Copyright (C) 2016, Rafael Juan Vicente Garcia <rafaelj.vicente@gmail.com>
 *
 * This file is part of ESPINA.
 *
 * ESPINA is free software: you can redistribute it and/or modify
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

#ifndef APP_PANELS_SEGMENTATIONPROPERTIES_ISSUEPROPERTY_H_
#define APP_PANELS_SEGMENTATIONPROPERTIES_ISSUEPROPERTY_H_

// ESPINA
#include "ui_IssueProperty.h"

// Qt
#include <QWidget>

namespace ESPINA
{
  /** \class IssueProperty
   * \brief Implements the widget for an issue.
   *
   */
  class IssueProperty
  : public QWidget
  , private Ui::IssueProperty
  {
    public:
      /** \brief IssueProperty class constructor.
       * \param[in] warning text.
       * \param[in] suggestion text.
       * \param[in] icon image to show.
       * \param[in] parent pointer of the QWidget parent of this one.
       *
       */
      IssueProperty(const QString &warning, const QString &suggestion, QWidget *parent = nullptr, QPixmap icon = QPixmap());

      /** \brief IssueProperty class virtual destructor.
       *
       */
      virtual ~IssueProperty()
      {};
  };

} /* namespace ESPINA */

#endif /* APP_PANELS_SEGMENTATIONPROPERTIES_ISSUEPROPERTY_H_ */
