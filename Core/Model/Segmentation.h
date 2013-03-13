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

#include "Core/Model/PickableItem.h"
#include "Core/Model/Taxonomy.h"
#include "Core/Model/HierarchyItem.h"
#include "Channel.h"
//#include "Core/Model/Taxonomy.h"

#include <vtkAlgorithmOutput.h>
#include <itkCommand.h>
#include <itkSmartPointer.h>
#include <vtkSmartPointer.h>
#include <vtkImageConstantPad.h>
#include <vtkDiscreteMarchingCubes.h>

namespace EspINA
{

  typedef QSharedPointer<Segmentation> SegmentationSPtr;
  typedef QList<SegmentationSPtr> SegmentationSList;

  class Segmentation
  : public PickableItem
  , public HierarchyItem
  {
    Q_OBJECT
  public:
    static const ArgumentId NUMBER;
    static const ArgumentId OUTPUT;
    static const ArgumentId TAXONOMY;
    static const ArgumentId DEPENDENT; // Must be sync'ed with its parent segmentation
    static const ArgumentId USERS;//who have reviewed this segmentation

    static const int SelectionRole = Qt::UserRole + 2;

    typedef QString InfoTag;
    typedef QList<InfoTag> InfoTagList;

    class Extension;
    class Information;
    typedef Information *                       InformationExtension;
    typedef QList<InformationExtension>         InformationExtensionList;

    typedef QMap<ExtId, InformationExtension>   InformationExtensionProvider;
    typedef QMap<InfoTag, InformationExtension> InformationTagProvider;

  private:
    class SArguments
    : public Arguments
    {
    public:
      explicit SArguments() 
      : m_number(-1)
      , m_outputId(-1)
      , m_isInputSegmentationDependent(false){}

      explicit SArguments(const Arguments &args);

      void setNumber(unsigned int number)
      {
        m_number = number;
        (*this)[NUMBER] = QString::number(m_number);
      }
      unsigned int number() const {return m_number;}

      void setOutputId(Filter::OutputId oId)
      {
        m_outputId = oId;
        (*this)[OUTPUT] = QString::number(oId);
      }

      Filter::OutputId outputId() const {return m_outputId;}

      void addUser(const QString &user)
      {
        if ((*this)[USERS].isEmpty())
          (*this)[USERS] = user;
        else if (!users().contains(user))
          (*this)[USERS] += ',' + user;
      }

      QStringList users() const {return (*this)[USERS].split(',');}

      bool isInputSegmentationDependent() const
      { return m_isInputSegmentationDependent; }

      void setInputSegmentationDependent(bool value)
      { m_isInputSegmentationDependent = value;
        (*this)[DEPENDENT] = value?"1":"0";
      }

      virtual QString serialize() const;

    private:
      unsigned int m_number;
      int m_outputId;
      bool m_isInputSegmentationDependent;
    };

  public:
    explicit Segmentation(FilterSPtr filter,
                          const Filter::OutputId &outputNb);
    virtual ~Segmentation();

    void changeFilter(FilterSPtr filter, const Filter::OutputId &outputNb);

    /// Model Item Interface
    virtual QVariant data(int role=Qt::DisplayRole) const;
    virtual bool setData(const QVariant& value, int role = Qt::UserRole +1);
    virtual ModelItemType type() const {return SEGMENTATION;}
    virtual QString serialize() const;

    virtual void initialize(const Arguments &args = Arguments());

    void updateCacheFlag();

    /// Get the sample where the segmentation is located
    SampleSPtr sample();
    /// Get the channel from where segmentation was created
    ChannelSPtr channel();

    /// Selectable Item Interface
    virtual const FilterSPtr filter() const {return m_filter;}
    virtual FilterSPtr filter() { return PickableItem::filter(); }
    virtual const Filter::OutputId outputId() const {return m_args.outputId();}

    SegmentationVolume::Pointer volume();
    const SegmentationVolume::Pointer volume() const;

    void setNumber(unsigned int number) {m_args.setNumber(number);}
    unsigned int number() const {return m_args.number();}
    void setTaxonomy(TaxonomyElementSPtr tax);
    TaxonomyElementSPtr taxonomy() const {return m_taxonomy;}

    void modifiedByUser(QString user) { m_args.addUser(user);  }

    // State
    bool visible() const {return m_isVisible;}
    void setVisible(bool visible);
    QStringList users() const {return m_args.users();}
    bool isInputSegmentationDependent() const {return m_args.isInputSegmentationDependent();}
    void setInputSegmentationDependent(bool dependent) { m_args.setInputSegmentationDependent(dependent); };


    /// Return the list of segmentations which compose this segmentation
    SegmentationSList components();

    /// Return the list of segmentations which this segmentation is a component
    SegmentationSList componentOf();

    /// Add a new extension to the segmentation
    /// Extesion won't be available until requirements are satisfied
    void addExtension(Segmentation::InformationExtension extension);

    Segmentation::InformationExtension informationExtension(const ModelItem::ExtId &name) const;

    /// optional args should be deprecated in future versions
    void initializeExtensions();
    void invalidateExtensions();

    virtual InfoTagList availableInformations() const;
    virtual QVariant information(const InfoTag &tag) const;

  private:
    mutable SArguments m_args;

    FilterSPtr          m_filter;
    TaxonomyElementSPtr m_taxonomy;

    bool m_isVisible;
    bool m_isInputSegmentationDependent;
    QColor m_color;

    InformationExtensionProvider m_informationExtensions;
    InformationTagProvider       m_informationTagProvider;
  };

  SegmentationPtr  segmentationPtr(ModelItemPtr     item);
  SegmentationPtr  segmentationPtr(PickableItemPtr  item);
  SegmentationSPtr segmentationPtr(ModelItemSPtr   &item);
  SegmentationSPtr segmentationPtr(PickableItemSPtr &item);
}
#endif // PRODUCTS_H
