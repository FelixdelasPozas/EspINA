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

#ifndef ESPINA_SUPPORT_REFINE_TOOL_H
#define ESPINA_SUPPORT_REFINE_TOOL_H

// ESPINA
#include <Support/Factory/FilterRefinerFactory.h>
#include <ToolGroups/ToolGroup.h>


class QUndoStack;

namespace ESPINA
{
  /** \class EditionFilterFactory
   * \brief Factory for segmentation edition filters.
   *
   */
  class EditionFilterFactory
  : public FilterFactory
  {
    public:
      /** \brief Returns the close filter's signatures.
       *
       */
      static FilterTypeList CloseFilters();

      /** \brief Returns the open filter's signatures.
       *
       */
      static FilterTypeList OpenFilters();

      /** \brief Returns the dilate filter's signatures.
       *
       */
      static FilterTypeList DilateFilters();

      /** \brief Returns the erode filter's signatures.
       *
       */
      static FilterTypeList ErodeFilters();

      /** \brief Returns the image logic (addition & subtraction) filter's signatures.
       *
       */
      static FilterTypeList ImageLogicFilters();

      /** \brief Returns the fill holes filter's signatures.
       *
       */
      static FilterTypeList FillHolesFilters();

      /** \brief Returns the interpolation filter's signatures.
       *
       */
      static FilterTypeList InterpolationFilters();

      static const Filter::Type CLOSE_FILTER;           /** close filter signature.            */
      static const Filter::Type CLOSE_FILTER_V4;        /** close filter old signature.        */
      static const Filter::Type OPEN_FILTER;            /** open filter signature.             */
      static const Filter::Type OPEN_FILTER_V4;         /** open filter old signature.         */
      static const Filter::Type DILATE_FILTER;          /** dilate filter signature.           */
      static const Filter::Type DILATE_FILTER_V4;       /** dilate filter old signature.       */
      static const Filter::Type ERODE_FILTER;           /** erode filter signature.            */
      static const Filter::Type ERODE_FILTER_V4;        /** erode filter old signature.        */
      static const Filter::Type FILL_HOLES_FILTER;      /** fill holes filter signature.       */
      static const Filter::Type FILL_HOLES_FILTER_V4;   /** fill holes filter old signature.   */
      static const Filter::Type FILL_HOLES2D_FILTER;    /** fill holes filter 2D signature.    */
      static const Filter::Type IMAGE_LOGIC_FILTER;     /** image logic filters old signature. */
      static const Filter::Type ADDITION_FILTER;        /** addition filter signature.         */
      static const Filter::Type SUBTRACTION_FILTER;     /** subtraction filter signature.      */
      static const Filter::Type SLICE_INTERPOLATION_FILTER;     /** slice interpolation filter signature.      */

    private:
      virtual FilterTypeList providedFilters() const;

      virtual FilterSPtr createFilter(InputSList inputs, const Filter::Type& filter, SchedulerSPtr scheduler) const;

    private:
      /** \brief Returns true if the given filter type corresponds to a close filter and false otherwise.
       * \param[in] type filter type.
       *
       */
      bool isCloseFilter       (const Filter::Type &type) const;

      /** \brief Returns true if the given filter type corresponds to a opend filter and false otherwise.
       * \param[in] type filter type.
       *
       */
      bool isOpenFilter        (const Filter::Type &type) const;

      /** \brief Returns true if the given filter type corresponds to a dilate filter and false otherwise.
       * \param[in] type filter type.
       *
       */
      bool isDilateFilter      (const Filter::Type &type) const;

      /** \brief Returns true if the given filter type corresponds to a erode filter and false otherwise.
       * \param[in] type filter type.
       *
       */
      bool isErodeFilter       (const Filter::Type &type) const;

      /** \brief Returns true if the given filter type corresponds to a fill holes filter and false otherwise.
       * \param[in] type filter type.
       *
       */
      bool isFillHolesFilter   (const Filter::Type &type) const;

      /** \brief Returns true if the given filter type corresponds to a fill holes filter and false otherwise.
       *
       */
      bool isFillHoles2DFilter(const Filter::Type &type) const;

      /** \brief Returns true if the given filter type corresponds to an addition filter and false otherwise.
       * \param[in] type filter type.
       *
       */
      bool isAdditionFilter    (const Filter::Type &type) const;

      /** \brief Returns true if the given filter type corresponds to a subtraction filter and false otherwise.
       * \param[in] type filter type.
       *
       */
      bool isSubstractionFilter(const Filter::Type &type) const;

      /** \brief Returns true if the given filter type corresponds to a slice interpolation filter and false otherwise.
       * \param[in] type filter type.
       *
       */
      bool isSliceInterpolationFilter(const Filter::Type &type) const;

    private:
      mutable DataFactorySPtr m_dataFactory; /** data factory object. */
  };

  class EditToolGroup
  : public ToolGroup
  , private Support::WithContext
  {
    Q_OBJECT
  public:
    /** \brief EditionToolGroup class constructor.
     * \param[in] filterRefiners filter refiners factory.
     * \param[in] context application context
     *
     */
    explicit EditToolGroup(Support::FilterRefinerFactory &filterRefiners,
                           Support::Context               &context);

    /** \brief EditionToolGroup class virtual destructor.
     *
     */
    virtual ~EditToolGroup();

  private:
    /** \brief Registers the refiners in the factory.
     * \param[in] filterRefiner filter refiner object.
     *
     */
    void registerFilterRefiners(Support::FilterRefinerFactory& filterRefiner);

    /** \brief Modifies the gui and tool paramenters when the manual edition is activated.
     *
     */
    void initManualEditionTool();

    /** \brief Modifies the gui and tool parameters when any of the morphological tools is activated.
     *
     */
    void initCODETools();

    /** \brief Modifies the gui and tool parameters when any of the fill holes tools is activated.
     *
     */
    void initFillHolesTools();

    /** \brief Modifies the gui and tool parameters when a logic operation tool is activated.
     *
     */
    void initImageLogicTools();

    /** \brief Modifies the gui and tool parameters when the split tool is activated.
     *
     */
    void initSplitTool();

    /** \brief Modifies the gui and tool parameters when a interpolation operation tool is activated.
     *
     */
    void initSliceInterpolationTool();
  };

} // namespace ESPINA

#endif // ESPINA_SUPPORT_REFINE_TOOL_H
