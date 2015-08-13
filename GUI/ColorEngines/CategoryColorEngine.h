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

#ifndef ESPINA_CATEGORY_COLOR_ENGINE_H
#define ESPINA_CATEGORY_COLOR_ENGINE_H

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
      class EspinaGUI_EXPORT CategoryColorEngine
      : public ColorEngine
      {
        Q_OBJECT
      public:
        /** \brief CategoryColorEngine class constructor.
         *
         */
        explicit CategoryColorEngine()
        : ColorEngine("CategoryColorEngine", tr("Category"))
        {}

        virtual QColor color(ConstSegmentationAdapterPtr seg);

        virtual LUTSPtr lut(ConstSegmentationAdapterPtr seg);

        virtual ColorEngine::Composition supportedComposition() const
        { return ColorEngine::Color; }

      protected slots:
        /** \brief Updates the internal color tables when a category color changes.
         * \param[in] category adapter
         *
         */
        void updateCategoryColor(CategoryAdapterSPtr category);

      private:
        LUTMap m_LUT;
      };
    }
  }
} // namespace ESPINA

#endif // ESPINA_CATEGORY_COLOR_ENGINE_H
