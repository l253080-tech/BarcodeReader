
#include "generatetab.h"
#include "Barcode_Encoder.h"

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QPainter>
#include <QFileDialog>

#include <string>

GenerateTab::GenerateTab(QWidget *parent)
    : QWidget(parent)
{
    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(30, 25, 30, 25);
    mainLayout->setSpacing(15);

    // Heading
    QLabel *heading = new QLabel("Barcode Generator", this);
    heading->setStyleSheet(
        "font-size: 26px;"
        "font-weight: bold;"
        "color: #172554;"
        );

    // Instructions
    QLabel *instructions = new QLabel(
        "Enter a six-digit number to generate its barcode.", this
        );

    instructions->setStyleSheet(
        "font-size: 14px;"
        "color: #64748B;"
        );

    // Number input
    numberInput = new QLineEdit(this);
    numberInput->setPlaceholderText("Enter 6 digits");
    numberInput->setMaxLength(6);
    numberInput->setMinimumHeight(45);

    QRegularExpression expression("[0-9]{0,6}");

    QRegularExpressionValidator *validator =
        new QRegularExpressionValidator(expression, this);

    numberInput->setValidator(validator);

    // Generate button
    generateButton = new QPushButton("Generate Barcode", this);
    generateButton->setMinimumHeight(45);

    // Save button
    saveButton = new QPushButton("Save as PNG", this);
    saveButton->setMinimumHeight(45);
    saveButton->setObjectName("saveButton");
    saveButton->setEnabled(false);

    // Buttons side by side
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(15);

    buttonLayout->addWidget(generateButton);
    buttonLayout->addWidget(saveButton);

    // Preview heading
    QLabel *previewHeading = new QLabel("Barcode Preview", this);
    previewHeading->setStyleSheet(
        "font-size: 20px;"
        "font-weight: bold;"
        "color: #172554;"
        );

    // Barcode preview
    barcodePreview = new QLabel(
        "Your barcode will appear here", this
        );

    barcodePreview->setObjectName("barcodePreview");
    barcodePreview->setAlignment(Qt::AlignCenter);
    barcodePreview->setMinimumSize(400, 200);

    barcodePreview->setStyleSheet(
        "background-color: white;"
        "border: 2px dashed #CBD5E1;"
        "border-radius: 10px;"
        "color: #94A3B8;"
        "font-size: 15px;"
        );

    // Barcode number
    barcodeNumber = new QLabel("------", this);
    barcodeNumber->setAlignment(Qt::AlignCenter);

    barcodeNumber->setStyleSheet(
        "font-size: 22px;"
        "font-weight: bold;"
        "letter-spacing: 5px;"
        "color: #172554;"
        );

    // Status message
    messageLabel = new QLabel(
        "Enter a number to get started.", this
        );

    messageLabel->setStyleSheet(
        "font-size: 13px;"
        "color: #64748B;"
        );

    // Add widgets to main layout
    mainLayout->addWidget(heading);
    mainLayout->addWidget(instructions);

    mainLayout->addSpacing(5);

    mainLayout->addWidget(numberInput);
    mainLayout->addLayout(buttonLayout);

    mainLayout->addSpacing(10);

    mainLayout->addWidget(previewHeading);
    mainLayout->addWidget(barcodePreview, 1);
    mainLayout->addWidget(barcodeNumber);

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
            messageLabel->setText(
                "Error: Barcode generation failed."
                );
            messageLabel->setStyleSheet("color: red;");
            return;
        }

        // Create a white image for the barcode
        int moduleWidth = 6;
        int barcodeHeight = 90;
        int quietZone = 10;

        int imageWidth =
            (static_cast<int>(pattern.length()) + 2 * quietZone)
            * moduleWidth;

        barcodeImage = QPixmap(imageWidth, 150);
        barcodeImage.fill(Qt::white);

        // Draw barcode stripes
        QPainter painter(&barcodeImage);

        painter.setPen(Qt::NoPen);
        painter.setBrush(Qt::black);

        for (int i = 0; i < static_cast<int>(pattern.length()); i++) {

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

        // Enable saving
        saveButton->setEnabled(true);

        messageLabel->setText(
            "Barcode generated successfully."
            );
        messageLabel->setStyleSheet("color: #15803D;");
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

            messageLabel->setText(
                "Barcode saved successfully."
                );
            messageLabel->setStyleSheet("color: #15803D;");
        }
        else {

            messageLabel->setText(
                "Error: Unable to save barcode."
                );
            messageLabel->setStyleSheet("color: red;");
        }
    });
}
