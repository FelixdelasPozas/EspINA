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

class Channel;
class ViewManager;
class SliceView;
class HueSelector;

class ChannelInspector
: public QDialog
, private Ui::ChannelInspector
{
  Q_OBJECT
  public:
    explicit ChannelInspector(Channel *, QWidget *parent = 0);
    virtual ~ChannelInspector();

  signals:
    void channelUpdated();

  public slots:
    void unitsChanged(int);
    void spacingChanged(double unused = 0);
    void changeSpacing();
    void opacityCheckChanged(int);
    void opacityChanged(int);
    void checkChanges();
    void newHSV(int,int,int);
    void newHSV(int);
    void saturationChanged(int);
    void contrastChanged(int);
    void brightnessChanged(int);

  private:
    // helpher methods
    void applyModifications();

    bool m_spacingModified;
    double m_range[2];

    Channel     *m_channel;
    ViewManager *m_viewManager;

    SliceView   *m_view;
    HueSelector *m_hueSelector;
};

#endif /* CHANNELINSPECTOR_H_ */
