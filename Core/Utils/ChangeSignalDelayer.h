/*

 Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef ESPINA_CHANGE_SIGNAL_DELAYER_H_
#define ESPINA_CHANGE_SIGNAL_DELAYER_H_

#include "Core/EspinaCore_Export.h"

// ESPINA
#include <Core/Analysis/Data.h>

namespace ESPINA
{
  class EspinaCore_EXPORT ChangeSignalDelayer
  {
    public:
      /** brief ChangeSignalDelayer class constructor.
       * \param[in] data, Data smart pointer.
       *
       * Delays the emission of the dataChanged signal of the data.
       */
      explicit ChangeSignalDelayer(DataSPtr data)
      : m_data(data)
      {
        m_data->blockSignals(true);
      }

      /** brief ChangeSignalDelayer class destructor.
       *
       */
      ~ChangeSignalDelayer()
      {
        m_data->blockSignals(false);
        m_data->updateModificationTime();
      }

    private:
      DataSPtr m_data;
  };

} /* namespace ESPINA */

#endif // ESPINA_CHANGE_SIGNAL_DELAYER_H_
