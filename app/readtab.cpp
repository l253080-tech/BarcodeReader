
#include "readtab.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QPixmap>
#include <QFileInfo>

ReadTab::ReadTab(QWidget *parent)
    : QWidget(parent)
{
    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Heading
    QLabel *heading = new QLabel("Barcode Reader", this);
    heading->setStyleSheet("font-size: 24px; font-weight: bold;");

    // Instructions
    QLabel *instructions = new QLabel(
        "Upload an image containing a barcode.", this);

    // Upload button
    uploadButton = new QPushButton("Upload Image", this);

    // Image preview area
    imagePreview = new QLabel("No image selected", this);
    imagePreview->setAlignment(Qt::AlignCenter);
    imagePreview->setMinimumSize(400, 200);
    imagePreview->setStyleSheet(
        "border: 2px dashed gray;"
        "background-color: #f5f5f5;"
        );

    // Filename
    fileNameLabel = new QLabel("No file selected", this);

    // Decode button
    decodeButton = new QPushButton("Process / Decode", this);
    decodeButton->setEnabled(false);
    // Result section heading
    resultHeading = new QLabel("Decoding Result", this);
    resultHeading->setStyleSheet(
        "font-size: 20px;"
        "font-weight: bold;"
        );

    // Dedicated output area
    resultLabel = new QLabel("------", this);
    resultLabel->setAlignment(Qt::AlignCenter);
    resultLabel->setMinimumHeight(75);
    resultLabel->setStyleSheet(
        "font-size: 30px;"
        "font-weight: bold;"
        "color: #16803c;"
        "border: 2px solid #cccccc;"
        "border-radius: 8px;"
        "background-color: white;"
        );

    // Status message
    messageLabel = new QLabel(
        "Status: Waiting for barcode processing...", this
        );
    // Add widgets
    mainLayout->addWidget(heading);
    mainLayout->addWidget(instructions);
    mainLayout->addWidget(uploadButton);
    mainLayout->addWidget(imagePreview);
    mainLayout->addWidget(fileNameLabel);
    mainLayout->addWidget(decodeButton);
    mainLayout->addSpacing(15);
    mainLayout->addWidget(resultHeading);
    mainLayout->addWidget(resultLabel);
    mainLayout->addWidget(messageLabel);

    mainLayout->addStretch();

    // Upload image functionality
    connect(uploadButton, &QPushButton::clicked, this, [this]() {

        QString path = QFileDialog::getOpenFileName(
            this,
            "Select Barcode Image",
            "",
            "Image Files (*.png *.jpg *.jpeg *.bmp)"
            );

        // User cancelled the dialog
        if (path.isEmpty())
            return;

        QPixmap image(path);

        // Check whether image loaded successfully
        if (image.isNull()) {
            messageLabel->setText("Error: Unable to load image.");
            decodeButton->setEnabled(false);
            return;
        }

        // Store image path
        imagePath = path;

        // Display image
        imagePreview->setPixmap(
            image.scaled(
                imagePreview->size(),
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation
                )
            );

        // Display filename
        fileNameLabel->setText(
            QFileInfo(path).fileName()
            );

        // Enable decoding
        decodeButton->setEnabled(true);

        // Clear previous messages/results
        resultLabel->setText("------");
        messageLabel->setText("Image uploaded successfully.");
    });
}

