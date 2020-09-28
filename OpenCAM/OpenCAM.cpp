#include "OpenCAM.h"
#include "stdafx.h"

#include "Import.h"
#include <OpenGl_GraphicDriver.hxx>
#include "View.h"

#include <TopoDS.hxx>
#include <TopoDS_Iterator.hxx>
#include <TopoDS_TEdge.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Vertex.hxx>
#include <BRep_TEdge.hxx>
#include <BRep_Tool.hxx>
#include <Poly_Polygon3D.hxx>
#include <Poly_PolygonOnTriangulation.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>

#include "OperationProfile.h"

#include <BRep_TVertex.hxx>
#include "ShapeSolid.h"
#include "ShapeFace.h"
#include "ShapeEdge.h"

 
void dumpShape(const TopoDS_Shape & shape, int indent)
{
    static const char* ShapeType[] = {
       "TopAbs_COMPOUND",
       "TopAbs_COMPSOLID",
       "TopAbs_SOLID",
       "TopAbs_SHELL",
       "TopAbs_FACE",
       "TopAbs_WIRE",
       "TopAbs_EDGE",
       "TopAbs_VERTEX",
       "TopAbs_SHAPE"
    };
    qDebug() << qPrintable(QString(indent*2,' ')) << ShapeType[shape.ShapeType()] << " NbChildren:" << shape.NbChildren();
    Handle(TopoDS_TShape) tshape = shape.TShape();
    if (shape.ShapeType() == TopAbs_VERTEX)
    {
        Handle(BRep_TVertex) tVertex = Handle(BRep_TVertex)::DownCast(tshape);
        qDebug() << qPrintable(QString(indent * 2, ' ')) << tVertex->Pnt().X() << "," << tVertex->Pnt().Y() << "," << tVertex->Pnt().Z();
    }
    else if (shape.ShapeType() == TopAbs_EDGE)
    {
        Handle(BRep_TEdge) tEdge = Handle(BRep_TEdge)::DownCast(tshape);
        qDebug() << qPrintable(QString(indent * 2, ' ')) << " Curves:" << tEdge->Curves().Size();
        for (auto it = tEdge->Curves().cbegin(); it != tEdge->Curves().cend(); it++)
        {
            Handle(BRep_CurveRepresentation) c = *it;
            qDebug() << qPrintable(QString(indent * 2, ' '))
                << " IsCurve3D:" << c->IsCurve3D()
                << " IsCurveOnSurface:" << c->IsCurveOnSurface()
                << " IsCurveOnClosedSurface:" << c->IsCurveOnClosedSurface()
                << " IsPolygon3D:" << c->IsPolygon3D()
                << " IsPolygonOnSurface:" << c->IsPolygonOnSurface()
                << " IsPolygonOnTriangulation:" << c->IsPolygonOnTriangulation()
                << " IsPolygonOnClosedTriangulation:" << c->IsPolygonOnClosedTriangulation()
                << " IsPolygonOnClosedSurface:" << c->IsPolygonOnClosedSurface()
                << " IsRegularity:" << c->IsRegularity();

            if (c->IsCurve3D())
            {
                Handle(Geom_Curve) curve3d = c->Curve3D();
                qDebug() << qPrintable(QString(indent * 2, ' ')) << "Curve isClosed:" << curve3d->IsClosed() << " isPeriodic:" << curve3d->IsPeriodic() << " First:" << curve3d->FirstParameter() << " Last:" << curve3d->LastParameter();
            }
            if (c->IsCurveOnSurface() || c->IsCurveOnClosedSurface())
            {
                Handle(Geom_Surface) surface = c->Surface();
            }
            //Handle(Geom2d_Curve) pcurve = c->PCurve();
            //Handle(Geom2d_Curve) curve = c->PCurve2();
            //Handle(Poly_Polygon3D) polygon3d = c->Polygon3D();
            //Handle(Poly_Polygon2D) polygon = c->Polygon();
            //Handle(Poly_Polygon2D) polygon2 = c->Polygon2();
            //Handle(Poly_Triangulation) triang = c->Triangulation();
            //Handle(Poly_PolygonOnTriangulation) pont = c->PolygonOnTriangulation();
            //Handle(Poly_PolygonOnTriangulation) pont2 = c->PolygonOnTriangulation2();
            //Handle(Geom_Surface) surface2 = c->Surface2();

        }
    }
    if (shape.NbChildren() > 0)
    {
        TopoDS_Iterator children;
        for (children.Initialize(shape); children.More(); children.Next())
        {
            dumpShape(children.Value(), indent+1);
        }
    }
}

