/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
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

#ifndef ESPINA_REPRESENTATION_PIPELINE_H
#define ESPINA_REPRESENTATION_PIPELINE_H

#include <memory>
#include <QString>
#include <QList>
#include <Core/Utils/NmVector3.h>

#include <vtkSmartPointer.h>

class vtkProp;

namespace ESPINA {

  class RepresentationPipeline
  {
  public:
    using Type  = QString;
    using Actor = vtkSmartPointer<vtkProp>;

    class Settings
    {
      using Pair = QPair<QVariant, bool>;
    public:
      template<typename T>
      void setValue(const QString &tag, T value);

      template<typename T>
      T getValue(const QString &tag) const
      { return m_properties[tag].first.value<T>(); }

      bool isModified(const QString &tag) const
      { return m_properties.value(tag, Pair(QVariant(), false)).second; }

      bool hasPendingChanges() const;

      void apply(const Settings &settings);

      void commit();

    private:
      QMap<QString, Pair> m_properties;
    };

  public:
    virtual ~RepresentationPipeline()
    {}

    /** \brief Returns the type of the representation pipeline
     *
     */
    Type type() const
    { return m_type; }

    /** \brief Returns the serialized settings of the representation
     *
     */
    virtual QString serializeSettings();

    /** \brief Restores the settings for the representation
     * \param[in] settings serialization
     *
     */
    virtual void restoreSettings(QString settings);

    virtual bool pick(const NmVector3 &point, vtkProp *actor) = 0;

    /** \brief Sets the crosshair point position for this representation
     * \param[in] point crosshair point
     *
     */
    void setCrosshairPoint(const NmVector3 &point);

    /** \brief Returns the crosshair point for this representation.
     *
     */
    NmVector3 crosshairPoint() const;

    Nm crosshairPosition(const Plane &plane) const;

    /** \brief Returns if the crosshair point has been modified since last update
     *
     */
    bool isCrosshairPointModified() const;

    bool isCrosshairPositionModified(const Plane &plane) const;

    /** \brief Updates the representation.
     *
     */
    virtual void update() = 0;

    /** \brief Returns the list of actors that comprise this representation.
     *
     */
    virtual QList<Actor> getActors() = 0;

    virtual void applySettings(const Settings& settings) = 0;

  protected:
    explicit RepresentationPipeline(Type type);

  protected:
    Settings  m_state;

  private:
    Type  m_type;
  };

  /** \brief
   *
   */
  template<typename T>
  void RepresentationPipeline::Settings::setValue(const QString &tag, T value)
  {
    Pair pair = m_properties.value(tag, Pair(QVariant(), false));

    pair.second |= pair.first.value<T>() != value;

    if (pair.second)
    {
      pair.first  = value;
      m_properties.insert(tag, pair);
    }
  }

  using RepresentationPipelineSPtr  = std::shared_ptr<RepresentationPipeline>;
  using RepresentationPipelineSList = QList<RepresentationPipelineSPtr>;
}

#endif // ESPINA_REPRESENTATIONPIPELINE_H
