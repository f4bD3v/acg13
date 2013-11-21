/*******************************************************************************
 *  designer.cpp
 *******************************************************************************
 *  Copyright (c) 2013 Alexandre Kaspar <alexandre.kaspar@a3.epfl.ch>
 *  For Advanced Computer Graphics, at the LGG / EPFL
 * 
 *        DO NOT REDISTRIBUTE
 ***********************************************/

#include <nori/camera.h>
#include <nori/designer.h>
#include <nori/designer/meshentry.h>
#include <nori/designer/viewer.h>
#include <nori/obj.h>
#include <nori/parser.h>
#include <nori/scene.h>
#include <iostream>
#include <string>
#include <vector>
// qt
#include <QColorDialog>
#include <QFile>
#include <QFileDialog>
#include <QInputDialog>
#include <QLabel>
#include <QListWidgetItem>
#include <QString>
#include <QTextStream>

using namespace nori;

//
// ##### UI Helpers ############################################################
//

enum ItemRole {
    DataEntry = Qt::UserRole,
    DataState = Qt::UserRole + 1
};

void newState(MeshEntry *entry) {
    // we trigger a dummy state change to update the view
    // and since the model changes, we can link to its change event
    int state = entry->item->data(DataState).value<int>();
    entry->item->setData(DataState, state + 1);
}

QColor textColorForMeshColor(const QColor &meshColor) {
    float gray = 0.299 * meshColor.red() + 0.587 * meshColor.green() + 0.114 * meshColor.blue();
    return gray > 186.0f ? QColor(0, 0, 0) : QColor(255, 255, 255);
}

class MeshEntryDelegate : public QStyledItemDelegate {
    QLabel *label;
public:

    MeshEntryDelegate() {
        label = new QLabel();
    }

    ~MeshEntryDelegate() {
        delete label;
    }

    void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const {
        label->resize(option.rect.size());
        MeshEntry *entry = static_cast<MeshEntry *> (index.data(DataEntry).value<void *>());
        // update the label
        if (entry == NULL || !entry->isValid()) return;
        bool selected = entry->selected;
        // style
        QColor back = QColor::fromRgbF(entry->color.r(), entry->color.g(), entry->color.b());
        QColor front = textColorForMeshColor(back);
        label->setAutoFillBackground(true);
        QString css("color: %1; background-color: %2; font-weight: %3; text-decoration: %4;");
        label->setStyleSheet(
                css
                .arg(front.name())
                .arg(back.name())
                .arg(selected ? "bold" : "normal")
                .arg(entry->visible ? "none" : "line-through")
                );
        QString prefix(selected ? "* " : "   ");
        label->setText(prefix % entry->name());

        painter->save();
        painter->translate(option.rect.topLeft());
        label->render(painter);
        painter->restore();
    }
};

//
// ##### UI Constructor ########################################################
//

Designer::Designer() {
    widget.setupUi(this);

    // setup refresh at some interval
    QTimer *intervalTimer = new QTimer(this);
    timer.reset(intervalTimer);
    timer->setInterval(REFRESH_INTERVAL);
    connect(intervalTimer, SIGNAL(timeout()), widget.viewer, SLOT(updateGL()));
    timer->start();

    // main scene actions
    QAction *act = new QAction("Load scene", this);
    act->setShortcut(QKeySequence::Open);
    loadAction.reset(act);
    connect(act, SIGNAL(triggered()), this, SLOT(on_loadButton_clicked()));
    act = new QAction("Save scene", this);
    act->setShortcut(QKeySequence::Save);
    saveAction.reset(act);
    connect(act, SIGNAL(triggered()), this, SLOT(on_saveButton_clicked()));
    act = new QAction("Clear scene", this);
    act->setShortcut(QKeySequence::Delete);
    resetAction.reset(act);
    connect(act, SIGNAL(triggered()), this, SLOT(on_clearButton_clicked()));

    // mesh actions
#define MESH_ACTION(what, str) \
        act = new QAction(str, this); \
        what##Action.reset(act); \
        connect(act, SIGNAL(triggered()), this, SLOT(what())); \
        widget.sceneList->addAction(act)

    // contextual menu
    widget.sceneList->setContextMenuPolicy(Qt::ActionsContextMenu);
    MESH_ACTION(addMesh, "Add new mesh");
    MESH_ACTION(removeMesh, "Remove this mesh");
    MESH_ACTION(toggleMesh, "Hide this mesh");
    MESH_ACTION(toggleLight, "Set as light");
    MESH_ACTION(setMeshColor, "Set color");
    MESH_ACTION(resetMesh, "Reset the mesh transform");

    // mesh selection
    connect(widget.sceneList, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(updateUI()));
    connect(widget.sceneList->model(), SIGNAL(dataChanged(const QModelIndex&, const QModelIndex &)), this, SLOT(updateUI()));
    connect(widget.sceneList, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
            widget.viewer, SLOT(selectionChanged(QListWidgetItem*, QListWidgetItem*)),
            Qt::QueuedConnection);
    widget.sceneList->setSelectionMode(QAbstractItemView::SingleSelection);
    widget.sceneList->setSelectionBehavior(QAbstractItemView::SelectRows);
    delegate.reset(new MeshEntryDelegate());
    widget.sceneList->setItemDelegate(delegate.data());

    // select the mesh (to spread ui setup)
    updateUI();

    // we show it
    show();
}

