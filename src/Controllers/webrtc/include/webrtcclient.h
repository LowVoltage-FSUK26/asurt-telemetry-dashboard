#ifndef WEBRTCCLIENT_H
#define WEBRTCCLIENT_H

#include <gst/gst.h>
#include <gst/webrtc/webrtc.h>
#include <QObject>
#include <QVideoSink>
#include <QVideoFrame>
#include <QWebSocket>
#include <QVideoFrameFormat>
#include <QSize>

class webrtcclient : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool isStreaming READ isStreaming NOTIFY streamConnected)
public:
    explicit webrtcclient(QObject *parent = nullptr);
    Q_INVOKABLE void startPipeline(const QString &tailscaleIP);
    Q_INVOKABLE void setVideoSink(QVideoSink *sink);
    bool isStreaming() const { return m_isStreaming; }
    Q_INVOKABLE void setPushToTalk(bool active);

signals:
    void streamConnected();
    void streamLost();
    void frameReady(QVideoFrame frame);

private:
    GstElement *pipeline = nullptr;
    GstElement *webrtcbin = nullptr;
    QWebSocket *signalingSocket = nullptr;
    GstElement *decode     = nullptr;
    QVideoSink *videoSink  = nullptr;
    GstElement *pttValve = nullptr;
    bool m_isStreaming = false;

    static void onNegotiationNeeded(GstElement *webrtc, gpointer user_data);
    static void onIceCandidate(GstElement *webrtc, guint mline,
                               gchar *candidate, gpointer user_data);
    static void onPadAdded(GstElement *webrtc, GstPad *pad, gpointer user_data);
    static void onAnswerCreated(GstPromise *promise, gpointer user_data);
    static GstFlowReturn onNewSample(GstElement *sink, gpointer user_data);

};

#endif // WEBRTCCLIENT_H
