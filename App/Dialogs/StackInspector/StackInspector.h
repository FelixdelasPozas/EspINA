/*

 Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef ESPINA_STACK_INSPECTOR_H_
#define ESPINA_STACK_INSPECTOR_H_

#include <QDialog>
#include <ui_StackInspector.h>

// ESPINA
#include <Core/Types.h>
#include <Extensions/SLIC/StackSLIC.h>
#include <GUI/Types.h>
#include <GUI/Widgets/HueSelector.h>
#include <GUI/Representations/ManualPipelineSources.h>
#include <Support/Context.h>

class QCloseEvent;

namespace ESPINA
{
  class ViewManager;
  class View2D;
  class HueSelector;

  /** \class StackInspector
   * \brief Implements a dialog to explore and modify stacks.
   *
   */
  class StackInspector
  : public QDialog
  , private Ui::StackInspector
  , private Support::WithContext
  {
      Q_OBJECT
    public:
      /** \brief StackInspector class constructor.
       * \param[in] channel channel adapter raw pointer.
       * \param[in] model model adapter smart pointer.
       * \param[in] scheduler scheduler smart pointer.
       * \param[in] parent parent widget.
       */
      explicit StackInspector(ChannelAdapterSPtr channel, Support::Context &context);

      /** \brief StackInspector class destructor.
       *
       */
      virtual ~StackInspector();

      /** \brief Manages dialog close event.
       * Re-implemented from base class because we need to reset stack
       * properties before the dialog gets destroyed.
       *
       */
      void closeEvent(QCloseEvent *event) override;

    signals:
      void spacingUpdated();
      void computeSLIC(unsigned char parameter_m_s, unsigned char parameter_m_c, Extensions::StackSLIC::SLICVariant variant, unsigned int max_iterations, double tolerance);

    private slots:
      /** \brief Manages the change of units from the UI.
       */
      void unitsChanged();

      /** \brief Keeps track of the changes in spacing.
       */
      void onSpacingChanged();

      /** \brief Manages the change of state of opacity checkbox.
       * \param[in] value state of the checkbox.
       *
       */
      void onOpacityCheckChanged(int value);

      /** \brief Manages the change of value of the opacity slider.
       * \param[in] value value of the slider.
       *
       */
      void onOpacityChanged(int value);

      /** \brief Changes the HUE value of the stack based on h,s,v values
       *        returned by HueSelector.
       * \param[in] h hue value
       * \param[in] s saturation value (unused)
       * \param[in] v color value (unused)
       *
       */
      void newHSV(int h,int s,int v);

      /** \brief Changes the HUE value of the stack based on the
       *        value of the spinbox of the UI.
       * \param[in] value hue value.
       */
      void newHSV(int value);

      /** \brief Changes the saturation of the stack.
       * \param[in] value value of the saturation slider.
       *
       */
      void saturationChanged(int value);

      /** \brief Changes the contrast of the stack.
       * \param[in] value value of the contrast slider.
       */
      void contrastChanged(int value);

      /** \brief Changes the brightness of the stack.
       * \param[in] value value of the brigthness slider.
       */
      void brightnessChanged(int value);

      /** \brief Apply the changes of the UI to the stack.
       *
       */
      void onChangesAccepted();

      /** \brief Resets the stack to previous values.
       *
       */
      void onChangesRejected();

      /** \brief
       * \param[in] value
       */
      void radioEdgesChanged(bool value);

      /** \brief Applies edges changes to the stack.
       *
       */
      void applyEdgesChanges();

      /** \brief Manages the changes in the background color value of the UI spinbox.
       * \param[in] value, value of the color spinbox.
       */
      void changeEdgeDetectorBgColor(int value);

      /** \brief Manages the changes in the threshold value of the UI spinbox.
       * \param[in] value value of the threshold spinbox.
       */
      void changeEdgeDetectorThreshold(int value);

      /** \brief Updates the GUI when the optimal checkbox state changed.
       * \param[in] unused unused value.
       */
      void onOptimalStateChanged(int unused);

      /** \brief Updates the stack filter on change.
       * \param[in] state checkbox state.
       *
       */
      void onStreamingChanged(int state);

      /** \brief Starts the SLIC task with the selected parameters
       */
      void onComputeSLIC();

      /** \brief Updates the main and slic tabs moving the view.
       * \param[in] index Current tab index.
       *
       */
      void onCurrentTabChanged(int index);

    private:
      /** \brief Helper method to update views after changes to the stack.
       *
       */
      void applyModifications();

      /** \brief Helper method to initialize the properties tab.
       *
       */
      void initPropertiesTab();

      /** \brief Helper method to initialize the view in properties tab.
       *
       */
      void initSliceView();

      /** \brief Helper method to initialize the spacing values.
       *
       */
      void initSpacingSettings();

      /** \brief Helper method to initialize the opacity values.
       *
       */
      void initOpacitySettings();

      /** \brief Helper method to initalize the coloring values.
       *
       */
      void initColorSettings();

      /** \brief Helper method to initialize the stack miscellaneous settings.
       *
       */
      void initMiscSettings();

      /** \breif Helper method to invalidate the stack representations.
       *
       */
      void invalidateStackRepresentation();

      /** \brief Changes the spacing of the stack and associated segmentations.
       *
       */
      void updateStackPreview();

      /** \brief Helper method to change the spacing of the stack .
       *
       */
      void changeStackSpacing();

      /** \brief Initializes the pixel value selector widget.
       *
       */
      void initPixelValueSelector();

      /** \brief Returns the current spacing of the stack.
       *
       */
      NmVector3 currentSpacing() const;

    private:
      using BackgroundSelector = GUI::Widgets::PixelValueSelector;

      GUI::View::ViewState      m_viewState;          /** state of the view in the properties tab.                                               */
      bool                      m_spacingModified;    /** true if the spacing has been modified, false otherwise.                                */
      bool                      m_edgesModified;      /** true if the edges have been modified, false otherwise.                                 */
      BackgroundSelector       *m_pixelSelector;      /** pixel value selector widget for edges tab.                                             */
      ChannelAdapterSPtr        m_stack;              /** stack.                                                                                 */
      ManualPipelineSources     m_sources;            /** manual sources for the view in the properties tab.                                     */
      std::shared_ptr<View2D>   m_view;               /** view in the properties tab.                                                            */
      HueSelector              *m_hueSelector;        /** hue selector widget in the properties tab.                                             */
      bool                      m_useDistanceToEdges; /** true to use the distance to edges to compute segmentations distances, false otherwise. */
      int                       m_backgroundColor;    /** edge background color.                                                                 */
      int                       m_threshold;          /** threshold to compute edge voxels (background color +/- threshold).                     */

      // properties backup
      NmVector3 m_spacing;    /** stack spacing.    */
      double    m_opacity;    /** stack opacity.    */
      double    m_hue;        /** stack hue.        */
      double    m_saturation; /** stack saturation. */
      double    m_brightness; /** stack brightness. */
      double    m_contrast;   /** stack contrast.   */

      // slic representation manager
      ESPINA::GUI::Representations::Managers::TemporalPrototypesSPtr m_slicRepresentation;
  };

} // namespace ESPINA

#endif // ESPINA_STACK_INSPECTOR_H_
