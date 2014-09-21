/*

    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

#ifndef ESPINA_SAMPLE_ADAPTER_H
#define ESPINA_SAMPLE_ADAPTER_H

// ESPINA
#include "GUI/Model/NeuroItemAdapter.h"
#include <Core/Utils/NmVector3.h>
#include <Core/Utils/Bounds.h>

namespace ESPINA
{
  class SampleAdapter;
  using SampleAdapterPtr   = SampleAdapter *;
  using SampleAdapterList  = QList<SampleAdapterPtr>;
  using SampleAdapterSPtr  = std::shared_ptr<SampleAdapter>;
  using SampleAdapterSList = QList<SampleAdapterSPtr>;

  /** \class SampleAdapter.
   *
   */
  class EspinaGUI_EXPORT SampleAdapter
  : public NeuroItemAdapter
  {
  public:
  	/** brief SampleAdater class virtual destructor.
  	 *
  	 */
    virtual ~SampleAdapter();

    /** brief Implements ItemAdapter::data().
     *
     */
    virtual QVariant data(int role = Qt::DisplayRole) const;

    /** brief Implements ItemAdapter::setData().
     *
     */
    virtual bool setData(const QVariant& value, int role = Qt::UserRole +1);

    /** brief Implements ItemAdapter::type().
     *
     */
    virtual ItemAdapter::Type type() const
    { return Type::SAMPLE; }

    /** brief Sets the name of the sample.
     * \param[in] name, name of the sample.
     */
    void setName(const QString& name);

    /** brief Returns the name of the sample.
     *
     */
    QString name() const;

    /** brief Sets the position (origin) of the sample.
     *
     */
    void setPosition(const NmVector3& point);

    /** brief Returns the position (origin) of the sample.
     *
     */
    NmVector3 position() const;

    /** \brief Set the spatial bounds in nanometers of the Sample in the analysis frame reference.
     * \param[in] bounds, minimal bounds of the sample.
     *
     */
    void setBounds(const Bounds& bounds);

    /** \brief Return the spatial bounds in nanometers of the Sample in the analysis frame reference
     *
     */
    Bounds bounds() const;

  private:
    /** brief SampleAdapter class constructor.
     * \param[in] sample, smart pointer of the sample to adapt.
     *
     */
    explicit SampleAdapter(SampleSPtr sample);

  private:
    SampleSPtr m_sample;

    friend class ModelFactory;
    friend class ModelAdapter;
    friend class QueryAdapter;

    friend bool operator==(SampleAdapterSPtr lhs, SampleSPtr rhs);
    friend bool operator==(SampleSPtr lhs, SampleAdapterSPtr rhs);
  };

  /** brief Equality operation between a sample adapter smart pointer and a sample smart pointer.
   * \param[in] lhs, sample adapter smart pointer.
   * \param[in] rhs, sample smart pointer.
   *
   */
  bool EspinaGUI_EXPORT operator==(SampleAdapterSPtr lhs, SampleSPtr rhs);

  /** brief Equality operation between a sample smart pointer and a sample adapter smart pointer.
   * \param[in] lhs, sample smart pointer.
   * \param[in] rhs, sample adapter smart pointer.
   *
   */
  bool EspinaGUI_EXPORT operator==(SampleSPtr lhs, SampleAdapterSPtr rhs);

  /** brief Inequality operation between a sample adapter smart pointer and a sample smart pointer.
   * \param[in] lhs, sample adapter smart pointer.
   * \param[in] rhs, sample smart pointer.
   *
   */
  bool EspinaGUI_EXPORT operator!=(SampleAdapterSPtr lhs, SampleSPtr rhs);

  /** brief Inequality operation between a sample smart pointer and a sample adapter smart pointer.
   * \param[in] lhs, sample smart pointer.
   * \param[in] rhs, sample adapter smart pointer.
   *
   */
  bool EspinaGUI_EXPORT operator!=(SampleSPtr lhs, SampleAdapterSPtr rhs);

  /** brief Returns the sample adapter raw pointer given a item adapter raw pointer.
   * \param[in] item, item adapter raw pointer.
   *
   */
  SampleAdapterPtr EspinaGUI_EXPORT samplePtr(ItemAdapterPtr item);

}// namespace ESPINA

#endif // ESPINA_SAMPLE_ADAPTER_H