void dumpShape(Handle(AIS_Shape) shape)
{
    const TopoDS_Shape& s = shape->Shape();
    dumpShape(s,0);
}

OpenCAM::OpenCAM(QWidget *parent) :
    QMainWindow(parent),
    partView(new PartView(this))
{
    setCentralWidget(partView);

    createActions();
    createStatusBar();
    createDockWindows();

    setWindowTitle(tr("OpenCAM"));

    TCollection_ExtendedString a3DName("Visu3D");

    myViewer = Viewer(a3DName.ToExtString(), "", 1000.0, V3d_XposYnegZpos, Standard_True, Standard_True);

    myViewer->SetDefaultLights();
    myViewer->SetLightOn();
   
    myContext = new AIS_InteractiveContext(myViewer);

    myView = new View(myContext, this);
    this->setCentralWidget(myView);
    connect(myView, &View::selectionChanged, this, &OpenCAM::onViewSelectionChanged);
    
    QToolBar* viewToolBar = addToolBar(tr("View"));
    viewToolBar->addActions( *(myView->getViewActions()) );

    QToolBar* operationsToolBar = addToolBar(tr("Operations"));
    QAction *a = new QAction("Profile");
    connect(a, SIGNAL(triggered()), this, SLOT(operationProfile()));
    operationsToolBar->addAction(a);

    readSettings();

    {
        Import anTrans = new Import(this/*, "Translator"*/);
        QString filename("d:\\GCode\\!10-20-30 block.stp");
        Handle(TopTools_HSequenceOfShape) shapes = anTrans.importModel(Import::FormatSTEP, filename);
        QFileInfo fi(filename);
        AddShapes(shapes, fi.baseName());
        for (int i = 1; i <= shapes->Length(); i++)
        {
            Handle(AIS_Shape) shape = new AIS_Shape(shapes->Value(i));
            dumpShape(shape);
            myContext->Display(shape, AIS_Shaded, 1, false);
            //for (int i = 0; i <= 6; i++)
            myContext->SetSelectionModeActive(shape, 1, true);  // vertex
            myContext->SetSelectionModeActive(shape, 2, true);  // edge
            //myContext->SetSelectionModeActive(shape, 3, true);  // wire
            myContext->SetSelectionModeActive(shape, 4, true);  // face
        }
        myContext->UpdateCurrentViewer();
        myView->fitAll();
    }
}



void OpenCAM::closeEvent(QCloseEvent* event)
{
    writeSettings();
}

void OpenCAM::writeSettings()
{
    QSettings settings;

    settings.beginGroup("MainWindow");
        settings.setValue("geometry", saveGeometry());
    settings.endGroup();
}

void OpenCAM::readSettings()
{
    QSettings settings;

    settings.beginGroup("MainWindow");
        const QByteArray geometry = settings.value("geometry", QByteArray()).toByteArray();
        if (geometry.isEmpty()) 
        {
            const QRect availableGeometry = screen()->availableGeometry();
            resize(availableGeometry.width() / 3, availableGeometry.height() / 2);
            move((availableGeometry.width() - width()) / 2, (availableGeometry.height() - height()) / 2);
        }
        else 
        {
            restoreGeometry(geometry);
        }   
    settings.endGroup();
}

void OpenCAM::operationFinished(int result)
{
    dlgOperation.clear();
}

