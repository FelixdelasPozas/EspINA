/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>

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


#ifndef SEEDGROWSEGMENTATIONPREFERENCES_H
#define SEEDGROWSEGMENTATIONPREFERENCES_H

#include <../frontend/IPreferencePanel.h>
#include "ui_SeedGrowSegmentationPreferences.h"

const QString BEST_PIXEL("SeedGrowSegmentation::BestPixel");
const QString DEFAULT_VOI_X("SeedGrowSegmentation::DafaultVOI::X");
const QString DEFAULT_VOI_Y("SeedGrowSegmentation::DafaultVOI::Y");
const QString DEFAULT_VOI_Z("SeedGrowSegmentation::DafaultVOI::Z");

class BestPixelSelector;
class SeedGrowSegmentationSettings;
class SeedGrowSegmentationPanel : public QWidget, public Ui::SeedGrowSegmentationPreferences
{
  Q_OBJECT
public:
  explicit SeedGrowSegmentationPanel(SeedGrowSegmentationSettings* pref);
  virtual ~SeedGrowSegmentationPanel();
  
public slots:
  void setBestPixelValue(int value);
  
private:
  BestPixelSelector *m_selector;
};

class SeedGrowSegmentationSettings : public IPreferencePanel
{
  Q_OBJECT
public:
  SeedGrowSegmentationSettings(BestPixelSelector *sel);
  
  virtual const QString shortDescription() {return "Seed Grow Segmentation";}
  virtual const QString longDescription() {return "Seed Grow Segmentation Preferences";}
  virtual const QIcon icon() {return QIcon(":/bestPixelSelector.svg");}

  virtual QWidget* widget();
  
  
  int xSize(){return m_xSize;}
  int ySize(){return m_ySize;}
  int zSize(){return m_zSize;}
public slots:
  void setXSize(int value);
  void setYSize(int value);
  void setZSize(int value);
  
  void setBestPixelValue(int value);
  
protected:
  BestPixelSelector *m_selector;
  int m_xSize, m_ySize, m_zSize;

  friend class SeedGrowSegmentationPanel;
};

#endif // SEEDGROWSEGMENTATIONPREFERENCES_H
