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

#ifndef GUI_COLORENGINES_CONNECTIONSCOLORENGINE_H_
#define GUI_COLORENGINES_CONNECTIONSCOLORENGINE_H_

#include <GUI/EspinaGUI_Export.h>

// ESPINA
#include <GUI/Utils/ColorRange.h>
#include <GUI/ColorEngines/ColorEngine.h>

namespace ESPINA
{
  namespace GUI
  {
    namespace ColorEngines
    {
      /** \class ConnectionsColorEngine
       * \brief Colors the segmentations according to the number of connections it has.
       *
       */
      class EspinaGUI_EXPORT ConnectionsColorEngine
      : public ColorEngine
      {
        public:
          /** \brief ConnectionsColorEngine class constructor.
           *
           */
          ConnectionsColorEngine();

          /** \brief ConnectionsColorEngine class virtual destructor.
           *
           */
          ~ConnectionsColorEngine();

          virtual QColor color(ConstSegmentationAdapterPtr segmentation);

          virtual LUTSPtr lut (ConstSegmentationAdapterPtr segmentation);

          virtual Composition supportedComposition() const
          { return ColorEngine::Color; }

          virtual ColorEngineSPtr clone()
          { return std::make_shared<ConnectionsColorEngine>(); }

          /** \brief Sets the maximum and minimum values to use to apply to the HUE range.
           * \param[in] minimum Minimum number of connections.
           * \param[in] maximum Maximum number of connections.
           *
           */
          void setRange(const unsigned int minimum, const unsigned int maximum);

        private:
          Utils::ColorRange *m_HUERange; /** hue range object. */
      };
    }
  }
}


#endif // GUI_COLORENGINES_CONNECTIONSCOLORENGINE_H_
