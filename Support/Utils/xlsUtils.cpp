/*
 *
 *    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
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

// ESPINA
#include "xlsUtils.h"

// Boost
#include <boost/numeric/conversion/cast.hpp>

xlslib_core::cell_t * createCell(xlslib_core::worksheet *sheet, int row, int column, const QVariant &value)
{
  // NOTE: in Excel97 format all numbers are stored as doubles internally.
  // If the number can be converted to double without losing precision it
  // will be stored as number, otherwise as text. But text can't be used in
  // formulas so if one value in a column is text and try to use the formula
  // SUM on the column the value in text won't be counted.
  // Double has a mantissa of 53 bits, it should suffice for most values.
  //
  // TODO: there's a "formula trick" to store a large number as the direct
  // formula "=number" but it doesn't work with xlslib? To review further.
  using boost::numeric_cast;

  xlslib_core::cell_t *cell = nullptr;

  try
  {
    switch(value.type())
    {
      case QVariant::Int:
        cell = sheet->number(row, column, numeric_cast<double>(value.toInt()));
        break;
      case QVariant::Double:
        cell = sheet->number(row, column, value.toDouble());
        break;
      case QVariant::UInt:
        cell = sheet->number(row, column, numeric_cast<double>(value.toUInt()));
        break;
      case QVariant::LongLong:
        cell = sheet->number(row, column, numeric_cast<double>(value.toLongLong()));
        break;
      case QVariant::ULongLong:
        cell = sheet->number(row, column, numeric_cast<double>(value.toULongLong()));
        break;
      default:
        cell = sheet->label(row, column, value.toString().toStdString());
        break;
    }
  }
  catch(...)
  {
    switch(value.type())
    {
      case QVariant::Int:
        cell = sheet->label(row, column, QString::number(value.toInt()).toStdString());
        break;
      case QVariant::UInt:
        cell = sheet->label(row, column, QString::number(value.toUInt()).toStdString());
        break;
      case QVariant::LongLong:
        cell = sheet->label(row, column, QString::number(value.toLongLong()).toStdString());
        break;
      case QVariant::ULongLong:
        cell = sheet->label(row, column, QString::number(value.toULongLong()).toStdString());
        break;
      default:
        // can't happen
        break;
    }
  }

  return cell;
}



