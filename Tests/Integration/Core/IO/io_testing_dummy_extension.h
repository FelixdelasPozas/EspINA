/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2013 Félix de las Pozas Álvarez <felixdelaspozas@gmail.com>

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
#ifndef SEGMENTATIONEXTENSIONSUPPORT_H_
#define SEGMENTATIONEXTENSIONSUPPORT_H_

#include "Core/Analysis/Extensions/SegmentationExtension.h"

using namespace EspINA;

class DummySegmentationExtension
: public SegmentationExtension
{
  public:
    const SegmentationExtension::Type TYPE = "DummySegmentationExtension";
  public:
    DummySegmentationExtension() {};
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

    void onSegmentationSet(SegmentationPtr seg) {};
    bool validCategory(const QString &classificationName) const { return true; };

    InfoTagList availableInformations() const
    {
      InfoTagList list;
      list << InfoTag("Tag1") << InfoTag("Tag2");
      return list;
    };

    QVariant information(const InfoTag &tag) const
    {
      if (tag == InfoTag("Tag1"))
          return QVariant("prueba1");

      if (tag == InfoTag("Tag2"))
        return QVariant("prueba2");

      return QVariant();
    }

    virtual QString toolTipText() const
    { return QString("ToolTip"); };

    virtual SegmentationExtension::Type type() const
    { return TYPE; };
};

#endif /* SEGMENTATIONEXTENSIONSUPPORT_H_ */