Designer::~Designer() {
}

//
// ##### Main actions ##########################################################
//

void Designer::on_loadButton_clicked() {
    QString sceneName = QFileDialog::getOpenFileName(this, tr("Load scene"), ".", tr("XML Scene (*.xml);; Any file (*)"));
    std::cout << "Loading scene <" << sceneName.toStdString() << ">\n";
    if (QFile::exists(sceneName)) {
        // try to load the scene
        QScopedPointer<NoriObject> root(loadScene(sceneName));
        if (root->getClassType() != NoriObject::EScene) {
            std::cerr << "Sorry, but this file does not contains a scene!\n";
            return;
        }
        std::cout << "It is a valid scene.\n";
        Scene *s = static_cast<Scene *> (root.data());

        std::cout << "Load transform\n";

        // XXX load the
        // - original scene transform
        widget.viewer->setInitialTransform(s->getCamera()->getTransform().inverse()); // we want the world to camera transform!
        std::cout << "Init Transform = " << s->getCamera()->getTransform().toLineString().toStdString() << "\n";
        // - camera configuration
        widget.fov->setValue(s->getCamera()->getParameters().toFloat());
        widget.viewWidth->setValue(s->getCamera()->getOutputSize().coeff(0));
        widget.viewHeight->setValue(s->getCamera()->getOutputSize().coeff(1));

        std::cout << "Load meshes:\n";
        // - meshes
        const std::vector<Mesh *> &meshList = s->getMeshes();
        int i = 0;
        for (std::vector<Mesh *>::const_iterator it = meshList.begin(); it != meshList.end(); ++it) {
            std::cout << "Mesh #" << ++i << "\n";
            // do something with this mesh
            addMesh(*it);
        }
    }
}

void Designer::on_saveButton_clicked() {
    QString sceneName = QFileDialog::getSaveFileName(this, "Save scene", ".", "XML Scene (*.xml)");

    // save content to scene file with a template!
    QFile file(sceneName);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);
    out << QString("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n\
<!-- Generated with Nori's designer -->\n\
\n\
<scene>\n\
	<integrator type=\"ao\"/>\n\
\n\
	<sampler type=\"independent\">\n\
		<integer name=\"sampleCount\" value=\"32\"/>\n\
	</sampler>\n\
\n\
	<camera type=\"perspective\">\n\
		<transform name=\"toWorld\" value=\"%1\"/>\n\
		<float name=\"fov\" value=\"%2\"/>\n\
		<integer name=\"width\" value=\"%3\"/>\n\
		<integer name=\"height\" value=\"%4\"/>\n\
	</camera>\n\
        \n").arg(widget.viewer->cameraTransform().toLineString())
            .arg(widget.fov->value())
            .arg(widget.viewWidth->value())
            .arg(widget.viewHeight->value());
    const QString identity(Transform().toLineString());
    for(int i = 0, n = widget.sceneList->count(); i < n; ++i){
        QListWidgetItem *item = widget.sceneList->item(i);
        MeshEntry *mesh = static_cast<MeshEntry *>(item->data(DataEntry).value<void *>());
        // color part
        QString color(mesh->light ? "\
		<luminaire type=\"area\">\n\
			<color name=\"radiance\" value=\"%1\"/>\n\
		</luminaire>\n\
		<bsdf type=\"diffuse\">\n\
			<color name=\"albedo\" value=\"0,0,0\"/>\n\
		</bsdf>\n" : "\
		<bsdf type=\"diffuse\">\n\
			<color name=\"albedo\" value=\"%1\"/>\n\
		</bsdf>\n");
        QString rgb = QString("%1, %2, %3").arg(mesh->color.r()).arg(mesh->color.g()).arg(mesh->color.b());
        
        // transform part
        QString transform(mesh->transform.toLineString());
        QString transformLine = transform == identity ? 
            QString("") : QString("		<transform name=\"toWorld\" value=\"%1\"/>\n").arg(transform);
        
        // full mesh part
        out << QString("\n\
	<mesh type=\"obj\">\n\
		<string name=\"filename\" value=\"%1\"/>\n\
%2\
%3\
	</mesh>\n").arg(mesh->name())
                  .arg(transformLine)
                  .arg(color.arg(rgb));
    }
    out << "</scene>\n";
}

