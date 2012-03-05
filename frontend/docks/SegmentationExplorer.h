/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef SEGMENTATIONEXPLORER_H
#define SEGMENTATIONEXPLORER_H

//----------------------------------------------------------------------------
// File:    SegmentationExplorer.h
// Purpose: Dock widget to manage segmentations in the model
//----------------------------------------------------------------------------
#include <common/gui/EspinaDockWidget.h>
#include <ui_SegmentationExplorer.h>

class EspinaModel;

#define DEBUG

#ifdef DEBUG
class ModelTest;
#endif

class SegmentationExplorer : public EspinaDockWidget
{
  Q_OBJECT

  class GUI;
  class State;
public:
  explicit SegmentationExplorer(QSharedPointer<EspinaModel> model, QWidget *parent = 0);
  virtual ~SegmentationExplorer();

protected slots:
  void deleteSegmentation();

protected:
  GUI *m_gui;
  QSharedPointer<EspinaModel> m_baseModel;
  State *m_state;
private:
#ifdef DEBUG
  QSharedPointer<ModelTest>   m_modelTester;
#endif
};

#endif // SEGMENTATIONEXPLORER_H
