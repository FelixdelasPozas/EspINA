/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

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


#ifndef EDITORTOOLBAR_H
#define EDITORTOOLBAR_H

#include <QToolBar>
#include <common/gui/DynamicWidget.h>

#include <common/editor/PencilSelector.h>
#include <common/model/Segmentation.h>
#include <common/pluginInterfaces/FilterFactory.h>
#include <common/selection/SelectionHandler.h>

class FreeFormSource;


class EditorToolBar
: public QToolBar
, public DynamicWidget
, public FilterFactory
{
  Q_OBJECT
  class FreeFormCommand;
  class DrawCommand;
  class EraseCommand;
  class CODECommand;//CloseOpenDilateErode Command

public:
  class Settings;
  class SettingsPanel;

public:
  explicit EditorToolBar(QWidget *parent = 0);

  virtual void setActivity(QString activity){}
  virtual void setLOD(){}
  virtual void decreaseLOD(){}
  virtual void increaseLOD(){}

  virtual Filter* createFilter(const QString filter,
                               Filter::NamedInputs inputs,
                               const ModelItem::Arguments args);

protected slots:
  void startDrawing(bool draw);
  void drawSegmentation(SelectionHandler::MultiSelection msel);
  void stopDrawing();
  void stateChanged(PencilSelector::State state);
  void combineSegmentations();
  void substractSegmentations();
  void erodeSegmentations();
  void dilateSegmentations();
  void openSegmentations();
  void closeSegmentations();
  void fillHoles();

  SegmentationList selectedSegmentations();

private:
  QAction *m_draw;
  QAction *m_addition;
  QAction *m_substraction;
  QAction *m_erode;
  QAction *m_dilate;
  QAction *m_open;
  QAction *m_close;
  QAction *m_fill;

  Settings       *m_settings;
  PencilSelector *m_pencilSelector;
  Filter         *m_currentSource;
  Segmentation   *m_currentSeg;
};

#endif // EDITORTOOLBAR_H
