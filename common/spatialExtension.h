#ifndef SPATIALREPRESENTATION_H
#define SPATIALREPRESENTATION_H

#include <EspinaPlugin.h>
class vtkFilter;

namespace SpatialExtension
{

  static const ExtensionId ID = "00_SpatialExtension";
  
  class SampleRepresentation : public ISampleRepresentation
  {
  public:
    SampleRepresentation(Sample* sample);
    virtual ~SampleRepresentation();

    virtual QString id();
    virtual void render(pqView *view, ViewType type = VIEW_3D);
    virtual pqPipelineSource *pipelineSource();
    
    void setSpacing(double x, double y, double z);

  public slots:
    virtual void requestUpdate(bool force = false){};
    
  private:
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

}
#endif // SPATIALREPRESENTATION_H
