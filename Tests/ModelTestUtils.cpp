/*
 *    Copyright (C) 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.
 *
 *    ESPINA is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ModelTestUtils.h"

#include <QModelIndex>
#include <iostream>

using namespace std;
using namespace ESPINA;

//------------------------------------------------------------------------
bool Testing::checkRowCount(const QAbstractItemModel &model, int expectedRowCount)
{
  bool error = false;

  int rowCount = model.rowCount();

  if (rowCount != expectedRowCount)
  {
    cerr << "Wrong number of rows in model: Expected " << expectedRowCount << " instead of " << rowCount << " rows" << endl;
    error = true;
  }

  return error;
}

//------------------------------------------------------------------------
bool Testing::checkRowCount(const QModelIndex &index, int expectedRowCount)
{
  bool error = false;

  auto model = index.model();

  int rowCount = model->rowCount(index);

  if (rowCount != expectedRowCount)
  {
    cerr << index.data().toString().toStdString() <<  ": Expected " << expectedRowCount << " instead of " << rowCount << " rows" << endl;
    error = true;
  }

  return error;
}

//------------------------------------------------------------------------
bool Testing::checkDisplayRole(const QModelIndex& index, const QString& value)
{
  bool error = false;

  auto name  = index.data(Qt::DisplayRole).toString();
  if (name != value)
  {
    cerr << "Unexpected display role value: " << name.toStdString() << endl;
    error = true;
  }

  return error;
}

//------------------------------------------------------------------------
bool Testing::checkDisplayRoleContains(const QModelIndex& index, const QString& value)
{
  bool error = false;

  auto name  = index.data(Qt::DisplayRole).toString();
  if (!name.contains(value))
  {
    cerr << "Unexpected display role value: " << name.toStdString() << endl;
    error = true;
  }

  return error;
}
