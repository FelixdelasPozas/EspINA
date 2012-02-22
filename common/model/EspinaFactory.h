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


class EspinaFactory
{
public:
  static EspinaFactory *instance();

  void registerFilter(const QString filter, FilterFactory *factory);

  SamplePtr createSample(const QString id,     const QString args = "");
  FilterPtr createFilter(const QString filter, const QString args);
  SegmentationPtr createSegmentation(Filter *parent, pqData data);
//   void addSampleExtension(ISampleExtension *ext);

//   void addSegmentationExtension(ISegmentationExtension *ext);
//   QStringList segmentationAvailableInformations();

private:
  EspinaFactory(){};

  static EspinaFactory *m_instance;

  QMap<QString, FilterFactory *> m_filterFactory;
//   QList<ISegmentationExtension *> m_segExtensions;
//   QList<ISampleExtension *> m_sampleExtensions;
//   QList<IViewWidget *> m_widgets;
};

#endif // ESPinaFACTORY_H
