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


#ifndef MARGINSCHANNELEXTENSION_H
#define MARGINSCHANNELEXTENSION_H

#include "common/extensions/ChannelExtension.h"
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>

#include <QMutex>

class Channel;
class Segmentation;

class MarginsChannelExtension
: public ChannelExtension
{
public:
  static const ExtId ID;

  static const InfoTag LEFT_MARGIN;
  static const InfoTag TOP_MARGIN;
  static const InfoTag UPPER_MARGIN;
  static const InfoTag RIGHT_MARGIN;
  static const InfoTag BOTTOM_MARGIN;
  static const InfoTag LOWER_MARGIN;

  static const ModelItem::ArgumentId MARGINTYPE;

  explicit MarginsChannelExtension();
  virtual ~MarginsChannelExtension();

  virtual ExtId id();
  virtual void initialize(ModelItem::Arguments args = ModelItem::Arguments());
  virtual QString serialize() const;

  virtual ExtIdList dependencies() const
  {return ChannelExtension::dependencies();}

  virtual InfoList availableInformations() const
  {return ChannelExtension::availableInformations();}

  virtual RepList availableRepresentations() const
  {return ChannelExtension::availableRepresentations();}

  virtual QVariant information(InfoTag tag) const;

  virtual ChannelExtension* clone();

  void computeMarginDistance(Segmentation *seg);

  vtkSmartPointer<vtkPolyData> margins();

protected:
  void computeMargins();

private:
  bool                         m_useChannelBounds;
  ModelItem::Arguments         m_args;
  vtkSmartPointer<vtkPolyData> m_borders;
  QMutex                       m_borderMutex;

  vtkSmartPointer<vtkPolyData> m_PolyDataFaces[6];
  std::map<unsigned int, unsigned long int> m_ComputedSegmentations;

  // builds a surface for each face the first time one is needed
  void ComputeSurfaces();

  friend class MarginDetector;
};

#endif // MARGINSCHANNELEXTENSION_H
