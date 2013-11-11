/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  Jorge Peña Pastor <jpena@cesvima.upm.es>
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

#ifndef ESPINA_OUTPUTADAPTER_H
#define ESPINA_OUTPUTADAPTER_H

#include <Core/Analysis/Output.h>
#include <GUI/Representations/Representation.h>

namespace EspINA {

  class RepresentationFactory;
  using RepresentationFactorySPtr = std::shared_ptr<RepresentationFactory>;

  class OutputAdapter
  : public QObject
  {
    Q_OBJECT
  public:
    OutputAdapter(OutputSPtr output);

    Output::Id id() const
    { return m_output->id(); }

    Bounds bounds() const
    { return m_output->bounds(); }

    void update()
    { m_output->update();}

    NmVector3 spacing() const
    { return m_output->spacing(); }

    TimeStamp lastModified()
    { return m_output->lastModified(); }

    DataSPtr data(const Data::Type& type) const
    { return m_output->data(type); }

  private:
    OutputSPtr m_output;
  };

  using OutputAdapterSPtr = std::shared_ptr<OutputAdapter>;
}

#endif // ESPINA_OUTPUTADAPTER_H
