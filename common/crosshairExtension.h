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


#ifndef CROSSHAIREXTENSION_H
#define CROSSHAIREXTENSION_H

#include <EspinaPlugin.h>

namespace LabelMapExtension {
class SampleRepresentation;
}

class vtkFilter;

class CrosshairRepresentation : public ISampleRepresentation
{
  Q_OBJECT
  
public:
  CrosshairRepresentation(Sample* sample);
  virtual ~CrosshairRepresentation();
  
  virtual QString id();
  virtual void render(pqView* view, ViewType type = VIEW_3D);
  virtual pqPipelineSource* pipelineSource();
  
public slots:
  virtual void requestUpdate(bool force){}
  
  void setSlice(int slice, ViewType type);
  void centerOn(int x, int y, int z);
  
protected slots:
  void internalRepresentationUpdated();
  
private:
  vtkFilter *m_planes[3];
  LabelMapExtension::SampleRepresentation *m_internalRep;
  bool m_disabled;
};


class CrosshairExtension : public ISampleExtension
{
  static const ExtensionId ID;
public:
  virtual ExtensionId id() {return ID;}
  virtual void initialize(Sample* sample);
  virtual void addInformation(ISampleExtension::InformationMap& map);
  virtual void addRepresentations(ISampleExtension::RepresentationMap& map);
  
  virtual ISampleExtension* clone();
};

#endif // CROSSHAIREXTENSION_H
