/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>

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


#ifndef SAMPLEPROXY_H
#define SAMPLEPROXY_H

#include <QAbstractProxyModel>

// Forward declaration
class EspinaModel;
class Sample;
class Segmentation;

/// Group by Sample Espina Proxy
class SampleProxy : public QAbstractProxyModel
{
  Q_OBJECT
public:
  SampleProxy(QObject* parent = 0);
  virtual ~SampleProxy();

  virtual void setSourceModel(EspinaModel *sourceModel);

  virtual QVariant data(const QModelIndex& proxyIndex, int role = Qt::DisplayRole) const;

  virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
  virtual int columnCount(const QModelIndex& parent = QModelIndex()) const {return 1;}
  virtual QModelIndex parent(const QModelIndex& child) const;
  virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;

  virtual QModelIndex mapFromSource(const QModelIndex& sourceIndex) const;
  virtual QModelIndex mapToSource(const QModelIndex& proxyIndex) const;

  protected slots:
	  void sourceRowsInserted(const QModelIndex & sourceParent, int start, int end);
  void sourceRowsAboutToBeRemoved(const QModelIndex & sourceParent, int start, int end);
  void sourceRowsRemoved(const QModelIndex & sourceParent, int start, int end);
  void sourceDataChanged(const QModelIndex& sourceTopLeft, const QModelIndex& sourceBottomRight);

protected:
  bool indices(const QModelIndex &topLeft, const QModelIndex &bottomRight, QModelIndexList &result);
  void updateSegmentations() const;
  Sample *origin(Segmentation *seg) const;
  int numberOfSegmentations(Sample *sample) const;
  int numberOfSubSamples(Sample *sample) const;

private:
  EspinaModel *m_model;
  QList<Sample *> m_samples;
  mutable QMap<const Sample *, QList<Segmentation *> > m_sampleSegs;
};

#endif // SAMPLEPROXY_H
