/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
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

#ifndef ESPINA_CHANNEL_REPRESENTATION_FACTORY_H
#define ESPINA_CHANNEL_REPRESENTATION_FACTORY_H

#include <Support/Representations/RepresentationFactory.h>
#include <Support/Context.h>

namespace ESPINA
{
  /** \class StackRepresentationFactory
   * \brief Representation factory for stack's representations.
   */
  class StackRepresentationFactory
  : public RepresentationFactory
  {
    public:
      /** \brief StackRepresentationFactory class constructor.
       *
       */
      explicit StackRepresentationFactory();

    private:
      /** \brief Creates the stack representations.
       * \param[in] context application context.
       * \param[in] supportedViews view flags of the vies the representations will be shown.
       *
       */
      virtual Representation doCreateRepresentation(Support::Context &context, ViewTypeFlags supportedViews) const;

      /** \brief Creates the slice representations for stacks.
       * \param[out] representation Representation object.
       * \param[in] context application context.
       * \param[in] supportedViews view flags of the vies the representations will be shown.
       *
       */
      void createSliceRepresentation(Representation &representation, Support::Context &context, ViewTypeFlags supportedViews) const;

    private:
      static const unsigned int WINDOW_SIZE; /** window size for buffered representations. */
  };
}

#endif // ESPINA_CHANNEL_REPRESENTATION_FACTORY_H
