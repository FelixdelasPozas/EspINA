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


#ifndef MODELITEMEXTENSION_H
#define MODELITEMEXTENSION_H

#include "common/model/ModelItem.h"

#include <QStringList>
#include <QVariant>

class ModelItemExtension
{
public:
  virtual ~ModelItemExtension(){}

  virtual QString id() = 0;
  virtual QStringList dependencies() const  = 0;
  virtual QStringList availableInformations()    const = 0;
  virtual QStringList availableRepresentations() const = 0;
  virtual QVariant    information(QString info)  const = 0;
  virtual void initialize(ModelItem::Arguments args = ModelItem::Arguments()) {}
//   virtual SegmentationRepresentation *representation(QString rep) = 0;
//   virtual void setArguments(QString args) {}

protected:
  ModelItemExtension() : m_init(false) {}
  mutable bool m_init;

  QStringList m_availableRepresentations;
  QStringList m_availableInformations;
};

#endif // MODELITEMEXTENSION_H
