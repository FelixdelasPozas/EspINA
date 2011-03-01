
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
#include "slicer.h"
#include "distance.h"

// Qt
#include <QString>
#include <QMap>

//Forward declaration
class ObjectManager;
class QMenu;
class pqPipelineSource;
class SliceWidget;
class VolumeWidget;
class Segmentation;
class Stack;
class UnitExplorer;
class SelectionManager;

/// MainWindow for the default ParaView application.
class EspinaMainWindow : public QMainWindow
{
  Q_OBJECT
  typedef QMainWindow Superclass;
public:
  EspinaMainWindow();
  ~EspinaMainWindow();

protected slots:
  void loadTrace();
  void loadData(pqPipelineSource *source);
  void importData(pqPipelineSource *source){}//TODO
  void toggleVisibility(bool visible);


private:
  EspinaMainWindow(const EspinaMainWindow&); // Not implemented.
  void operator=(const EspinaMainWindow&); // Not implemented.

  void buildFileMenu(QMenu &menu);

  class pqInternals;
  pqInternals* Internals;
  ObjectManager *m_productManager;
  QMap<QString,Stack *> m_stacks;
  SliceBlender *m_planes[SLICE_PLANE_LAST+1];
  SliceWidget *m_xy, *m_yz, *m_xz;
  VolumeWidget *m_3d;
  DistUnit m_unit;
  UnitExplorer *m_unitExplorer;
  SelectionManager *m_selectionManager;
};

#endif //ESPINA_MAIN_WINDOW_H


