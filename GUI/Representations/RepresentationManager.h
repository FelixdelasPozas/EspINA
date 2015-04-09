/*

    Copyright (C) 2014  Jorge Pe�a Pastor <jpena@cesvima.upm.es>

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

#ifndef ESPINA_REPRESENTATION_MANAGER_H
#define ESPINA_REPRESENTATION_MANAGER_H

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include <Core/EspinaTypes.h>
#include <GUI/View/SelectableView.h>
#include <GUI/View/ViewTypeFlags.h>
#include <GUI/View/CoordinateSystem.h>
#include <GUI/Representations/RepresentationPipeline.h>
#include <GUI/Representations/RepresentationPool.h>

// Qt
#include <QString>
#include <QIcon>

namespace ESPINA
{
  class RepresentationManager;
  using RepresentationManagerSPtr  = std::shared_ptr<RepresentationManager>;
  using RepresentationManagerSList = QList<RepresentationManagerSPtr>;

  class RenderView;

  class EspinaGUI_EXPORT RepresentationManager
  : public QObject
  {
    Q_OBJECT
  public:
    enum class Status: int8_t {
      IDLE,
      PENDING_DISPLAY
    };

    enum FlagValue
    {
      HAS_ACTORS   = 0x1,
      EXPORTS_3D   = 0x2,
      NEEDS_ACTORS = 0x4
    };

  Q_DECLARE_FLAGS(Flags, FlagValue)

  public:
    virtual ~RepresentationManager()
    {}

    /** \brief Sets the name of the representation manager
     *
     */
    void setName(const QString &name);

    /** \brief Returns the name of the representation manager
     *
     */
    QString name() const;

    /** \brief Sets the description of the representation manager
     *
     */
    void setDescription(const QString &description);

    /** \brief Returns the description of the representation manager
     *
     */
    QString description() const;

    /** \brief Sets the icon of the representation manager
     *
     */
    void setIcon(const QIcon &icon);

    /** \brief Returns the icon of the representation manager
     *
     */
    QIcon icon() const;

    Flags flags() const;

    /** \brief Sets the view where representation are managed
     *
     */
    void setView(RenderView *view);

    ViewTypeFlags supportedViews() const
    { return m_supportedViews; }

    /** \brief Returns if managed representations are visible or not
     *
     */
    bool representationsVisibility() const
    { return m_representationsShown; }

    /** \brief Shows all representations
     *
     */
    void show(TimeStamp t);

    /** \brief Hides all representations
     *
     */
    void hide(TimeStamp t);

    /** \brief Returns if the manager has been requested to display its actors
     *
     */
    bool isActive();

    /** \brief Returns the status of the manager
     *
     */
    Status status() const;

    /** \brief Returns the range of ready pipelines.
     *
     */
    virtual TimeRange readyRange() const = 0;

    /** \brief Updates view's actors with those available at the given time.
     *
     */
    void display(TimeStamp t);

    /** \brief Returns the item picked
     *
     */
    virtual ViewItemAdapterPtr pick(const NmVector3 &point, vtkProp *actor) const = 0;

    /** \brief Returns a new instance of the class.
     *
     */
    RepresentationManagerSPtr clone();

  public slots:
    void onCrosshairChanged(NmVector3 crosshair, TimeStamp t);

    virtual void onSceneResolutionChanged(const NmVector3 &resolution, TimeStamp t) {}

    virtual void onSceneBoundsChanged(const Bounds &bounds, TimeStamp t) {}

  signals:
    void renderRequested();

  protected slots:
    void emitRenderRequest(TimeStamp t);

    void invalidateRepresentations();

    void waitForDisplay();

  protected:
    explicit RepresentationManager(ViewTypeFlags supportedViews);

    bool representationsShown() const;

    void setFlag(const FlagValue flag, const bool value);

  private:
    virtual void setCrosshair(const NmVector3 &crosshair, TimeStamp t) = 0;

    virtual void displayImplementation(TimeStamp t) = 0;

    virtual void onShow(TimeStamp t) = 0;

    virtual void onHide(TimeStamp t) = 0;

    virtual RepresentationManagerSPtr cloneImplementation() = 0;

    bool hasNewerFrames(TimeStamp t) const;

    void showRepresentations(TimeStamp t);

  protected:
    RenderView *m_view;

  private:
    QString m_name;
    QIcon   m_icon;
    QString m_description;
    bool    m_representationsShown;
    Status  m_status;
    Flags   m_flags;

    ViewTypeFlags m_supportedViews;
    NmVector3     m_crosshair;
    TimeStamp     m_lastRequestTime;
    TimeStamp     m_lastRenderRequestTime;

    RepresentationManagerSList m_childs;
    RepresentationPoolSList    m_pools;
  };

  class RepresentationManager2D
  {
  public:
    /** \brief Class RepresentationManager2D virtual destructor.
     *
     */
    virtual ~RepresentationManager2D()
    {};

    virtual void setPlane(Plane plane) = 0;

    virtual void setRepresentationDepth(Nm depth) = 0;
  };

  Q_DECLARE_OPERATORS_FOR_FLAGS(RepresentationManager::Flags)
}// namespace ESPINA

#endif // ESPINA_REPRESENTATION_MANAGER_H
