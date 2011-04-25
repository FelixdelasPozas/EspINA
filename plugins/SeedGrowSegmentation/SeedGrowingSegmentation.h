
/*=========================================================================

   Program: ParaView
   Module:    SegmentationToolbarActions.h

   Copyright (c) 2005-2008 Sandia Corporation, Kitware Inc.
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

=========================================================================*/
#ifndef SEEDGROWINGSEGMENTATION_H
#define SEEDGROWINGSEGMENTATION_H

#include "EspinaPlugin.h"
#include "iSegmentationPlugin.h"
#include "filter.h"
#include "selectionManager.h"

#include <QToolButton>

class QMenu;//Forward declarations
class QSpinBox;
class Product;



//! Seed Growing Segmenation Plugin
class SeedGrowingSegmentation 
: public ISegmentationPlugin
, public EspinaPlugin
{
  Q_OBJECT
  
public:
  SeedGrowingSegmentation(QObject* parent);
  
  void LoadAnalisys(EspinaParamList& args);
  
protected slots:
  //! Changes the method to select the input seed
  void changeSeedSelector(QAction *seedSel);
  //! Wait for Seed Selection
  void waitSeedSelection(bool wait);
  //! Starts the segmentation filter putting a seed at @x, @y, @z.
  void startSegmentation(SelectionHandler::Selection sel, SelectionHandler::VtkRegions regions);
  
signals:
  void segmentationCreated(ProcessingTrace *);
  void productCreated(Segmentation *);
  void selectionAborted(SelectionHandler *);
  
private:
  
  void buildSelectors();
  void buildUI();
  
  void initBlurTable();
  void initGrowTable();
  
  void addPixelSelector(QAction *action, SelectionHandler *handler);
  
  void buildSubPipeline(Product* input, EspinaParamList args);
  
private:
  QSpinBox *m_threshold;
  QToolButton *m_segButton;
  QMenu *m_selectors;
  SelectionHandler *m_seedSelector;
  QMap<QAction *, SelectionHandler *> m_seedSelectors;
  
  TranslatorTable m_tableBlur;
  TranslatorTable m_tableGrow;
};

#endif// SEEDGROWINGSEGMENTATION_H