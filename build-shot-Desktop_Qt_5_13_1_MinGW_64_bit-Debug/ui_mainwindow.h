/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.13.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *topHorizontalLayout;
    QLabel *inputLabel;
    QLineEdit *inputLineEdit;
    QPushButton *inputPushButton;
    QHBoxLayout *bottomHorizontalLayout;
    QLabel *outputLabel;
    QLineEdit *outputLineEdit;
    QPushButton *outputPushButton;
    QCheckBox *showButton;
    QVBoxLayout *verticalLayout_2;
    QPushButton *processButton;
    QVBoxLayout *verticalLayout_3;
    QProgressBar *progressBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(446, 248);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        verticalLayout = new QVBoxLayout(centralWidget);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        topHorizontalLayout = new QHBoxLayout();
        topHorizontalLayout->setSpacing(6);
        topHorizontalLayout->setObjectName(QString::fromUtf8("topHorizontalLayout"));
        inputLabel = new QLabel(centralWidget);
        inputLabel->setObjectName(QString::fromUtf8("inputLabel"));

        topHorizontalLayout->addWidget(inputLabel);

        inputLineEdit = new QLineEdit(centralWidget);
        inputLineEdit->setObjectName(QString::fromUtf8("inputLineEdit"));

        topHorizontalLayout->addWidget(inputLineEdit);

        inputPushButton = new QPushButton(centralWidget);
        inputPushButton->setObjectName(QString::fromUtf8("inputPushButton"));

        topHorizontalLayout->addWidget(inputPushButton);


        verticalLayout->addLayout(topHorizontalLayout);

        bottomHorizontalLayout = new QHBoxLayout();
        bottomHorizontalLayout->setSpacing(6);
        bottomHorizontalLayout->setObjectName(QString::fromUtf8("bottomHorizontalLayout"));
        outputLabel = new QLabel(centralWidget);
        outputLabel->setObjectName(QString::fromUtf8("outputLabel"));

        bottomHorizontalLayout->addWidget(outputLabel);

        outputLineEdit = new QLineEdit(centralWidget);
        outputLineEdit->setObjectName(QString::fromUtf8("outputLineEdit"));

        bottomHorizontalLayout->addWidget(outputLineEdit);

        outputPushButton = new QPushButton(centralWidget);
        outputPushButton->setObjectName(QString::fromUtf8("outputPushButton"));

        bottomHorizontalLayout->addWidget(outputPushButton);


        verticalLayout->addLayout(bottomHorizontalLayout);

        showButton = new QCheckBox(centralWidget);
        showButton->setObjectName(QString::fromUtf8("showButton"));

        verticalLayout->addWidget(showButton);

        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        processButton = new QPushButton(centralWidget);
        processButton->setObjectName(QString::fromUtf8("processButton"));

        verticalLayout_2->addWidget(processButton);


        verticalLayout->addLayout(verticalLayout_2);

        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setSpacing(6);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        progressBar = new QProgressBar(centralWidget);
        progressBar->setObjectName(QString::fromUtf8("progressBar"));
        progressBar->setValue(24);

        verticalLayout_3->addWidget(progressBar);


        verticalLayout->addLayout(verticalLayout_3);

        MainWindow->setCentralWidget(centralWidget);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "Video processing", nullptr));
        inputLabel->setText(QCoreApplication::translate("MainWindow", "Input video:", nullptr));
        inputPushButton->setText(QCoreApplication::translate("MainWindow", "Browse", nullptr));
        outputLabel->setText(QCoreApplication::translate("MainWindow", "Output path:", nullptr));
        outputPushButton->setText(QCoreApplication::translate("MainWindow", "Browse", nullptr));
        showButton->setText(QCoreApplication::translate("MainWindow", "Show video processing", nullptr));
        processButton->setText(QCoreApplication::translate("MainWindow", "Start process", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
