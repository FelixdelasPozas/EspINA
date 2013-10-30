/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>

    This program is free software: you can redistribute it and/or modify
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


#ifndef ESPINA_CORE_FACTORY_H
#define ESPINA_CORE_FACTORY_H

#include "EspinaCore_Export.h"

#include "Core/Factory/ExtensionProviderFactory.h"
#include "Core/Factory/FilterFactory.h"

#include <QStringList>
#include <QMap>

namespace EspINA
{

  class EspinaCore_EXPORT CoreFactory
  {
  public:
    struct Factory_Already_Registered_Exception{};
    struct Unknown_Type_Exception{};

  public:
    explicit CoreFactory();
    ~CoreFactory();

    void registerFilter(FilterFactoryPtr factory, const Filter::Type &filter) throw (Factory_Already_Registered_Exception);

    void registerExtensionProvider(ExtensionProviderFactoryPtr factory, const ExtensionProvider::Type& provider) throw (Factory_Already_Registered_Exception);

    SampleSPtr createSample(const QString& name = QString()) const;

    FilterSPtr createFilter(OutputSList inputs, const Filter::Type& filter) const throw (Unknown_Type_Exception);

    ChannelSPtr createChannel(FilterSPtr filter, Output::Id output) const;

    SegmentationSPtr createSegmentation(FilterSPtr filter, Output::Id output) const;

    ExtensionProviderSPtr createExtensionProvider(const ExtensionProvider::Type provider) const throw (Unknown_Type_Exception);

  private:
    QMap<Filter::Type, FilterFactoryPtr>                       m_filterFactories;
    QMap<ExtensionProvider::Type, ExtensionProviderFactoryPtr> m_providerFactories;
  };

}// namespace EspINA

#endif // ESPINA_CORE_FACTORY_H
