/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef ESPINA_COUNTING_FRAME_H
#define ESPINA_COUNTING_FRAME_H

#include "CountingFramePlugin_Export.h"

#include <QStandardItemModel>

#include "vtkCountingFrameSliceWidget.h"
#include "vtkCountingFrame3DWidget.h"
#include "CountingFrameInteractorAdapter.h"
#include <Tasks/ApplyCountingFrame.h>
#include <GUI/View/Widgets/EspinaWidget.h>
#include <Core/Utils/Bounds.h>

#include <vtkCommand.h>
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>

namespace EspINA
{
namespace CF {

  enum CFType
  {
    ADAPTIVE,
    ORTOGONAL
  };

  class CountingFrameExtension;

  class CountingFramePlugin_EXPORT CountingFrame
  : public QObject
  , public EspinaWidget
  , public vtkCommand
  {
    Q_OBJECT
  protected:
    using CountingFrame2DWidgetAdapter = CountingFrameInteractorAdapter<vtkCountingFrameSliceWidget>;
    using CountingFrame3DWidgetAdapter = CountingFrameInteractorAdapter<vtkCountingFrame3DWidget>;

    class CountingFrameSliceWidget
    : public SliceWidget
    {
    public:
      explicit CountingFrameSliceWidget(vtkCountingFrameSliceWidget *widget)
      : SliceWidget(widget)
      {
        m_slicedWidget = dynamic_cast<CountingFrame2DWidgetAdapter *>(widget);
      }

      virtual void setSlice(Nm pos, Plane plane)
      {
        m_slicedWidget->SetSlice(pos);
        SliceWidget::setSlice(pos, plane);
      }

      CountingFrame2DWidgetAdapter *widget() const
      { return m_slicedWidget; }

    private:
      CountingFrame2DWidgetAdapter *m_slicedWidget;
    };

  public:
    const int INCLUSION_FACE;
    const int EXCLUSION_FACE;

    enum Role
    {
      DescriptionRole = Qt::UserRole + 1
    };

    using Id = QString;

  public:
    vtkTypeMacro(CountingFrame, vtkCommand);

    virtual ~CountingFrame();

    virtual CFType cfType() const = 0;

    CountingFrameExtension *extension()
    { return m_extension; }

    ChannelPtr channel() const;

    void deleteFromExtension();

    void setMargins(Nm inclusion[3], Nm exclusion[3]);

    void margins(Nm inclusion[3], Nm exclusion[3]);

    virtual QString typeName() const = 0;

    virtual QString description() const;

    Id id() const
    { return m_id; }

    void setId(Id id)
    { m_id = id; }

    void setVisible(bool visible);

    void setEnabled(bool enable);

    bool isVisible() const
    { return m_visible; }

    void setHighlighted(bool highlighted);

    bool isHighlighted() const
    { return m_highlight; }

    /** \brief Return total volume in pixels
     *
     */
    double totalVolume() const
    {
      QReadLocker lock(&m_volumeMutex);
      return m_totalVolume;
    }

    /** \brief Return inclusion volume in pixels
     *
     */
    double inclusionVolume() const
    {
      QReadLocker lock(&m_volumeMutex);
      return m_inclusionVolume;
    }

    /** \brief Return exclusion volume in pixels
     *
     */
    double exclusionVolume() const
    {
      QReadLocker lock(&m_volumeMutex);
      return totalVolume() - inclusionVolume();
    }

    /** \brief Return the polydata defining the Counting Framge
     *
     */
    virtual vtkSmartPointer<vtkPolyData> polyData() const
    {
      return countingFramePolyData();
    }

    virtual void Execute(vtkObject* caller, long unsigned int eventId, void* callData);

    Nm left()  const {return m_inclusion[0];}
    Nm top()   const {return m_inclusion[1];}
    Nm front() const {return m_inclusion[2];}
    Nm right() const {return m_exclusion[0];}
    Nm bottom()const {return m_exclusion[1];}
    Nm back()  const {return m_exclusion[2];}

    void setCategoryConstraint(const QString &category);

    QString categoryConstraint() const { return m_categoryConstraint; }

    virtual vtkAbstractWidget *create3DWidget(View3D *view) = 0;

    void apply();

  signals:
    void modified(CountingFrame *);
    void changedVisibility();

  protected:
    explicit CountingFrame(CountingFrameExtension *extension,
                           Nm inclusion[3],
                           Nm exclusion[3],
                           SchedulerSPtr scheduler);

    void updateCountingFrame();

    virtual void updateCountingFrameImplementation() = 0;

    Nm equivalentVolume(const Bounds &bounds);

    void setTotalVolume(double volume)
    {
//       QWriteLocker lock(&m_volumeMutex);
      m_totalVolume = volume;
    }

    void setInclusionVolume(double volume)
    {
//       QWriteLocker lock(&m_volumeMutex);
      m_inclusionVolume = volume;
    }

    vtkSmartPointer<vtkPolyData> channelEdgesPolyData() const;
    vtkSmartPointer<vtkPolyData> countingFramePolyData() const;

  protected slots:
    void onCountingFrameApplied();

  protected:
    SchedulerSPtr m_scheduler;

    mutable QReadWriteLock       m_countingFrameMutex;
    vtkSmartPointer<vtkPolyData> m_countingFrame;

    mutable QReadWriteLock       m_channelEdgesMutex;
    vtkSmartPointer<vtkPolyData> m_channelEdges;

    mutable QReadWriteLock m_volumeMutex;
    Nm                     m_inclusionVolume;
    Nm                     m_totalVolume;

    CountingFrameExtension *m_extension;

    Id   m_id;

    mutable QReadWriteLock m_marginsMutex;
    Nm                     m_inclusion[3];
    Nm                     m_exclusion[3];

    QString m_categoryConstraint;

    // TODO: Change to private (may need some changes in the API)
    mutable QMutex m_widgetMutex;
    QMap<View2D *, CountingFrameSliceWidget *>     m_widgets2D;
    QMap<View3D *, CountingFrame3DWidgetAdapter *> m_widgets3D;


  private:
    mutable QReadWriteLock m_stateMutex;
    bool m_visible;
    bool m_enable;
    bool m_highlight;


    mutable QMutex m_applyMutex;
    ApplyCountingFrameSPtr m_applyCountingFrame;
  };

  using CountingFrameList = QList<CountingFrame *>;
} // namsepace CF
} // namespace EspINA

#endif // BOUNDINGREGION_H
