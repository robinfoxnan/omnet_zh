/********************************************************************************
** Form generated from reading UI file 'comboselectiondialog.ui'
**
** Created by: Qt User Interface Compiler version 5.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_COMBOSELECTIONDIALOG_H
#define UI_COMBOSELECTIONDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_ComboSelectionDialog
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *label;
    QHBoxLayout *horizontalLayout;
    QLabel *label_2;
    QComboBox *comboBox;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *ComboSelectionDialog)
    {
        if (ComboSelectionDialog->objectName().isEmpty())
            ComboSelectionDialog->setObjectName(QStringLiteral("ComboSelectionDialog"));
        ComboSelectionDialog->resize(582, 120);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(ComboSelectionDialog->sizePolicy().hasHeightForWidth());
        ComboSelectionDialog->setSizePolicy(sizePolicy);
        verticalLayout = new QVBoxLayout(ComboSelectionDialog);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        label = new QLabel(ComboSelectionDialog);
        label->setObjectName(QStringLiteral("label"));

        verticalLayout->addWidget(label);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        label_2 = new QLabel(ComboSelectionDialog);
        label_2->setObjectName(QStringLiteral("label_2"));

        horizontalLayout->addWidget(label_2);

        comboBox = new QComboBox(ComboSelectionDialog);
        comboBox->setObjectName(QStringLiteral("comboBox"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(comboBox->sizePolicy().hasHeightForWidth());
        comboBox->setSizePolicy(sizePolicy1);

        horizontalLayout->addWidget(comboBox);


        verticalLayout->addLayout(horizontalLayout);

        buttonBox = new QDialogButtonBox(ComboSelectionDialog);
        buttonBox->setObjectName(QStringLiteral("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(ComboSelectionDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), ComboSelectionDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), ComboSelectionDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(ComboSelectionDialog);
    } // setupUi

    void retranslateUi(QDialog *ComboSelectionDialog)
    {
        ComboSelectionDialog->setWindowTitle(QApplication::translate("ComboSelectionDialog", "Set Up Network", Q_NULLPTR));
        label->setText(QApplication::translate("ComboSelectionDialog", "\351\200\211\346\213\251\344\270\200\344\270\252\347\275\221\347\273\234\345\233\276. \347\275\221\347\273\234\345\260\206\344\275\277\347\224\250\345\234\250INI [General] \350\212\202\344\270\255\345\256\232\344\271\211\347\232\204\345\217\202\346\225\260.", Q_NULLPTR));
        label_2->setText(QApplication::translate("ComboSelectionDialog", "\351\200\211\346\213\251\347\275\221\347\273\234", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class ComboSelectionDialog: public Ui_ComboSelectionDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_COMBOSELECTIONDIALOG_H
