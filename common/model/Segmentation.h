/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2011  Jorge Peña Pastor<jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

//----------------------------------------------------------------------------
// File:    Segmentation.h
// Purpose: Model biological structures which have been extracted from one or
//          more channels.
//----------------------------------------------------------------------------
#ifndef SEGMENTATION_H
#define SEGMENTATION_H

#include <common/extensions/SegmentationExtension.h>
#include "common/tools/PickableItem.h"
#include "common/model/Taxonomy.h"
#include <itkImageToVTKImageFilter.h>
#include <vtkAlgorithmOutput.h>
#include <itkCommand.h>
#include <itkSmartPointer.h>
#include <vtkSmartPointer.h>
#include <vtkImageConstantPad.h>
#include <vtkDiscreteMarchingCubes.h>

class Channel;
// Forward declarations
class Sample;
class Filter;
class pqPipelineSource;

class Segmentation
: public PickableItem
{
  Q_OBJECT
public:
  static const ArgumentId NUMBER;
  static const ArgumentId OUTPUT;
  static const ArgumentId TAXONOMY;
  static const ArgumentId USERS;//who have reviewed this segmentation

  static const int SelectionRole = Qt::UserRole + 2;

private:
  class SArguments : public Arguments
  {
  public:
    explicit SArguments() : m_number(-1), m_outputNumber(-1){}
    explicit SArguments(const ModelItem::Arguments args);

    void setNumber(unsigned int number)
    {
      m_number = number;
      (*this)[NUMBER] = QString::number(m_number);
    }
    unsigned int number() const {return m_number;}

    void setOutputNumber(Filter::OutputNumber outputNb)
    {
      m_outputNumber = outputNb;
      (*this)[OUTPUT] = QString::number(outputNb);
    }

    Filter::OutputNumber outputNumber() const {return m_outputNumber;}

    void addUser(const QString &user)
    {
      if ((*this)[USERS].isEmpty())
        (*this)[USERS] = user;
      else if (!users().contains(user))
        (*this)[USERS] += ',' + user;
    }

    QStringList users() const {return (*this)[USERS].split(',');}

    virtual QString serialize() const;

  private:
    unsigned int m_number;
    int m_outputNumber;
  };

public:
  explicit Segmentation(Filter *filter, Filter::OutputNumber outputNb);
  virtual ~Segmentation();

  Filter *filter() const {return m_filter;}
  unsigned int outputNumber() const {return m_args.outputNumber();}

  void changeFilter(Filter *filter, Filter::OutputNumber outputNb);

  /// Model Item Interface
  virtual QString id() const;
  virtual QVariant data(int role=Qt::DisplayRole) const;
  virtual bool setData(const QVariant& value, int role = Qt::UserRole +1);
  virtual ItemType type() const {return ModelItem::SEGMENTATION;}
  virtual QString serialize() const;

  virtual void initialize(Arguments args = Arguments());
  virtual void initializeExtensions(Arguments args = Arguments());

  void updateCacheFlag();

  /// Get the channel from where segmentation was created
  Channel *channel();

  /// Selectable Item Interface
  virtual Filter* filter(){return m_filter;}
  virtual Filter::OutputNumber outputNumber() {return m_args.outputNumber();}
  virtual EspinaVolume *itkVolume() const;
  virtual EspinaVolume *itkVolume();

  virtual vtkAlgorithmOutput* vtkVolume();
  virtual vtkAlgorithmOutput* mesh();

  void setNumber(unsigned int number) {m_args.setNumber(number);}
  unsigned int number() const {return m_args.number();}
  void setTaxonomy(TaxonomyElement *tax);
  TaxonomyElement *taxonomy() const {return m_taxonomy;}
  //void bounds(double val[3]);

  // State
  bool visible() const {return m_isVisible;}
  void setVisible(bool visible);
  QStringList users() const {return m_args.users();}

  /// Add a new extension to the segmentation
  /// Extesion won't be available until requirements are satisfied
  void addExtension(SegmentationExtension *ext);
  //   QStringList availableRepresentations() const;
  //   ISegmentationRepresentation *representation(QString rep);

  virtual QStringList availableInformations() const;
  virtual QVariant information(QString info);
  virtual ModelItemExtension* extension(QString name);

public slots:
  virtual void notifyModification(bool force = false);

private:
  Filter *m_filter;
  SArguments m_args;
  TaxonomyElement *m_taxonomy;

  bool m_isVisible;
  QColor m_color;
  //   mutable double m_bounds[6];

  // itk to vtk filter
  typedef itk::ImageToVTKImageFilter<EspinaVolume> itk2vtkFilterType;
  itk2vtkFilterType::Pointer itk2vtk;

  // vtkPolydata generation filter
  vtkSmartPointer<vtkImageConstantPad> m_padfilter;
  vtkSmartPointer<vtkDiscreteMarchingCubes> m_march;

  friend class Filter;
};

#endif // PRODUCTS_H
