/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

//----------------------------------------------------------------------------
// File:    Sample.h
// Purpose: Model a physical sample.
//          It provides channels and segmentations with a link to the
//          physical world
//----------------------------------------------------------------------------
#ifndef SAMPLE_H
#define SAMPLE_H

#include "ModelItem.h"

#include "Channel.h"
#include "Segmentation.h"

#include <QList>
#include <QSharedPointer>
#include <QString>

class Channel;
class Segmentation;

class Sample : public ModelItem
{
public:
  explicit Sample(QString ID);
  virtual ~Sample();

  QString id(){return m_ID;}
  // Relative to brain center (in nm)
  double *origin();
  void setOrigin(double origin[3]);

  double *size();/*nm*/
  void setSize(double size[3]);

  void addChannel(ChannelPtr channel);
  void addSegmentation(SegmentationPtr seg);

  /// ModelItem Interface
  virtual QVariant data(int role) const;
  virtual ItemType type() const {return SAMPLE;}

private:
  QString m_ID;
  double  m_origin[3];//nm
  double  m_size[3];//nm

  QList<ChannelPtr>      m_channels;
  QList<SegmentationPtr> m_segmentations;
};

typedef QSharedPointer<Sample> SamplePtr;

#endif // SAMPLE_H
