/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>
 * 
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 * 
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 * 
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef CROSSHAIREXTENSION_H
#define CROSSHAIREXTENSION_H

#include <EspinaPlugin.h>

namespace LabelMapExtension {
  class SampleRepresentation;
}

class vtkFilter;

namespace CrosshairExtension {
  
  static const QString ID = "CrosshairExtensionExtension";
  
  class SampleRepresentation : public ISampleRepresentation
  {
    Q_OBJECT
    
  public:
    static const ISampleRepresentation::RepresentationId ID;
    
  
    SampleRepresentation(Sample* sample);
    virtual ~SampleRepresentation();
    
    virtual QString id();
    virtual void render(pqView* view, ViewType type = VIEW_3D);
    virtual pqPipelineSource* pipelineSource();
    
  public slots:
    virtual void requestUpdate(bool force){}
    
    void setSlice(int slice, ViewType type, bool update=true);
    int slice(ViewType type);
    void centerOn(int x, int y, int z);
    
  protected slots:
    void internalRepresentationUpdated();
    
  private:
    vtkFilter *m_planes[3];
    LabelMapExtension::SampleRepresentation *m_internalRep;
    bool m_disabled;
    int m_center[4];
  };
  
  
  class SampleExtension : public ISampleExtension
  {
  public:
    SampleExtension();
    virtual ~SampleExtension();
    
    virtual ExtensionId id() {return ID;}
    virtual void initialize(Sample* sample);
    virtual QStringList dependencies();
    virtual ISampleRepresentation* representation(QString rep);
    virtual QVariant information(QString info);
    
    virtual ISampleExtension* clone();
  
  private:
    SampleRepresentation *m_crossRep;
  };

//   static SampleRepresentation *SampleRepresentation(Sample *sample);
  
} // namespace CrosshairExtension

#endif // CROSSHAIREXTENSION_H
