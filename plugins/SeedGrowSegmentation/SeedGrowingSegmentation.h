
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
#include "interfaces.h"
#include "traceNodes.h"

//Forward declarations
class QSpinBox;
class QToolButton;
class IPixelSelector;
class Product;

//! Seed Growing Segmenation Plugin
class SeedGrowingSegmentation : public ISegmentationPlugin, public EspinaPlugin, public ISelectionHandler
{
  Q_OBJECT
  
public:
  SeedGrowingSegmentation(QObject* parent);
  
  void LoadAnalisys(EspinaParamList& args);
  
  //! Implements ISelectionHandler interface
  void handle(const Selection sel);
  void abortSelection();
  
  //! Implements ISegmentationPlugin interface
  void execute();
  
public slots:
  /// Callback for each action triggerred.
  void onAction(QAction* action);
  void setActive(bool active);
  
signals:
  void segmentationCreated(ProcessingTrace *);
  void productCreated(Segmentation *);
  void waitingSelection(ISelectionHandler *);
  void selectionAborted(ISelectionHandler *);
  
private:
  void buildUI();
  
  void buildSubPipeline(Product* input, EspinaParamList args);
  
private:
  QSpinBox *m_threshold;
  QToolButton *m_addSeed;
  IPixelSelector *m_selector;
  Selection m_sel;
  TranslatorTable m_tableBlur;
  TranslatorTable m_tableGrow;
};

#endif// SEEDGROWINGSEGMENTATION_H