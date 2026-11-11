// #include "readtab.h"
// #include "cameradialog.h"
// #include "ImageScanner.h"
// #include "Barcode_Decoder.h"

// #include <QLabel>
// #include <QPushButton>
// #include <QVBoxLayout>
// #include <QHBoxLayout>
// #include <QFileDialog>
// #include <QPixmap>
// #include <QFileInfo>
// #include <QDir>
// #include <QImage>
// #include <QImageReader>

// #include <string>
// #include <vector>

// ReadTab::ReadTab(QWidget *parent)
//     : QWidget(parent)
// {
//     // Main layout
//     QVBoxLayout *mainLayout = new QVBoxLayout(this);

//     // Heading
//     QLabel *heading = new QLabel("Barcode Reader", this);
//     heading->setStyleSheet("font-size: 24px; font-weight: bold;");

//     // Instructions
//     QLabel *instructions = new QLabel(
//         "Upload an image containing a barcode.", this);

//     // Upload button
//     uploadButton = new QPushButton("Upload Image", this);

//     // Camera button
//     cameraButton = new QPushButton("Use Camera", this);

//     // Image preview area
//     imagePreview = new QLabel("No image selected", this);
//     imagePreview->setAlignment(Qt::AlignCenter);
//     imagePreview->setMinimumSize(400, 200);
//     imagePreview->setStyleSheet(
//         "border: 2px dashed gray;"
//         "background-color: #f5f5f5;"
//         );

//     // Filename
//     fileNameLabel = new QLabel("No file selected", this);

//     // Decode button
//     decodeButton = new QPushButton("Process / Decode", this);
//     decodeButton->setEnabled(false);

//     // Result section heading
//     resultHeading = new QLabel("Decoding Result", this);
//     resultHeading->setStyleSheet(
//         "font-size: 20px;"
//         "font-weight: bold;"
//         );

//     // Dedicated output area
//     resultLabel = new QLabel("------", this);
//     resultLabel->setAlignment(Qt::AlignCenter);
//     resultLabel->setMinimumHeight(75);
//     resultLabel->setStyleSheet(
//         "font-size: 30px;"
//         "font-weight: bold;"
//         "color: #16803c;"
//         "border: 2px solid #cccccc;"
//         "border-radius: 8px;"
//         "background-color: white;"
//         );

//     // Status message
//     messageLabel = new QLabel(
//         "Status: Waiting for barcode processing...", this
//         );

//     // Add widgets
//     mainLayout->addWidget(heading);
//     mainLayout->addWidget(instructions);
//     mainLayout->addWidget(uploadButton);
//     mainLayout->addWidget(cameraButton);
//     mainLayout->addWidget(imagePreview);
//     mainLayout->addWidget(fileNameLabel);
//     mainLayout->addWidget(decodeButton);
//     mainLayout->addSpacing(15);
//     mainLayout->addWidget(resultHeading);
//     mainLayout->addWidget(resultLabel);
//     mainLayout->addWidget(messageLabel);

//     mainLayout->addStretch();

//     // Upload image functionality
//     connect(uploadButton, &QPushButton::clicked, this, [this]() {

//         QString path = QFileDialog::getOpenFileName(
//             this,
//             "Select Barcode Image",
//             "",
//             "Image Files (*.png *.jpg *.jpeg *.bmp)"
//             );

//         // User cancelled the dialog
//         if (path.isEmpty())
//             return;

//         QPixmap image(path);

//         // Check whether image loaded successfully
//         if (image.isNull()) {
//             messageLabel->setText("Error: Unable to load image.");
//             decodeButton->setEnabled(false);
//             return;
//         }

//         // Store image path
//         imagePath = path;

//         // Display image
//         imagePreview->setPixmap(
//             image.scaled(
//                 imagePreview->size(),
//                 Qt::KeepAspectRatio,
//                 Qt::SmoothTransformation
//                 )
//             );

//         // Display filename
//         fileNameLabel->setText(
//             QFileInfo(path).fileName()
//             );

//         // Enable decoding
//         decodeButton->setEnabled(true);

//         // Clear previous messages/results
//         resultLabel->setText("------");
//         messageLabel->setText("Image uploaded successfully.");
//     });

//     // Use camera functionality
//     connect(cameraButton, &QPushButton::clicked, this, [this]() {
//         CameraDialog dialog(this);

//         // User closed the camera without capturing
//         if (dialog.exec() != QDialog::Accepted)
//             return;

//         QImage captured = dialog.capturedImage();
//         if (captured.isNull()) {
//             messageLabel->setText("Error: No picture was captured.");
//             return;
//         }

//         // Save to a temporary file so it follows the same path as an uploaded image
//         QString path = QDir::temp().filePath("camera_capture.png");
//         if (!captured.save(path)) {
//             messageLabel->setText("Error: Unable to save the captured picture.");
//             return;
//         }

