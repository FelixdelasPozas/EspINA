#ifndef _RENDERER_H_
#define _RENDERER_H_

enum Rep3D 
{
	POINTS = 0
	, SURFACE = 2
	, MESH = 2
	, OUTLINE = 3
	, VOLUME = 4
	, SLICE = 6
	, HIDEN = 100
};

class Renderer
{
public:
	Renderer(){}
	virtual ~Renderer() = 0;

	virtual void render(Segmentation *, double r, double g, double b) = 0;
};

class MeshRenderer : public Renderer
{
public:
	MeshRenderer() : Renderer(){}
	~MeshRenderer(){}

	virtual void render(Segmentation *, double r, double g, double b) = 0;
};

class VolumeRenderer : public Renderer
{
public:
	VolumeRenderer() : Renderer(){}
	~VolumeRenderer(){}

	virtual void render(Segmentation *, double r, double g, double b) = 0;
};

#endif// _RENDERER_H_
