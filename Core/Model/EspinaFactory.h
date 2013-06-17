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


#ifndef ESPinaFACTORY_H
#define ESPinaFACTORY_H

#include "EspinaCore_Export.h"

#include "Core/EspinaTypes.h"
#include "Core/Model/Filter.h"
#include "Channel.h"
#include "Segmentation.h"
#include <GUI/ISettingsPanel.h>
#include <GUI/Renderers/Renderer.h>
#include <Core/IO/SegFileReader.h>

#include <QStringList>
#include <QMap>

namespace EspINA
{
  const QString CHANNEL_FILES = QObject::tr("Channel Files (*.mha *.mhd *.tif *.tiff)");
  const QString SEG_FILES     = QObject::tr("Espina Analysis (*.seg)");

  class EspinaCore_EXPORT EspinaFactory
  {
  public:
    explicit EspinaFactory();
    ~EspinaFactory();

    QStringList supportedFiles() const;

    void registerFilter(IFilterCreatorPtr creator, const QString &filter);

    void registerReaderFactory(IFileReaderPtr     reader,
                               const QString     &description,
                               const QStringList &extensions);

    void registerChannelExtension(Channel::ExtensionPtr extension);
    void unregisterChannelExtension(Channel::ExtensionPtr extension);
    Channel::ExtensionList channelExtensions() const
    { return m_channelExtensions; }
    Channel::ExtensionPtr channelExtension(ModelItem::ExtId extensionId) const;

    void registerSegmentationExtension(Segmentation::InformationExtension extension);
    void unregisterSegmentationExtension(Segmentation::InformationExtension extension);
    Segmentation::InformationExtensionList segmentationExtensions() const
    { return m_segmentationExtensions; }
    Segmentation::InformationExtension segmentationExtension(ModelItem::ExtId extensionId) const;
    Segmentation::InformationExtension informationProvider(Segmentation::InfoTag tag) const;

    void registerSettingsPanel(ISettingsPanelPtr panel)
    {m_settingsPanels << panel;}
    void unregisterSettingsPanel(ISettingsPanelPtr panel)
    {m_settingsPanels.removeOne(panel);}

    void registerRenderer  (IRenderer *renderer);
    void unregisterRenderer(IRenderer *renderer);


    ISettingsPanelList settingsPanels() const
    {return m_settingsPanels;}

    QMap<QString, IRenderer *> renderers() const
    {return m_renderers;}


    FilterSPtr createFilter(const QString              &filter,
                                 const Filter::NamedInputs  &inputs,
                                 const ModelItem::Arguments &args);

    bool readFile(const QString &file, const QString &ext, IOErrorHandler *handler = NULL);


    SampleSPtr createSample(const QString &id, const QString &args = "");

    ChannelSPtr createChannel(FilterSPtr filter, const FilterOutputId &oId);

    SegmentationSPtr createSegmentation(FilterSPtr filter, const FilterOutputId &oId);


  private:
    QMap<QString, IFilterCreatorPtr> m_filterCreators;
    QMap<QString, IFileReaderPtr>    m_fileReaders;

    Channel::ExtensionList                 m_channelExtensions;
    Segmentation::InformationExtensionList m_segmentationExtensions;

    QMap<QString, IRenderer *> m_renderers;

    ISettingsPanelList m_settingsPanels;

    QStringList m_supportedFiles;
    QStringList m_supportedExtensions;
  };

  typedef EspinaFactory *EspinaFactoryPtr;
  //typedef boost::shared_ptr<EspinaFactory> EspinaFactorySPtr;

}// namespace EspINA

#endif // ESPinaFACTORY_H
