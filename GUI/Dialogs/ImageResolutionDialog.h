/*
 * Copyright (C) 2016, Rafael Juan Vicente Garcia <rafaelj.vicente@gmail.com>
 *
 * This file is part of ESPINA.
 *
 * ESPINA is free software: you can redistribute it and/or modify
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

#ifndef ESPINA_IMAGE_SIZE_DIALOG_H
#define ESPINA_IMAGE_SIZE_DIALOG_H

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include "ui_ImageResolutionDialog.h"

// Qt
#include <QDialog>

namespace ESPINA
{

  class EspinaGUI_EXPORT ImageResolutionDialog: public QDialog,
      private Ui::ImageResolutionDialog
  {
    Q_OBJECT

    public:

      /** \brief ImageResolutionDialog class constructor.
       * \param[in] parent, Parent widget.
       * \param[in] width, image width.
       * \param[in] height, image height.
       */
      ImageResolutionDialog(QWidget *parent, int width, int height);

      /** \brief ImageResolutionDialog class constructor.
       * \param[in] parent, Parent widget.
       * \param[in] width, image width.
       * \param[in] height, image height.
       * \param[in] image, image to show.
       */
      ImageResolutionDialog(QWidget *parent, int width, int height,
          QImage& image);

      /** \brief ImageResolutionDialog class destructor.
       */
      virtual ~ImageResolutionDialog()
      {
      }
      ;

      /** \brief Return the ratio of the image.
       */
      const double getRatio() const;

      /** \brief Return the initial height of the image.
       */
      const int getInitialHeight() const;

      /** \brief Return the initial width of the image.
       */
      const int getInitialWidth() const;

      /** \brief Return the height of the image.
       */
      int getHeight() const;

      /** \brief Return the width of the image.
       */
      int getWidth() const;

      /** \brief Return the initial size of the image.
       */
      const QSize getInitialSize() const;

      /** \brief Return the size of the image.
       */
      QSize getSize() const;

      /** \brief Return magnification based on expected size.
       */
      int getMagnifcation() const;

    private slots:

      void onHeightChanged(int value);
      void onWidthChanged(int value);

    private:

      const double m_ratio; /** height/width ratio */
      const int m_initialHeight; /** initial height */
      const int m_initialWidth; /** initial width */
      int m_height; /** height */
      int m_width; /** width */
  };
}

#endif // ESPINA_CATEGORY_SELECTOR_DIALOG_H
