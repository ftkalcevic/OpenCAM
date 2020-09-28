#include "ShapeSolid.h"


ShapeSolid::ShapeSolid(const TopoDS_Shape& tshape, QString name) :
	Shape(tshape, name)
{
}

ShapeSolid::~ShapeSolid()
{
}
