
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


#include <QActionGroup>
#include <plugins/FilterFactory.h>

#include <selection/SelectionHandler.h>
#include "SeedGrowSelector.h"

#include <QSharedPointer>
#include <QWidgetAction>

class SeedGrowSegmentationFilter;
//Forward declarations
class SegmentAction;
class DefaultVOIAction;
class ThresholdAction;


//! Seed Growing Segmenation Plugin
class SeedGrowSegmentation
: public QActionGroup
, public FilterFactory
//       , public IFilterFactory
{
  Q_OBJECT
public:
  SeedGrowSegmentation(QObject* parent);
  virtual ~SeedGrowSegmentation();

  virtual FilterPtr createFilter(const QString filter, const QString args);

protected slots:
  /// Wait for Seed Selection
  void waitSeedSelection(QAction *action);
  /// Abort current selection
  void abortSelection();
  void onSelectionAborted();
  /// Starts the segmentation filter putting using @msel as seed
  void startSegmentation(SelectionHandler::MultiSelection msel);
  void modifyLastFilter(int value);

signals:
//   void productCreated(Segmentation *);
  void selectionAborted(SelectionHandler *);

private:
  void addPixelSelector(QAction *action, SelectionHandler *handler);
  void buildSelectors();

private:
  ThresholdAction  *m_threshold;
  DefaultVOIAction *m_useDefaultVOI;
  SegmentAction    *m_segment;
  QMap<QAction *, SelectionHandler *> m_selectors;
  QSharedPointer<SeedGrowSelector> m_selector;
//   SeedGrowSegmentationSettings *m_preferences;
};

#endif// SEEDGROWINGSEGMENTATION_H