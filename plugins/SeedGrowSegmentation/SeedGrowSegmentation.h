
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
#include <iSegmentationPlugin.h>
#include "selectionManager.h"

#include <QToolButton>

class QMenu;//Forward declarations
class QSpinBox;
class Product;
class QString;



//! Seed Growing Segmenation Plugin
class SeedGrowSegmentation
      : public ISegmentationPlugin
      , public IFilterFactory
{
  Q_OBJECT

public:
  SeedGrowSegmentation(QObject* parent);

  IFilter *createFilter(QString filter, ITraceNode::Arguments& args);

protected slots:
  //! Changes the method to select the input seed
  void changeSeedSelector(QAction *seedSel);
  //! Wait for Seed Selection
  void waitSeedSelection(bool wait);
  //! Abort current selection
  void abortSelection();
  //! Starts the segmentation filter putting a seed at @x, @y, @z.
  void startSegmentation(ISelectionHandler::Selection sel);

signals:
  void segmentationCreated(ProcessingTrace *);
  void productCreated(Segmentation *);
  void selectionAborted(ISelectionHandler *);

private:
  void buildSelectors();
  void buildUI();

  void addPixelSelector(QAction *action, ISelectionHandler *handler);

private:
  QSpinBox *m_threshold;
  QToolButton *m_segButton;
  QMenu *m_selectors;
  ISelectionHandler *m_seedSelector;
  QMap<QAction *, ISelectionHandler *> m_seedSelectors;
};

#endif// SEEDGROWINGSEGMENTATION_H