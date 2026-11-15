
#include "generatetab.h"
#include "Barcode_Encoder.h"

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QPainter>
#include <QFileDialog>

GenerateTab::GenerateTab(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QLabel *heading = new QLabel("Barcode Generator", this);
    heading->setStyleSheet(
        "font-size: 26px; font-weight: bold;"
        );

    QLabel *instructions = new QLabel(
        "Enter a six-digit number to generate its barcode.", this
        );

    numberInput = new QLineEdit(this);
    numberInput->setPlaceholderText("Enter 6 digits");
    numberInput->setMaxLength(6);
    numberInput->setMinimumHeight(40);

    QRegularExpression expression("[0-9]{0,6}");

    QRegularExpressionValidator *validator =
        new QRegularExpressionValidator(expression, this);

    numberInput->setValidator(validator);

    generateButton = new QPushButton("Generate Barcode", this);
    generateButton->setMinimumHeight(40);

    QLabel *previewHeading = new QLabel("Barcode Preview", this);
    previewHeading->setStyleSheet(
        "font-size: 19px; font-weight: bold;"
        );

    barcodePreview = new QLabel("Your barcode will appear here", this);
    barcodePreview->setAlignment(Qt::AlignCenter);
    barcodePreview->setMinimumSize(400, 180);
    barcodePreview->setStyleSheet(
        "background-color: white;"
        "border: 2px dashed #999999;"
        "border-radius: 10px;"
        "color: #777777;"
        );

    barcodeNumber = new QLabel("------", this);
    barcodeNumber->setAlignment(Qt::AlignCenter);
    barcodeNumber->setStyleSheet(
        "font-size: 22px;"
        "font-weight: bold;"
        "letter-spacing: 5px;"
        );

    saveButton = new QPushButton("Save as PNG", this);
    saveButton->setMinimumHeight(40);
    saveButton->setEnabled(false);

    messageLabel = new QLabel(
        "Enter a number to get started.", this
        );

    mainLayout->addWidget(heading);
    mainLayout->addWidget(instructions);
    mainLayout->addWidget(numberInput);
    mainLayout->addWidget(generateButton);

    mainLayout->addSpacing(15);

    mainLayout->addWidget(previewHeading);
    mainLayout->addWidget(barcodePreview);
    mainLayout->addWidget(barcodeNumber);

    mainLayout->addSpacing(10);

    mainLayout->addWidget(saveButton);
    mainLayout->addWidget(messageLabel);
    mainLayout->addStretch();

    // Generate actual barcode
    connect(generateButton, &QPushButton::clicked, this, [this]() {

        QString number = numberInput->text();

        if (number.length() != 6) {
            messageLabel->setText(
                "Error: Please enter exactly 6 digits."
                );
            messageLabel->setStyleSheet("color: red;");
            return;
        }

        // Convert QString into std::string
        std::string input = number.toStdString();

        // Call Member 2's encoder
        std::string pattern = BarcodeEncoder::encode(input);

        if (pattern.empty()) {
            messageLabel->setText("Error: Barcode generation failed.");
            messageLabel->setStyleSheet("color: red;");
            return;
        }

        // Create a white image for the barcode
        int moduleWidth = 6;
        int barcodeHeight = 90;
        int quietZone = 10;

        int imageWidth =
            (pattern.length() + 2 * quietZone) * moduleWidth;

        barcodeImage = QPixmap(imageWidth, 150);
        barcodeImage.fill(Qt::white);

        // Draw barcode stripes
        QPainter painter(&barcodeImage);

        painter.setPen(Qt::NoPen);
        painter.setBrush(Qt::black);

        for (int i = 0; i < (int)pattern.length(); i++) {

            if (pattern[i] == '1') {

                int x = (quietZone + i) * moduleWidth;

                painter.drawRect(
                    x, 25, moduleWidth, barcodeHeight
                    );
            }
        }

        painter.end();

        // Display barcode in preview
        barcodePreview->setPixmap(
            barcodeImage.scaled(
                barcodePreview->size(),
                Qt::KeepAspectRatio,
                Qt::FastTransformation
                )
            );

        // Display original six digits
        barcodeNumber->setText(number);

        saveButton->setEnabled(true);

        messageLabel->setText("Barcode generated successfully.");
        messageLabel->setStyleSheet("color: green;");
    });

    // Save barcode as PNG
    connect(saveButton, &QPushButton::clicked, this, [this]() {

        QString path = QFileDialog::getSaveFileName(
            this,
            "Save Barcode",
            "barcode.png",
            "PNG Image (*.png)"
            );

        if (path.isEmpty())
            return;

        if (!path.endsWith(".png", Qt::CaseInsensitive))
            path += ".png";

        if (barcodeImage.save(path, "PNG")) {
            messageLabel->setText("Barcode saved successfully.");
            messageLabel->setStyleSheet("color: green;");
        }
        else {
            messageLabel->setText("Error: Unable to save barcode.");
            messageLabel->setStyleSheet("color: red;");
        }
    });
}
