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

#ifndef ESPINA_REPRESENTATION_STATE_LIST_H
#define ESPINA_REPRESENTATION_STATE_LIST_H

#include <GUI/Model/ViewItemAdapter.h>

namespace ESPINA
{
  class RepresentationsState
  : public QObject
  {
    Q_OBJECT

  signals:
    void representationAdded();
    void representationUpdated();
    void representationRemoved();
  };

  using RepresentationsStateSPtr  = std::shared_ptr<RepresentationsState>;
  using RepresentationsStateSList = QList<RepresentationsStateSPtr>;

  template<typename T>
  class StateList
  : public RepresentationsState
  {
  public:
    void addRepresentation(typename T::Item item);
    void updateRepresentation(typename T::Item item);
    void removeRepresentation(typename T::Item item);

  private:
    QMap<typename T::Item, T> m_states;
  };
}

#endif // ESPINA_REPRESENTATION_STATE_LIST_H
