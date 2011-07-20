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


#ifndef LABELMAPEXTENSION_H
#define LABELMAPEXTENSION_H

#include "EspinaPlugin.h"

class vtkFilter;
class Sample;
namespace LabelMapExtension
{
  static const ExtensionId ID = "02_LabelMapExtension";
  
  class SampleRepresentation : public ISampleRepresentation
  {
    Q_OBJECT
  public:
    SampleRepresentation(Sample* sample);
    virtual ~SampleRepresentation();
    
    virtual QString id();
    virtual pqPipelineSource* pipelineSource();
    virtual void render(pqView* view, ViewType type = VIEW_3D);
    
  public slots:
    virtual void requestUpdate(bool force=false);
  
  public slots:
    void setEnable(bool value);
  
  private:
    vtkFilter *m_rep;
    bool m_enable;
    int m_numberOfBlendedSeg;
  };
  
  class SampleExtension : public ISampleExtension
  {
  public:
    SampleExtension(QAction *toggleVisibility);
    virtual ~SampleExtension();
    virtual ExtensionId id() {return ID;}
    virtual void initialize(Sample* sample);
    virtual void addInformation(InformationMap& map);
    virtual void addRepresentations(RepresentationMap& map);
    
    virtual ISampleExtension* clone();
    
  private:
    
    QAction *m_toggleVisibility;
  };

}//namespace LabelMapExtension

#endif // LABELMAPEXTENSION_H
