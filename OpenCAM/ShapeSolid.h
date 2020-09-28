#pragma once

#include "Shape.h"

class ShapeSolid :
    public Shape
{
    Q_OBJECT
public:
    ShapeSolid(const TopoDS_Shape& tshape, QString name);
    virtual ~ShapeSolid();
};

