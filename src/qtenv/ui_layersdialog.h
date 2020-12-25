/********************************************************************************
** Form generated from reading UI file 'layersdialog.ui'
**
** Created by: Qt User Interface Compiler version 5.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LAYERSDIALOG_H
#define UI_LAYERSDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_LayersDialog
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *intro;
    QGridLayout *gridLayout;
    QScrollArea *hideFigScrollArea;
    QWidget *hideFigWidget;
    QVBoxLayout *verticalLayout_4;
    QLabel *showFigures;
    QLabel *hideFigures;
    QScrollArea *showFigScrollArea;
    QWidget *showFigWidget;
    QVBoxLayout *verticalLayout_3;
    QLabel *note;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *LayersDialog)
    {
        if (LayersDialog->objectName().isEmpty())
            LayersDialog->setObjectName(QStringLiteral("LayersDialog"));
        LayersDialog->resize(583, 366);
        verticalLayout = new QVBoxLayout(LayersDialog);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        intro = new QLabel(LayersDialog);
        intro->setObjectName(QStringLiteral("intro"));
        intro->setWordWrap(true);

        verticalLayout->addWidget(intro);

        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        hideFigScrollArea = new QScrollArea(LayersDialog);
        hideFigScrollArea->setObjectName(QStringLiteral("hideFigScrollArea"));
        hideFigScrollArea->setWidgetResizable(true);
        hideFigScrollArea->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
        hideFigWidget = new QWidget();
        hideFigWidget->setObjectName(QStringLiteral("hideFigWidget"));
        hideFigWidget->setGeometry(QRect(0, 0, 274, 239));
        verticalLayout_4 = new QVBoxLayout(hideFigWidget);
        verticalLayout_4->setObjectName(QStringLiteral("verticalLayout_4"));
        hideFigScrollArea->setWidget(hideFigWidget);

        gridLayout->addWidget(hideFigScrollArea, 1, 1, 1, 1);

        showFigures = new QLabel(LayersDialog);
        showFigures->setObjectName(QStringLiteral("showFigures"));

        gridLayout->addWidget(showFigures, 0, 0, 1, 1);

        hideFigures = new QLabel(LayersDialog);
        hideFigures->setObjectName(QStringLiteral("hideFigures"));

        gridLayout->addWidget(hideFigures, 0, 1, 1, 1);

        showFigScrollArea = new QScrollArea(LayersDialog);
        showFigScrollArea->setObjectName(QStringLiteral("showFigScrollArea"));
        showFigScrollArea->setWidgetResizable(true);
        showFigScrollArea->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
        showFigWidget = new QWidget();
        showFigWidget->setObjectName(QStringLiteral("showFigWidget"));
        showFigWidget->setGeometry(QRect(0, 0, 274, 239));
        verticalLayout_3 = new QVBoxLayout(showFigWidget);
        verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
        verticalLayout_3->setSizeConstraint(QLayout::SetDefaultConstraint);
        showFigScrollArea->setWidget(showFigWidget);

        gridLayout->addWidget(showFigScrollArea, 1, 0, 1, 1);


        verticalLayout->addLayout(gridLayout);

        note = new QLabel(LayersDialog);
        note->setObjectName(QStringLiteral("note"));

        verticalLayout->addWidget(note);

        buttonBox = new QDialogButtonBox(LayersDialog);
        buttonBox->setObjectName(QStringLiteral("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(LayersDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), LayersDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), LayersDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(LayersDialog);
    } // setupUi

    void retranslateUi(QDialog *LayersDialog)
    {
        LayersDialog->setWindowTitle(QApplication::translate("LayersDialog", "Select Layers", Q_NULLPTR));
        intro->setText(QApplication::translate("LayersDialog", "\351\200\211\346\213\251\350\246\201\346\230\276\347\244\272\347\232\204\345\233\276\343\200\202\345\246\202\346\236\234\345\233\276\344\270\255\345\214\205\345\220\253\345\210\227\350\241\250\344\270\255\347\232\204\346\240\207\347\255\276\345\260\206\350\242\253\346\230\276\347\244\272.", Q_NULLPTR));
        showFigures->setText(QApplication::translate("LayersDialog", "\346\230\276\347\244\272\347\232\204\346\240\207\347\255\276", Q_NULLPTR));
        hideFigures->setText(QApplication::translate("LayersDialog", "\351\232\220\350\227\217\347\232\204\346\240\207\347\255\276", Q_NULLPTR));
        note->setText(QApplication::translate("LayersDialog", "\346\217\220\347\244\272: \344\270\215\345\214\205\345\220\253\346\240\207\347\255\276\347\232\204\345\233\276\344\270\200\347\233\264\351\203\275\344\274\232\346\230\276\347\244\272", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class LayersDialog: public Ui_LayersDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LAYERSDIALOG_H
