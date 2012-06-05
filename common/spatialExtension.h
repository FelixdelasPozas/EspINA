#ifndef SPATIALREPRESENTATION_H
#define SPATIALREPRESENTATION_H

#include <EspinaPlugin.h>
class vtkFilter;

namespace SpatialExtension
{

  static const QString ID = "SpatialExtension";
  
  class SampleRepresentation : public ISampleRepresentation
  {
  public:
    static const ISampleRepresentation::RepresentationId ID;
  
    SampleRepresentation(Sample* sample);
    virtual ~SampleRepresentation();

    virtual QString id();
    virtual void render(pqView *view, ViewType type = VIEW_3D);
    virtual pqPipelineSource *pipelineSource();
    
    void setSpacing(double x, double y, double z);
    void spacing(double value[3]) const;

  public slots:
    virtual void requestUpdate(bool force = false){};
    
  private:
    vtkFilter *m_rep;
    double m_spacing[3];
  };

  class SampleExtension : public ISampleExtension
  {
  public:
    SampleExtension();
    virtual ~SampleExtension();
    virtual ExtensionId id(){return ID;}
    virtual void initialize(Sample* sample);
    virtual ISampleRepresentation* representation(QString rep);
    virtual QVariant information(QString info);
    
    virtual ISampleExtension* clone();
   
  private:
    SampleRepresentation *m_spatialRep;
  };

}
#endif // SPATIALREPRESENTATION_H
