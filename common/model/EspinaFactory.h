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

#include "common/model/Sample.h"
#include "common/plugins/FilterFactory.h"
#include "common/plugins/ReaderFactory.h"
#include "common/extensions/SegmentationExtension.h"

class EspinaFactory
{
public:
  static EspinaFactory *instance();

  void registerFilter(const QString filter, FilterFactory *factory);
  void registerReader(const QString extension, ReaderFactory *factory);
  void registerSegmentationExtension(SegmentationExtension::SPtr extension);

  Filter *createFilter(const QString filter, const QString args);
  Sample *createSample(const QString id,     const QString args = "");
  Segmentation *createSegmentation(Filter* parent, int output, pqData data);

  void readFile(const QString file, const QString ext);
//   void addSampleExtension(ISampleExtension *ext);

//   void addSegmentationExtension(ISegmentationExtension *ext);
//   QStringList segmentationAvailableInformations();

private:
  EspinaFactory();

  static EspinaFactory *m_instance;

  QMap<QString, FilterFactory *> m_filterFactory;
  QList<SegmentationExtension::SPtr> m_segExtensions;
//   QList<ISampleExtension *> m_sampleExtensions;
  QMap<QString, ReaderFactory *> m_readers;
};

#endif // ESPinaFACTORY_H
