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
#include "common/processing/pqData.h"
#include "common/selection/SelectableItem.h"
#include "common/model/Taxonomy.h"

// Forward declarations
class Sample;
class Filter;
class pqPipelineSource;

class Segmentation : public SelectableItem
{
  static const QString NUMBER;
  static const QString OUTPUT;

  class SArguments : public Arguments
  {
  public:
    void setNumber(unsigned int number)
    {
      Number = number;
      (*this)[NUMBER] = QString::number(Number);
    }
    unsigned int number() const {return Number;}

    void setOutput(int output)
    {
      Output = output;
      (*this)[OUTPUT] = QString::number(Output);
    }
    int output() const {return Output;}
  private:
    unsigned int Number;
    int Output;
  };

public:
  explicit Segmentation(Filter *filter, int output, pqData data);
  virtual ~Segmentation();

  pqOutputPort *outputPort();

  /// Model Item Interface
  virtual QString id() const;
  virtual QVariant data(int role) const;
  virtual ItemType type() const {return ModelItem::SEGMENTATION;}
  virtual QString serialize() const;
  /// Selectable Item Interface
  virtual pqData volume() {return m_data;}

  void setNumber(unsigned int number) {m_args.setNumber(number);}
  unsigned int number() const {return m_args.number();}
  void setTaxonomy(TaxonomyNode *tax) {m_tax = tax;}
  TaxonomyNode *taxonomy() const {return m_tax;}
//   virtual void color(double* rgba);
//   virtual void setSelected(bool value) {m_isSelected = value;}
//   virtual bool isSelected() {return m_isSelected;}

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
  Filter             *m_filter;
  pqData              m_data;
  SArguments          m_args;
  TaxonomyNode       *m_tax;

  QMap<QString, SegmentationExtension::SPtr> m_extensions;
  QMap<QString, SegmentationExtension::SPtr> m_pendingExtensions;
  QList<SegmentationExtension::SPtr>         m_insertionOrderedExtensions;
//   QMap<ISegmentationRepresentation::RepresentationId, ISegmentationExtension *> m_representations;
  QMap<QString, SegmentationExtension::SPtr> m_informations;

//   bool m_isSelected;
};

typedef QSharedPointer<Segmentation> SegmentationPtr;

#endif // PRODUCTS_H