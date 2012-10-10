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


#ifndef FILTERFACTORY_H
#define FILTERFACTORY_H

#include "common/model/Filter.h"


class EspinaFactory;
class FilterFactory
{
public:
  virtual ~FilterFactory(){}

  virtual void initFilterFactory(EspinaFactory *factory) = 0;

  virtual Filter *createFilter(const QString filter,
                               Filter::NamedInputs inputs,
                               const ModelItem::Arguments args) = 0;
};
Q_DECLARE_INTERFACE(FilterFactory,
                    "es.upm.cesvima.EspINA.FilterFactory/1.1")
#endif // FILTERFACTORY_H
