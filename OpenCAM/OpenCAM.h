#pragma once

#include <QtWidgets/QMainWindow>
#include "PartView.h"
#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>
#include <AIS_InteractiveObject.hxx>
#include <V3d_Viewer.hxx>
#include "View.h"

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

    QMenu* viewMenu;
    PartView* partView;

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

public slots:
    void onViewSelectionChanged();
};
