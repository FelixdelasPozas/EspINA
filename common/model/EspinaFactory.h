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

#include "common/extensions/ChannelExtension.h"
#include "common/extensions/SegmentationExtension.h"
#include "common/extensions/SampleExtension.h"
#include "common/model/Channel.h"
#include "common/model/Sample.h"
#include "common/pluginInterfaces/FilterFactory.h"
#include "common/pluginInterfaces/ReaderFactory.h"

const QString CHANNEL_FILES = QObject::tr("Channel Files (*.mha *.mhd *.tif *.tiff)");
const QString SEG_FILES     = QObject::tr("Espina Analysis (*.seg)");

class Renderer;
class ISettingsPanel;
class EspinaFactory
{
public:
  static EspinaFactory *instance();

  QStringList supportedFiles() const;

  void registerFilter(const QString filter, FilterFactory *factory);
  void registerReaderFactory(ReaderFactory *readerFactory,
			     const QString description,
			     const QStringList extensions);
  void registerSampleExtension(SampleExtension::SPtr extension);
  void registerChannelExtension(ChannelExtension::SPtr extension);
  void registerSegmentationExtension(SegmentationExtension::SPtr extension);
  void registerSettingsPanel(ISettingsPanel *panel){m_settingsPanels << panel;}
  void registerRenderer(Renderer *renderer);

  QList<ISettingsPanel *> settingsPanels() const {return m_settingsPanels;}
  QMap<QString, Renderer *> renderers() const {return m_renderers;}

  Filter  *createFilter (const QString filter,
                         Filter::NamedInputs inputs,
                         const ModelItem::Arguments args);
  Sample  *createSample (const QString id, const QString args = "");
  Channel *createChannel(Filter *filter, OutputNumber output);
  Segmentation *createSegmentation(Filter* parent, OutputNumber output);

  bool readFile(const QString file, const QString ext);

private:
  EspinaFactory();

  static EspinaFactory *m_instance;

  QMap<QString, FilterFactory *>     m_filterFactory;
  QList<SegmentationExtension::SPtr> m_segExtensions;
  QList<SampleExtension::SPtr>       m_sampleExtensions;
  QList<ChannelExtension::SPtr>      m_channelExtensions;
  QMap<QString, ReaderFactory *>     m_readers;
  QList<ISettingsPanel *>            m_settingsPanels;
  QMap<QString, Renderer *>          m_renderers;
  QStringList                        m_supportedFiles;
  QStringList                        m_supportedExtensions;
};

#endif // ESPinaFACTORY_H
