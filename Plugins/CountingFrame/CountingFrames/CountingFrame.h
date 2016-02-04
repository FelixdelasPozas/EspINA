/*
 *    
 *    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

// Plugin
#include "CountingFramePlugin_Export.h"
#include "vtkCountingFrameSliceWidget.h"
#include "vtkCountingFrame3DWidget.h"
#include "CountingFrameInteractorAdapter.h"

// Qt
#include <QStandardItemModel>

// ESPINA
#include <Tasks/ApplyCountingFrame.h>
#include <Core/Utils/Bounds.h>

// VTK
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>

namespace ESPINA
{
  class RenderView;

  namespace CF
  {

    enum CFType
    {
      ADAPTIVE,
      ORTOGONAL
    };

    class vtkCountingFrameCommand;
    class CountingFrameExtension;

    class CountingFramePlugin_EXPORT CountingFrame
    : public QObject
    {
      Q_OBJECT
    protected:
      using CountingFrame2DWidgetAdapter = CountingFrameInteractorAdapter<vtkCountingFrameSliceWidget>;
      using CountingFrame3DWidgetAdapter = CountingFrameInteractorAdapter<vtkCountingFrame3DWidget>;

    public:
      const int INCLUSION_FACE;
      const int EXCLUSION_FACE;

      enum Role
      {
        DescriptionRole = Qt::UserRole + 1
      };

      using Id = QString;

    public:
      /** \brief CountingFrame class virtual destructor.
       *
       */
      virtual ~CountingFrame();

      /** \brief Returns the counting frame type.
       *
       */
      virtual CFType cfType() const = 0;

      /** \brief Returns a pointer to the counting frame's extension.
       *
       */
      CountingFrameExtension *extension()
      { return m_extension; }

      /** \brief Returns the channel of the counting frame.
       *
       */
      ChannelPtr channel() const;

      /** \brief Deletes this object from it's own extension.
       *
       */
      void deleteFromExtension();

      /** \brief Sets the inclusion and exclusion margins of the counting frame.
       * \param[in] inclusion inclusion margins in each of the axis.
       * \param[in] exclusion exclusion margins in each of the axis.
       *
       */
      void setMargins(Nm inclusion[3], Nm exclusion[3]);

      /** \brief Returns the inclusion and exclusion margins.
       * \param[out] inclusion inclusion margins in each of the axis.
       * \param[out] exclusion exclusion margins in each of the axis.
       *
       */
      void margins(Nm inclusion[3], Nm exclusion[3]);

      /** \brief Returns the string corresponding to the counting frame type.
       *
       */
      virtual QString typeName() const = 0;

      /** \brief Returns the counting frame's description.
       *
       */
      virtual QString description() const;

      /** \brief Returns the counting frame's id.
       *
       */
      Id id() const
      { return m_id; }

      /** \brief Sets the id of the counting frame.
       *
       */
      void setId(Id id);

      /** \brief Shows/hides the counting frame.
       * \param[in] visible true to show and false otherwise.
       *
       */
      void setVisible(bool visible);

      /** \brief Enables/disables the counting frame.
       * \param[in] enable true to enable and false otherwise.
       *
       */
      void setEnabled(bool enable);

      /** \brief Returns true if the counting frame is visible and false otherwise.
       *
       */
      bool isVisible() const
      { return m_visible; }

      /** \brief Highlights/dims the counting frame representation.
       * \param[in] true to highlight and false to dim.
       *
       */
      void setHighlighted(bool highlighted);

      /** \brief Returns true if the counting frame representation is highlighted.
       *
       */
      bool isHighlighted() const
      { return m_highlight; }

      /** \brief Return total volume of the counting frame in pixels.
       *
       */
      double totalVolume() const
      {
        QReadLocker lock(&m_volumeMutex);
        return m_totalVolume;
      }

      /** \brief Return inclusion volume of the counting frame in pixels.
       *
       */
      double inclusionVolume() const
      {
        QReadLocker lock(&m_volumeMutex);
        return m_inclusionVolume;
      }

      /** \brief Return exclusion volume of the counting frame in pixels.
       *
       */
      double exclusionVolume() const
      {
        QReadLocker lock(&m_volumeMutex);
        return totalVolume() - inclusionVolume();
      }

      /** \brief Return the vtkPolyData obejct defining the counting frame limits.
       *
       */
      virtual vtkSmartPointer<vtkPolyData> polyData() const
      {
        return countingFramePolyData();
      }

      /** \brief Returns the left counting frame margin.
       *
       */
      Nm left()  const {return m_inclusion[0];}

      /** \brief Returns the top counting frame margin.
       *
       */
      Nm top()   const {return m_inclusion[1];}

      /** \brief Returns the fron counting frame margin.
       *
       */
      Nm front() const {return m_inclusion[2];}

      /** \brief Returns the right counting frame margin.
       *
       */
      Nm right() const {return m_exclusion[0];}

      /** \brief Returns the bottom counting frame margin.
       *
       */
      Nm bottom()const {return m_exclusion[1];}

      /** \brief Returns the back counting frame margin.
       *
       */
      Nm back()  const {return m_exclusion[2];}

      /** \brief Sets the category of the segmentations this counting frame will apply to or an empty strin to apply to all.
       * \param[in] category category name.
       *
       */
      void setCategoryConstraint(const QString &category);

      /** \brief Returns the category of the segmentations this counting frame applies to, or an empty string if applies to all.
       *
       */
      QString categoryConstraint() const
      { return m_categoryConstraint; }

      /** \brief Creates a slice widget for this counting frame for the given view.
       * \param[in] view view to show the widget.
       *
       */
      vtkCountingFrameSliceWidget *createSliceWidget(RenderView *view);

      /** \brief Creates a 3D widget for this counting frame for the given view.
       * \param[in] view view to show the widget.
       *
       */
      vtkCountingFrame3DWidget *createWidget(RenderView *view);

      /** \brief Hides and deletes the given slice widget.
       * \param[in] widget slice widget to delete.
       *
       */
      void deleteSliceWidget(vtkCountingFrameSliceWidget *widget);

      /** \brief Hides and deletes the given 3D widget.
       * \param[in] widget 3D widget to delete.
       *
       */
      void deleteWidget(vtkCountingFrame3DWidget *widget);

      /** \brief Computes the inclusion of the constrained segmentations to the counting frame.
       *
       */
      void apply();

    signals:
      void modified(CountingFrame *);

      void applied(CountingFrame *);

      void changedVisibility();

    protected:
      /** \brief CountingFrame class constructor.
       * \param[in] extension counting frame extension of this object.
       * \param[in] inclusion inclusion margins in each of the axis.
       * \param[in] exclusion exclusion margins in each of the axis.
       * \param[in] scheduler application task scheduler.
       *
       */
      explicit CountingFrame(CountingFrameExtension *extension,
                             Nm inclusion[3],
                             Nm exclusion[3],
                             SchedulerSPtr scheduler);

      /** \brief Updates the counting frame inclusion/exclusion values of all the constrained segmentations and
       * recomputes the widgets geometry.
       *
       */
      void updateCountingFrame();

      /** \brief Actual implementation of the updating procedure for subclasses.
       *
       */
      virtual void updateCountingFrameImplementation() = 0;

      /** \brief Returns the volume in cubic nanometers of the given bounds.
       * \param[in] bounds bounds of a volume.
       *
       */
      Nm equivalentVolume(const Bounds &bounds);

      /** \brief Sets the total volume of the counting frame.
       *
       */
      void setTotalVolume(double volume)
      {
        m_totalVolume = volume;
      }

      /** \brief Sets the inclusion volume of the counting frame.
       *
       */
      void setInclusionVolume(double volume)
      {
        m_inclusionVolume = volume;
      }

      /** \brief Returns the vtkPolyData object that defines the margins of the counting frame's channel.
       *
       */
      vtkSmartPointer<vtkPolyData> channelEdgesPolyData() const;

      /** \brief Returns the vtkPolyData object that defines the margins of the counting frame.
       *
       */
      vtkSmartPointer<vtkPolyData> countingFramePolyData() const;

    protected slots:
      /** \brief Helper method to emit the applied signal after a counting frame has been completely applied to the constrained segmentations.
       *
       */
      void onCountingFrameApplied();

    protected:
      SchedulerSPtr m_scheduler; /** task scheduler. */

      mutable QReadWriteLock       m_countingFrameMutex;  /** lock for m_countingFrame. */
      vtkSmartPointer<vtkPolyData> m_countingFrame;       /** counting frame limits .   */

      mutable QReadWriteLock       m_channelEdgesMutex;   /** lock for m_channelEdges. */
      vtkSmartPointer<vtkPolyData> m_channelEdges;        /** channel's margins.       */

      mutable QReadWriteLock m_volumeMutex;               /** lock for total and inclusion volumes.   */
      Nm                     m_inclusionVolume;           /** total volume of the counting frame.     */
      Nm                     m_totalVolume;               /** inclusion volume of the counting frame. */

      CountingFrameExtension *m_extension;                /** extension corresponding to this counting frame. */

      Id m_id;                                            /** counting frame id. */

      mutable QReadWriteLock m_marginsMutex;              /** lock for inclusion/exclusion margins. */
      Nm                     m_inclusion[3];              /** inclusion margins.                    */
      Nm                     m_exclusion[3];              /** exclusion margins.                    */

      QString m_categoryConstraint;                       /** name of the category of the segmentation this counting frame will apply. */

      // TODO: Change to private (may need some changes in the API)
      mutable QMutex m_widgetMutex;                       /** lock for widget interacion. */
      vtkSmartPointer<vtkCountingFrameCommand> m_command; /** vtk command to manage interaction in the views. */

    private:
      friend class vtkCountingFrameCommand;

      mutable QReadWriteLock m_stateMutex;                /** lock of the visible/highlight properties of the widgets. */
      bool m_visible;                                     /** true if widgets are visible and false otherwise. */
      bool m_enable;                                      /** true if counting frame is enabled and false otherwise. */
      bool m_highlight;                                   /** true if the widgets are highlighted and false otherwise. */

      ApplyCountingFrameSPtr m_applyCountingFrame;        /** task to apply the counting frame to the constrained segmentations. */

      QList<vtkCountingFrameSliceWidget *> m_widgets2D;   /** list of 2D widgets of this counting frame. */
      QList<vtkCountingFrame3DWidget    *> m_widgets3D;   /** list of 3D widgets of this counting frame. */
    };

    using CountingFrameList = QList<CountingFrame *>;

    class vtkCountingFrameCommand
    : public vtkCommand
    {
    public:
      vtkTypeMacro(vtkCountingFrameCommand, vtkCommand);

      static vtkCountingFrameCommand *New()
      { return new vtkCountingFrameCommand(); }

      virtual void Execute(vtkObject *, unsigned long int, void*) override;

      /** \brief Sets the counting frame of the interacting widget this command is observing.
       * \param[in] cf counting frame object pointer.
       */
      virtual void setCountingFrame(CountingFrame *cf)
      { m_cf = cf; }

    private:
      /** \brief vtkCountingFrameCommand class private constructor.
       *
       */
      explicit vtkCountingFrameCommand()
      : m_cf{nullptr}
      {}

      /** \brief vtkCountingFrameCommand class private destructor.
       *
       */
      virtual ~vtkCountingFrameCommand()
      {};

      CountingFrame *m_cf; /** counting frame owner of the observed widget. */
    };
  } // namsepace CF
} // namespace ESPINA

#endif // BOUNDINGREGION_H
