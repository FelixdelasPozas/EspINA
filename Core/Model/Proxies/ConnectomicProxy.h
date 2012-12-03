/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2012  Laura Fernandez Soria <laura.fernandez@ctb.upm.es>

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

#ifndef CONNECTOMICPROXY_H
#define CONNECTOMICPROXY_H

#include <QAbstractProxyModel>
#include <QSortFilterProxyModel>

class EspinaModel;
class Segmentation;
class QModelIndex;

const QString CONECTOMICA = "Conectomica";

class ConnectomicProxy: public QSortFilterProxyModel
{
  Q_OBJECT
  public:
    ConnectomicProxy(QObject *parent = 0);
    virtual ~ConnectomicProxy() {};

    virtual Segmentation* FilterBy() const  { return m_seg; }
    void setFilterBy(Segmentation *seg) { m_seg = seg; };

  protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;

  private:
    Segmentation *m_seg;
};

#endif // CONNECTOMICPROXY_H
