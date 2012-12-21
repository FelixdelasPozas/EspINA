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

#include "Core/EspinaTypes.h"
#include "Core/Model/Filter.h"
#include "Channel.h"
#include "Segmentation.h"
#include <GUI/ISettingsPanel.h>

#include <QStringList>
#include <QMap>

namespace EspINA
{
  const QString CHANNEL_FILES = QObject::tr("Channel Files (*.mha *.mhd *.tif *.tiff)");
  const QString SEG_FILES     = QObject::tr("Espina Analysis (*.seg)");

  class EspinaFactory
  {
  public:
    explicit EspinaFactory();
    ~EspinaFactory();

    QStringList supportedFiles() const;

    void registerFilter(IFilterCreatorPtr creator, const QString &filter);

    void registerReaderFactory(IFileReaderPtr     reader,
                               const QString     &description,
                               const QStringList &extensions);

    void registerChannelExtension(ChannelExtensionPtr extension);

    void registerSampleExtension(SampleExtensionPtr extension);

    void registerSegmentationExtension(SegmentationExtensionPtr extension);

    void registerSettingsPanel(ISettingsPanelPrototype panel)
    {m_settingsPanels << panel;}

    void registerRenderer(IRendererPtr renderer);


    ISettingsPanelPrototypeList settingsPanels() const
    {return m_settingsPanels;}

    QMap<QString, IRendererPtr> renderers() const
    {return m_renderers;}


    FilterSPtr createFilter(const QString              &filter,
                                 const Filter::NamedInputs  &inputs,
                                 const ModelItem::Arguments &args);

    bool readFile(const QString &file, const QString &ext);


    SampleSPtr createSample(const QString &id, const QString &args = "");

    SharedChannelPtr createChannel(FilterSPtr filter, const Filter::OutputId &oId);

    SegmentationSPtr createSegmentation(FilterSPtr filter, const Filter::OutputId &oId);


  private:
    QMap<QString, IFilterCreatorPtr> m_filterCreators;
    QMap<QString, IFileReaderPtr>    m_fileReaders;

    QList<ChannelExtensionPtr>      m_channelExtensions;
    QList<SampleExtensionPtr>       m_sampleExtensions;
    QList<SegmentationExtensionPtr> m_segExtensions;

    QMap<QString, IRendererPtr> m_renderers;

    ISettingsPanelPrototypeList m_settingsPanels;

    QStringList m_supportedFiles;
    QStringList m_supportedExtensions;
  };

}// namespace EspINA

#endif // ESPinaFACTORY_H
