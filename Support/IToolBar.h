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

#ifndef ITOOLBAR_H
#define ITOOLBAR_H

#include "EspinaCore_Export.h"

// Qt
#include <QToolBar>

// c++
#include <limits.h>

class QUndoStack;

namespace EspINA
{
  class ModelAdapter;
  class ViewManager;

  class EspinaCore_EXPORT IToolBar
  : public QToolBar
  {
    Q_OBJECT
  public:
    explicit IToolBar(QWidget *parent = 0);
    explicit IToolBar(const QString &title, QWidget *parent = 0);
    virtual ~IToolBar();

  public slots:
    /// Restore toolbar state to its initial state. Every widgets and tools
    /// created by this toolbar must be removed
    virtual void resetToolbar() = 0;

    virtual void abortOperation() = 0;

  protected:
    int m_undoIndex;
  };

} // namespace EspINA

#endif //ITOOLBAR_H
