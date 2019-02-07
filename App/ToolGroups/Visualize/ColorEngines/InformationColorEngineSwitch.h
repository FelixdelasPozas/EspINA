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

#ifndef ESPINA_INFORMATION_COLOR_ENGINE_SWITCH_H
#define ESPINA_INFORMATION_COLOR_ENGINE_SWITCH_H

// ESPINA
#include <GUI/ColorEngines/InformationColorEngine.h>
#include <GUI/Representations/Managers/TemporalManager.h>
#include <GUI/Utils/ColorRange.h>
#include <Support/Widgets/ColorEngineSwitch.h>

// VTK
#include <vtkSmartPointer.h>
#include "../../../../GUI/Dialogs/RangeDefinitionDialog/ColorEngineRangeDefinitionDialog.h"

class vtkScalarBarActor;
class vtkLookupTable;
class vtkLegendBoxActor;

class QLabel;

namespace ESPINA
{
  namespace GUI
  {
    class ColorEngineRangeDefinitionDialog;

    namespace Widgets
    {
      class ToolButton;
    }
  }

  //-----------------------------------------------------------------------------
  /** \class ColorEngineWidgetRepresentation
   * \brief Base class for displaying a color range widget on the views. Implements interface of TemporalRepresentation object.
   *
   */
  class InformationColorEngineWidgetRepresentationImplementation
  {
    public:
      /** \struct Properties
       * \brief Properties of the widget. The class that holds the struct is responsible to delete the 'colors' attrib on
       * destruction.
       *
       */
      struct Properties
      {
          QString                                             title;        /** widget's title.                       */
          GUI::ColorEngineRangeDefinitionDialog::Position     position;     /** position.                             */
          GUI::ColorEngineRangeDefinitionDialog::Orientation  orientation;  /** orientation.                          */
          GUI::ColorEngineRangeDefinitionDialog::TextPosition textPosition; /** text position.                        */
          unsigned int                                        numLabels;    /** number of labels.                     */
          double                                              width;        /** width ratio [0,1].                    */
          double                                              height;       /** height ratio [0,1].                   */
          double                                              barRatio;     /** bar size relative to widget in [0,1]. */
          unsigned int                                        decimals;     /** number of decimals of labels.         */
          GUI::Utils::ColorRange                             *colors;       /** color range object.                   */
          QStringList                                         categories;   /** categories of categorical data.       */

          /** \brief Properties struct constructor.
           *
           */
          Properties(QString &title,
                     GUI::ColorEngineRangeDefinitionDialog::Position pos,
                     GUI::ColorEngineRangeDefinitionDialog::Orientation orient,
                     GUI::ColorEngineRangeDefinitionDialog::TextPosition tPosition,
                     unsigned int num,
                     double width,
                     double height,
                     double ratio,
                     unsigned int dec,
                     GUI::Utils::ColorRange *colors)
          : title{title}, position{pos}, orientation{orient}, textPosition{tPosition}, numLabels{num}, width{width}, height{height}, barRatio{ratio}, decimals{dec}, colors{colors}
          {};

          /** \brief Properties struct constructor.
           *
           */
          Properties()
          : title{QString("Unknown")}
          , position{GUI::ColorEngineRangeDefinitionDialog::Position::TOP_LEFT}
          , orientation{GUI::ColorEngineRangeDefinitionDialog::Orientation::VERTICAL}
          , textPosition{GUI::ColorEngineRangeDefinitionDialog::TextPosition::PRECEDE}
          , numLabels{4}, width{0.1}, height{0.5}, barRatio{0.25}, decimals{6}, colors{new GUI::Utils::RangeHSV(0,1)}
          {};

          /** \brief Properties struct destructor.
           *
           */
          ~Properties()
          { /** colors deleted in switch. */ }
      };

      /** \brief ColorEngineWidgetRepresentation class constructor.
       *
       */
      explicit InformationColorEngineWidgetRepresentationImplementation();

      /** \brief ColorEngineWidgetRepresentation class virtual destructor.
       *
       */
      virtual ~InformationColorEngineWidgetRepresentationImplementation();

      /** \brief Sets the widget properties.
       * \param[in] properties Widget properties struct.
       *
       */
      void setProperties(const Properties &properties);

      /** \brief Returns the widget properties.
       *
       */
      const Properties getProperties() const;

