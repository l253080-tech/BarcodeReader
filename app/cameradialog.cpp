#include "cameradialog.h"

#include <QCamera>
#include <QCameraDevice>
#include <QImageCapture>
#include <QLabel>
#include <QMediaCaptureSession>
#include <QMediaDevices>
#include <QPushButton>
#include <QVBoxLayout>
#include <QVideoWidget>

CameraDialog::CameraDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Camera");
    resize(640, 520);
//n
    auto *layout = new QVBoxLayout(this);

    m_status = new QLabel("Point the camera at the barcode and press Capture.", this);
    layout->addWidget(m_status);

    // No camera connected: show a clear message instead of crashing.
    const QCameraDevice device = QMediaDevices::defaultVideoInput();
    if (device.isNull()) {
        m_status->setText("No camera found. Connect a camera and try again.");
        auto *closeButton = new QPushButton("Close", this);
        connect(closeButton, &QPushButton::clicked, this, &QDialog::reject);
        layout->addWidget(closeButton);
        return;
    }

    m_video = new QVideoWidget(this);
    m_video->setMinimumSize(480, 360);
    layout->addWidget(m_video, 1);

    m_captureButton = new QPushButton("Capture", this);
    layout->addWidget(m_captureButton);

    m_camera = new QCamera(device, this);
    m_capture = new QImageCapture(this);
    m_session = new QMediaCaptureSession(this);
    m_session->setCamera(m_camera);
    m_session->setImageCapture(m_capture);
    m_session->setVideoOutput(m_video);

    connect(m_captureButton, &QPushButton::clicked, this, [this]() {
        m_capture->capture();
    });

    // A picture was captured: keep it and close the dialog with "accepted".
    connect(m_capture, &QImageCapture::imageCaptured, this,
            [this](int, const QImage &image) {
                m_image = image;
                accept();
            });

    connect(m_capture, &QImageCapture::errorOccurred, this,
            [this](int, QImageCapture::Error, const QString &message) {
                m_status->setText("Capture failed: " + message);
            });

    connect(m_camera, &QCamera::errorOccurred, this,
            [this](QCamera::Error, const QString &message) {
                m_status->setText("Camera error: " + message);
            });

    // Switch the camera off when the dialog closes.
    connect(this, &QDialog::finished, this, [this](int) {
        if (m_camera)
            m_camera->stop();
    });

    m_camera->start();
}

CameraDialog::~CameraDialog()
{
    if (m_camera)
        m_camera->stop();
}

QImage CameraDialog::capturedImage() const
{
    return m_image;
}
