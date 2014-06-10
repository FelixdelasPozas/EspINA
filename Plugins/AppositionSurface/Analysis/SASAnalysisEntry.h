/*
    Copyright (c) 2013, <copyright holder> <email>
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
        * Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
        * Neither the name of the <organization> nor the
        names of its contributors may be used to endorse or promote products
        derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY <copyright holder> <email> ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL <copyright holder> <email> BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef ESPINA_SAS_ANALYSIS_ENTRY_H
#define ESPINA_SAS_ANALYSIS_ENTRY_H

#include "AppositionSurfacePlugin_Export.h"
#include "ui_SASAnalysisEntry.h"

// EspINA
#include <Core/EspinaTypes.h>
#include <Core/Analysis/Extension.h>
#include <GUI/Model/ModelAdapter.h>
#include <GUI/Model/SegmentationAdapter.h>
#include <GUI/Widgets/InformationSelector.h>
#include <Support/ViewManager.h>

// xlslib
#include <common/xlconfig.h>
#include <xlslib.h>
#include <xlslib/workbook.h>

class QUndoStack;

namespace EspINA
{
  class AppositionSurfacePlugin_EXPORT SASAnalysisEntry
  : public QWidget
  , public Ui::SASAnalysisEntry
  {
    Q_OBJECT
  public:
    explicit SASAnalysisEntry(SegmentationAdapterList segmentations,
                              ModelAdapterSPtr        model,
                              QUndoStack             *undoStack,
                              ViewManagerSPtr         viewManager,
                              QWidget                *parent);
    void displayInformation();

    bool exportToXLS(xlslib_core::workbook &wb);

    void defineQuery(InformationSelector::GroupedInfo tags);

  protected slots:
    void defineQuery();
    void saveAnalysis();
    void syncGeometry();

  private:
    bool exportToCSV(const QString &filename);
    bool exportToXLS(const QString &filename);

  private:
    ModelAdapterSPtr m_model;
    QUndoStack      *m_undoStack;
    ViewManagerSPtr  m_viewManager;

    QString     m_title;

    SegmentationAdapterList m_synapses;

    SegmentationExtension::InfoTagList m_synapseTags;
    SegmentationExtension::InfoTagList m_sasTags;
  };

} // namespace EspINA

#endif // ESPINA_SAS_ANALYSIS_ENTRY_H
