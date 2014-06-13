/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2014  <copyright holder> <email>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ESPINA_SAS_ANALYSIS_DIALOG_H
#define ESPINA_SAS_ANALYSIS_DIALOG_H

// Plugin
#include "SASAnalysisEntry.h"
#include "ui_SASAnalysisDialog.h"

// EspINA
#include <Core/EspinaTypes.h>
#include <GUI/Model/ModelAdapter.h>
#include <Support/ViewManager.h>

// Qt
#include <QDialog>
#include <QUndoStack>

namespace EspINA
{
  class SASAnalysisDialog
  : public QDialog
  , private Ui::SASAnalysisDialog
  {
    Q_OBJECT
  public:
    explicit SASAnalysisDialog(SegmentationAdapterList segmentations,
                               ModelAdapterSPtr        model,
                               QUndoStack             *undoStack,
                               ModelFactorySPtr        factory,
                               ViewManagerSPtr         viewManager,
                               QWidget                *parent);
  protected:
    virtual void closeEvent(QCloseEvent* event);

  private slots:
    void configureInformation();
    void exportInformation();
    void cl(bool);

  private:
    void createTabs(QMap<QString, SegmentationAdapterList> tabs);

    QStringList toStringList(SegmentationExtension::InfoTagList tags) const;

  private:
    ModelAdapterSPtr m_model;
    ModelFactorySPtr m_factory;
    QUndoStack      *m_undoStack;
    ViewManagerSPtr  m_viewManager;

    SegmentationAdapterList m_synapses;

    QList<SASAnalysisEntry *> m_entries;
    SegmentationExtension::InfoTagList m_synapseTags;
    SegmentationExtension::InfoTagList m_sasTags;
  };
}

#endif // ESPINA_SAS_ANALYSIS_DIALOG_H
