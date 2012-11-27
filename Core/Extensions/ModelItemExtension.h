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

#include "Core/Model/ModelItem.h"

#include <QStringList>
#include <QVariant>

class ModelItemExtension
: public QObject
{
public:
  typedef QString ExtId;
  typedef QString InfoTag;
  typedef QString RepTag;
  typedef QList<ExtId>   ExtIdList;
  typedef QList<InfoTag> InfoList;
  typedef QList<RepTag>  RepList;

public:
  virtual ~ModelItemExtension(){}

  virtual ExtId id() = 0;
  /// List of extension names which need to be loaded to use the extension
  virtual ExtIdList dependencies() const  = 0;
  /// List of information tags provided by the extension
  virtual InfoList availableInformations()    const = 0;
  /// List of representation tags provided by the extension
  virtual RepList availableRepresentations() const = 0;
  /// Information associated with @tag
  virtual QVariant information(ModelItemExtension::InfoTag tag)  const = 0;

  virtual void initialize(ModelItem::Arguments args = ModelItem::Arguments()) = 0;

protected:
  ModelItemExtension() : m_init(false) {}
  mutable bool m_init;

  InfoList m_availableInformations;
  RepList  m_availableRepresentations;
};

#endif // MODELITEMEXTENSION_H
