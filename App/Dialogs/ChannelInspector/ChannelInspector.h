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

#ifndef ESPINA_CHANNEL_INSPECTOR_H_
#define ESPINA_CHANNEL_INSPECTOR_H_

#include <QDialog>
#include <ui_ChannelInspector.h>

// ESPINA
#include <Core/Types.h>
#include <GUI/Types.h>
#include <GUI/Widgets/HueSelector.h>
#include <GUI/Representations/ManualPipelineSources.h>
#include <GUI/View/RepresentationInvalidator.h>
#include <Support/Context.h>

using ESPINA::GUI::View::RepresentationInvalidator;

class QCloseEvent;

namespace ESPINA
{
  class ViewManager;
  class View2D;
  class HueSelector;

  class ChannelInspector
  : public QDialog
  , private Ui::ChannelInspector
  , private Support::WithContext
  {
    Q_OBJECT
  public:
    /** \brief ChannelInspector class constructor.
     * \param[in] channel channel adapter raw pointer.
     * \param[in] model model adapter smart pointer.
     * \param[in] scheduler scheduler smart pointer.
     * \param[in] parent parent widget.
     */
    explicit ChannelInspector(ChannelAdapterSPtr channel, Support::Context &context);

    /** \brief Channel Inspector class destructor.
     *
     */
    virtual ~ChannelInspector();

    /** \brief Manages dialog close event.
     * Re-implemented from base class because we need to reset channel
     * properties before the dialog gets destroyed.
     *
     */
    void closeEvent(QCloseEvent *event) override;

  signals:
    void spacingUpdated();

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

    /** \brief Changes the HUE value of the channel based on h,s,v values
     *        returned by HueSelector.
     * \param[in] h hue value
     * \param[in] s saturation value (unused)
     * \param[in] v color value (unused)
     *
     */
    void newHSV(int h,int s,int v);

    /** \brief Changes the HUE value of the channel based on the
     *        value of the spinbox of the UI.
     * \param[in] value hue value.
     */
    void newHSV(int value);

    /** \brief Changes the saturation of the channel.
     * \param[in] value value of the saturation slider.
     *
     */
    void saturationChanged(int value);

    /** \brief Changes the contrast of the channel.
     * \param[in] value value of the contrast slider.
     */
    void contrastChanged(int value);

    /** \brief Changes the brightness of the channel.
     * \param[in] value value of the brigthness slider.
     */
    void brightnessChanged(int value);

    /** \brief Apply the changes of the UI to the channel.
     *
     */
    void onChangesAccepted();

    /** \brief Resets the channel to previous values.
     *
     */
    void onChangesRejected();

    /** \brief
     * \param[in] value
     */
    void radioEdgesChanged(bool value);

    /** \brief Applies edges changes to the channel.
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

  private:
    /** \brief Helper method to update views after changes to the channel.
     *
     */
    void applyModifications();

    void initPropertiesTab();

    void initSliceView();

    void initSpacingSettings();

    void initOpacitySettings();

    void initColorSettings();

    void invalidateChannelRepresentation();

    /** \brief Changes the spacing of the channel and associated segmentations.
     *
     */
    void updateStackPreview();

    void changeStackSpacing();

    /** \brief Initializes the pixel value selector widget.
     *
     */
    void initPixelValueSelector();

    NmVector3 currentSpacing() const;

  private:
    GUI::View::ViewState m_viewState;

    bool   m_spacingModified;
    bool   m_edgesModified;

    GUI::Widgets::PixelValueSelector *m_pixelSelector;

    ChannelAdapterSPtr m_channel;

    ManualPipelineSources     m_sources;
    std::shared_ptr<View2D>   m_view;

    HueSelector *m_hueSelector;

    bool m_useDistanceToEdges;
    int  m_backgroundColor;
    int  m_threshold;

    // properties backup
    NmVector3 m_spacing;
    double    m_opacity;
    double    m_hue;
    double    m_saturation;
    double    m_brightness;
    double    m_contrast;
  };

} // namespace ESPINA

#endif /* ESPINA_CHANNEL_INSPECTOR_H_ */
