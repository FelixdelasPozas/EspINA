/*
 *    Copyright (C) 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.
 *
 *    ESPINA is free software: you can redistribute it and/or modify
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

#ifndef ESPINA_GUI_INFORMATION_COLOR_ENGINE_H
#define ESPINA_GUI_INFORMATION_COLOR_ENGINE_H

#include <GUI/EspinaGUI_Export.h>

// ESPINA
#include <Core/Analysis/Extensions.h>
#include <GUI/Types.h>
#include <GUI/ColorEngines/ColorEngine.h>

namespace ESPINA
{
  namespace GUI
  {
    namespace ColorEngines
    {
      /** \class InformationColorEngine
       * \brief Colors the segmentations according to some information key and value.
       *
       */
      class EspinaGUI_EXPORT InformationColorEngine
      : public ColorEngine
      {
        public:
          /** \brief InformationColorEngine class constructor.
           *
           */
          explicit InformationColorEngine();

          /** \brief InformationColorEngine class destructor.
           *
           */
          virtual ~InformationColorEngine();

          /** \brief Sets the information key and range values.
           * \param[in] key extension information key.
           * \param[in] min minimum value.
           * \param[in] max maximum value.
           *
           */
          void setInformation(const Core::SegmentationExtension::InformationKey &key, double min, double max);

          /** \brief Returns the used information key,.
           *
           */
          Core::SegmentationExtension::InformationKey information() const
          { return m_key; }

          virtual QColor color(ConstSegmentationAdapterPtr segmentation);

          virtual LUTSPtr lut(ConstSegmentationAdapterPtr segmentation);

          virtual Composition supportedComposition() const
          { return ColorEngine::Color; }

          /** \brief Returns the used color range.
           *
           */
          Utils::RangeHSV *colorRange() const
          { return m_colorRange; }

        private:
          Core::SegmentationExtension::InformationKey m_key;        /** used key to get segmentation information. */
          Utils::RangeHSV                            *m_colorRange; /** used color range.                         */
          QMap<QColor, LUTSPtr>                       m_luts;       /** color-lookuptable map.                    */
      };
    }
  }
}

#endif // ESPINA_GUI_INFORMATION_COLOR_ENGINE_H
