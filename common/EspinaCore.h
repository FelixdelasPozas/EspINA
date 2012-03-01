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


#ifndef ESPINACORE_H
#define ESPINACORE_H

#include "model/Taxonomy.h"
#include "model/EspinaModel.h"

#include <QSharedPointer>
#include <QUndoStack>
#include "gui/ViewManager.h"


class EspinaCore
: public QObject
{
  Q_OBJECT
public:
  virtual ~EspinaCore(){}

  static EspinaCore *instance();

  QSharedPointer<EspinaModel> model() {return m_model;}
  QSharedPointer<QUndoStack>  undoStack() {return m_undoStack;}

  void setActiveTaxonomy(TaxonomyNode *tax);
  TaxonomyNode *activeTaxonomy(){return m_activeTaxonomy;}

  void setSample(Sample *sample) {m_sample = sample; emit sampleSelected(sample);}
  Sample *sample(){return m_sample;}

  QSharedPointer<ViewManager> viewManger() {return m_viewManager;}

  void loadFile(const QString file);

protected:
  void loadChannel(const QString file);

public slots:
  void closeCurrentAnalysis();

signals:
  void sampleSelected(Sample *);
  void currentAnalysisClosed();

private:
  explicit EspinaCore();

  static EspinaCore *m_singleton;

  TaxonomyNode               *m_activeTaxonomy;
  Sample                     *m_sample;
  EspinaModelPtr              m_model;
  QSharedPointer<QUndoStack>  m_undoStack;
  QSharedPointer<ViewManager> m_viewManager;
};

#endif // ESPINACORE_H
