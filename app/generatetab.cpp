
#include "generatetab.h"

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QRegularExpression>
#include <QRegularExpressionValidator>

GenerateTab::GenerateTab(QWidget *parent)
    : QWidget(parent)
{
    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Heading
    QLabel *heading = new QLabel("Barcode Generator", this);
    heading->setStyleSheet(
        "font-size: 26px;"
        "font-weight: bold;"
        );

    QLabel *instructions = new QLabel(
        "Enter a six-digit number to generate its barcode.", this
        );

    // Input field
    numberInput = new QLineEdit(this);
    numberInput->setPlaceholderText("Enter 6 digits");
    numberInput->setMaxLength(6);
    numberInput->setMinimumHeight(40);

    QRegularExpression expression("[0-9]{0,6}");
    QRegularExpressionValidator *validator =
        new QRegularExpressionValidator(expression, this);

    numberInput->setValidator(validator);

    // Generate button
    generateButton = new QPushButton("Generate Barcode", this);
    generateButton->setMinimumHeight(40);

    // Barcode preview heading
    QLabel *previewHeading = new QLabel("Barcode Preview", this);
    previewHeading->setStyleSheet(
        "font-size: 19px;"
        "font-weight: bold;"
        );

    // Barcode display area
    barcodePreview = new QLabel("Your barcode will appear here", this);
    barcodePreview->setAlignment(Qt::AlignCenter);
    barcodePreview->setMinimumSize(400, 180);
    barcodePreview->setStyleSheet(
        "background-color: white;"
        "border: 2px dashed #999999;"
        "border-radius: 10px;"
        "color: #777777;"
        "font-size: 16px;"
        );

    // Number below barcode
    barcodeNumber = new QLabel("------", this);
    barcodeNumber->setAlignment(Qt::AlignCenter);
    barcodeNumber->setStyleSheet(
        "font-size: 22px;"
        "font-weight: bold;"
        "letter-spacing: 5px;"
        );

    // Save button
    saveButton = new QPushButton("Save as PNG", this);
    saveButton->setMinimumHeight(40);
    saveButton->setEnabled(false);

    // Status message
    messageLabel = new QLabel("Enter a number to get started.", this);

    // Add widgets to layout
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

    // Generate button validation
    connect(generateButton, &QPushButton::clicked, this, [this]() {

        QString number = numberInput->text();

        if (number.length() != 6) {
            messageLabel->setText(
                "Error: Please enter exactly 6 digits."
                );
            messageLabel->setStyleSheet("color: red;");
            return;
        }

        messageLabel->setText(
            "Valid input! Ready to generate barcode."
            );
        messageLabel->setStyleSheet("color: green;");
    });
}



