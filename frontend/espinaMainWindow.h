/*=========================================================================

   Program: Espina
   Module:    espinaViewMainWindow.h

   Copyright (c) 2005,2006 Sandia Corporation, Kitware Inc.
   All rights reserved.

   ParaView is a free software; you can redistribute it and/or modify it
   under the terms of the ParaView license version 1.2.

   See License_v1.2.txt for the full ParaView license.
   A copy of this license can be obtained by contacting
   Kitware Inc.
   28 Corporate Drive
   Clifton Park, NY 12065
   USA

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

========================================================================*/
#ifndef ESPINA_MAIN_WINDOW_H
#define ESPINA_MAIN_WINDOW_H

#include <QMainWindow>

//Necessary to use Planes enumeration
#include "distance.h"

// Qt
#include <QString>
#include <QList>

class QTreeView;
class QComboBox;
class TaxonomyNode;
class SampleProxy;
//Forward declaration
class EspINA;
class SliceView;
class VolumeView;
class QMenu;
class SelectionManager;
class QAbstractItemModel;
class QModelIndex;
// #include "pqLoadDataReaction.h" // TODO debug

/// MainWindow for the default ParaView application.
class EspinaMainWindow : public QMainWindow
{
  Q_OBJECT
  typedef QMainWindow Superclass;
  class pqInternals;
public:
  EspinaMainWindow();
  ~EspinaMainWindow();
  
protected slots:
//   void loadData(pqPipelineSource *source);
  void loadFile(QString method);
  void saveFile();
  void importFile(); // Local load 
  void exportFile(); // Local save

  // Manage Taxonomy Editor
  void addTaxonomyElement();
  void addTaxonomyChildElement();
  void removeTaxonomyElement();
  void changeTaxonomyColor();
  void resetTaxonomy();
  
  // Manage Sample Explorer
  void focusOnSample();
  
  void toggleVisibility(bool visible);
  virtual bool eventFilter(QObject* obj, QEvent* event);
  
  void setGroupView(int idx);
  void deleteSegmentations();

  //TODO delete
  void autoLoadStack();
  
private:
  EspinaMainWindow(const EspinaMainWindow&); // Not implemented.
  void operator=(const EspinaMainWindow&); // Not implemented.

  void buildFileMenu(QMenu &menu);

  EspINA *m_espina;
  DistUnit m_unit;
  
  // GUI
  pqInternals* Internals;
  QComboBox* m_taxonomySelector;
  QTreeView *m_taxonomyView;
  SliceView *m_xy, *m_yz, *m_xz;;
  VolumeView *m_3d;
  SelectionManager *m_selectionManager;
  QStringList m_groupingName;
  QList<QAbstractItemModel *> m_groupingModel;
  QList<QModelIndex> m_groupingRoot;
  int m_lastTaxonomyId;
  SampleProxy *sampleProxy;
};

#endif //ESPINA_MAIN_WINDOW_H