void OpenCAM::operationProfile()
{
    dlgOperation = new OperationProfile(this, myContext );
    dlgOperation->setModal(true);
    dlgOperation->show();
    dlgOperation->raise();
    connect(dlgOperation, SIGNAL(finished(int)), this, SLOT(operationFinished(int)));
    dlgOperation->activateWindow();
    onViewSelectionChanged();


    // Tool table
    // Feeds & Speeds table (plus override)
    // Modeless dialog so we can always select things, change view, or...
    // Modal, but then switch to modeless when we need to select things
    // If we are modeless, we need to ensure we don't go off and do other things

    /* Profile
        - VisualMill
            - Machining Features/Regions
            - Tool
            - Feeds & Speeds
            - Clearance
            - Cut Parameters
                - pattern, stepover, direction, stock to leave
            - Cut Levels
                - top/bottom
                - step down
                - rough/finish
            - Entry/Exit
                - approach for first cut.
                - approach between levels
                - 2d/3d, plunge, ramp, spiral
            - Advanced Cut Parameters
                - bridges/tabs
            - Sorting
        - Fusion360
            - Tools
            - geometry
            - heights
            - passes
            - linking
    */
    
}


void OpenCAM::onViewSelectionChanged()
{
    qDebug() << "onViewSelectionChanged";
    QList< QSharedPointer<Shape> > selections;
    for (myContext->InitSelected(); myContext->MoreSelected(); myContext->NextSelected())
    {
        const TopoDS_Shape& aSelShape = myContext->SelectedShape();
        QSharedPointer<Shape> shape = FindShape(aSelShape);
        if (shape.isNull())
        {
            qDebug() << "Can't find shape";
        }
        else
        {
            qDebug() << "Selected shape " << shape->name;
            selections.append(shape);
        }
    }

    if (dlgOperation)
    {
        dlgOperation->onShapeSelectionChanged( selections );
    }
    else
    {
        for (myContext->InitSelected(); myContext->MoreSelected(); myContext->NextSelected())
        {
            //Handle(AIS_InteractiveObject) s = myContext->SelectedInteractive();

            const TopoDS_Shape& aSelShape = myContext->SelectedShape();
            qDebug() << "    ShapeType:" << aSelShape.ShapeType();

            TopExp_Explorer explorer;
            Standard_Integer nbEdges = 0;
            Standard_Integer nbNonEdges = 0;
            for (explorer.Init(aSelShape, TopAbs_EDGE); explorer.More(); explorer.Next())
            {
                const TopoDS_Edge& edge = TopoDS::Edge(explorer.Current());
                TopoDS_Vertex v1, v2;
                TopExp::Vertices(edge, v1, v2);
                gp_Pnt p1 = BRep_Tool::Pnt(v1);
                gp_Pnt p2 = BRep_Tool::Pnt(v2);
                nbEdges++;
                nbNonEdges++;
            }

            //if (aSelShape.ShapeType() == TopAbs_EDGE)
            //{
            //    Handle(TopoDS_TShape) tShape = aSelShape.TShape();
            //    Handle(TopoDS_TEdge) tEdge = Handle(TopoDS_TEdge)::DownCast(tShape);
            //    Handle(BRep_TEdge) brptEdge = Handle(BRep_TEdge)::DownCast(tEdge);

            //    TopExp_Explorer explorer;
            //    Standard_Integer nbEdges = 0;
            //    Standard_Integer nbNonEdges = 0;
            //    for (explorer.Init(aSelShape, TopAbs_VERTEX); explorer.More(); explorer.Next())
            //    {
            //        const TopoDS_Vertex& currentEdge = TopoDS::Vertex(explorer.Current());
            //        nbEdges++;
            //        nbNonEdges++;
            //    }
            //    
            //    for (explorer.Init(aSelShape, TopAbs_WIRE); explorer.More(); explorer.Next())
            //    {
            //        const TopoDS_Wire& currentEdge = TopoDS::Wire(explorer.Current());
            //        nbEdges++;
            //        nbNonEdges++;
            //    }

            //    const BRep_ListOfCurveRepresentation& c = brptEdge->Curves();
            //    int size = c.Size();
            //    for (auto it = c.cbegin(); it!=c.cend(); it++)
            //    {
            //        Handle(const BRep_CurveRepresentation) cr = *it;
            //        bool b;
            //        b = cr->IsCurve3D();
            //        b = cr->IsCurveOnSurface();
            //        b = cr->IsCurveOnClosedSurface();
            //        b = cr->IsRegularity();
            //        b = cr->IsPolygon3D();
            //        b = cr->IsPolygonOnTriangulation();
            //        b = cr->IsPolygonOnClosedTriangulation();
            //        b = cr->IsPolygonOnSurface();
            //        b = cr->IsPolygonOnClosedSurface();
            //        if (cr->IsCurve3D())
            //        {
            //            gp_Pnt d0, d1;
            //        }
            //        if (cr->IsPolygon3D())
            //        {
            //            int n = cr->Polygon3D()->NbNodes();
            //            bool b = cr->Polygon3D()->HasParameters();
            //        }
            //        if (cr->IsPolygonOnTriangulation())
            //        {
            //            int n = cr->PolygonOnTriangulation()->NbNodes();
            //            for (int i = 0; i < n; i++)
            //            {
            //                cr->PolygonOnTriangulation()->Nodes();
            //            }
            //            bool b = cr->PolygonOnTriangulation()->HasParameters();
            //            n = 0;
            //        }
            //    }
            //
            //}
            //qDebug() << "    Kind:" << s->Type() << " Signature:" << s->Signature() << " Type:" << typeid(s).name();
            //std::stringstream str;
            //s->DumpJson(str);
            //qDebug() << "    " << str.str().c_str();
            //qDebug() << "    " << s->DynamicType()->Name();

            //Handle(AIS_Shape) shape = Handle(AIS_Shape)::DownCast(s);
            //qDebug() << "    " << shape->Shape().ShapeType();
            //if (shape->Shape().ShapeType() == TopAbs_COMPOUND)
            //{
            //    TopoDS_Iterator it;
            //    for (it.Initialize(shape->Shape()); it.More(); it.Next())
            //    {
            //        qDebug() << "        " << it.Value().ShapeType();
            //    }
            //}

        }
    }
}


