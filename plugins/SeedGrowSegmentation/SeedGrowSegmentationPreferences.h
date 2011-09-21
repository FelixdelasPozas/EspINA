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

class BestPixelSelector;
class SeedGrowSegmentationPanel : public QWidget, public Ui::SeedGrowSegmentationPreferences
{
  Q_OBJECT
public:
  explicit SeedGrowSegmentationPanel(BestPixelSelector *selector);
  virtual ~SeedGrowSegmentationPanel();
  
public slots:
  void setBestPixelValue(int value);
  
private:
  BestPixelSelector *m_selector;
};

class SeedGrowSegmentationPreferences : public IPreferencePanel
{
public:
  SeedGrowSegmentationPreferences(BestPixelSelector *sel) : m_selector(sel){}
  
  virtual const QString shortDescription() {return "Seed Grow Segmentation";}
  virtual const QString longDescription() {return "Seed Grow Segmentation Preferences";}
  virtual const QIcon icon() {return QIcon(":/bestPixelSelector.svg");}

  virtual QWidget* widget();
  
private:
  BestPixelSelector *m_selector;
};

#endif // SEEDGROWSEGMENTATIONPREFERENCES_H
