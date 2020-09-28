#pragma once
#include "Shape.h"
class ShapeEdge :
    public Shape
{
    Q_OBJECT
public:
    ShapeEdge(const TopoDS_Shape& tshape, QString name);
    virtual ~ShapeEdge();
};

