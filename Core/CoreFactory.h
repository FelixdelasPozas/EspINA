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

#include "Core/EspinaTypes.h"
#include "Core/Analysis/Filter.h"
#include "Core/Analysis/Extensions/ExtensionProvider.h"

#include <QStringList>
#include <QMap>

namespace EspINA
{
  class FilterCreator;
  using FilterCreatorPtr = FilterCreator *;

  class ExtensionProvider;

  //const QString CHANNEL_FILES = QObject::tr("Channel Files (*.mha *.mhd *.tif *.tiff)");
  //const QString SEG_FILES     = QObject::tr("Espina Analysis (*.seg)");

  class EspinaCore_EXPORT CoreFactory
  {
  public:
    explicit CoreFactory();
    ~CoreFactory();

    void registerFilter(FilterCreatorPtr creator, const Filter::Type &filter);

    SampleSPtr createSample(const QString& name = QString()) const;

    FilterSPtr createFilter(OutputSList inputs, Filter::Type& filter) const;

    ChannelSPtr createChannel(OutputSPtr output) const;

    SegmentationSPtr createSegmentation(OutputSPtr output) const;

    void registerExtensionProvider  (ExtensionProviderPtr provider);
    void unregisterExtensionProvider(ExtensionProviderPtr provider);

    ExtensionProviderList extensionProviders() const
    { return m_extensionProviders;}

    /** \brief Return the extension provider with the given type or nullptr if not found
     *
     */
    ExtensionProviderPtr extensionProvider(const ExtensionProvider::Type& type) const;

  private:
    QMap<Filter::Type, FilterCreatorPtr> m_filterCreators;

    ExtensionProviderList m_extensionProviders;
  };
}// namespace EspINA

#endif // ESPINA_CORE_FACTORY_H
