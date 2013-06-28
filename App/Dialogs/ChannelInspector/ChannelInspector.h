/*
 * ChannelInspector.h
 *
 *  Created on: Dec 16, 2012
 *      Author: Félix de las Pozas Álvarez
 */

#ifndef CHANNELINSPECTOR_H_
#define CHANNELINSPECTOR_H_

// EspINA
#include <ui_ChannelInspector.h>
#include <GUI/QtWidget/HueSelector.h>

// Qt
#include <QDialog>

class QCloseEvent;

namespace EspINA
{
  class Channel;
  class ViewManager;
  class SliceView;
  class HueSelector;
  class EspinaModel;


  class ChannelInspector
  : public QDialog
  , private Ui::ChannelInspector
  {
    Q_OBJECT
  public:
    explicit ChannelInspector(Channel *, EspinaModel *, QWidget *parent = 0);
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

    bool m_spacingModified;
    bool m_edgesModified;
    double m_range[2];

    Channel     *m_channel;
    ViewManager *m_viewManager;
    EspinaModel *m_model;

    SliceView   *m_view;
    HueSelector *m_hueSelector;

    bool m_adaptiveEdgesEnabled;
    int m_backgroundColor;
    int m_threshold;

    // properties backup
    double m_spacing[3];
    double m_opacity;
    double m_hue;
    double m_saturation;
    double m_brightness;
    double m_contrast;
  };

} // namespace EspINA

#endif /* CHANNELINSPECTOR_H_ */
