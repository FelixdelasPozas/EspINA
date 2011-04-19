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


#ifndef ESPINAFACTORY_H
#define ESPINAFACTORY_H

#include "ui/volumeView.h"

class Segmentation;
class pqPipelineSource;
class ISegmentationExtension;

#include <QList>
#include <QString>

class EspINAFactory
{
public:
  static EspINAFactory *instance();
  
  Segmentation *CreateSegmentation(pqPipelineSource* source, int portNumber, QString parentHash);
  void addSegmentationExtension(ISegmentationExtension *ext);
  
  VolumeView *CreateVolumeView();
  void addViewWidget(IViewWidget *widget);
  
  //TODO: CreateSliceView
  //TODO: AddPreprocessedView
  
private:
  EspINAFactory(){};
  
  static EspINAFactory *m_instance;
  QList<ISegmentationExtension *> m_extensions;
  QList<IViewWidget *> m_widgets;
  //TODO: SliceViewExtension List
};

#endif // ESPINAFACTORY_H