      /** \brief Sets the separation of the widget to the margins of the view.
       * \param[in] margin Distance in pixels to the border of the view.
       *
       */
      void setMargin(unsigned int margin)
      { m_margin = margin; }

    protected:
      /** \brief Implements TemporalRepresentation::initialize(RenderView *).
       *
       */
      void baseInitialize(RenderView *view);

      /** \brief Implements TemporalRepresentation::uninitialize().
       *
       */
      void baseUninitialize();

      /** \brief Implements TemporalRepresentation::show().
       *
       */
      void baseShow();

      /** \brief Implements TemporalRepresentation::hide().
       *
       */
      void baseHide();

      /** \brief Implements TemporalRepresentation::display(FrameCSPtr &).
       *
       */
      void baseDisplay(const GUI::Representations::FrameCSPtr &frame);

      /** \brief Positions the widget on the view.
       *
       */
      void placeWidget();

    private:
      /** \brief Helper method to create the actor to be inserted on the view.
       *
       */
      void createActors();

      /** \brief Updates the actors' properties.
       *
       */
      void updateActors();

      bool                    m_active;     /** true if the actor is in the view and false otherwise. */
      RenderView             *m_view;       /** view of the widget.                                   */
      Properties              m_properties; /** widget properties.                                    */
      unsigned int            m_margin;     /** distance in pixels from the view's border.            */

      vtkSmartPointer<vtkLookupTable>    m_lut;       /** bar colors.   */
      vtkSmartPointer<vtkScalarBarActor> m_scalarBar; /** bar actor.    */
      vtkSmartPointer<vtkLegendBoxActor> m_legend;    /** legend actor. */
  };

  //-----------------------------------------------------------------------------
  /** \class ColorEngineWidgetRepresentation2D
   * \brief Class for displaying a color range widget on the 2D views
   *
   */
  class InformationColorEngineWidgetRepresentation2D
  : public GUI::Representations::Managers::TemporalRepresentation2D
  , public InformationColorEngineWidgetRepresentationImplementation
  {
      Q_OBJECT
    public:
      /** \brief InformationColorEngineWidgetRepresentation2D class constructor
       *
       */
      explicit InformationColorEngineWidgetRepresentation2D()
      {};

      /** \brief InformationColorEngineWidgetRepresentation2D class virtual destructor.
       *
       */
      virtual ~InformationColorEngineWidgetRepresentation2D()
      {};

    protected:
      virtual void initialize(RenderView *view)                           override final;
      virtual void uninitialize()                                         override final { baseUninitialize();   };
      virtual void show()                                                 override final { baseShow();           };
      virtual void hide()                                                 override final { baseHide();           };
      virtual void display(const GUI::Representations::FrameCSPtr &frame) override final { baseDisplay(frame);   };

      virtual bool acceptCrosshairChange(const NmVector3 &crosshair) const override final
      { return false; };

      virtual bool acceptSceneResolutionChange(const NmVector3 &resolution) const override final
      { return false; };

      virtual bool acceptSceneBoundsChange(const Bounds &bounds) const override final
      { return false; };

      virtual bool acceptInvalidationFrame(const GUI::Representations::FrameCSPtr frame) const override final
      { return false; };

      virtual void setPlane(Plane plane)
      { /* empty */ };

      virtual void setRepresentationDepth(Nm depth)
      { /* empty */ };

    private slots:
      void onViewResized() { placeWidget(); };

    private:
      virtual GUI::Representations::Managers::TemporalRepresentation2DSPtr cloneImplementation()
      { return std::make_shared<InformationColorEngineWidgetRepresentation2D>(); };
  };

