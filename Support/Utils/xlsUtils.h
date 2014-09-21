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

#ifndef XLSUTILS_H
#define XLSUTILS_H

// ESPINA
#include "Support/EspinaSupport_Export.h"

// Qt
#include <QVariant>

// xls
#include <common/xlconfig.h>
#include <xlslib.h>

/** brief Creates a cell in the specified xls sheet, row and column with the specified content.
 * \param[in] sheet, xls worksheet raw pointer.
 * \param[in] row, row of the cell.
 * \param[in] column, column of the cell.
 * \param[in] value, content of the cell.
 *
 */
xlslib_core::cell_t * EspinaSupport_EXPORT createCell(xlslib_core::worksheet *sheet, int row, int column, const QVariant &value);


#endif // XLSUTILS_H
