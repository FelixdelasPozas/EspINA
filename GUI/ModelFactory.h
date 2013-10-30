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


#ifndef ESPINA_MODEL_FACTORY_H
#define ESPINA_MODEL_FACTORY_H

#include "EspinaGUI_Export.h"

#include "GUI/Model/FilterAdapter.h"

#include <memory>

#include <QStringList>
#include <QMap>

namespace EspINA
{
  class FilterCreator;
  using FilterCreatorPtr = FilterCreator *;

  class ExtensionProvider;

  class SampleAdapter;
  using SampleAdapterSPtr = std::shared_ptr<SampleAdapter>;
  
  class ChannelAdapter;
  using ChannelAdapterSPtr = std::shared_ptr<ChannelAdapter>;

//   class FilterAdapter;
//   using FilterAdapterSPtr = std::shared_ptr<FilterAdapter>;

  //const QString CHANNEL_FILES = QObject::tr("Channel Files (*.mha *.mhd *.tif *.tiff)");
  //const QString SEG_FILES     = QObject::tr("Espina Analysis (*.seg)");

  class EspinaGUI_EXPORT ModelFactory
  {
  public:
    explicit ModelFactory(SchedulerSPtr scheduler);
    ~ModelFactory();

    void registerFilter(FilterCreatorPtr creator, const Filter::Type &filter);

    SampleAdapterSPtr createSample(const QString& name = QString()) const;

    //FilterAdapterSPtr createFilter(OutputSList inputs, Filter::Type& filter) const;

    template<typename T>
    std::shared_ptr<FilterAdapter<T>> createFilter(OutputSList inputs, Filter::Type type) const
    {
      std::shared_ptr<T> filter{new T(inputs, type, m_scheduler)};
      return std::shared_ptr<FilterAdapter<T>>(new FilterAdapter<T>(filter));
    }

    ChannelAdapterSPtr createChannel(FilterAdapterSPtr filter, Output::Id output) const;
// 
//     SegmentationSPtr createSegmentation(OutputSPtr output) const;
// 
//     void registerExtensionProvider  (ExtensionProviderPtr provider);
//     void unregisterExtensionProvider(ExtensionProviderPtr provider);
// 
//     ExtensionProviderList extensionProviders() const
//     { return m_extensionProviders;}
// 
//     /** \brief Return the extension provider with the given type or nullptr if not found
//      *
//      */
 //   ExtensionProviderPtr extensionProvider(const ExtensionProvider::Type& type) const;

  private:
    SchedulerSPtr m_scheduler;
    QMap<Filter::Type, FilterCreatorPtr> m_filterCreators;

    //ExtensionProviderList m_extensionProviders;
  };
}// namespace EspINA

#endif // ESPINA_CORE_FACTORY_H
