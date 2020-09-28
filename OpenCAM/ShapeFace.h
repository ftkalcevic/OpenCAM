#pragma once

#include "Shape.h"

class ShapeFace :
    public Shape
{
    Q_OBJECT
public:
    ShapeFace(const TopoDS_Shape& tshape, QString name);
    virtual ~ShapeFace();
};

