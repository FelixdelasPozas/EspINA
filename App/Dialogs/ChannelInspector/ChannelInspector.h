/*
 * ChannelInspector.h
 *
 *  Created on: Dec 16, 2012
 *      Author: Félix de las Pozas Álvarez
 */

#ifndef ESPINA_CHANNEL_INSPECTOR_H_
#define ESPINA_CHANNEL_INSPECTOR_H_

// EspINA
#include <Core/EspinaTypes.h>
#include <GUI/Model/ChannelAdapter.h>
#include <GUI/Model/ModelAdapter.h>
#include <ui_ChannelInspector.h>
#include <GUI/Widgets/HueSelector.h>

// Qt
#include <QDialog>

class QCloseEvent;

namespace EspINA
{
  class ViewManager;
  class View2D;
  class HueSelector;

  class ChannelInspector
  : public QDialog
  , private Ui::ChannelInspector
  {
    Q_OBJECT
  public:
    explicit ChannelInspector(ChannelAdapterPtr channel, ModelAdapterSPtr model, SchedulerSPtr scheduler, QWidget *parent = 0);
    virtual ~ChannelInspector();

    // re-implemented from base class because we need to reset channel
    // properties before the dialog gets destroyed.
    void closeEvent(QCloseEvent *event);

  signals:
    void spacingUpdated();

  public slots:
    void unitsChanged(int);
    void spacingChanged(double unused = 0);
    void changeSpacing();
    void opacityCheckChanged(int);
    void opacityChanged(int);
    void newHSV(int,int,int);
    void newHSV(int);
    void saturationChanged(int);
    void contrastChanged(int);
    void brightnessChanged(int);
    void acceptedChanges();
    void rejectedChanges();
    void radioEdgesChanged(bool);
    void applyEdgesChanges();
    void changeEdgeDetectorBgColor(int value);
    void changeEdgeDetectorThreshold(int value);

  private:
    // helpher methods
    void applyModifications();

    bool   m_spacingModified;
    bool   m_edgesModified;
//    double m_range[2];

    ChannelAdapterPtr m_channel;
//    ViewManagerSPtr    m_viewManager;
    ModelAdapterSPtr   m_model;
    SchedulerSPtr      m_scheduler;
    View2D            *m_view;
    HueSelector       *m_hueSelector;

    bool m_adaptiveEdgesEnabled;
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

} // namespace EspINA

#endif /* ESPINA_CHANNEL_INSPECTOR_H_ */
