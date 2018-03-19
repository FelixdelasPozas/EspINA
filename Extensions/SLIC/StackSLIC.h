/*
 * Copyright (C) 2018, Álvaro Muñoz Fernández <golot@golot.es>
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

#ifndef EXTENSIONS_SLIC_STACKSLIC_H_
#define EXTENSIONS_SLIC_STACKSLIC_H_

#include <Core/Analysis/Extensions.h>
#include <Core/Analysis/Persistent.h>
#include <Core/Utils/TemporalStorage.h>
#include <Extensions/EdgeDistances/ChannelEdges.h>

namespace ESPINA
{

class StackSLIC: public ESPINA::Core::StackExtension
  {
    public:
      StackSLIC(const State &state);
      virtual ~StackSLIC();
      static const Type TYPE;
      virtual QString type() const
      {
        return TYPE;
      }
      virtual State state() const;

      virtual Snapshot snapshot() const;

      virtual bool invalidateOnChange() const
      {
        return false;
      }

      virtual TypeList dependencies() const
      {
        Extension::TypeList deps;
        deps << Extensions::ChannelEdges::TYPE;
        return deps;
      }

      virtual InformationKeyList availableInformation() const;

    protected:

  };

}

#endif /* EXTENSIONS_SLIC_STACKSLIC_H_ */
