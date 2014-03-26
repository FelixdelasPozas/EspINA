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


#ifndef SYNAPTICAPPOSITIONSURFACEANALYSIS_H
#define SYNAPTICAPPOSITIONSURFACEANALYSIS_H

#include "AppositionSurfacePlugin_Export.h"

#include "ui_SynapticAppositionSurfaceAnalysis.h"
#include <Core/EspinaTypes.h>
#include <Core/Model/Segmentation.h>

class QUndoStack;

namespace EspINA
{
  class ModelAdapter;
  class ViewManager;

  class AppositionSurfacePlugin_EXPORT SASAnalysisEntry
  : public QDialog
  , public Ui::SynapticAppositionSurfaceAnalysis
  {
    Q_OBJECT
  public:
    explicit SASAnalysisEntry(SegmentationList segmentations,
                                               ModelAdapter     *model,
                                               QUndoStack      *undoStack,
                                               ViewManager     *viewManager,
                                               QWidget         *parent);
    void displayInformation();

  protected slots:
    void defineQuery();
    void saveAnalysis();
    void syncGeometry();

  private:
    bool exportToCSV(const QString &filename);
    bool exportToXLS(const QString &filename);

  private:
    ModelAdapter *m_model;
    QUndoStack  *m_undoStack;
    ViewManager *m_viewManager;

    SegmentationList m_synapses;
    SegmentationList m_sas;

    Segmentation::InfoTagList m_synapseTags;
    Segmentation::InfoTagList m_sasTags;
  };

} // namespace EspINA

#endif // SYNAPTICAPPOSITIONSURFACEANALYSIS_H
