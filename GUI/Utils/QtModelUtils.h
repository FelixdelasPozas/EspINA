/*

    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

#ifndef QTMODELUTILS_H
#define QTMODELUTILS_H

#include "GUI/EspinaGUI_Export.h"

// Qt
#include <QModelIndex>

namespace QtModelUtils
{
  /** \brief Find the QModelIndex children of @parent whose @role is equal to @value.
   * \param[in] parent model index parent of the one searching for.
   * \param[in] value.
   * \param[in] role.
   *
   */
  QModelIndex EspinaGUI_EXPORT findChildIndex(QModelIndex parent, QVariant value, int role = Qt::DisplayRole);

  /** \brief Return a list of all QModelIndex between @topLeft index and @bottomRight index.
   * \param[in] topLeft, model index.
   * \param[in] bottomRight, model index.
   *
   */
  QModelIndexList EspinaGUI_EXPORT indices(const QModelIndex &topLeft, const QModelIndex &bottomRight);

  /** \brief Return whether a node has leaf nodes.
   * \param[in] index, model index ot check.
   *
   */
  bool EspinaGUI_EXPORT isInnerNode(const QModelIndex &index);

  /** \brief Return whether a node is leaf.
   * \param[in] index, model index to check.
   *
   */
  bool EspinaGUI_EXPORT isLeafNode(const QModelIndex &index);

  /** \brief Writes all QObject properties to the debug stream.
   * \param[in] obj Pointer to QObject to inspect its properties.
   *
   */
  void EspinaGUI_EXPORT dumpQObjectProperties(QObject *obj);

} // namespace QtModelUtils

#endif // QTMODELUTILS_H
