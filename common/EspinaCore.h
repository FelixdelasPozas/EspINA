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
#include <QFileInfo>
#include <QUndoStack>
#include "common/gui/ViewManager.h"
#include "common/settings/GeneralSettings.h"
#include "common/settings/ColorEngineSettings.h"


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

  GeneralSettings &settings(){return m_settings;}
  ColorEngineSettings &colorSettings(){return m_colorSettigns;}

  QSharedPointer<ViewManager> viewManger() {return m_viewManager;}

  bool loadFile(const QFileInfo file);

protected:
  bool loadChannel(const QFileInfo file);

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
  QSharedPointer<EspinaModel> m_model;
  QSharedPointer<QUndoStack>  m_undoStack;
  QSharedPointer<ViewManager> m_viewManager;
  GeneralSettings             m_settings;
  ColorEngineSettings         m_colorSettigns;
};

#endif // ESPINACORE_H
