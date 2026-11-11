#ifndef CAMERADIALOG_H
#define CAMERADIALOG_H

#include <QDialog>
#include <QImage>

class QCamera;
class QImageCapture;
class QLabel;
class QMediaCaptureSession;
class QPushButton;
class QVideoWidget;

// Shows the live camera preview and a Capture button.
// Usage (in the Read tab):
//     CameraDialog dialog(this);
//     if (dialog.exec() == QDialog::Accepted) {
//         QImage image = dialog.capturedImage();   // send this to the same code as "Upload Image"
//     }
class CameraDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CameraDialog(QWidget *parent = nullptr);
    ~CameraDialog() override;

    // The captured picture. Empty if the user closed the dialog without capturing.
    QImage capturedImage() const;

private:
    QCamera *m_camera = nullptr;
    QImageCapture *m_capture = nullptr;
    QMediaCaptureSession *m_session = nullptr;
    QVideoWidget *m_video = nullptr;
    QPushButton *m_captureButton = nullptr;
    QLabel *m_status = nullptr;
    QImage m_image;
};

#endif // CAMERADIALOG_H
