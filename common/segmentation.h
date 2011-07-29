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

#ifndef SEGMENTATION_H
#define SEGMENTATION_H

#include "products.h"

// Forward declarations
class pqPipelineSource;
class Sample;

class Segmentation : public EspinaProduct
{
public:
  Segmentation(EspinaFilter *parent, vtkFilter *creator, int portNumber);
  virtual ~Segmentation();
  
  //! Reimplement ITraceNode Interface
  virtual QString label() const {return QString("Segmentation %1").arg(m_id);}
  
  //! Reimplement IModelItem Interface
  virtual QVariant data(int role = Qt::UserRole + 1) const;
  virtual bool setData(const QVariant& value, int role = Qt::UserRole +1);
  
  //! Add a new extension to the segmentation
  //! If extension pre-requirements are not satisfied it won't be available
  //! until all of them are satisfied
  void addExtension(ISegmentationExtension *ext);
  //! Are supposed to be used for sort time 
  ISegmentationExtension *extension(ExtensionId extId);
  
  QStringList availableRepresentations();
  ISegmentationRepresentation *representation(QString rep);
  
  QStringList availableInformations();
  QVariant information(QString info);
  
  void initialize();
  
private:
  QMap<ExtensionId, ISegmentationExtension *> m_extensions;
  QMap<ExtensionId, ISegmentationExtension *> m_pendingExtensions;
  QList<ISegmentationExtension *> m_insertionOrderedExtensions;
  QMap<ISegmentationRepresentation::RepresentationId, ISegmentationExtension *> m_representations;
  QMap<QString, ISegmentationExtension *> m_informations;
  int m_id;
  static int s_newId;
//   ISegmentationExtension::InformationMap m_infoMap;
//   ISegmentationExtension::RepresentationMap m_repMap;
};

#endif // PRODUCTS_H