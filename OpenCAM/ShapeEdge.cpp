#include "ShapeEdge.h"


ShapeEdge::ShapeEdge(const TopoDS_Shape& tshape, QString name) :
	Shape(tshape, name)
{
}

ShapeEdge::~ShapeEdge()
{
}
