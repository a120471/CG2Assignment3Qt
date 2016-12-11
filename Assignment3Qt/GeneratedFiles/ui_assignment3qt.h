/********************************************************************************
** Form generated from reading UI file 'assignment3qt.ui'
**
** Created by: Qt User Interface Compiler version 5.5.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ASSIGNMENT3QT_H
#define UI_ASSIGNMENT3QT_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Assignment3QtClass
{
public:
    QWidget *centralWidget;
    QWidget *layoutWidget;
    QFormLayout *formLayout;
    QLabel *label_RW;
    QLineEdit *resolutionW;
    QLabel *label_RH;
    QLineEdit *resolutionH;
    QLabel *label_AA;
    QLineEdit *antiAliasing;
    QLabel *label_ICR;
    QLineEdit *imageScaleRatio;
    QSpacerItem *verticalSpacer;
    QLabel *label_CP;
    QLineEdit *CameraPos;
    QLabel *label_LA;
    QLineEdit *CameraLookAt;
    QSpacerItem *verticalSpacer_2;
    QLabel *label_SD;
    QPushButton *pushButton_Browse;
    QLineEdit *sceneDataPath;
    QLabel *label_TValue;
    QPushButton *pushButton_Render;
    QSpacerItem *verticalSpacer_3;
    QLabel *label_Image;

    void setupUi(QMainWindow *Assignment3QtClass)
    {
        if (Assignment3QtClass->objectName().isEmpty())
            Assignment3QtClass->setObjectName(QStringLiteral("Assignment3QtClass"));
        Assignment3QtClass->setEnabled(true);
        Assignment3QtClass->resize(1389, 922);
        centralWidget = new QWidget(Assignment3QtClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        centralWidget->setMinimumSize(QSize(0, 0));
        layoutWidget = new QWidget(centralWidget);
        layoutWidget->setObjectName(QStringLiteral("layoutWidget"));
        layoutWidget->setGeometry(QRect(1220, 10, 161, 901));
        formLayout = new QFormLayout(layoutWidget);
        formLayout->setSpacing(6);
        formLayout->setContentsMargins(11, 11, 11, 11);
        formLayout->setObjectName(QStringLiteral("formLayout"));
        formLayout->setLabelAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        formLayout->setContentsMargins(0, 0, 0, 0);
        label_RW = new QLabel(layoutWidget);
        label_RW->setObjectName(QStringLiteral("label_RW"));

        formLayout->setWidget(1, QFormLayout::LabelRole, label_RW);

        resolutionW = new QLineEdit(layoutWidget);
        resolutionW->setObjectName(QStringLiteral("resolutionW"));

        formLayout->setWidget(1, QFormLayout::FieldRole, resolutionW);

        label_RH = new QLabel(layoutWidget);
        label_RH->setObjectName(QStringLiteral("label_RH"));

        formLayout->setWidget(3, QFormLayout::LabelRole, label_RH);

        resolutionH = new QLineEdit(layoutWidget);
        resolutionH->setObjectName(QStringLiteral("resolutionH"));

        formLayout->setWidget(3, QFormLayout::FieldRole, resolutionH);

        label_AA = new QLabel(layoutWidget);
        label_AA->setObjectName(QStringLiteral("label_AA"));

        formLayout->setWidget(5, QFormLayout::LabelRole, label_AA);

        antiAliasing = new QLineEdit(layoutWidget);
        antiAliasing->setObjectName(QStringLiteral("antiAliasing"));

        formLayout->setWidget(5, QFormLayout::FieldRole, antiAliasing);

        label_ICR = new QLabel(layoutWidget);
        label_ICR->setObjectName(QStringLiteral("label_ICR"));

        formLayout->setWidget(6, QFormLayout::LabelRole, label_ICR);

        imageScaleRatio = new QLineEdit(layoutWidget);
        imageScaleRatio->setObjectName(QStringLiteral("imageScaleRatio"));

        formLayout->setWidget(6, QFormLayout::FieldRole, imageScaleRatio);

        verticalSpacer = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding);

        formLayout->setItem(7, QFormLayout::SpanningRole, verticalSpacer);

        label_CP = new QLabel(layoutWidget);
        label_CP->setObjectName(QStringLiteral("label_CP"));
        label_CP->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);

        formLayout->setWidget(8, QFormLayout::LabelRole, label_CP);

        CameraPos = new QLineEdit(layoutWidget);
        CameraPos->setObjectName(QStringLiteral("CameraPos"));

        formLayout->setWidget(8, QFormLayout::FieldRole, CameraPos);

        label_LA = new QLabel(layoutWidget);
        label_LA->setObjectName(QStringLiteral("label_LA"));
        label_LA->setLayoutDirection(Qt::LeftToRight);

        formLayout->setWidget(9, QFormLayout::LabelRole, label_LA);

        CameraLookAt = new QLineEdit(layoutWidget);
        CameraLookAt->setObjectName(QStringLiteral("CameraLookAt"));

        formLayout->setWidget(9, QFormLayout::FieldRole, CameraLookAt);

        verticalSpacer_2 = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding);

        formLayout->setItem(10, QFormLayout::SpanningRole, verticalSpacer_2);

        label_SD = new QLabel(layoutWidget);
        label_SD->setObjectName(QStringLiteral("label_SD"));

        formLayout->setWidget(11, QFormLayout::LabelRole, label_SD);

        pushButton_Browse = new QPushButton(layoutWidget);
        pushButton_Browse->setObjectName(QStringLiteral("pushButton_Browse"));

        formLayout->setWidget(11, QFormLayout::FieldRole, pushButton_Browse);

        sceneDataPath = new QLineEdit(layoutWidget);
        sceneDataPath->setObjectName(QStringLiteral("sceneDataPath"));
        sceneDataPath->setEnabled(true);
        sceneDataPath->setReadOnly(true);

        formLayout->setWidget(12, QFormLayout::SpanningRole, sceneDataPath);

        label_TValue = new QLabel(layoutWidget);
        label_TValue->setObjectName(QStringLiteral("label_TValue"));
        label_TValue->setAlignment(Qt::AlignCenter);

        formLayout->setWidget(15, QFormLayout::SpanningRole, label_TValue);

        pushButton_Render = new QPushButton(layoutWidget);
        pushButton_Render->setObjectName(QStringLiteral("pushButton_Render"));

        formLayout->setWidget(14, QFormLayout::SpanningRole, pushButton_Render);

        verticalSpacer_3 = new QSpacerItem(20, 540, QSizePolicy::Minimum, QSizePolicy::Expanding);

        formLayout->setItem(13, QFormLayout::SpanningRole, verticalSpacer_3);

        label_Image = new QLabel(centralWidget);
        label_Image->setObjectName(QStringLiteral("label_Image"));
        label_Image->setEnabled(true);
        label_Image->setGeometry(QRect(10, 10, 1200, 900));
        QSizePolicy sizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(label_Image->sizePolicy().hasHeightForWidth());
        label_Image->setSizePolicy(sizePolicy);
        label_Image->setAutoFillBackground(false);
        label_Image->setScaledContents(false);
        label_Image->setAlignment(Qt::AlignCenter);
        Assignment3QtClass->setCentralWidget(centralWidget);
        QWidget::setTabOrder(resolutionW, resolutionH);
        QWidget::setTabOrder(resolutionH, antiAliasing);
        QWidget::setTabOrder(antiAliasing, imageScaleRatio);
        QWidget::setTabOrder(imageScaleRatio, CameraPos);
        QWidget::setTabOrder(CameraPos, CameraLookAt);
        QWidget::setTabOrder(CameraLookAt, pushButton_Browse);
        QWidget::setTabOrder(pushButton_Browse, sceneDataPath);
        QWidget::setTabOrder(sceneDataPath, pushButton_Render);

        retranslateUi(Assignment3QtClass);

        QMetaObject::connectSlotsByName(Assignment3QtClass);
    } // setupUi

    void retranslateUi(QMainWindow *Assignment3QtClass)
    {
        Assignment3QtClass->setWindowTitle(QApplication::translate("Assignment3QtClass", "Assignment3Qt", 0));
        label_RW->setText(QApplication::translate("Assignment3QtClass", "ResolutionW", 0));
        resolutionW->setText(QApplication::translate("Assignment3QtClass", "400", 0));
        label_RH->setText(QApplication::translate("Assignment3QtClass", "ResolutionH", 0));
        resolutionH->setText(QApplication::translate("Assignment3QtClass", "300", 0));
        label_AA->setText(QApplication::translate("Assignment3QtClass", "AntiAliasing", 0));
        antiAliasing->setText(QApplication::translate("Assignment3QtClass", "1", 0));
        label_ICR->setText(QApplication::translate("Assignment3QtClass", "ImageScale", 0));
        imageScaleRatio->setText(QApplication::translate("Assignment3QtClass", "3", 0));
        label_CP->setText(QApplication::translate("Assignment3QtClass", "CameraPos", 0));
        CameraPos->setText(QApplication::translate("Assignment3QtClass", "0.0, 1.0, 6.0", 0));
        label_LA->setText(QApplication::translate("Assignment3QtClass", "LookAt", 0));
        CameraLookAt->setText(QApplication::translate("Assignment3QtClass", "0.0, 0.0, 0.0", 0));
        label_SD->setText(QApplication::translate("Assignment3QtClass", "sceneData", 0));
        pushButton_Browse->setText(QApplication::translate("Assignment3QtClass", "Browse...", 0));
        sceneDataPath->setText(QApplication::translate("Assignment3QtClass", "../sceneData.txt", 0));
        label_TValue->setText(QApplication::translate("Assignment3QtClass", "Time: 0s", 0));
        pushButton_Render->setText(QApplication::translate("Assignment3QtClass", "Render", 0));
        label_Image->setText(QApplication::translate("Assignment3QtClass", "Result Image", 0));
    } // retranslateUi

};

namespace Ui {
    class Assignment3QtClass: public Ui_Assignment3QtClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ASSIGNMENT3QT_H
