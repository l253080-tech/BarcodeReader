
#include "readtab.h"
<<<<<<< Updated upstream

=======
#include "cameradialog.h"
#include "ImageScanner.h"
#include "Barcode_Decoder.h"

#include <QStandardPaths>
>>>>>>> Stashed changes
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
    mainLayout->setContentsMargins(30, 25, 30, 25);
    mainLayout->setSpacing(15);

    // Heading
    QLabel *heading = new QLabel("Barcode Reader", this);

    heading->setStyleSheet(
        "font-size: 26px;"
        "font-weight: bold;"
        "color: #172554;"
        );

    // Instructions
    QLabel *instructions = new QLabel(
        "Upload an image or capture a barcode using your camera.",
        this
        );

    instructions->setStyleSheet(
        "font-size: 14px;"
        "color: #64748B;"
        );

    // Upload button
    uploadButton = new QPushButton("Upload Image", this);
    uploadButton->setMinimumHeight(45);

<<<<<<< Updated upstream
=======
    // Camera button
    cameraButton = new QPushButton("Use Camera", this);
    cameraButton->setMinimumHeight(45);
    cameraButton->setObjectName("secondaryButton");

    // Upload and Camera buttons side by side
    QHBoxLayout *inputButtonLayout = new QHBoxLayout();
    inputButtonLayout->setSpacing(15);

    inputButtonLayout->addWidget(uploadButton);
    inputButtonLayout->addWidget(cameraButton);

>>>>>>> Stashed changes
    // Image preview area
    imagePreview = new QLabel("No image selected", this);

    imagePreview->setObjectName("imagePreview");
    imagePreview->setAlignment(Qt::AlignCenter);
    imagePreview->setMinimumSize(400, 200);

    imagePreview->setStyleSheet(
        "border: 2px dashed #CBD5E1;"
        "border-radius: 10px;"
        "background-color: #F8FAFC;"
        "color: #94A3B8;"
        "font-size: 15px;"
        );

    // Filename
    fileNameLabel = new QLabel("No file selected", this);

    fileNameLabel->setStyleSheet(
        "font-size: 12px;"
        "color: #64748B;"
        );

    // Decode button
    decodeButton = new QPushButton("Process / Decode", this);
    decodeButton->setMinimumHeight(45);
    decodeButton->setEnabled(false);
    // Result section heading
    resultHeading = new QLabel("Decoding Result", this);

    resultHeading->setStyleSheet(
        "font-size: 20px;"
        "font-weight: bold;"
        "color: #172554;"
        );

    // Result output area
    resultLabel = new QLabel("------", this);

    resultLabel->setAlignment(Qt::AlignCenter);
    resultLabel->setMinimumHeight(75);

    resultLabel->setStyleSheet(
        "font-size: 30px;"
        "font-weight: bold;"
        "color: #15803D;"
        "border: 1px solid #BBF7D0;"
        "border-radius: 10px;"
        "background-color: #F0FDF4;"
        );

    // Status message
    messageLabel = new QLabel(
        "Status: Waiting for barcode processing...",
        this
        );
<<<<<<< Updated upstream
    // Add widgets
    mainLayout->addWidget(heading);
    mainLayout->addWidget(instructions);
    mainLayout->addWidget(uploadButton);
    mainLayout->addWidget(imagePreview);
=======

    messageLabel->setStyleSheet(
        "font-size: 13px;"
        "color: #64748B;"
        );

    // Add widgets to main layout
    mainLayout->addWidget(heading);
    mainLayout->addWidget(instructions);

    mainLayout->addSpacing(5);

    mainLayout->addLayout(inputButtonLayout);

    mainLayout->addWidget(imagePreview, 1);
>>>>>>> Stashed changes
    mainLayout->addWidget(fileNameLabel);

    mainLayout->addWidget(decodeButton);

    mainLayout->addSpacing(5);

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

            messageLabel->setText(
                "Error: Unable to load image."
                );

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

        messageLabel->setText(
            "Image uploaded successfully."
            );
        messageLabel->setStyleSheet("color: #15803D;");
    });
}

<<<<<<< Updated upstream
=======
    // Use camera functionality
    connect(cameraButton, &QPushButton::clicked, this, [this]() {

        CameraDialog dialog(this);

        // User closed camera without capturing
        if (dialog.exec() != QDialog::Accepted)
            return;

        QImage captured = dialog.capturedImage();

        if (captured.isNull()) {

            messageLabel->setText(
                "Error: No picture was captured."
                );
            return;
        }

        // Save captured image to temporary folder
        QString folder =
            QStandardPaths::writableLocation(
                QStandardPaths::PicturesLocation
                ) + "/BarcodeReader";

        QDir().mkpath(folder);

        QString path = folder + "/camera_capture.png";

        if (!captured.save(path)) {

            messageLabel->setText(
                "Error: Unable to save the captured picture."
                );
            return;
        }

        // Store image path
        imagePath = path;

        // Display captured image
        imagePreview->setPixmap(
            QPixmap::fromImage(captured).scaled(
                imagePreview->size(),
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation
                )
            );

        // Display filename
        fileNameLabel->setText(
            "Camera capture (saved as "
            + QDir::toNativeSeparators(path) + ")"
            );

        // Enable decoding
        decodeButton->setEnabled(true);

        // Clear previous results
        resultLabel->setText("------");

        messageLabel->setText(
            "Picture captured. Press Process / Decode."
            );
        messageLabel->setStyleSheet("color: #15803D;");
    });

    // Decode functionality
    // Image -> widths -> 6 digits -> result label
    connect(decodeButton, &QPushButton::clicked, this, [this]() {

        // Load image and apply EXIF rotation
        QImageReader reader(imagePath);
        reader.setAutoTransform(true);

        QImage image = reader.read();

        if (image.isNull()) {

            resultLabel->setText("------");

            messageLabel->setText(
                "Error: Unable to read the image."
                );
            return;
        }

        // Step 1: Image -> bar/space widths
        std::vector<int> widths =
            ImageScanner::getWidths(image);

        if (widths.empty()) {

            resultLabel->setText("------");

            messageLabel->setText(
                "No barcode found. Try a clearer, straighter image."
                );
            return;
        }

        // Step 2: Widths -> 6 digits
        std::string digits = decodeWidths(widths);

        if (digits.empty()) {

            resultLabel->setText("------");

            messageLabel->setText(
                "Barcode found but could not be decoded "
                "(pattern/checksum error)."
                );
            return;
        }

        // Display decoded barcode
        resultLabel->setText(
            QString::fromStdString(digits)
            );

        messageLabel->setText(
            "Barcode decoded successfully."
            );

        messageLabel->setStyleSheet("color: #15803D;");
    });
}
>>>>>>> Stashed changes
