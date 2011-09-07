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


#ifndef COUNTINGREGIONEXTENSION_H
#define COUNTINGREGIONEXTENSION_H

#include "CountingRegion.h"
#include "EspinaPlugin.h"

class vtkSMProxy;
// Forward declaration
class Segmentation;
class pqPipelineSource;
class vtkFilter;
class pq3DWidget;

class CountingRegion::SegmentationExtension : public ISegmentationExtension
{
public:
  SegmentationExtension();
  virtual ~SegmentationExtension();
  
  
  //! Implement ISegmentationExtension 
  virtual ExtensionId id();
  virtual void initialize(Segmentation *seg);
  virtual ISegmentationRepresentation* representation(QString rep);
  virtual QVariant information(QString info);
  
  void updateRegions(QMap<QString, BoundingRegion* >& regions);
  
  virtual ISegmentationExtension* clone();
  
private:
  vtkFilter *m_discarted;
};

class CountingRegion::BoundingRegion : public ISampleRepresentation
{
  Q_OBJECT
public:
  static const ISampleRepresentation::RepresentationId ID;
      
  BoundingRegion(Sample* sample);
  virtual ~BoundingRegion();
  
  virtual QString id();
  virtual void render(pqView* view, ViewType type = VIEW_3D);
  virtual pqPipelineSource* pipelineSource();
  
  virtual void setInclusive(int left, int top, int upper) = 0;
  virtual void setExclusive(int right, int bottom, int lower) = 0;
  
public slots:
  virtual void requestUpdate(bool force = false){}
  
protected:
  vtkFilter *m_boundigRegion;
};

class RectangularRegion : public CountingRegion::BoundingRegion
{
public:
  RectangularRegion(Sample* sample, int left, int top, int upper, int right, int bottom, int lower);
  virtual ~RectangularRegion();
  
  virtual void render(pqView* view, ViewType type = VIEW_3D);
  
  virtual void setInclusive(int left, int top, int upper);
  virtual void setExclusive(int right, int bottom, int lower);
  
private:
  vtkSMProxy *m_box;
  pq3DWidget *m_widget[4];
};

class AdaptiveRegion : public CountingRegion::BoundingRegion
{
public:
  AdaptiveRegion(Sample* sample, int left, int top, int upper, int right, int bottom, int lower);
  virtual ~AdaptiveRegion();
  
  virtual void render(pqView* view, ViewType type = VIEW_3D);
  
  virtual void setInclusive(int left, int top, int upper);
  virtual void setExclusive(int right, int bottom, int lower);
  
private:
  pq3DWidget *m_widget[4];
};

class CountingRegion::SampleExtension : public ISampleExtension
{
public:
    SampleExtension();
    virtual ~SampleExtension();
    
    virtual ExtensionId id() {return ID;}
    virtual void initialize(Sample* sample);
    virtual ISampleRepresentation* representation(QString rep);
    virtual QStringList availableRepresentations() {return m_regions.keys();}
    virtual QVariant information(QString info);
    
    QString createAdaptiveRegion(int left, int top, int upper, int right, int bottom, int lower);
    QString createRectangularRegion(int left, int top, int upper, int right, int bottom, int lower);
    void removeRegion(QString &name);
    
    QMap<QString, BoundingRegion *> &regions() {return m_regions;}
    
    virtual ISampleExtension* clone();
private:
  QMap<QString, BoundingRegion *> m_regions;
  int m_numRepresentations;
};

#endif // COUNTINGREGIONEXTENSION_H