  //-----------------------------------------------------------------------------
  /** \class ColorEngineWidgetRepresentation3D
   * \brief Class for displaying a color range widget on the 3D views
   *
   */
  class InformationColorEngineWidgetRepresentation3D
  : public GUI::Representations::Managers::TemporalRepresentation3D
  , public InformationColorEngineWidgetRepresentationImplementation
  {
      Q_OBJECT
    public:
      /** \brief InformationColorEngineWidgetRepresentation3D class constructor.
       *
       */
      explicit InformationColorEngineWidgetRepresentation3D()
      {};

      /** \brief InformationColorEngineWidgetRepresentation3D class virtual destructor.
       *
       */
      virtual ~InformationColorEngineWidgetRepresentation3D()
      {};

    protected:
      virtual void initialize(RenderView *view)                           override final;
      virtual void uninitialize()                                         override final { baseUninitialize();   };
      virtual void show()                                                 override final { baseShow();           };
      virtual void hide()                                                 override final { baseHide();           };
      virtual void display(const GUI::Representations::FrameCSPtr &frame) override final { baseDisplay(frame);   };

      virtual bool acceptCrosshairChange(const NmVector3 &crosshair) const override final
      { return false; };

      virtual bool acceptSceneResolutionChange(const NmVector3 &resolution) const override final
      { return false; };

      virtual bool acceptSceneBoundsChange(const Bounds &bounds) const override final
      { return false; };

      virtual bool acceptInvalidationFrame(const GUI::Representations::FrameCSPtr frame) const override final
      { return false; };

    private slots:
      void onViewResized() { placeWidget(); };

    private:
      virtual GUI::Representations::Managers::TemporalRepresentation3DSPtr cloneImplementation()
      { return std::make_shared<InformationColorEngineWidgetRepresentation3D>(); };
  };

  //-----------------------------------------------------------------------------
  /** \class InformationColorEngineSwitch
   * \brief Switch for coloring by property.
   *
   */
  class InformationColorEngineSwitch
  : public Support::Widgets::ColorEngineSwitch
  {
      Q_OBJECT

    public:
      /** \brief InformationColorEngineSwitch class constructor.
       * \param[in] context application context.
       *
       */
      explicit InformationColorEngineSwitch(Support::Context& context);

      /** \brief InformationColorEngineSwitch class virtual destructor.
       *
       */
      virtual ~InformationColorEngineSwitch();

      using InformationKey = Core::SegmentationExtension::InformationKey;

      virtual void restoreSettings(std::shared_ptr<QSettings> settings);

      virtual void saveSettings(std::shared_ptr<QSettings> settings);

    private:
      /** \brief Creates the property selection widget.
       *
       */
      void createPropertySelector();

      /** \brief Creates the color range widget.
       *
       */
      void createColorRange();

      /** \brief Creates the range limits dialog button.
       *
       */
      void createLimitsButton();

      /** \brief Helper method to return the color engine.
       *
       */
      GUI::ColorEngines::InformationColorEngine *informationColorEngine() const;

      /** \brief Updates the colors of the segmentations and the property selection label widget.
       *
       */
      void update();

      /** \brief Helper method to update the property selection label widget.
       *
       */
      void updateLink();

      /** \brief Creates the settings widgets.
       *
       */
      void createWidgets();

      /** \brief Creates the widget representation factory.
       *
       */
      void createRepresentationFactory();

      /** \brief Removes and destroys the representation factory.
       *
       */
      void removeAndDestroyRepresentationFactory();

      /** \brief Launches the task to get the minimum and maximum values of the selected property.
       * \param[in] key Property to update the range.
       *
       */
      void updateRange(const InformationKey &key);

      /** \brief Updates the visual properties of the widgets.
       *
       */
      void updateWidgetsSettings();

    private slots:
      /** \brief Displays the dialog to change the coloring property and manages the result.
       *
       */
      void changeProperty();

      /** \brief Updates the current range values when a segmentation is removed or added.
       *
       */
      void updateCurrentRange();

      /** \brief Helper method to enable/disable the tool.
       *
       */
      void onToolToggled(bool checked);

      /** \brief Helper method to check for the returning value of the task and to inform the user of
       * any failure.
       */
      void onTaskFinished();

      /** \brief Aborts the currently running task.
       *
       */
      void abortTask();

      /** \brief Shows the change range limits dialog.
       *
       */
      void onLimitsButtonPressed();

      /** \brief Shows the range configuration dialog.
       *
       */
      void onRangeButtonClicked();

      /** \brief Stores the clone pointer to update properties later of widgets in 2d views.
       * \param[in] clone Widget 2d clone.
       *
       */
      void onWidgetCloned(GUI::Representations::Managers::TemporalRepresentation2DSPtr clone);

      /** \brief Stores the clone pointer to update properties later of widgets in 3d views.
       * \param[in] clone Widget 3d clone.
       *
       */
      void onWidgetCloned(GUI::Representations::Managers::TemporalRepresentation3DSPtr clone);

      /** \brief Searchs and removes the destroyed widgets from the widgets list.
       * \param[in] object QObject pointer.
       *
       */
      void onWidgetDestroyed(QObject *object);

      /** \brief Creates/destroys the widgets depending on the value.
       * \param[in] value QCheckbox state value.
       *
       */
      void onWidgetsActivated(int value);

      /** \brief Updates the widgets when a property is modified.
       *
       */
      void onWidgetPropertiesModified();

    private:
      using PrototypesSPtr     = GUI::Representations::Managers::TemporalPrototypesSPtr;
      using RepresentationSPtr = GUI::Representations::Managers::TemporalRepresentationSPtr;

      InformationKey            m_key;          /** coloring property.                                                                    */
      bool                      m_needUpdate;   /** true if the coloring values needs to be updated.                                      */
      TaskSPtr                  m_task;         /** task to compute the maximum and minimum values of the property for all segmentations. */
      QLabel                   *m_property;     /** property link label widget.                                                           */
      double                    m_minimum;      /** maximum of the range or max of range if not set.                                      */
      double                    m_maximum;      /** maximum of the range or min of range if not set.                                      */
      GUI::Widgets::ToolButton *m_rangeButton;  /** color range button.                                                                   */
      GUI::Widgets::ToolButton *m_limitsButton; /** range limits button.                                                                  */
      PrototypesSPtr            m_repFactory;   /** range widget representation factory                                                   */
      QList<RepresentationSPtr> m_widgets;      /** cloned widgets list.                                                                  */

      InformationColorEngineWidgetRepresentationImplementation::Properties m_properties; /** widgets properties.                          */
  };