Handle(V3d_Viewer) OpenCAM::Viewer(const Standard_ExtString,
    const Standard_CString,
    const Standard_Real theViewSize,
    const V3d_TypeOfOrientation theViewProj,
    const Standard_Boolean theComputedMode,
    const Standard_Boolean theDefaultComputedMode)
{
    static Handle(OpenGl_GraphicDriver) aGraphicDriver;

    if (aGraphicDriver.IsNull())
    {
        Handle(Aspect_DisplayConnection) aDisplayConnection;
#if !defined(_WIN32) && !defined(__WIN32__) && (!defined(__APPLE__) || defined(MACOSX_USE_GLX))
        aDisplayConnection = new Aspect_DisplayConnection(OSD_Environment("DISPLAY").Value());
#endif
        aGraphicDriver = new OpenGl_GraphicDriver(aDisplayConnection);
    }

    Handle(V3d_Viewer) aViewer = new V3d_Viewer(aGraphicDriver);
    aViewer->SetDefaultViewSize(theViewSize);
    aViewer->SetDefaultViewProj(theViewProj);
    aViewer->SetComputedMode(theComputedMode);
    aViewer->SetDefaultComputedMode(theDefaultComputedMode);
    return aViewer;
}


void OpenCAM::createActions()
{
    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
    QToolBar* fileToolBar = addToolBar(tr("File"));

    const QIcon newIcon = QIcon::fromTheme("document-new", QIcon(":/images/new.png"));
    QAction* newProjectAct = new QAction(newIcon, tr("&New"), this);
    newProjectAct->setShortcuts(QKeySequence::New);
    newProjectAct->setStatusTip(tr("Create a new CAM project"));
    connect(newProjectAct, &QAction::triggered, this, &OpenCAM::newProject);
    fileMenu->addAction(newProjectAct);
    fileToolBar->addAction(newProjectAct);

    const QIcon importIcon = QIcon::fromTheme("document-new", QIcon(":/images/new.png"));
    QAction* importAct = new QAction(newIcon, tr("&Import"), this);
    //importAct->setShortcuts(QKeySequence::);
    importAct->setStatusTip(tr("Create a new form letter"));
    connect(importAct, &QAction::triggered, this, &OpenCAM::import);
    fileMenu->addAction(importAct);
    fileToolBar->addAction(importAct);

    fileMenu->addSeparator();

    QAction* quitAct = fileMenu->addAction(tr("&Quit"), this, &QWidget::close);
    quitAct->setShortcuts(QKeySequence::Quit);
    quitAct->setStatusTip(tr("Quit the application"));

    viewMenu = menuBar()->addMenu(tr("&View"));

    menuBar()->addSeparator();

    QMenu* helpMenu = menuBar()->addMenu(tr("&Help"));

    QAction* aboutAct = helpMenu->addAction(tr("&About"), this, &OpenCAM::about);
    aboutAct->setStatusTip(tr("Show the application's About box"));

    QAction* aboutQtAct = helpMenu->addAction(tr("About &Qt"), qApp, &QApplication::aboutQt);
    aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
}

