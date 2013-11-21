/*******************************************************************************
 *  designer.h
 *******************************************************************************
 *  Copyright (c) 2013 Alexandre Kaspar <alexandre.kaspar@a3.epfl.ch>
 *  For Advanced Computer Graphics, at the LGG / EPFL
 * 
 *        DO NOT REDISTRIBUTE
 ***********************************************/

#ifndef _DESIGNER_H
#define	_DESIGNER_H

#include <nori/object.h>
#include <nori/transform.h>
#include <QWidget>
#include <QtGui>
#include <QScopedPointer>
#include <QVector>
#include "ui_designer.h"

using namespace nori;

class MeshEntry;
class GLSceneViewer;
class MeshEntryDelegate;
class QMenu;
class QAction;

#define REFRESH_INTERVAL 100

class Designer : public QWidget {
    Q_OBJECT
public:
    Designer();
    virtual ~Designer();
    
    friend class GLSceneViewer;
    
private:
    Ui::designer widget;
    QScopedPointer<QTimer> timer;
    QScopedPointer<QMenu> menu;

private:
    QScopedPointer<MeshEntryDelegate> delegate;

private:
    // main actions
    QScopedPointer<QAction> loadAction, saveAction, resetAction;
    // scene list actions
    QScopedPointer<QAction> addMeshAction, removeMeshAction, toggleMeshAction;
    QScopedPointer<QAction> toggleLightAction, setMeshColorAction, resetMeshAction;

private:
    MeshEntry *selectedEntry();
    void clearSelection();
    
signals:
    void killMesh(Mesh *mesh);

private slots:
    // scene actions
    void on_loadButton_clicked();
    void on_saveButton_clicked();
    void on_clearButton_clicked();

    // mesh actions
    void addMesh(const Mesh *mesh = NULL);
    void removeMesh();
    void toggleMesh();
    void toggleLight();
    void setMeshColor();
    void resetMesh();
    // void resetMesh_delayed(Mesh *mesh);
    
    // interactions
    void updateUI(MeshEntry *entry = NULL);
};

#endif	/* _DESIGNER_H */
