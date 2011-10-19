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


#ifndef VOLUMETRICEXTENSION_H
#define VOLUMETRICEXTENSION_H

#include <EspinaPlugin.h>

class pqScalarsToColors;

class VolumetricRepresentation : public ISegmentationRepresentation
{
public:
  static const ISegmentationRepresentation::RepresentationId ID;
  
  VolumetricRepresentation(Segmentation* seg);
  virtual ~VolumetricRepresentation();
  
  virtual QString id();
  virtual void render(pqView* view);
  virtual pqPipelineSource* pipelineSource();
  
public slots:
  virtual void requestUpdate(bool force=false) {};
  
private:
  pqScalarsToColors *m_LUT;  
};

class VolumetricExtension : public ISegmentationExtension
{
public:
  static const ExtensionId ID;
  
public:
  VolumetricExtension();
  virtual ~VolumetricExtension();
  
  virtual ExtensionId id();
  virtual void initialize(Segmentation* seg);
  virtual QStringList dependencies() {return ISegmentationExtension::dependencies();}
  virtual QStringList availableRepresentations() {return ISegmentationExtension::availableRepresentations();}
  virtual ISegmentationRepresentation* representation(QString rep);
  virtual QStringList availableInformations() {return ISegmentationExtension::availableInformations();}
  virtual QVariant information(QString info);
  
  virtual ISegmentationExtension* clone();
  
private:
  VolumetricRepresentation *m_volRep;
};

#endif // VOLUMETRICEXTENSION_H
