#pragma once

#include <QtWidgets/QMainWindow>
#include "PartView.h"
#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>
#include <AIS_InteractiveObject.hxx>
#include <V3d_Viewer.hxx>
#include <TopoDS.hxx>
#include <TopTools_HSequenceOfShape.hxx>

#include "View.h"
#include "Operation.h"
#include "Shape.h"

class OpenCAM : public QMainWindow
{
    Q_OBJECT

public:
    OpenCAM(QWidget *parent = Q_NULLPTR);
    void about();
    void newProject();
    void import();

private:
    void createActions();
    void createStatusBar();
    void createDockWindows();
    void AddShape(const TopoDS_Shape& tshape, QString shapeName, int index, QSharedPointer<Shape> parent);
    void AddShapes(Handle(TopTools_HSequenceOfShape) shapes, QString shapesName);
    void DumpShapes();
    void DumpShape(QSharedPointer<Shape> shape);
    QSharedPointer<Shape> FindShape(const TopoDS_Shape& aSelShape);

    void writeSettings();
    void readSettings();
    virtual void closeEvent(QCloseEvent* event);

    QMenu* viewMenu;
    PartView* partView;
    QPointer<Operation> dlgOperation;

    // 3d view
    Handle(V3d_Viewer) Viewer(const Standard_ExtString theName,
                              const Standard_CString theDomain,
                              const Standard_Real theViewSize,
                              const V3d_TypeOfOrientation theViewProj,
                              const Standard_Boolean theComputedMode,
                              const Standard_Boolean theDefaultComputedMode);

    //ApplicationCommonWindow* myApp;
    //QList<MDIWindow*>              myViews;
    Handle(V3d_Viewer)             myViewer;
    Handle(AIS_InteractiveContext) myContext;
    int                            myIndex;
    int                            myNbViews;
    View *                         myView;
    QList<QSharedPointer<Shape>>  shapes;

public slots:
    void onViewSelectionChanged();
    void operationProfile();
    void operationFinished(int result);
};
