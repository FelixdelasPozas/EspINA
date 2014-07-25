/*
    
    Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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
#ifndef SEGMENTATIONEXTENSIONSUPPORT_H_
#define SEGMENTATIONEXTENSIONSUPPORT_H_

#include "Core/Analysis/Extension.h"

using namespace ESPINA;

class DummySegmentationExtension
: public SegmentationExtension
{
  public:
    const SegmentationExtension::Type TYPE = "DummySegmentationExtension";
  public:
    DummySegmentationExtension() : SegmentationExtension(InfoCache()) {
      updateInfoCache("Tag1", "prueba1");
      updateInfoCache("Tag2", "prueba2");
    };
    virtual ~DummySegmentationExtension() {};

    virtual bool invalidateOnChange() const
    { return false; }

    virtual State state() const
    { return "Diameter=27"; }

    virtual Snapshot snapshot() const
    { 
      Snapshot snapshot;

      QByteArray data;
      snapshot << SnapshotData(QString("%1.txt").arg(TYPE), data);

      return snapshot;
    }

    virtual TypeList dependencies() const{ return TypeList(); }

    virtual void onExtendedItemSet(Segmentation* item){}
    virtual QVariant cacheFail(const QString& tag) const {return QVariant();}
    bool validCategory(const QString &classificationName) const { return true; };

    InfoTagList availableInformations() const
    {
      InfoTagList list;
      list << InfoTag("Tag1") << InfoTag("Tag2");
      return list;
    };

    virtual QString toolTipText() const
    { return QString("ToolTip"); };

    virtual SegmentationExtension::Type type() const
    { return TYPE; };
};

#endif /* SEGMENTATIONEXTENSIONSUPPORT_H_ */