//         // Store image path
//         imagePath = path;

//         // Display image
//         imagePreview->setPixmap(
//             QPixmap::fromImage(captured).scaled(
//                 imagePreview->size(),
//                 Qt::KeepAspectRatio,
//                 Qt::SmoothTransformation
//                 )
//             );

//         // Display filename
//         fileNameLabel->setText("Camera capture");

//         // Enable decoding
//         decodeButton->setEnabled(true);

//         // Clear previous messages/results
//         resultLabel->setText("------");
//         messageLabel->setText("Picture captured from the camera.");
//     });

//     // Decode functionality: image -> widths -> 6 digits -> result label
//     connect(decodeButton, &QPushButton::clicked, this, [this]() {

//         // Load image, applying EXIF rotation (important for phone photos)
//         QImageReader reader(imagePath);
//         reader.setAutoTransform(true);
//         QImage image = reader.read();

//         if (image.isNull()) {
//             resultLabel->setText("------");
//             messageLabel->setText("Error: Unable to read the image.");
//             return;
//         }

//         // Step 1: image -> bar/space widths (37 values, 68 modules total)
//         std::vector<int> widths = ImageScanner::getWidths(image);
//         if (widths.empty()) {
//             resultLabel->setText("------");
//             messageLabel->setText("No barcode found. Try a clearer, straighter image.");
//             return;
//         }

//         // Step 2: widths -> 6 digits
//         std::string digits = decodeWidths(widths);
//         if (digits.empty()) {
//             resultLabel->setText("------");
//             messageLabel->setText("Barcode found but could not be decoded (pattern/checksum error).");
//             return;
//         }

//         resultLabel->setText(QString::fromStdString(digits));
//         messageLabel->setText("Barcode decoded successfully.");
//     });
// }
#include "readtab.h"
#include "cameradialog.h"
#include "ImageScanner.h"
#include "Barcode_Decoder.h"
#include <QStandardPaths>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QPixmap>
#include <QFileInfo>
#include <QDir>
#include <QImage>
#include <QImageReader>

#include <string>
#include <vector>

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

    // Camera button
    cameraButton = new QPushButton("Use Camera", this);

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
    mainLayout->addWidget(cameraButton);
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

    // Use camera functionality
    connect(cameraButton, &QPushButton::clicked, this, [this]() {
        CameraDialog dialog(this);

        // User closed the camera without capturing
        if (dialog.exec() != QDialog::Accepted)
            return;

        QImage captured = dialog.capturedImage();
        if (captured.isNull()) {
            messageLabel->setText("Error: No picture was captured.");
            return;
        }

        // Save to a temporary file so it follows the same path as an uploaded image
        // Change this folder to wherever you want the captures saved
        QString folder = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)
                         + "/BarcodeReader";
        QDir().mkpath(folder);                         // create the folder if missing
        QString path = folder + "/camera_capture.png";
        if (!captured.save(path)) {
            messageLabel->setText("Error: Unable to save the captured picture.");
            return;
        }

        // Store image path
        imagePath = path;

        // Display image
        imagePreview->setPixmap(
            QPixmap::fromImage(captured).scaled(
                imagePreview->size(),
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation
                )
            );

        // Display filename
        fileNameLabel->setText(
            "Camera capture (saved as " + QDir::toNativeSeparators(path) + ")");

        // Enable decoding
        decodeButton->setEnabled(true);

        // Clear previous messages/results
        resultLabel->setText("------");
        messageLabel->setText("Picture captured. Press Process / Decode.");


    });

    // Decode functionality: image -> widths -> 6 digits -> result label
    connect(decodeButton, &QPushButton::clicked, this, [this]() {

        // Load image, applying EXIF rotation (important for phone photos)
        QImageReader reader(imagePath);
        reader.setAutoTransform(true);
        QImage image = reader.read();

        if (image.isNull()) {
            resultLabel->setText("------");
            messageLabel->setText("Error: Unable to read the image.");
            return;
        }

        // Step 1: image -> bar/space widths (37 values, 68 modules total)
        std::vector<int> widths = ImageScanner::getWidths(image);
        if (widths.empty()) {
            resultLabel->setText("------");
            messageLabel->setText("No barcode found. Try a clearer, straighter image.");
            return;
        }

        // Step 2: widths -> 6 digits
        std::string digits = decodeWidths(widths);
        if (digits.empty()) {
            resultLabel->setText("------");
            messageLabel->setText("Barcode found but could not be decoded (pattern/checksum error).");
            return;
        }

        resultLabel->setText(QString::fromStdString(digits));
        messageLabel->setText("Barcode decoded successfully.");
    });
}