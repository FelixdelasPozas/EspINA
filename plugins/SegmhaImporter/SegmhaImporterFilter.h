/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor <jpena@cesvima.upm.es>

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
#ifndef SEGMHAIMPORTERFILTER_H
#define SEGMHAIMPORTERNFILTER_H

#include <common/model/Filter.h>

#include <QPushButton>


class SegmhaImporterFilter 
: public Filter
{
public:
//   /// Constructor interactivo
  explicit SegmhaImporterFilter(const QString file);
  /// Create a new filter from given arguments
  explicit SegmhaImporterFilter(Arguments args);
  virtual ~SegmhaImporterFilter();

  // Implements Model Item Interface
  virtual QString serialize() const{return QString();}
  virtual QVariant data(int role) const {return QVariant();}
  virtual ItemType type() const {return ModelItem::FILTER;}

  //Implements Filter Interface
  virtual pqData preview(){return pqData();}
  virtual int numProducts() const{return 0;}
  virtual pqData product(int index) const{return pqData();}

private:
  pqFilter *m_segReader;
  QMap<pqData, QString> m_blocks;

//   friend class SetupWidget;
};


#endif // SEEDGROWSEGMENTATIONFILTER_H
