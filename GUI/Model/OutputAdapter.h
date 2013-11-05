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
    OutputAdapter(OutputSPtr output, RepresentationFactorySPtr factory);

    Output::Id id() const
    { return m_output->id(); }

    Bounds bounds() const
    { return m_output->bounds(); }

    RepresentationSPtr representation(Representation::Type type) const;

    RepresentationSList representations() const
    { return m_representations.values(); }

    RepresentationTypeList representationTypes() const
    { return m_representations.keys(); }

    void update()
    { m_output->update();}

    TimeStamp lastModified()
    { return m_output->lastModified(); }

  private:
    OutputSPtr m_output;
    RepresentationFactorySPtr m_factory;
    QMap<Representation::Type, RepresentationSPtr> m_representations;
  };

  using OutputAdapterSPtr = std::shared_ptr<OutputAdapter>;
}

#endif // ESPINA_OUTPUTADAPTER_H
