/*
 * 
 * Copyright (C) 2014  Jorge Peña Pastor<jpena@cesvima.upm.es>
 *
 * This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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
#ifndef ESPINA_SEGMENTATION_ADAPTER_H
#define ESPINA_SEGMENTATION_ADAPTER_H

#include "GUI/Model/ViewItemAdapter.h"
#include <Core/Analysis/Extension.h>

namespace ESPINA
{
  class CategoryAdapter;
  using CategoryAdapterPtr   = CategoryAdapter *;
  using CategoryAdapterSPtr  = std::shared_ptr<CategoryAdapter>;

  class SegmentationAdapter;
  using SegmentationAdapterPtr   = SegmentationAdapter *;
  using SegmentationAdapterSet   = QSet<SegmentationAdapterPtr>;
  using SegmentationAdapterList  = QList<SegmentationAdapterPtr>;
  using SegmentationAdapterSPtr  = std::shared_ptr<SegmentationAdapter>;
  using SegmentationAdapterSList = QList<SegmentationAdapterSPtr>;

  static const int NumberRole = TypeRole + 1;

  class EspinaGUI_EXPORT SegmentationAdapter
  : public ViewItemAdapter
  {
  public:
    virtual ~SegmentationAdapter();

    virtual QVariant data(int role = Qt::DisplayRole) const;

    virtual bool setData(const QVariant& value, int role = Qt::UserRole +1);

    virtual ItemAdapter::Type type() const
    { return Type::SEGMENTATION; }

    // TODO 2014-05-13: create a new method with this name to add a new type of input which is
    //       invariant to the segmentation and rename this method to something else?
    virtual InputSPtr asInput() const;

    virtual void changeOutput(InputSPtr input);

    void setNumber(unsigned int number);

    unsigned int number() const;

    void setCategory(CategoryAdapterSPtr category);

    CategoryAdapterSPtr category() const;

    void modifiedByUser(const QString& user);

    QStringList users() const;

    /**
     * Extesion won't be available until requirements are satisfied
     */
    void addExtension(SegmentationExtensionSPtr extension);

    void deleteExtension(SegmentationExtensionSPtr extension);

    /** \brief Check whether or not there is an extension with the given name
     *
     */
    bool hasExtension(const SegmentationExtension::Type& type) const;

    /** \brief Return the extension with the especified name
     *
     *  Important: It the segmentation doesn't contain any extension with
     *  the requested name, but there exist an extension prototype registered
     *  in the factory, a new instance will be created and attached to the
     *  segmentation.
     *  If there is no extension with the given name registered in the factory
     *  a Undefined_Extension exception will be thrown
     */
     SegmentationExtensionSPtr extension(const SegmentationExtension::Type& type) const;

     SegmentationExtensionSList extensions() const;

    virtual SegmentationExtension::InfoTagList informationTags() const;

    virtual QVariant information(const SegmentationExtension::InfoTag& tag) const;

    bool isInformationReady(const SegmentationExtension::InfoTag& tag) const;

    Bounds bounds() const;

  private:
    explicit SegmentationAdapter(FilterAdapterSPtr filter, SegmentationSPtr segmentation);

  private:
    SegmentationSPtr    m_segmentation;
    CategoryAdapterSPtr m_category;

    friend class ModelFactory;
    friend class ModelAdapter;
    friend class QueryAdapter;

    friend bool operator==(SegmentationAdapterSPtr lhs, SegmentationSPtr        rhs);
    friend bool operator==(SegmentationSPtr        lhs, SegmentationAdapterSPtr rhs);

  };

  bool operator==(SegmentationAdapterSPtr lhs, SegmentationSPtr        rhs);
  bool operator==(SegmentationSPtr        lhs, SegmentationAdapterSPtr rhs);

  bool operator!=(SegmentationAdapterSPtr lhs, SegmentationSPtr        rhs);
  bool operator!=(SegmentationSPtr        lhs, SegmentationAdapterSPtr rhs);

  SegmentationAdapterPtr segmentationPtr(ItemAdapterPtr item);
}
#endif // ESPINA_SEGMENTATION_ADAPTER_H
