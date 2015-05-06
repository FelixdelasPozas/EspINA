/*

 Copyright (C) 2015 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

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

#ifndef ESPINA_CROSSHAIR_REPRESENTATION_FACTORY_H_
#define ESPINA_CROSSHAIR_REPRESENTATION_FACTORY_H_

// ESPINA
#include <GUI/ColorEngines/ColorEngine.h>
#include <Support/Representations/RepresentationFactory.h>

namespace ESPINA
{
  class CrosshairRepresentationFactory
  : public RepresentationFactory
  {
    public:
      /** \brief CrosshairRepresentationFactory class virtual destructor.
       *
       */
      virtual ~CrosshairRepresentationFactory()
      {};

    private:
      virtual Representation doCreateRepresentation(Support::Context &context, ViewTypeFlags supportedViews) const override;

      void createCrosshair2D(Representation &representation, Support::Context &context) const;

      void createCrosshair3D(Representation &representation, Support::Context &context) const;

      void createCrosshair(const QString   &icon,
                           const QString   &description,
                           Representation  &representation,
                           ViewTypeFlags    flags,
                           Support::Context &context) const;
  };

} // namespace ESPINA

#endif // ESPINA_CROSSHAIR_REPRESENTATION_FACTORY_H_
