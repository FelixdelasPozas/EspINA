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
  
  class IssueProperty: public QWidget, private Ui::IssueProperty
  {
    Q_OBJECT
    public:

      /** \brief IssueProperty class constructor.
       * \param[in] warning text.
       * \param[in] suggestion text.
       * \param[in] parent pointer of the QWidget parent of this one.
       */
      IssueProperty(QString warning, QString suggestion, QWidget *parent);

      /** \brief IssueProperty class constructor.
       * \param[in] icon image to show.
       * \param[in] warning text.
       * \param[in] suggestion text.
       * \param[in] parent pointer of the QWidget parent of this one.
       */
      IssueProperty(QPixmap icon, QString warning, QString suggestion,
          QWidget *parent);

      /** \brief IssueProperty class destructor.
       */
      virtual ~IssueProperty();
  };

} /* namespace ESPINA */

#endif /* APP_PANELS_SEGMENTATIONPROPERTIES_ISSUEPROPERTY_H_ */
