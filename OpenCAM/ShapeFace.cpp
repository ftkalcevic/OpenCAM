#include "ShapeFace.h"


ShapeFace::ShapeFace(const TopoDS_Shape& tshape, QString name) :
	Shape(tshape, name)
{
}

ShapeFace::~ShapeFace()
{
}