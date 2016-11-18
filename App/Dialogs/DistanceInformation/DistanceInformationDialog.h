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

#ifndef DISTANCEINFORMATIONDIALOG_H_
#define DISTANCEINFORMATIONDIALOG_H_

// Qt
#include <QDebug>
#include <QDialog>

// ESPINA
#include <Core/Types.h>
#include <GUI/Model/ModelAdapter.h>
#include <Support/Context.h>
#include "DistanceInformationOptionsDialog.h"

// VTK
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

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
      explicit DistanceInformationDialog(SegmentationAdapterList selectedSegmentations,
                                         DistanceInformationOptionsDialog::Options options,
                                         Support::Context &context);

      /** \brief DistanceInformationDialog class destructor.
       *
       */
      virtual ~DistanceInformationDialog()
      {};

    private:
      /** \brief Calculate distance between the two given segmentations.
       * \param[in] first segmentation.
       * \param[in] second segmentation.
       *///TODO
      const Nm centroidDistance(SegmentationAdapterPtr first,
                                SegmentationAdapterPtr second);

      /** \brief Calculate distance between the two given segmentations.
       * \param[in] first segmentation.
       * \param[in] second segmentation.
       * \param[out] cached distance
       *///TODO
      const bool isDistanceCached(SegmentationAdapterPtr first,
                                 SegmentationAdapterPtr second,
                                 Nm *distance = nullptr);

      /** \brief Calculate distance between the two given segmentations.
       * \param[in] first segmentation.
       * \param[in] second segmentation.
       *///TODO
      const NmVector3 getCentroid(SegmentationAdapterPtr seg);

      /** \brief Calculate distance between the two given segmentations.
       * \param[in] first segmentation.
       * \param[in] second segmentation.
       *///TODO
      const Nm meshDistance(SegmentationAdapterPtr first,
                            SegmentationAdapterPtr second);


      QHash<SegmentationAdapterPtr, QHash<SegmentationAdapterPtr, Nm>> m_cachedDistances;
      SegmentationAdapterList m_selectedSegmentations;
      Support::Context &m_context;
      DistanceInformationOptionsDialog::Options m_options;
      const double m_ERROR_VAL;
  };

} // namespace ESPINA

#endif /* ESPINA_DISTANCEINFORMATIONDIALOG_H_ */
