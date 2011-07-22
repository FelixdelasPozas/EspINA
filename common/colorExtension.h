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


#ifndef COLORREPRESENTATION_H
#define COLORREPRESENTATION_H

#include <EspinaPlugin.h>

class vtkFilter;
class pqScalarsToColors;
class vtkSMRGBALookupTableProxy;
class Sample;

namespace ColorExtension
{

  static const ExtensionId ID = "01_ColorExtension";
  
  class SampleRepresentation : public ISampleRepresentation
  {
  public:
    SampleRepresentation(Sample* sample);
    virtual ~SampleRepresentation();
    
    virtual QString id();
  virtual void render(pqView* view, ViewType type = VIEW_3D);
    virtual pqPipelineSource* pipelineSource();
    
  public slots:
    virtual void requestUpdate(bool force=false){}
    
  private:
    pqScalarsToColors *m_LUT;
    vtkFilter *m_rep;
  };

  class SegmentationRepresentation : public ISegmentationRepresentation
  {
  public:
    SegmentationRepresentation(Segmentation* seg);
    virtual ~SegmentationRepresentation();
    
    virtual QString id();
    virtual void render(pqView* view);
    virtual pqPipelineSource* pipelineSource();
    
  public slots:
    virtual void requestUpdate(bool force=false);
    
  private:
    vtkSMRGBALookupTableProxy *m_LUT;
    vtkFilter *m_rep;
  };
  
  class SampleExtension : public ISampleExtension
  {
  public:
    virtual ExtensionId id(){return ID;}
    virtual void initialize(Sample* sample);
    virtual void addInformation(InformationMap& map);
    virtual void addRepresentations(RepresentationMap& map);
    
    virtual ISampleExtension* clone();
  };


  class SegmentationExtension : public ISegmentationExtension
  {
  public:
    virtual ExtensionId id() {return ID;}
    virtual void initialize(Segmentation* seg);
    virtual ISegmentationRepresentation* representation(QString rep);
    virtual QVariant information(QString info);
    
    virtual ISegmentationExtension* clone();
  };
  
}//namespace ColorExtension

#endif // COLORREPRESENTATION_H
