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

#include "Core/Model/ModelItem.h"

namespace EspINA
{

  typedef QSharedPointer<Sample> SampleSPtr;
  typedef QList<SampleSPtr>      SampleSList;

  class Sample
  : public ModelItem
  {
  public:
    explicit Sample(const QString &id);
    explicit Sample(const QString &id, const QString &args);
    virtual ~Sample();

    // Relative to brain center (in nm)
    void position   (double pos[3]);
    void setPosition(double pos[3]);

    void bounds   (double value[6]);//nm
    void setBounds(double value[6]);//nm

    /// ModelItem Interface
    virtual QString id() const {return m_ID;}
    virtual QVariant data(int role=Qt::DisplayRole) const;
    virtual QString serialize() const;
    virtual ModelItemType type() const {return SAMPLE;}

    virtual void initialize(const Arguments &args = Arguments());
    virtual void initializeExtensions(const Arguments &args = Arguments());

    void setId(const QString &id) {m_ID = id;}

    ChannelList      channels();
    SegmentationList segmentations();

  private:
    mutable Arguments m_args;

    QString m_ID;
    double  m_position[3];//nm
    double  m_bounds[6];//nm

    ChannelList      m_channels;
    SegmentationList m_segmentations;
  };

  SamplePtr samplePtr(ModelItemPtr item);
  SampleSPtr samplePtr(ModelItemSPtr &item);
}// namespace EspINA

#endif // SAMPLE_H
