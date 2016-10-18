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

#ifndef APP_DIALOGS_DISTANCEINFORMATION_DISTANCEINFORMATIONDIALOG_H_
#define APP_DIALOGS_DISTANCEINFORMATION_DISTANCEINFORMATIONDIALOG_H_

// Qt
#include <QDebug>
#include <QDialog>

// ESPINA
#include <Core/Types.h>
#include <GUI/Model/ModelAdapter.h>
#include <Support/Context.h>
#include "DistanceInformationOptionsDialog.h"

using DistOpts = ESPINA::DistanceInformationOptionsDialog::DistanceInformationOptions;

namespace ESPINA
{
  /** \class DistanceInformationDialog
    * \brief Shows the report dialog.
    *
    */
  class DistanceInformationDialog
  : public QDialog
  {
    public:
      /** \brief DistanceInformationDialog class constructor.
       * \param[in] parent widget.
       * \param[in] parent widget.
       */
      explicit DistanceInformationDialog(SegmentationAdapterList input, DistOpts options, Support::Context &context);

      /** \brief DistanceInformationDialog class destructor.
       *
       */
      virtual ~DistanceInformationDialog()
      {};

    private:
      SegmentationAdapterList input;
      Support::Context &context;
      DistOpts options;
  };

} // namespace ESPINA

#endif /* ESPINA_DISTANCEINFORMATIONDIALOG_H_ */
