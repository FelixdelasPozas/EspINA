/*
 *
 *    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ESPINA_VISUALIZATION_STATE_H
#define ESPINA_VISUALIZATION_STATE_H

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include <Core/Types.h>
#include <Core/Analysis/Extensions.h>

// ITK
#include <itkLabelImageToShapeLabelMapFilter.h>
#include <itkStatisticsLabelObject.h>

namespace ESPINA
{
  class EspinaGUI_EXPORT VisualizationState
  : public Core::SegmentationExtension
  {
    public:
      static const Type TYPE;

    public:
      /** \brief VisualizationState class virtual destructor.
       *
       */
      virtual ~VisualizationState();

      virtual Type type() const
      { return TYPE; }

      virtual const TypeList dependencies() const
      { return TypeList(); }

      virtual bool validCategory(const QString& classificationName) const
      { return true; }

      virtual const InformationKeyList availableInformation() const;

      virtual QVariant information(const Key &tag) const;

      /** \brief Sets the state of a representation.
       * \param[in] representation representation name.
       * \param[in] state string with the state of the representation.
       *
       */
      void setState(const QString& representation, const State& state);

      /** \brief Returns the state of the representation as a string.
       *
       */
      QString representationState(const QString& representation);

    private:
      /** \brief VisualizationState class constructor.
       * \param[in] infoCache data cache of the extension.
       *
       */
      explicit VisualizationState(const State &state);

      QMap<QString, QString> m_state;
  };

  using VisualizationStateSPtr = std::shared_ptr<VisualizationState>;

}// namespace ESPINA


#endif // ESPINA_VISUALIZATION_STATE_H