void Designer::on_clearButton_clicked() {
    // it'll be awful, let's not listen to events here
    disconnect(widget.sceneList, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
            widget.viewer, SLOT(selectionChanged(QListWidgetItem*, QListWidgetItem*)));

    // clear selection
    clearSelection();
    // clear model
    widget.sceneList->clear();
    // clear the scene content
    widget.viewer->clear();

    // now we can listen again
    connect(widget.sceneList, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
            widget.viewer, SLOT(selectionChanged(QListWidgetItem*, QListWidgetItem*)),
            Qt::QueuedConnection);
}

//
// ##### Mesh actions ##########################################################
//

void Designer::addMesh(const Mesh *mesh) {
    QString meshName;
    bool newFile = mesh == NULL;
    if (newFile) {
        meshName = QFileDialog::getOpenFileName(this, tr("Load mesh"), ".", tr("WaveFront file (*.obj);; Any file (*)"));
        if (QFile::exists(meshName)) {
            mesh = loadOBJFile(meshName);
            if (mesh == NULL) {
                std::cerr << "!!! Problem while loading the mesh...\n";
                return;
            }
        } else {
            return;
        }
        meshName = QFileInfo(meshName).fileName();
    } else {
        meshName = mesh->getName();
    }
    // store the new mesh
    QListWidgetItem *item = new QListWidgetItem(meshName);
    widget.sceneList->addItem(item);
    MeshEntry *entry = new MeshEntry(this, mesh, item);
    if (newFile) {
        // we have to put it in front of the camera
        entry->transform = widget.viewer->initMeshTransform();
    }
    item->setData(DataEntry, QVariant::fromValue((void*) entry));
    item->setData(DataState, QVariant(0));
    widget.viewer->addMesh(entry);
    clearSelection();
}

void Designer::removeMesh() {
    MeshEntry *entry = selectedEntry();
    QListWidgetItem *item = entry->item;
    int row = widget.sceneList->row(item);
    // 1 = remove from model
    item->setData(Qt::UserRole, QVariant::fromValue((void*) NULL));
    QListWidgetItem *dead = widget.sceneList->takeItem(row);
    if (dead != item) throw NoriException("Not the same item removed!");
    clearSelection(); // it shouldn't be selected anymore now
    // 2 = let the viewer clear itself
    entry->remove();
}

void Designer::toggleMesh() {
    MeshEntry *entry = selectedEntry();
    entry->visible = !entry->visible;
    newState(entry);
}

void Designer::toggleLight() {
    MeshEntry *entry = selectedEntry();
    entry->light = !entry->light;
    newState(entry);
}

void Designer::setMeshColor() {
    MeshEntry *entry = selectedEntry();
    const Color3f &cur = entry->color;
    QColor color = QColorDialog::getColor(QColor::fromRgbF(cur.r(), cur.g(), cur.b()), this);
    if (color.isValid()) {
        entry->color = Color3f(color.redF(), color.greenF(), color.blueF());
        newState(entry);
    }
}

void Designer::resetMesh() {
    MeshEntry *entry = selectedEntry();
    if (!entry->hasOriginalTransform()) {
        entry->resetTransform();
    }
    updateUI(entry);
}

//
// ##### Selection #############################################################
//

void Designer::clearSelection() {
    widget.sceneList->clearSelection();
    widget.sceneList->clearFocus();
}

MeshEntry *Designer::selectedEntry() {
    QList<QListWidgetItem *> selection = widget.sceneList->selectedItems();
    if (selection.size() == 0) return NULL;
    if (selection.size() != 1) {
        std::cerr << "!!! Warning: Select only one mesh!\n";
        return NULL;
    }
    QListWidgetItem *item = selection.front();
    return static_cast<MeshEntry*> (item->data(DataEntry).value<void *>());
}

//
// ##### Interactions ##########################################################
//

void Designer::updateUI(MeshEntry* entry) {
    // NULL argument means we must fetch it
    if (!entry) entry = selectedEntry();

    // But is there really no selection?
    bool flag = entry == NULL;
    // mesh actions
    removeMeshAction->setDisabled(flag);
    toggleMeshAction->setDisabled(flag);
    toggleLightAction->setDisabled(flag);
    setMeshColorAction->setDisabled(flag);

    if (entry) {

        // set actions text
        toggleMeshAction->setText(entry->visible ? "Hide this mesh" : "Show this mesh");
        toggleLightAction->setText(entry->light ? "Set as mesh" : "Set as light");
        // resetMeshAction->setDisabled(entry->hasOriginalTransform()); XXX yes but how to update from GLSceneViewer interactions?
    }
}