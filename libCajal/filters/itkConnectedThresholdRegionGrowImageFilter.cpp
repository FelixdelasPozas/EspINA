#ifndef __itkConnectedThresholdRegionGrowImageFilter_txx
#define __itkConnectedThresholdRegionGrowImageFilter_txx

// TODO: revisar tipos. creo que algunos no son los correctos (de momento funciona porque la imágen de entrada y la de salida tienen el mismo tipo)
// TODO: usar queue y set en los lugares en los que haga falta. Mirar si ITK tienen este tipo de estructuras de datos implementadas
// TODO: resolver dudas entre RegioGrowImageFilter e ImageToImageFilter para saber de qué clase debe heredar realmente. Probablemente deba heredar directamente de ConnectedThresholdImageFilter.
// TODO: poner correctamente las constantes y el manejo de memoria
// TODO: si el threshold es lo suficientemente alto como para que abarque la mayor parte de la imágen, el rendimiento decae a mínimos (varios ordenes de magnitud peor que la solución original).

//#include <queue>
//#include <set>
#include <vector>
//#include <cmath>
//#include <algorithm>

#include "itkImage.h"
#include "itkSimpleDataObjectDecorator.h"
#include "itkConnectedThresholdImageFilter.h"
#include "itkFloodFilledImageFunctionConditionalIterator.h"
#include "itkImageRegionIteratorWithIndex.h"
#include "itkImageRegionIterator.h"
#include "itkImageRegionConstIterator.h"
#include "itkIndex.h"
#include "itkSize.h"

#include "itkConnectedThresholdRegionGrowImageFilter.h"

