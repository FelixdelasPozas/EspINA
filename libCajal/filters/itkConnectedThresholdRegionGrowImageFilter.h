#ifndef __itkConnectedThresholdRegionGrowImageFilter_h
#define __itkConnectedThresholdRegionGrowImageFilter_h

#include "itkImage.h"
#include "itkSimpleDataObjectDecorator.h"
#include "itkRegionGrowImageFilter.h"

namespace itk
{

template <class TImage>
class GridPixel
{
public:
	typedef          TImage              ImageType;
	typedef typename TImage::IndexType   IndexType;
	typedef typename TImage::Pointer     Pointer;
	typedef typename TImage::RegionType  RegionType;
	typedef typename TImage::SizeType    SizeType;
	typedef typename TImage::PixelType   PixelType;

	GridPixel ()
	{
		m_Visited = false;
		m_Output = NULL;
	}

	bool Tested ( IndexType seed )
	{
		// NOTE: Llamar solo si la salida ha sido inicializada
		//       No se está comprobando la condición

		return m_Output->GetPixel(seed) > 0;
	}

	void InitializeOutput ()
	{
		m_Output = ImageType::New();
		m_Output->SetRegions( m_OutputRegion );
		m_Output->Allocate();
		m_Output->FillBuffer ( NumericTraits<PixelType>::Zero );
	}

	void Initialize ( IndexType index, SizeType size )
	{
		m_OutputRegion.SetIndex( index );
		m_OutputRegion.SetSize ( size  );
	}

	std::vector<IndexType> GetSeeds()
	{
		return m_Seeds;
	}

	Pointer GetOutput()
	{
		return m_Output;
	}

	RegionType GetOutputRegion()
	{
		return m_OutputRegion;
	}

	void AddSeed( IndexType seed )
	{
		m_Seeds.push_back( seed );
	}

	void ClearSeeds()
	{
		m_Seeds.clear();
	}

	bool HasSeed( IndexType seed )
	{
		for ( unsigned int i=0; i<m_Seeds.size(); i++ )
		{
			if ( seed == m_Seeds[i] )
				return true;
		}

		return false;
	}

	bool IsVisited()
	{
		return m_Visited;
	}

	void Visitable()
	{
		m_Visited = false;
	}

	void Visited()
	{
		m_Visited = true;
	}

private:
	bool                   m_Visited;
	std::vector<IndexType> m_Seeds;
	Pointer                m_Output;
	RegionType             m_OutputRegion;

};

typedef itk::Image<unsigned char,3> TInputImage;
typedef TInputImage TOutputImage;
//template <class TInputImage, class TOutputImage>
class ITK_EXPORT ConnectedThresholdRegionGrowImageFilter:
	public ConnectedThresholdImageFilter<TInputImage,TOutputImage>
{
public:
	/** Standard class typedefs. */
	typedef ConnectedThresholdRegionGrowImageFilter                 Self;
	typedef ConnectedThresholdImageFilter<TInputImage,TOutputImage> Superclass;
	typedef SmartPointer<Self>                                      Pointer;
	typedef SmartPointer<const Self>                                ConstPointer;

	/** Method for creation through the object factory. */
	itkNewMacro(Self);

	/** Run-time type information (and related methods). */
	itkTypeMacro(ConnectedThresholdRegionGrowImageFilter,ConnectedThresholdImageFilter);

	/** Image dimension constants */
	itkStaticConstMacro(InputImageDimension, unsigned int,
						TInputImage::ImageDimension);
	itkStaticConstMacro(OutputImageDimension, unsigned int,
						TOutputImage::ImageDimension);

	/** Type definition for the input image. */
	typedef TInputImage                          InputImageType;
	typedef /*typename*/TInputImage::Pointer        InputImagePointer;
	typedef /*typename*/TInputImage::ConstPointer   InputImageConstPointer;
	typedef /*typename*/TInputImage::IndexType      InputImageIndexType;
	typedef /*typename*/TInputImage::PixelType      InputImagePixelType;
	typedef /*typename*/TInputImage::SizeType       InputImageSizeType;

	/** Type definition for the output image. */
	typedef TOutputImage                      OutputImageType;
	typedef /*typename*/TOutputImage::Pointer    OutputImagePointer;
	typedef /*typename*/TOutputImage::RegionType OutputImageRegionType;
	typedef /*typename*/TOutputImage::IndexType  OutputImageIndexType;
	typedef /*typename*/TOutputImage::SizeType   OutputImageSizeType;
	typedef /*typename*/TOutputImage::PixelType  OutputImagePixelType;

	/** Type definition for the internal grid */
	// TODO: This should be private
	typedef GridPixel<TOutputImage>                   GridPixelType;
	typedef Image<GridPixelType, InputImageDimension> GridType;
	typedef /*typename*/ GridType::IndexType              GridIndexType;
	typedef /*typename*/ GridType::Pointer                GridPointer;
	typedef /*typename*/ GridType::RegionType             GridRegionType;
	typedef /*typename*/ GridType::SizeType               GridSizeType;
	typedef /*typename*/ GridType::OffsetType             GridOffsetType;

	/** Type of DataObjects to use for scalar inputs */
	typedef SimpleDataObjectDecorator<InputImagePixelType> InputPixelObjectType;

	// Inicializa los tamaños en los que se dividirá el grid
	void SetGridSize( GridSizeType gridSize );

	virtual void GenerateData();
	virtual OutputImageType * GetOutput(void);

protected:
	std::vector<GridIndexType>       m_VisitableRegions;

	GridPointer                      m_Grid;
	GridSizeType                     m_GridSize;
	GridSizeType                     m_GridNonOverlappedRegionSize;
	GridSizeType			         m_GridOverlappedRegionSize;

	OutputImagePointer               m_Output;

	OutputImageRegionType            m_BoundingBox;
	OutputImageIndexType             m_BoundingBoxIndex;
	OutputImageSizeType              m_BoundingBoxSize;

	InputImageSizeType               lastRegionSize;
	InputImageSizeType               m_InputImageSize;

	// Verdadero si el valor indicado se encuentra dentro del rango especificado
	bool IsInThreshold( const InputImagePixelType pixel );

	// Devuelve el índice correspondiente a la imagen temporal (grid)
	// a partir del índice de la imágen de entrada
	GridIndexType GetGridIndex( const InputImageIndexType inputIndex );

	virtual void Initialize();

	OutputImageIndexType GetOutputIndex ( GridIndexType gridIndex );

	GridOffsetType GetEdgeOffset( OutputImageRegionType region,
			                      OutputImageIndexType  index);

	GridIndexType ApplyOffset( GridIndexType gridIndex , GridOffsetType offset );

	void ApplyRegionGrowImageFilter();
	void ApplyConnectedThreshold( GridIndexType gridIndex );

	void AddVisitableNeighbor( GridIndexType index );
	void UpdateBoundingBox( GridIndexType index );

	void InitializeSeeds();
	void InitializeGrid();

	void MergeRegions();
};

}

// #include "itkConnectedThresholdRegionGrowImageFilter.cpp"

#endif