void OpenCAM::newProject()
{
}

void OpenCAM::import()
{
    static Import* anTrans = new Import(this/*, "Translator"*/);

    Handle(TopTools_HSequenceOfShape) shapes = anTrans->importModel();

    if (shapes.IsNull() || !shapes->Length())
    {
        QString msg = QObject::tr("Error translating data file");
        if (!anTrans->info().isEmpty())
            msg += QString("\n") + anTrans->info();
        QMessageBox::critical(this, QObject::tr("Error"), msg, QObject::tr("OK"), QString::null, QString::null, 0, 0);
    }

    for (int i = 1; i <= shapes->Length(); i++)
    {
        Handle(AIS_Shape) shape = new AIS_Shape(shapes->Value(i));
        dumpShape(shape);
        myContext->Display(shape, AIS_Shaded, 0, false);
        for ( int i = 0; i <= 6; i++ )
            myContext->SetSelectionModeActive(shape, i, true);
    }
    myContext->UpdateCurrentViewer();
    myView->fitAll();
}

void OpenCAM::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}



void OpenCAM::createDockWindows()
{
    // Default dock positions are to the left
    QDockWidget* dockOperations = new QDockWidget(tr("Operations"), this);
    dockOperations->setFeatures(QDockWidget::AllDockWidgetFeatures);
//    dockOperations->setWidget(childWidget);
    addDockWidget(Qt::LeftDockWidgetArea, dockOperations);
    viewMenu->addAction(dockOperations->toggleViewAction());

    QDockWidget* dockToolLibrary = new QDockWidget(tr("Tools"), this);
    dockToolLibrary->setFeatures(QDockWidget::AllDockWidgetFeatures);
    //    dockToolLibrary->setWidget(childWidget);
    addDockWidget(Qt::LeftDockWidgetArea, dockToolLibrary);
    viewMenu->addAction(dockToolLibrary->toggleViewAction());

    //connect(customerList, &QListWidget::currentTextChanged,
    //    this, &OpenCAM::insertCustomer);
    //connect(paragraphsList, &QListWidget::currentTextChanged,
    //    this, &OpenCAM::addParagraph);

    tabifyDockWidget(dockOperations, dockToolLibrary);
}


void OpenCAM::about()
{
    QMessageBox::about(this, tr("About OpenCAM"),
        tr("CAM program built with opencascade."));
}


