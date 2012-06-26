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
#include "Filter.h"

// Forward declarations
class Sample;
class Filter;
class pqPipelineSource;

class Segmentation
: public SelectableItem
{
  Q_OBJECT
public:
  static const ArgumentId FILTER;
  static const ArgumentId NUMBER;
  static const ArgumentId OUTPUT;
  static const ArgumentId TAXONOMY;
  static const ArgumentId USERS;//who have reviewed this segmentation

  static const int SelectionRole = Qt::UserRole + 2;

private:
  class SArguments : public Arguments
  {
  public:
    explicit SArguments(){}
    explicit SArguments(const ModelItem::Arguments args);

    void setNumber(unsigned int number)
    {
      Number = number;
      (*this)[NUMBER] = QString::number(Number);
    }
    unsigned int number() const {return Number;}

    void setOutputNumber(int outputNb)
    {
      OutputNumber = outputNb;
      (*this)[OUTPUT] = QString::number(outputNb);
    }

    int outputNumber() const {return OutputNumber;}

    void addUser(const QString &user)
    {
      if ((*this)[USERS].isEmpty())
	(*this)[USERS] = user;
      else if (!users().contains(user))
	(*this)[USERS] += ',' + user;
    }

    QStringList users() const {return (*this)[USERS].split(',');}

    virtual QString serialize(bool key = false) const;

  private:
    unsigned int Number;
    int OutputNumber;
  };

public:
  explicit Segmentation(Filter *filter, unsigned int outputNb);
  virtual ~Segmentation();

  Filter *filter() const {return m_filter;}

  /// Model Item Interface
  virtual QString id() const;
  virtual QVariant data(int role=Qt::DisplayRole) const;
  virtual bool setData(const QVariant& value, int role = Qt::UserRole +1);
  virtual ItemType type() const {return ModelItem::SEGMENTATION;}
  virtual QString serialize() const;
  virtual void initialize(Arguments args = Arguments());
  virtual void initializeExtensions();

  /// Selectable Item Interface
  virtual EspinaVolume *volume();

  void setNumber(unsigned int number) {m_args.setNumber(number);}
  unsigned int number() const {return m_args.number();}
  void setTaxonomy(TaxonomyNode *tax);
  TaxonomyNode *taxonomy() const {return m_taxonomy;}
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

  QStringList availableInformations() const;
  QVariant information(QString info);

private slots:
  void onColorEngineChanged();

private:
  Filter               *m_filter;
  EspinaVolume::Pointer m_volume;
  SArguments            m_args;
  TaxonomyNode         *m_taxonomy;

  bool           m_extInitialized;
  bool           m_isVisible;
  QColor         m_color;
//   mutable double m_bounds[6];

  friend class Filter;
};

#endif // PRODUCTS_H