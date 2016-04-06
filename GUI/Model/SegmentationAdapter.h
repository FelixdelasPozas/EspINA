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

#ifndef ESPINA_SEGMENTATION_ADAPTER_H
#define ESPINA_SEGMENTATION_ADAPTER_H

// ESPINA
#include <GUI/Model/ViewItemAdapter.h>
#include <Core/Analysis/Extensible.hxx>
#include <Core/Analysis/Extensions.h>

class QPixmap;
class QIcon;
namespace ESPINA
{
  /** class SegmentationAdapter.
   * \brief Model biological structures which have been extracted from one or more channels.
   */
  class EspinaGUI_EXPORT SegmentationAdapter
  : public ViewItemAdapter
  {
  public:
    using ReadLockExtensions  = Core::Analysis::ReadLockExtensions<SegmentationExtension, Segmentation>;
    using WriteLockExtensions = Core::Analysis::WriteLockExtensions<SegmentationExtension, Segmentation>;

  public:
    /** \brief SegmentationAdapter class virtual destructor.
     *
     */
    virtual ~SegmentationAdapter();

    virtual QVariant data(int role = Qt::DisplayRole) const;

    virtual bool setData(const QVariant& value, int role);

    virtual ItemAdapter::Type type() const
    { return Type::SEGMENTATION; }

    virtual InputSPtr asInput() const;

    /** \brief Sets the number of the segmentation.
     * \param[in] number
     *
     */
    void setNumber(unsigned int number);

    /** \brief Returns the number of the segmentation.
     *
     */
    unsigned int number() const;

    /** \brief Sets the category of the segmentation.
     * \param[in] category category adapter smart pointer.
     *
     */
    void setCategory(CategoryAdapterSPtr category);

    /** \brief Returns the category of the segmentation.
     *
     */
    CategoryAdapterSPtr category() const;

    /** \brief Adds the user to the list of users that have modified this segmentation.
     * \param[in] user user name.
     *
     */
    void modifiedByUser(const QString& user);

    /** \brief Returns the list of users that have modified this segmentation.
     *
     */
    QStringList users() const;

    ReadLockExtensions readOnlyExtensions() const;

    WriteLockExtensions extensions();

     /** \brief Returns the list of tags provided by the segmnetation extensions of the segmentation.
      *
      */
    virtual SegmentationExtension::InformationKeyList availableInformation() const;

    bool hasInformation(const SegmentationExtension::InformationKey &key) const
    { return availableInformation().contains(key); }

    /** \brief Returns the information specified by the tag.
     * \param[in] key segmentation extension information tag.
     *
     */
    virtual QVariant information(const SegmentationExtension::InformationKey &key) const;

    /** \brief Returns true if the information is available.
     *
     */
    bool isReady(const SegmentationExtension::InformationKey &key) const;

    /** \brief Returns a bounds that contain the segmentation.
     *
     * NOTE: Could or could not be the segmentation's minimal bounds.
     *
     */
    Bounds bounds() const;

  protected:
    virtual void changeOutputImplementation(InputSPtr input);

  private:
    /** \brief SegmentationAdapter class constructor.
     * \param[in] filter, filter adapter smart pointer.
     * \param[in] segmentation, smart pointer of the segmentation to adapt.
     *
     */
    explicit SegmentationAdapter(SegmentationSPtr segmentation);

    QPixmap appendImage(const QPixmap& original, const QString& image, bool slim = false) const;

  private:
    SegmentationSPtr    m_segmentation;
    CategoryAdapterSPtr m_category;

    friend class ModelFactory;
    friend class ModelAdapter;
    friend class QueryAdapter;

    friend bool operator==(SegmentationAdapterSPtr lhs, SegmentationSPtr rhs);
    friend bool operator==(SegmentationSPtr lhs, SegmentationAdapterSPtr rhs);
  };

  /** \brief Equality operation between a segmentation adapter smart pointer and a segmentation smart pointer.
   * \param[in] lhs segmentation adapter smart pointer.
   * \param[in] rhs segmentation smart pointer.
   *
   */
  bool EspinaGUI_EXPORT operator==(SegmentationAdapterSPtr lhs, SegmentationSPtr        rhs);

  /** \brief Equality operation between a segmentation adapter smart pointer and a segmentation smart pointer.
   * \param[in] lhs segmentation smart pointer.
   * \param[in] rhs segmentation adapter smart pointer.
   *
   */
  bool EspinaGUI_EXPORT operator==(SegmentationSPtr        lhs, SegmentationAdapterSPtr rhs);

  /** \brief Inequality operation between a segmentation adapter smart pointer and a segmentation smart pointer.
   * \param[in] lhs segmentation adapter smart pointer.
   * \param[in] rhs segmentation smart pointer.
   *
   */
  bool EspinaGUI_EXPORT operator!=(SegmentationAdapterSPtr lhs, SegmentationSPtr        rhs);

  /** \brief Inequality operation between a segmentation adapter smart pointer and a segmentation smart pointer.
   * \param[in] lhs segmentation smart pointer.
   * \param[in] rhs segmentation adapter smart pointer.
   *
   */
  bool EspinaGUI_EXPORT operator!=(SegmentationSPtr        lhs, SegmentationAdapterSPtr rhs);
}
#endif // ESPINA_SEGMENTATION_ADAPTER_H
