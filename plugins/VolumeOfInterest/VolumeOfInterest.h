
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
#ifndef VOLUMEOFINTEREST_H
#define VOLUMEOFINTEREST_H

#include <QActionGroup>
#include "EspinaPlugin.h"
#include "interfaces.h"
#include "selectionManager.h"

//Forward declarations
class QSpinBox;
class QToolButton;
class IPixelSelector;
class Product;

//! Volume Of Interest Plugin
class VolumeOfInterest 
: public QActionGroup
{
  Q_OBJECT
  
public:
  VolumeOfInterest(QObject* parent);
  
protected slots:
  //! Changes VOI enable state
  void enable(bool value);
  //! Changes the VOI
  void changeVOI(QAction *voi);
  //! Cancel current VOI
  void cancelVOI();
  
  void focusSampleChanged(Sample *sample);
  
signals:
  void voiCancelled(IVOI *);
  
private:
  void buildVOIs();
  void buildUI();
  
  void addVOI(QAction *action, IVOI *voi);
  
  void buildSubPipeline(Product* input, EspinaParamList args);
  
private:
  QToolButton *m_voiButton;
  QSpinBox *m_fromSlice;
  QSpinBox *m_toSlice;
  QMenu *m_VOIMenu;
  IVOI *m_activeVOI;
  QMap<QAction *, IVOI *> m_VOIs;
};

#endif// VOLUMEOFINTEREST_H