void OpenCAM::AddShape(const TopoDS_Shape& tshape, QString shapeName, int index, QSharedPointer<Shape> parent)
{
    QSharedPointer<Shape> shape;
    bool addChildren = false;
    QString name;
    switch (tshape.ShapeType())
    {
        case TopAbs_COMPOUND:
            // multiple solids, add them separately
        {
            int i = 0;
            TopoDS_Iterator children;
            for (children.Initialize(tshape); children.More(); children.Next(), i++)
            {
                const TopoDS_Shape& c = children.Value();
                AddShape(c, shapeName, i+1, nullptr);
            }
            break;
        }

        case TopAbs_COMPSOLID:
            Q_ASSERT_X(false, "AddShape", "Don't know what a COMPSOLID is");    // don't know what this is.
            break;

        case TopAbs_SOLID:
        {
            // new solid, add sub shapes
            Q_ASSERT(tshape.NbChildren() == 1);
            Q_ASSERT(parent.isNull());
            shape.reset(new ShapeSolid(tshape, shapeName + QString(".Solid%1").arg(index)));
            shapes.append(shape);
            parent = shape;
            addChildren = true;
            break;
        }
        case TopAbs_SHELL:
            // don't need shell, just sub shapes
            addChildren = true;
            break;
        case TopAbs_FACE:
        {
            // add face, 
            Q_ASSERT(tshape.NbChildren() == 1); // should be 1 wire child
            shape.reset(new ShapeFace(tshape, shapeName + QString(".Face%1").arg(index)));
            parent->Add(shape);
            parent = shape;
            addChildren = true;
            break;
        }
        case TopAbs_WIRE:
            // don't need wire, just sub shapes
            addChildren = true;
            break;
        case TopAbs_EDGE:
        {
            // add edges
            shape.reset(new ShapeEdge(tshape, shapeName + QString(".Edge%1").arg(index)));
            parent->Add(shape);
            parent = shape;
            addChildren = true;
            break;
        }
        case TopAbs_VERTEX:
            // ignore vertex, we can get them later when needed
            break;
        case TopAbs_SHAPE:
            Q_ASSERT_X(false, "AddShape", "Don't know how to handle TopAbs_SHAPE");
            break;
    }

    if (addChildren)
    {
        TopoDS_Iterator children;
        int i = 0;
        for (children.Initialize(tshape); children.More(); children.Next(), i++)
        {
            const TopoDS_Shape& c = children.Value();
            AddShape(c, parent->name, i+1, parent);
        }
    }

    // Iterate through add shape and iterate through sub shapes
    // TopoDS_TShape is unique for each shape instance.

}

void OpenCAM::AddShapes(Handle(TopTools_HSequenceOfShape) shapes, QString shapesName)
{
    // Iterate through add shape and iterate through sub shapes
    // TopoDS_TShape is unique for each shape instance.
    for (int i = 1; i <= shapes->Length(); i++)
    {
        Handle(AIS_Shape) shape = new AIS_Shape(shapes->Value(i));
        const TopoDS_Shape& s = shape->Shape();
        QString name;
        if (shapes->Length() == 1)
            name = shapesName;
        else
            name = QString("%1:%2").arg(shapesName).arg(i);
        AddShape(s, name, i, nullptr);
    }
    
    DumpShapes();
}



void OpenCAM::DumpShapes()
{
    for (auto it = shapes.cbegin(); it != shapes.cend(); it++)
    {
        DumpShape(*it);
    }

}

void OpenCAM::DumpShape(QSharedPointer<Shape> shape )
{
    qDebug() << shape << " " << shape->name;

    for (auto it = shape->children.cbegin(); it != shape->children.cend(); it++)
    {
        DumpShape(*it);
    }

}

QSharedPointer<Shape> OpenCAM::FindShape(const TopoDS_Shape& aSelShape)
{
    for (auto it = shapes.cbegin(); it != shapes.cend(); it++)
    {
        QSharedPointer<Shape> shape = *it;
        if (shape->tshape.IsEqual(aSelShape))
            return shape;

        QSharedPointer<Shape> match= shape->FindShape(aSelShape);
        if (!match.isNull())
            return match;
    }
    return QSharedPointer<Shape>();
}
 