  //-----------------------------------------------------------------------------
  /** \class UpdateColorEngineTask
   * \brief Class for computing the minimum and maximum numerical value of a segmentation property
   *        for a given group of segmentations.
   */
  class UpdateColorEngineTask
  : public Task
  {
      Q_OBJECT
    public:
      /** \brief UpdateColorEngineTask class constructor.
       * \param[in] key segmentation extension information key whose value will be used as coloring value.
       * \param[in] segmentations group of segmentation for coloring.
       * \param[in] factory model factory for creating segmentation extensions.
       * \param[in] scheduler application task scheduler.
       *
       */
        explicit UpdateColorEngineTask(const Core::SegmentationExtension::InformationKey &key,
                                       SegmentationAdapterList                            segmentations,
                                       ModelFactorySPtr                                   factory,
                                       SchedulerSPtr                                      scheduler);

        /** \brief Returns true if the coloring has failed and false otherwise.
         *
         */
        bool hasFailed() const
        { return m_failed; }

        /** \brief Returns the error message if the coloring error if the task failed or an empty string otherwise.
         *
         */
        const QString error() const
        { return m_error; }

        /** \brief Returns the information key used for coloring.
         *
         */
        const QString property() const
        { return m_key.value(); }

        /** \brief Returns the extension of the key used for coloring.
         *
         */
        const QString extension() const
        { return m_key.extension(); }

        /** \brief Returns the number of segmentations that do not have the selected property.
         *
         */
        const unsigned long invalid() const
        { return m_invalid; }

        /** \brief Returns the range minimum or 0 if failed.
         *
         */
        const double min() const;

        /** \brief Returns the range maximum or -1 if failed.
         *
         */
        const double max() const;

        /** \brief Returns true if the data obtained is categorical and false otherwise.
         *
         */
        const bool isCategorical() const;

        /** \brief Returns the obtained categories sorted.
         *
         */
        const QStringList categories() const;

    private:
      virtual void run() override final;

    private:
      const Core::SegmentationExtension::InformationKey m_key;           /** property key.                                               */
      SegmentationAdapterList                           m_segmentations; /** segmentation to color.                                      */
      ModelFactorySPtr                                  m_factory;       /** factory to create segmentation extensions.                  */
      QString                                           m_error;         /** empty if the task doesn't fail and error message otherwise. */
      bool                                              m_failed;        /** true if the coloring fails, false otherwise.                */
      unsigned long                                     m_invalid;       /** number of segmentations that do not have the property.      */
      double                                            m_min;           /** range minimum value or 0 if failed.                         */
      double                                            m_max;           /** range maximum value or -1 if failed.                        */
      QStringList                                       m_categories;    /** categories range.                                           */
  };
}

#endif // ESPINA_INFORMATION_COLOR_ENGINE_SWITCH_H
