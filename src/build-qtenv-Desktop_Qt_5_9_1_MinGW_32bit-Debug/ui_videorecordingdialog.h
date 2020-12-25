/********************************************************************************
** Form generated from reading UI file 'videorecordingdialog.ui'
**
** Created by: Qt User Interface Compiler version 5.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_VIDEORECORDINGDIALOG_H
#define UI_VIDEORECORDINGDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_VideoRecordingDialog
{
public:
    QGridLayout *gridLayout;
    QGroupBox *containerFormatGroup;
    QVBoxLayout *verticalLayout_3;
    QRadioButton *mp4;
    QRadioButton *mkv;
    QDialogButtonBox *buttonBox;
    QGroupBox *recordingAreaGroup;
    QVBoxLayout *verticalLayout;
    QRadioButton *window;
    QRadioButton *network;
    QRadioButton *border;
    QRadioButton *padding;
    QLabel *topLabel;
    QLabel *bottomLabel;
    QGroupBox *pixelFormatGroup;
    QVBoxLayout *verticalLayout_2;
    QRadioButton *compatible;
    QRadioButton *sharper;
    QPushButton *copyButton;
    QLabel *middleLabel;
    QLabel *commandLabel;

    void setupUi(QDialog *VideoRecordingDialog)
    {
        if (VideoRecordingDialog->objectName().isEmpty())
            VideoRecordingDialog->setObjectName(QStringLiteral("VideoRecordingDialog"));
        VideoRecordingDialog->resize(439, 680);
        gridLayout = new QGridLayout(VideoRecordingDialog);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        containerFormatGroup = new QGroupBox(VideoRecordingDialog);
        containerFormatGroup->setObjectName(QStringLiteral("containerFormatGroup"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(containerFormatGroup->sizePolicy().hasHeightForWidth());
        containerFormatGroup->setSizePolicy(sizePolicy);
        verticalLayout_3 = new QVBoxLayout(containerFormatGroup);
        verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
        mp4 = new QRadioButton(containerFormatGroup);
        mp4->setObjectName(QStringLiteral("mp4"));
        mp4->setChecked(true);

        verticalLayout_3->addWidget(mp4);

        mkv = new QRadioButton(containerFormatGroup);
        mkv->setObjectName(QStringLiteral("mkv"));

        verticalLayout_3->addWidget(mkv);


        gridLayout->addWidget(containerFormatGroup, 6, 0, 1, 1);

        buttonBox = new QDialogButtonBox(VideoRecordingDialog);
        buttonBox->setObjectName(QStringLiteral("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        gridLayout->addWidget(buttonBox, 11, 1, 1, 1);

        recordingAreaGroup = new QGroupBox(VideoRecordingDialog);
        recordingAreaGroup->setObjectName(QStringLiteral("recordingAreaGroup"));
        sizePolicy.setHeightForWidth(recordingAreaGroup->sizePolicy().hasHeightForWidth());
        recordingAreaGroup->setSizePolicy(sizePolicy);
        verticalLayout = new QVBoxLayout(recordingAreaGroup);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        window = new QRadioButton(recordingAreaGroup);
        window->setObjectName(QStringLiteral("window"));
        window->setChecked(true);

        verticalLayout->addWidget(window);

        network = new QRadioButton(recordingAreaGroup);
        network->setObjectName(QStringLiteral("network"));

        verticalLayout->addWidget(network);

        border = new QRadioButton(recordingAreaGroup);
        border->setObjectName(QStringLiteral("border"));

        verticalLayout->addWidget(border);

        padding = new QRadioButton(recordingAreaGroup);
        padding->setObjectName(QStringLiteral("padding"));

        verticalLayout->addWidget(padding);


        gridLayout->addWidget(recordingAreaGroup, 5, 0, 1, 3);

        topLabel = new QLabel(VideoRecordingDialog);
        topLabel->setObjectName(QStringLiteral("topLabel"));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(topLabel->sizePolicy().hasHeightForWidth());
        topLabel->setSizePolicy(sizePolicy1);
        topLabel->setWordWrap(true);

        gridLayout->addWidget(topLabel, 0, 0, 1, 3);

        bottomLabel = new QLabel(VideoRecordingDialog);
        bottomLabel->setObjectName(QStringLiteral("bottomLabel"));
        sizePolicy1.setHeightForWidth(bottomLabel->sizePolicy().hasHeightForWidth());
        bottomLabel->setSizePolicy(sizePolicy1);
        bottomLabel->setWordWrap(true);

        gridLayout->addWidget(bottomLabel, 10, 0, 1, 3);

        pixelFormatGroup = new QGroupBox(VideoRecordingDialog);
        pixelFormatGroup->setObjectName(QStringLiteral("pixelFormatGroup"));
        sizePolicy.setHeightForWidth(pixelFormatGroup->sizePolicy().hasHeightForWidth());
        pixelFormatGroup->setSizePolicy(sizePolicy);
        verticalLayout_2 = new QVBoxLayout(pixelFormatGroup);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        compatible = new QRadioButton(pixelFormatGroup);
        compatible->setObjectName(QStringLiteral("compatible"));
        compatible->setChecked(true);

        verticalLayout_2->addWidget(compatible);

        sharper = new QRadioButton(pixelFormatGroup);
        sharper->setObjectName(QStringLiteral("sharper"));

        verticalLayout_2->addWidget(sharper);


        gridLayout->addWidget(pixelFormatGroup, 6, 1, 1, 1);

        copyButton = new QPushButton(VideoRecordingDialog);
        copyButton->setObjectName(QStringLiteral("copyButton"));

        gridLayout->addWidget(copyButton, 9, 0, 1, 2, Qt::AlignRight);

        middleLabel = new QLabel(VideoRecordingDialog);
        middleLabel->setObjectName(QStringLiteral("middleLabel"));
        middleLabel->setWordWrap(true);

        gridLayout->addWidget(middleLabel, 7, 0, 1, 2);

        commandLabel = new QLabel(VideoRecordingDialog);
        commandLabel->setObjectName(QStringLiteral("commandLabel"));
        sizePolicy.setHeightForWidth(commandLabel->sizePolicy().hasHeightForWidth());
        commandLabel->setSizePolicy(sizePolicy);
        commandLabel->setStyleSheet(QStringLiteral("background-color: palette(base);"));
        commandLabel->setFrameShape(QFrame::StyledPanel);
        commandLabel->setWordWrap(true);
        commandLabel->setMargin(6);
        commandLabel->setTextInteractionFlags(Qt::TextSelectableByKeyboard|Qt::TextSelectableByMouse);

        gridLayout->addWidget(commandLabel, 8, 0, 1, 2);


        retranslateUi(VideoRecordingDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), VideoRecordingDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), VideoRecordingDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(VideoRecordingDialog);
    } // setupUi

    void retranslateUi(QDialog *VideoRecordingDialog)
    {
        VideoRecordingDialog->setWindowTitle(QApplication::translate("VideoRecordingDialog", "Video Recording", Q_NULLPTR));
        containerFormatGroup->setTitle(QApplication::translate("VideoRecordingDialog", "\345\260\201\350\243\205\346\240\274\345\274\217", Q_NULLPTR));
        mp4->setText(QApplication::translate("VideoRecordingDialog", "MP4", Q_NULLPTR));
        mkv->setText(QApplication::translate("VideoRecordingDialog", "MKV", Q_NULLPTR));
        recordingAreaGroup->setTitle(QApplication::translate("VideoRecordingDialog", "\345\211\252\345\210\207\345\260\272\345\257\270", Q_NULLPTR));
        window->setText(QApplication::translate("VideoRecordingDialog", "\346\225\264\344\270\252\347\252\227\345\217\243", Q_NULLPTR));
        network->setText(QApplication::translate("VideoRecordingDialog", "\347\275\221\347\273\234\345\233\276\351\203\250\345\210\206", Q_NULLPTR));
        border->setText(QApplication::translate("VideoRecordingDialog", "\347\275\221\347\273\234\345\233\276\345\270\246\350\276\271\346\241\206", Q_NULLPTR));
        padding->setText(QApplication::translate("VideoRecordingDialog", "\347\275\221\347\273\234\345\233\276\345\212\240\344\270\252\350\276\271\350\267\235", Q_NULLPTR));
        topLabel->setText(QApplication::translate("VideoRecordingDialog", "\345\275\225\345\210\266\345\212\237\350\203\275\344\274\232\345\257\274\345\207\272\344\270\200\347\263\273\345\210\227\347\232\204\346\210\252\345\233\276, \350\265\267\345\247\213\347\274\226\345\217\267\344\270\272'frames/${BASE}0000.png'. \346\210\252\345\233\276\345\217\257\344\273\245\344\275\277\347\224\250ffmpeg\346\213\274\346\216\245\344\270\272\350\247\206\351\242\221\343\200\202", Q_NULLPTR));
        bottomLabel->setText(QApplication::translate("VideoRecordingDialog", "\345\274\200\345\247\213\345\275\225\345\210\266\345\211\215\357\274\214\350\257\267\346\243\200\346\237\245\346\230\257\345\220\246\346\234\211\350\266\263\345\244\237\347\232\204\347\243\201\347\233\230\347\251\272\351\227\264\357\274\214\345\233\240\344\270\272\345\212\250\347\224\273\346\210\252\345\233\276\344\274\232\346\257\224\350\276\203\345\215\240\347\251\272\351\227\264\357\274\233<br/><br/>\345\275\223\345\275\225\345\210\266\346\227\266\357\274\214\344\270\273\347\252\227\345\217\243\347\232\204\345\260\272\345\257\270\344\274\232\350\207\252\345\212\250\346\233\264\346\224\271\357\274\214\344\273\245\344\276\277\351\200\202\345\272\224\347\274\226\347\240\201\351\200\211\351\241\271", Q_NULLPTR));
        pixelFormatGroup->setTitle(QApplication::translate("VideoRecordingDialog", "\345\203\217\347\264\240\346\240\274\345\274\217", Q_NULLPTR));
        compatible->setText(QApplication::translate("VideoRecordingDialog", "More compatible (yuv420p)", Q_NULLPTR));
        sharper->setText(QApplication::translate("VideoRecordingDialog", "Sharper image (yuv444p)", Q_NULLPTR));
        copyButton->setText(QApplication::translate("VideoRecordingDialog", "\346\213\267\350\264\235\345\210\260\345\211\252\350\264\264\346\235\277", Q_NULLPTR));
        middleLabel->setText(QApplication::translate("VideoRecordingDialog", "\345\275\225\345\210\266\345\220\216\347\224\250\346\255\244\345\221\275\344\273\244\345\220\210\345\271\266\345\233\276\347\211\207\344\270\272\350\247\206\351\242\221 (\351\234\200\350\246\201\345\256\211\350\243\205 ffmpeg and x264 ):", Q_NULLPTR));
        commandLabel->setText(QApplication::translate("VideoRecordingDialog", "<html><head/><body><p><span style=\" font-family:'monospace'; font-weight:600;\">ffmpeg -r FPS -f image2 -i \"frames/CONFIGURATION#RUN_%04d.png\" -filter:v \"crop=WIDTH:HEIGHT:X:Y\" -vcodec libx264 -pix_fmt yuv420p CONFIGURATION#RUN.mp4</span></p></body></html>", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class VideoRecordingDialog: public Ui_VideoRecordingDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_VIDEORECORDINGDIALOG_H