namespace itk
{

//template <class TInputImage, class TOutputImage>
void
ConnectedThresholdRegionGrowImageFilter/*<TInputImage, TOutputImage>*/
::GenerateData()
{
	this->ApplyRegionGrowImageFilter();
}

//template <class TInputImage, class TOutputImage>
void
ConnectedThresholdRegionGrowImageFilter/*<TInputImage, TOutputImage>*/
::ApplyRegionGrowImageFilter()
{
	GridIndexType                    gridIndex;
	std::vector<InputImageIndexType> seedList;

	this->Initialize();

	// Mientras que existan regiones visitables
	while ( !m_VisitableRegions.empty() )
	{
		gridIndex = m_VisitableRegions.back();
		m_VisitableRegions.pop_back();

		this->ApplyConnectedThreshold ( gridIndex );
	}
}

// Merge regions two by two until maximum number of regions is reached.
// If the max number of regions is 0, all the regions get merged together in one pass,
// using the bounding box of the final image.
//template <class TInputImage, class TOutputImage>
void
ConnectedThresholdRegionGrowImageFilter/*<TInputImage, TOutputImage>*/
::MergeRegions()
{
	// Inicializamos la región del BoundingBox
	m_BoundingBox.SetSize ( m_BoundingBoxSize  );
	m_BoundingBox.SetIndex( m_BoundingBoxIndex );

	OutputImageIndexType  outputIndex;
	OutputImageSizeType   outputSize;
	OutputImageRegionType outputRegion;

	for ( int i=0; i<InputImageDimension; i++ )
	{
		outputSize[i]  = m_BoundingBoxSize[i]  * m_GridNonOverlappedRegionSize[i];
		outputIndex[i] = m_BoundingBoxIndex[i] * m_GridNonOverlappedRegionSize[i];
	}

	outputRegion.SetIndex( outputIndex );
	outputRegion.SetSize ( outputSize  );

	// Detectamos si se ha llegado al extremo, en cuyo caso tenemos que
	// adaptar el tamaño de la última región
	for ( unsigned int i=0; i<InputImageDimension; i++ )
	{
		if ( (outputIndex[i] + outputSize[i] - 1) >= m_InputImageSize[i] )
		{
			outputSize[i] -= m_GridNonOverlappedRegionSize[i];
			outputSize[i] += lastRegionSize[i];
		}
	}

	outputRegion.SetIndex( outputIndex );
	outputRegion.SetSize ( outputSize  );

	// Se inicializa la imágen de salida
	m_Output = OutputImageType::New();
	m_Output->SetRegions( outputRegion );
	m_Output->Allocate();

	// Rellenamos la imágen con un valor de gris de 128
	m_Output->FillBuffer( (unsigned char) 0 );
	// NOTE: creo que se podría prescindir de la inicialización.
	// Aunque eso constituiría un punto débil, porque al no inicializar
	// la memoria no sabemos qué puede contener. Me imagino que dependerá
	// directamente de la política de reserva de memoria del SO.

	// Construimos el iterador sobre el grid
	typedef ImageRegionIteratorWithIndex<GridType> IteratorType;
	IteratorType it (m_Grid, m_BoundingBox);
	GridPixelType pixel;

	it.GoToBegin();

	while( !it.IsAtEnd())
	{
		// Comprobamos si el pixel ha sido actualizado
		pixel = it.Value();
		if ( pixel.IsVisited() )
		{
			// Creamos un iterador sobre la región que queremos copiar de la imágen de salida
			// Y copiamos los píxeles uno a uno.

			OutputImageRegionType inputRegion ( pixel.GetOutput()->GetLargestPossibleRegion().GetIndex(),
												m_GridNonOverlappedRegionSize);
			OutputImageRegionType outputRegion( this->GetOutputIndex( it.GetIndex() ),
												m_GridNonOverlappedRegionSize);

			ImageRegionConstIterator<OutputImageType> inputIt (pixel.GetOutput(), inputRegion );
			ImageRegionIterator     <OutputImageType> outputIt(m_Output         , outputRegion);

			inputIt.GoToBegin();
			outputIt.GoToBegin();
			while( !inputIt.IsAtEnd() )
			{
			  outputIt.Set( inputIt.Get() );
			  ++inputIt;
			  ++outputIt;
			}
		}

		// Continuamos con la iteración
		++it;
	}
}

// Inicializa los tamaños en los que se dividirá el grid
//template <class TInputImage, class TOutputImage>
void
ConnectedThresholdRegionGrowImageFilter//<TInputImage, TOutputImage>
::SetGridSize( GridSizeType gridSize )
{
	InputImageConstPointer inputImage     = this->GetInput();
	InputImageSizeType     inputImageSize = inputImage->GetRequestedRegion().GetSize();

	m_InputImageSize = this->GetInput()->GetLargestPossibleRegion().GetSize();

	// Inicializamos el grid
	for (int i=0; i<InputImageDimension; i++)
	{
		if ( gridSize[i] <= inputImageSize[i] )
			m_GridSize[i] = gridSize[i];
		else
			m_GridSize[i] = inputImageSize[i];

		m_GridNonOverlappedRegionSize[i] = (unsigned int) round( 1.0*inputImageSize[i] / m_GridSize[i] );
		m_GridOverlappedRegionSize[i] = m_GridNonOverlappedRegionSize[i] + 1;

		lastRegionSize[i] = m_InputImageSize[i] - ((m_GridSize[i]-1)*m_GridNonOverlappedRegionSize[i]);
	}
}

//template <class TInputImage, class TOutputImage>
/*typename*/ ConnectedThresholdRegionGrowImageFilter/*<TInputImage, TOutputImage>*/::OutputImageType *
ConnectedThresholdRegionGrowImageFilter/*<TInputImage, TOutputImage>*/
::GetOutput(void)
{
	this->MergeRegions();

	return m_Output;
}

// Verdadero si el valor indicado se encuentra dentro del rango especificado
//template <class TInputImage, class TOutputImage>
bool
ConnectedThresholdRegionGrowImageFilter/*<TInputImage, TOutputImage>*/
::IsInThreshold( const InputImagePixelType pixel )
{
	return (this->GetLowerInput()->Get() <= pixel) && (pixel <= this->GetUpperInput()->Get());
}

// Devuelve el índice correspondiente a la imagen temporal (grid)
// a partir del índice de la imágen de entrada
//template <class TInputImage, class TOutputImage>
/*typename*/ ConnectedThresholdRegionGrowImageFilter/*<TInputImage, TOutputImage>*/::GridIndexType
ConnectedThresholdRegionGrowImageFilter/*<TInputImage, TOutputImage>*/
::GetGridIndex( const InputImageIndexType inputIndex )
{
	GridIndexType index;

	for ( int i=0; i<InputImageDimension; i++ )
		index[i] = (unsigned int) floor( inputIndex[i] / m_GridOverlappedRegionSize[i] );

	return index;
}

//template <class TInputImage, class TOutputImage>
void
ConnectedThresholdRegionGrowImageFilter/*<TInputImage, TOutputImage>*/
::Initialize()
{
	this->InitializeGrid();

	this->InitializeSeeds();
}

//template <class TInputImage, class TOutputImage>
/*typename*/ ConnectedThresholdRegionGrowImageFilter/*<TInputImage, TOutputImage>*/::OutputImageIndexType
ConnectedThresholdRegionGrowImageFilter/*<TInputImage, TOutputImage>*/
::GetOutputIndex ( GridIndexType gridIndex )
{
	OutputImageIndexType outputIndex;

	for ( int i=0; i<InputImageDimension; i++ )
		outputIndex[i] = gridIndex[i] * m_GridNonOverlappedRegionSize[i];

	return outputIndex;
}

//template <class TInputImage, class TOutputImage>
/*typename*/ ConnectedThresholdRegionGrowImageFilter/*<TInputImage, TOutputImage>*/::GridOffsetType
ConnectedThresholdRegionGrowImageFilter/*<TInputImage, TOutputImage>*/
::GetEdgeOffset( OutputImageRegionType region,
							  OutputImageIndexType  index)
{
	GridOffsetType offset;

	OutputImageIndexType origin = region.GetIndex();
	OutputImageSizeType  size   = region.GetSize();

	offset.Fill(0);

	for ( int i=0; i<InputImageDimension; i++)
	{
		if ( index[i] <= origin[i] )
			offset[i] = -1;

		if ( (unsigned int) index[i] >= (origin[i] + size[i] - 1) )
			offset[i] = 1;
	}

	return offset;
}

//template <class TInputImage, class TOutputImage>
/*typename*/ ConnectedThresholdRegionGrowImageFilter/*<TInputImage, TOutputImage>*/::GridIndexType
ConnectedThresholdRegionGrowImageFilter/*<TInputImage, TOutputImage>*/
::ApplyOffset( GridIndexType gridIndex , GridOffsetType offset )
{
	GridIndexType index;

	for ( unsigned int i=0; i<InputImageDimension; i++ )
		index[i] = std::min<int>(m_GridSize[i] - 1, std::max<int>( 0, (gridIndex[i]+offset[i]) ));

	return index;
}

//template <class TInputImage, class TOutputImage>
void
ConnectedThresholdRegionGrowImageFilter/*<TInputImage, TOutputImage>*/
::ApplyConnectedThreshold( GridIndexType gridIndex )
{
	GridPixelType actualCell = m_Grid->GetPixel( gridIndex );

	// Si la celda del grid ya ha sido visitada, salimos sin hacer nada
	if (actualCell.IsVisited())
		return;

	// Inicializamos la imagen de salida
	actualCell.InitializeOutput();

	// Construimos el iterador sobre el grid
	typedef BinaryThresholdImageFunction<InputImageType, double> FunctionType;
	/*typename*/ FunctionType::Pointer function = FunctionType::New();
	function->SetInputImage ( this->GetInput() );
	function->ThresholdBetween ( this->GetLower(), this->GetUpper() );

	OutputImagePointer outputImage = actualCell.GetOutput();
	std::vector<InputImageIndexType> seeds = actualCell.GetSeeds();

	typedef FloodFilledImageFunctionConditionalIterator<OutputImageType, FunctionType> IteratorType;
	IteratorType it (outputImage, function, seeds );

	OutputImageIndexType index;
	GridIndexType        neighborIndex;
	GridOffsetType       offset, nullOffset;
	GridPixelType        neighbor;

	nullOffset.Fill(0);

	it.GoToBegin();

	while( !it.IsAtEnd())
	{
		// Se modifica el valor de salida
		it.Set(this->GetReplaceValue());

		// Añadimos la información de los laterales si es necesario
		index = it.GetIndex();

		offset = this->GetEdgeOffset(actualCell.GetOutputRegion(), index);
		if ( offset != nullOffset )
		{
			neighborIndex = this->ApplyOffset( gridIndex ,offset );
			neighbor = m_Grid->GetPixel( neighborIndex );

			if ( !neighbor.IsVisited() )
			{
				neighbor.AddSeed(index);
				this->AddVisitableNeighbor( neighborIndex );
			}
			else
			{
				if ( !neighbor.Tested(index) )
				{
					// TODO: aplicar el filtro de nuevo en la región pero tan solo a partir de las nuevas semillas
					neighbor.AddSeed(index);
					neighbor.Visitable();
					this->AddVisitableNeighbor( neighborIndex );
				}
			}

			m_Grid->SetPixel( neighborIndex, neighbor );

			this->UpdateBoundingBox( neighborIndex );
		}

		// Continuamos con la iteración
		++it;
	}

	// Marcamos la celda del grid como visitada y ponemos el puntero a la salida del roi
	actualCell.Visited();

	m_Grid->SetPixel( gridIndex, actualCell );
}

//template <class TInputImage, class TOutputImage>
void
ConnectedThresholdRegionGrowImageFilter/*<TInputImage, TOutputImage>*/
::AddVisitableNeighbor( GridIndexType index )
{
	bool found = false;

	for ( unsigned int i=0; i<m_VisitableRegions.size(); i++ )
	{
		if ( m_VisitableRegions[i] == index )
		{
			found = true;
			break;
		}
	}

	if ( !found )
		m_VisitableRegions.push_back( index );
}

//template <class TInputImage, class TOutputImage>
void
ConnectedThresholdRegionGrowImageFilter/*<TInputImage, TOutputImage>*/
::UpdateBoundingBox( GridIndexType index )
{
	for ( unsigned int i=0; i<InputImageDimension; i++ )
	{
		if ( index[i] < m_BoundingBoxIndex[i] )
		{
			m_BoundingBoxSize[i] += m_BoundingBoxIndex[i] - index[i];
			m_BoundingBoxIndex[i] = index[i];
		}
		if ( index[i] >= (unsigned int) (m_BoundingBoxIndex[i] + m_BoundingBoxSize[i]) )
		{
			m_BoundingBoxSize[i] += index[i] - (m_BoundingBoxIndex[i] + m_BoundingBoxSize[i] - 1);
		}
	}
}

//template <class TInputImage, class TOutputImage>
void
ConnectedThresholdRegionGrowImageFilter/*<TInputImage, TOutputImage>*/
::InitializeSeeds()
{
	std::vector<InputImageIndexType> seeds = this->m_SeedList;

	GridIndexType gridIndex;

	// Previamente vaciamos la lista de regiones visitables.
	while (!m_VisitableRegions.empty())
		m_VisitableRegions.pop_back();

	for ( unsigned int i=0; i<this->m_SeedList.size(); i++ )
	{
		if ( this->IsInThreshold( this->GetInput()->GetPixel(this->m_SeedList[i]) ) )
		{
			gridIndex = this->GetGridIndex( this->m_SeedList[i] );
			m_VisitableRegions.push_back( gridIndex );
			m_Grid->GetPixel( gridIndex ).AddSeed(this->m_SeedList[i]);
		}
	}

	m_BoundingBoxIndex = gridIndex;
	m_BoundingBoxSize.Fill( 1 );

}

//template <class TInputImage, class TOutputImage>
void
ConnectedThresholdRegionGrowImageFilter/*<TInputImage, TOutputImage>*/
::InitializeGrid()
{
	// Creamos una nueva imagen, de la misma dimensión que la imagen de entrada, pero cuyo tipo de pixel se corresponde con el descriptor de regiones
	GridRegionType       region;
	GridIndexType        start;
	OutputImageIndexType index;

	// Poneos a 0 el índice del grid
	// de entrada. Para ello hay que tener en cuenta el spacing de la imagen de entrada?
	start.Fill(0);

	// Dividimos la imágen en una celda como mínimo
	for ( unsigned int i=0; i<InputImageDimension; i++ )
		if ( m_GridSize[i] < 1 )
			m_GridSize[i] = 1;

	region.SetSize(m_GridSize);
	region.SetIndex(start);

	m_Grid = GridType::New();
	m_Grid->SetBufferedRegion( region );
	m_Grid->Allocate();

	// Se inicializa el grid.
	// Se crea un iterador sobre el grid y se llama al método Initialize(gridIndex)
	// de cada uno de los pixeles del mismo
	ImageRegionIteratorWithIndex<GridType> it(m_Grid, region);
	GridPixelType pixel;
	OutputImageSizeType outputSize;

	it.GoToBegin();

	while ( !it.IsAtEnd() )
	{
		pixel = it.Get();

		for (unsigned int i=0; i<InputImageDimension; i++ )
			if ( (unsigned int) it.GetIndex()[i] == m_GridSize[i] - 1 )
				outputSize[i] = lastRegionSize[i];
			else
				outputSize[i] = m_GridOverlappedRegionSize[i];

		for (unsigned int i=0; i<InputImageDimension; i++)
			index[i] = it.GetIndex()[i] * m_GridNonOverlappedRegionSize[i];

		pixel.Initialize( index, outputSize );

		it.Set(pixel);

		++it;
	}
}

}

#endif
