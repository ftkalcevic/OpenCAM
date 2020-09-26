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
#include <BRep_TEdge.hxx>
#include <Poly_Polygon3D.hxx>
#include <Poly_PolygonOnTriangulation.hxx>
#include <TopExp_Explorer.hxx>

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
    

    resize(sizeHint());
}

void OpenCAM::onViewSelectionChanged()
{
    qDebug() << "onViewSelectionChanged";
    for (myContext->InitSelected(); myContext->MoreSelected(); myContext->NextSelected())
    {
        //Handle(AIS_InteractiveObject) s = myContext->SelectedInteractive();

        const TopoDS_Shape& aSelShape = myContext->SelectedShape();
        qDebug() << "    ShapeType:" << aSelShape.ShapeType();

        if (aSelShape.ShapeType() == TopAbs_EDGE)
        {
            Handle(TopoDS_TShape) tShape = aSelShape.TShape();
            Handle(TopoDS_TEdge) tEdge = Handle(TopoDS_TEdge)::DownCast(tShape);
            Handle(BRep_TEdge) brptEdge = Handle(BRep_TEdge)::DownCast(tEdge);

            TopExp_Explorer explorer;
            Standard_Integer nbEdges = 0;
            Standard_Integer nbNonEdges = 0;
            for (explorer.Init(aSelShape, TopAbs_VERTEX); explorer.More(); explorer.Next())
            {
                const TopoDS_Vertex& currentEdge = TopoDS::Vertex(explorer.Current());
                nbEdges++;
                nbNonEdges++;
            }
            
            for (explorer.Init(aSelShape, TopAbs_WIRE); explorer.More(); explorer.Next())
            {
                const TopoDS_Wire& currentEdge = TopoDS::Wire(explorer.Current());
                nbEdges++;
                nbNonEdges++;
            }

            const BRep_ListOfCurveRepresentation& c = brptEdge->Curves();
            int size = c.Size();
            for (auto it = c.cbegin(); it!=c.cend(); it++)
            {
                Handle(const BRep_CurveRepresentation) cr = *it;
                bool b;
                b = cr->IsCurve3D();
                b = cr->IsCurveOnSurface();
                b = cr->IsCurveOnClosedSurface();
                b = cr->IsRegularity();
                b = cr->IsPolygon3D();
                b = cr->IsPolygonOnTriangulation();
                b = cr->IsPolygonOnClosedTriangulation();
                b = cr->IsPolygonOnSurface();
                b = cr->IsPolygonOnClosedSurface();
                if (cr->IsCurve3D())
                {
                    gp_Pnt d0, d1;
                }
                if (cr->IsPolygon3D())
                {
                    int n = cr->Polygon3D()->NbNodes();
                    bool b = cr->Polygon3D()->HasParameters();
                }
                if (cr->IsPolygonOnTriangulation())
                {
                    int n = cr->PolygonOnTriangulation()->NbNodes();
                    for (int i = 0; i < n; i++)
                    {
                        cr->PolygonOnTriangulation()->Nodes();
                    }
                    bool b = cr->PolygonOnTriangulation()->HasParameters();
                    n = 0;
                }
            }

        }
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
