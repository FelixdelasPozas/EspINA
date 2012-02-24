/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor<jpena@cesvima.upm.es>

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

//----------------------------------------------------------------------------
// File:    Segmentation.h
// Purpose: Model biological structures which have been extracted from one or
//          more channels.
//----------------------------------------------------------------------------
#ifndef SEGMENTATION_H
#define SEGMENTATION_H

#include <common/extensions/SegmentationExtension.h>
#include "common/model/Filter.h"
#include "common/processing/pqData.h"
#include "common/selection/SelectableItem.h"

// Forward declarations
class Sample;
class pqPipelineSource;

class Segmentation : public SelectableItem
{
public:
  explicit Segmentation(Filter *filter, pqData data);
//   Segmentation(EspinaFilter *parent, vtkFilter *creator, int portNumber);
  virtual ~Segmentation();

  pqOutputPort *outputPort();

  /// Model Item Interface
  virtual QVariant data(int role) const;
  virtual ItemType type() const {return ModelItem::SEGMENTATION;}
  /// Selectable Item Interface
  virtual pqData volume() {return m_data;}

//   //! Reimplement ITraceNode Interface
//   virtual QString label() const {return QString("%1 %2").arg(m_taxonomy->name()).arg(m_id);}
//   void setId(int id) {m_id = id;}
//   bool validId() {return m_id != -1;}
//   
//   virtual void color(double* rgba);
//   virtual void setSelected(bool value) {m_isSelected = value;}
//   virtual bool isSelected() {return m_isSelected;}
//   
//   //! Reimplement ModelItem Interface
//   virtual QVariant data(int role = Qt::UserRole + 1) const;
//   virtual bool setData(const QVariant& value, int role = Qt::UserRole +1);

  /// Add a new extension to the segmentation
  /// Extesion won't be available until requirements are satisfied
  void addExtension(SegmentationExtension::SPtr ext);
//   //! Are supposed to be used for sort time 
//   ISegmentationExtension *extension(ExtensionId extId);
//   
//   QStringList availableRepresentations() const;
//   ISegmentationRepresentation *representation(QString rep);
//   
//   QStringList availableInformations() const;
//   QVariant information(QString info) const;

  void initialize();

//   void notifyInternalUpdate();
//   
// signals:
//   void updated(Segmentation *);
  
private:
  Filter *m_filter;
  pqData  m_data;

  QMap<QString, SegmentationExtension::SPtr> m_extensions;
  QMap<QString, SegmentationExtension::SPtr> m_pendingExtensions;
  QList<SegmentationExtension::SPtr>         m_insertionOrderedExtensions;
//   QMap<ISegmentationRepresentation::RepresentationId, ISegmentationExtension *> m_representations;
  QMap<QString, SegmentationExtension::SPtr> m_informations;
//   int m_id;

//   bool m_isSelected;
};

typedef QSharedPointer<Segmentation> SegmentationPtr;

#endif // PRODUCTS_H