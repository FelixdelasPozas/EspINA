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

#include <QList>
#include <QString>

class Channel;
class Segmentation;

class Sample : public ModelItem
{
public:
  explicit Sample(const QString id);
  explicit Sample(const QString id, const QString args);
  virtual ~Sample();

  // Relative to brain center (in nm)
  void position(int pos[3]);
  void setPosition(int origin[3]);

  void bounds(double value[6]);   //nm
  void setBounds(double value[6]);//nm

  void addChannel(Channel *channel);
  void addSegmentation(Segmentation *seg);

  /// ModelItem Interface
  virtual QString id() const {return m_ID;}
  virtual QVariant data(int role) const;
  virtual ItemType type() const {return SAMPLE;}

  void setId(const QString id) {m_ID = id;}
private:
  QString m_ID;
  int  m_position[3];//nm
  double  m_bounds[6];//nm

  QList<Channel *>      m_channels;
  QList<Segmentation *> m_segmentations;
};

#endif // SAMPLE_H
