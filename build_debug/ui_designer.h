/********************************************************************************
** Form generated from reading UI file 'designer.ui'
**
** Created by: Qt User Interface Compiler version 4.8.5
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DESIGNER_H
#define UI_DESIGNER_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDial>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QListWidget>
#include <QtGui/QPushButton>
#include <QtGui/QSpinBox>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>
#include "nori/designer/viewer.h"

QT_BEGIN_NAMESPACE

class Ui_designer
{
public:
    QHBoxLayout *horizontalLayout;
    GLSceneViewer *viewer;
    QWidget *widget;
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBox_1;
    QHBoxLayout *horizontalLayout_2;
    QPushButton *loadButton;
    QPushButton *saveButton;
    QPushButton *clearButton;
    QGroupBox *groupBox_2;
    QHBoxLayout *horizontalLayout_3;
    QListWidget *sceneList;
    QGroupBox *groupBox_3;
    QHBoxLayout *horizontalLayout_6;
    QDial *fov;
    QVBoxLayout *verticalLayout_3;
    QSpinBox *viewWidth;
    QSpinBox *viewHeight;

    void setupUi(QWidget *designer)
    {
        if (designer->objectName().isEmpty())
            designer->setObjectName(QString::fromUtf8("designer"));
        designer->resize(624, 487);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(designer->sizePolicy().hasHeightForWidth());
        designer->setSizePolicy(sizePolicy);
        horizontalLayout = new QHBoxLayout(designer);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        viewer = new GLSceneViewer(designer);
        viewer->setObjectName(QString::fromUtf8("viewer"));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(1);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(viewer->sizePolicy().hasHeightForWidth());
        viewer->setSizePolicy(sizePolicy1);
        viewer->setMinimumSize(QSize(300, 300));

        horizontalLayout->addWidget(viewer);

        widget = new QWidget(designer);
        widget->setObjectName(QString::fromUtf8("widget"));
        QSizePolicy sizePolicy2(QSizePolicy::Minimum, QSizePolicy::Preferred);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(widget->sizePolicy().hasHeightForWidth());
        widget->setSizePolicy(sizePolicy2);
        widget->setMinimumSize(QSize(300, 0));
        widget->setMaximumSize(QSize(300, 16777215));
        verticalLayout = new QVBoxLayout(widget);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        groupBox_1 = new QGroupBox(widget);
        groupBox_1->setObjectName(QString::fromUtf8("groupBox_1"));
        horizontalLayout_2 = new QHBoxLayout(groupBox_1);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        loadButton = new QPushButton(groupBox_1);
        loadButton->setObjectName(QString::fromUtf8("loadButton"));

        horizontalLayout_2->addWidget(loadButton);

        saveButton = new QPushButton(groupBox_1);
        saveButton->setObjectName(QString::fromUtf8("saveButton"));

        horizontalLayout_2->addWidget(saveButton);

        clearButton = new QPushButton(groupBox_1);
        clearButton->setObjectName(QString::fromUtf8("clearButton"));

        horizontalLayout_2->addWidget(clearButton);


        verticalLayout->addWidget(groupBox_1);

        groupBox_2 = new QGroupBox(widget);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        QSizePolicy sizePolicy3(QSizePolicy::Preferred, QSizePolicy::Expanding);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(groupBox_2->sizePolicy().hasHeightForWidth());
        groupBox_2->setSizePolicy(sizePolicy3);
        horizontalLayout_3 = new QHBoxLayout(groupBox_2);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        sceneList = new QListWidget(groupBox_2);
        sceneList->setObjectName(QString::fromUtf8("sceneList"));
        sceneList->setMinimumSize(QSize(0, 200));
        sceneList->setMaximumSize(QSize(300, 10000000));

        horizontalLayout_3->addWidget(sceneList);


        verticalLayout->addWidget(groupBox_2);

        groupBox_3 = new QGroupBox(widget);
        groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
        QSizePolicy sizePolicy4(QSizePolicy::Expanding, QSizePolicy::Minimum);
        sizePolicy4.setHorizontalStretch(0);
        sizePolicy4.setVerticalStretch(0);
        sizePolicy4.setHeightForWidth(groupBox_3->sizePolicy().hasHeightForWidth());
        groupBox_3->setSizePolicy(sizePolicy4);
        horizontalLayout_6 = new QHBoxLayout(groupBox_3);
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        fov = new QDial(groupBox_3);
        fov->setObjectName(QString::fromUtf8("fov"));
        fov->setMinimum(1);
        fov->setMaximum(360);
        fov->setValue(25);
        fov->setWrapping(true);
        fov->setNotchTarget(5);
        fov->setNotchesVisible(true);

        horizontalLayout_6->addWidget(fov);

        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        viewWidth = new QSpinBox(groupBox_3);
        viewWidth->setObjectName(QString::fromUtf8("viewWidth"));
        viewWidth->setMinimum(1);
        viewWidth->setMaximum(10000);
        viewWidth->setValue(768);

        verticalLayout_3->addWidget(viewWidth);

        viewHeight = new QSpinBox(groupBox_3);
        viewHeight->setObjectName(QString::fromUtf8("viewHeight"));
        viewHeight->setMinimum(1);
        viewHeight->setMaximum(10000);
        viewHeight->setValue(768);

        verticalLayout_3->addWidget(viewHeight);


        horizontalLayout_6->addLayout(verticalLayout_3);


        verticalLayout->addWidget(groupBox_3);


        horizontalLayout->addWidget(widget);


        retranslateUi(designer);

        QMetaObject::connectSlotsByName(designer);
    } // setupUi

    void retranslateUi(QWidget *designer)
    {
        designer->setWindowTitle(QApplication::translate("designer", "Designer", 0, QApplication::UnicodeUTF8));
        groupBox_1->setTitle(QApplication::translate("designer", "Scene", 0, QApplication::UnicodeUTF8));
        loadButton->setText(QApplication::translate("designer", "Load", 0, QApplication::UnicodeUTF8));
        saveButton->setText(QApplication::translate("designer", "Save", 0, QApplication::UnicodeUTF8));
        clearButton->setText(QApplication::translate("designer", "Clear", 0, QApplication::UnicodeUTF8));
        groupBox_2->setTitle(QApplication::translate("designer", "Meshes", 0, QApplication::UnicodeUTF8));
        groupBox_3->setTitle(QApplication::translate("designer", "Camera", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        fov->setToolTip(QApplication::translate("designer", "Field of View (FOV)", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        viewWidth->setToolTip(QApplication::translate("designer", "Viewport width", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        viewWidth->setSuffix(QApplication::translate("designer", " px", 0, QApplication::UnicodeUTF8));
        viewWidth->setPrefix(QApplication::translate("designer", "width: ", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        viewHeight->setToolTip(QApplication::translate("designer", "Viewport height", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        viewHeight->setSuffix(QApplication::translate("designer", " px", 0, QApplication::UnicodeUTF8));
        viewHeight->setPrefix(QApplication::translate("designer", "height: ", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class designer: public Ui_designer {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DESIGNER_H
