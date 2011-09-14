/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

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

#ifndef SAMPLE_H
#define SAMPLE_H

#include "products.h"

// Forward declarations
class pqPipelineSource;

class Sample : public EspinaProduct
{
public:
  Sample(vtkFilter *creator, int portNumber, const QString &path=QString()) 
  : EspinaProduct(NULL, creator, portNumber)
  , m_extent(NULL)
  , m_path(path)
  {}
  virtual ~Sample();
  
  //virtual EspinaId id(){return name;}
  //! Reimplements ITraceNode Interface
  virtual QString getArguments();
  virtual QString label() const;
  
  virtual QVariant data(int role = Qt::UserRole + 1) const;
  virtual bool setData(const QVariant& value, int role = Qt::UserRole + 1);
  
  void extent(int *out);
  void bounds(double *out);
  void spacing(double* out);
  
  void setSpacing(double x, double y, double z);
  
  QList<Segmentation *> segmentations() const {return m_segs;}
  void addSegmentation(Segmentation *seg);
  void removeSegmentation(Segmentation *seg);
  
  //! Add a new extension to the sample
  //! If extension dependencies are not satisfied it won't be available
  //! until all of them are satisfied
  void addExtension(ISampleExtension *ext);
  //! Are supposed to be used for sort time 
  ISampleExtension *extension(ExtensionId extId);
    
  QStringList availableRepresentations();
  ISampleRepresentation *representation(QString rep);
  
  QStringList availableInformations();
  QVariant information(QString info);
  
  void initialize();
  
private:
  int *m_extent;
  double *m_bounds, *m_spacing;
  QMutex mutex;
  
private:
  QString m_path;
  QList<Segmentation *> m_segs;
  
  QMap<ExtensionId, ISampleExtension *> m_extensions;
  QMap<ExtensionId, ISampleExtension *> m_pendingExtensions;
  QList<ISampleExtension *> m_insertionOrderedExtensions;
  QMap<ISampleRepresentation::RepresentationId, ISampleExtension *> m_representations;
  QMap<QString, ISampleExtension *> m_informations;
};

#endif // SAMPLE_H