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

#ifndef PRODUCTS_H
#define PRODUCTS_H

#include "interfaces.h"
#include "processingTrace.h"
#include "EspinaPlugin.h"

// libCajal
#include "data/modelItem.h"
#include <data/taxonomy.h>

#include <QString>
#include <QMutex>

#include <Utilities/vxl/vcl/iso/vcl_iostream.h>

// Forward declarations
class pqPipelineSource;
class Sample;

// typedef QString EspinaId;
// class ISingleton
// {
// public:
//   virtual EspinaId id() = 0;
// };
class vtkFilter;

class vtkProduct
{
public:
  //! Creates a product for an already existing filter given its id
  vtkProduct(const QString &id);
  //! Creates a product for an already existing filter
  vtkProduct(vtkFilter *creator, int portNumber);
  
  QString id(); //TODO: idc vtkfilter creator + : + puerto
  vtkFilter *creator() {return m_creator;}
  int portNumber() {return m_portNumber;}
  pqOutputPort *outputPort();
 
protected:
  vtkFilter *m_creator;
  int m_portNumber;
};


class EspinaProduct
: public vtkProduct
, public ITraceNode
//, public IRenderable
, public IModelItem
{
public:
  
  enum RENDER_STYLE
  { VISIBLE   = 1
  , SELECTED  = 2
  , DISCARTED = 4
  };
  
public:
  EspinaProduct(vtkFilter *creator, int portNumber);
  
  //! Implements ITraceNode interface
  virtual QString getArguments() const;
  virtual void print(int indent = 0) const;
  virtual QString label() const {return "Product";}

  //! Implements deprecated IRenderable interface as part of its own interface
  void color(double *color);
  virtual bool visible() const { return m_style & VISIBLE; }
  virtual void setVisible(bool value) { m_style = RENDER_STYLE((m_style & !VISIBLE) | (value ? 1 : 0)); }
  virtual RENDER_STYLE style() const {return m_style;}
  
  
  //! Implements IModelItem Interface
  virtual QVariant data(int role = Qt::UserRole + 1) const;
  virtual TaxonomyNode *taxonomy() {return m_taxonomy;}
  virtual void setTaxonomy(TaxonomyNode *taxonomy) {m_taxonomy = taxonomy;} 
  virtual void setOrigin(Sample *sample) {    m_origin = sample;}
  virtual Sample *origin() {return m_origin;}
  
protected:
  double m_rgba[4];
  TaxonomyNode *m_taxonomy;
  Sample *m_origin;
  RENDER_STYLE m_style;
};


class Sample : public EspinaProduct
{
public:
  Sample(vtkFilter *creator, int portNumber) 
  : EspinaProduct(creator, portNumber)
  , m_extent(NULL)
  {}
  //virtual EspinaId id(){return name;}
  //! Reimplements ITraceNode Interface
  virtual QString label() const {return "Segmentation";}
  
  virtual QVariant data(int role = Qt::UserRole + 1) const;
  
  void extent(int *out);
  void bounds(double *out);
  void spacing(double *out);
  
private:
  int *m_extent;
  double *m_bounds, *m_spacing;
  QMutex mutex;
};

class Segmentation : public EspinaProduct
{
public:
  Segmentation(vtkFilter *creator, int portNumber);
  //Segmentation(const vtkProduct &product);
  //Segmentation(const Segmentation &seg);
  
  //! Reimplements ITraceNode Interface
  virtual QString label() const {return "Segmentation";}
  
  //! WARNING: Note that Segmentation constructor hides 3rd paramater (productName)
  //Segmentation(pqPipelineSource *source, int portNumber, const QString &parentHash = "");

  virtual QVariant data(int role = Qt::UserRole + 1) const;
  
  void addExtension(ISegmentationExtension *ext);
  //! Are supposed to be used for sort time 
  ISegmentationExtension *extension(ExtensionId extId);
  void initialize();
  
private:
  QMap<ExtensionId,ISegmentationExtension *> m_extensions;
  InformationMap m_infoMap;
  RepresentationMap m_repMap;
};

#endif // PRODUCTS_H

// class Product 
// : public ITraceNode
// , public ISingleton
// , public IRenderable
// , public IModelItem
// {
// public:
//   //Product(){}
//   Product(pqPipelineSource *source, int portNumber, const QString &traceName = "Product", const EspinaId & parentHash = "");
//   virtual ~Product(){}
// 
//   //! Implements ITraceNode interface
//   /*
//   virtual std::vector<ITraceNode *> inputs();
//   virtual std::vector<ITraceNode *> outputs();
//   /*
//   virtual void print(int indent = 0) const;
//   virtual EspinaParamList getArguments();
//   */
//   
//   //! Implements ISingleton
//   virtual EspinaId id();
//   
//   
//   //! Implements IRenderable
//   virtual pqOutputPort* outputPort();
//   virtual pqPipelineSource* sourceData();	
//   virtual int portNumber();
//   virtual void color(double* rgba);
//   
//   virtual QVariant data(int role = Qt::UserRole + 1) const;
//   virtual TaxonomyNode *taxonomy() {return m_taxonomy;}
//   virtual void setTaxonomy(TaxonomyNode *taxonomy){m_taxonomy = taxonomy;} 
//   virtual void setOrigin(Sample *sample) {m_sample = sample;}
//   virtual Sample *origin() {return m_sample;}
// 
//   virtual QString parentHash() {return m_parentHash;}
// protected:
//   double m_rgba[4];
//   QString m_hash, m_parentHash;
//   TaxonomyNode *m_taxonomy;
//   Sample *m_sample;
// };