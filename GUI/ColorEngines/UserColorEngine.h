/*

    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

#ifndef ESPINA_USER_COLOR_ENGINE_H
#define ESPINA_USER_COLOR_ENGINE_H

// ESPINA
#include "GUI/ColorEngines/ColorEngine.h"

// Qt
#include <QMap>

namespace ESPINA
{
  namespace GUI
  {
    namespace ColorEngines
    {
      class EspinaGUI_EXPORT UserColorEngine
      : public ColorEngine
      {
      public:
        /** \brief UserColorEngine class constructor.
         *
         */
        explicit UserColorEngine();

        virtual QColor color(ConstSegmentationAdapterPtr seg);

        virtual LUTSPtr lut(ConstSegmentationAdapterPtr seg);

        virtual ColorEngine::Composition supportedComposition() const
        { return ColorEngine::Color; }

        virtual ColorEngineSPtr clone()
        { return std::make_shared<UserColorEngine>(); }

      private:
        /** \brief Returns the next unused color.
         *
         */
        QColor nextColor();

      private:
        QMap<QString, QColor> m_userColors;
        QList<QColor>         m_colors;
        int                   m_lastColor;
        LUTMap                m_LUT;
      };
    }

  }
}// namespace ESPINA

#endif // ESPINA_USER_COLOR_ENGINE_H
