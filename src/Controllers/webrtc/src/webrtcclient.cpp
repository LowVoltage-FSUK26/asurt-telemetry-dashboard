#include "../include/webrtcclient.h"
#include <gst/sdp/sdp.h>
#include <QJsonDocument>
#include <QJsonObject>


webrtcclient::webrtcclient(QObject *parent) : QObject(parent) {}

void webrtcclient::startPipeline(const QString &ip) {
    gst_init(nullptr, nullptr);

    GError *err = nullptr;
    pipeline = gst_parse_launch(
        "webrtcbin name=recv bundle-policy=max-bundle "
        "autoaudiosrc ! audioconvert ! audioresample ! valve name=ptt_valve drop=true ! opusenc ! rtpopuspay pt=97 ! recv.",
        &err);

    if (err) {
        qDebug() << "[WebRTC] CRITICAL: Failed to parse outgoing pipeline:" << err->message;
        g_error_free(err);
        return;
    }

    webrtcbin = gst_bin_get_by_name(GST_BIN(pipeline), "recv");
    pttValve = gst_bin_get_by_name(GST_BIN(pipeline), "ptt_valve");

    GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
    gst_bus_add_signal_watch(bus);
    gst_object_unref(bus);


    g_signal_connect(webrtcbin, "on-negotiation-needed", G_CALLBACK(onNegotiationNeeded), this);
    g_signal_connect(webrtcbin, "on-ice-candidate", G_CALLBACK(onIceCandidate), this);
    g_signal_connect(webrtcbin, "pad-added", G_CALLBACK(onPadAdded), this);

    g_signal_connect(webrtcbin, "notify::ice-connection-state",
                     G_CALLBACK(+[](GstElement *webrtc, GParamSpec*, gpointer) {
                         GstWebRTCICEConnectionState state;
                         g_object_get(webrtc, "ice-connection-state", &state, nullptr);
                         qDebug() << "[WebRTC] ICE state changed:" << state;
                     }), this);

    g_signal_connect(webrtcbin, "notify::connection-state",
                     G_CALLBACK(+[](GstElement *webrtc, GParamSpec*, gpointer) {
                         GstWebRTCPeerConnectionState state;
                         g_object_get(webrtc, "connection-state", &state, nullptr);
                         qDebug() << "[WebRTC] Connection state changed:" << state;
                     }), this);

    signalingSocket = new QWebSocket();
    connect(signalingSocket, &QWebSocket::textMessageReceived,
            this, [this](const QString &msg) {
                qDebug() << "[WebRTC] Raw message:" << msg;
                QJsonDocument outerDoc = QJsonDocument::fromJson(msg.toUtf8());
                QJsonObject json;

                if (outerDoc.isObject()) {
                    json = outerDoc.object();
                }
                else {
                    QJsonValue val(outerDoc.toVariant().toString());
                    if (!val.toString().isEmpty()) {
                        json = QJsonDocument::fromJson(val.toString().toUtf8()).object();
                    }
                }

                if (json.isEmpty()) {
                    qDebug() << "[WebRTC] ERROR: Failed to parse signaling message";
                    return;
                }
                if (json.contains("sdp")) {
                    GstSDPMessage *sdp;
                    QByteArray sdpBytes = json["sdp"].toObject()["sdp"].toString().toUtf8();
                    gst_sdp_message_new_from_text(sdpBytes.data(), &sdp);
                    GstWebRTCSessionDescription *desc =
                        gst_webrtc_session_description_new(GST_WEBRTC_SDP_TYPE_OFFER, sdp);

                    g_signal_emit_by_name(webrtcbin, "set-remote-description", desc, nullptr);
                    gst_webrtc_session_description_free(desc);

                    qDebug() << "[WebRTC] SDP received, type:" << json["sdp"].toObject()["type"].toString();

                    GstPromise *promise = gst_promise_new_with_change_func(onAnswerCreated, this, nullptr);
                    g_signal_emit_by_name(webrtcbin, "create-answer", nullptr, promise);
                }
                if (json.contains("ice")) {
                    QJsonObject ice = json["ice"].toObject();
                    QString candidate = ice["candidate"].toString();

                    if (candidate.isEmpty()) {
                        qDebug() << "[WebRTC] ICE gathering complete (Empty candidate ignored).";
                        return;
                    }

                    g_signal_emit_by_name(webrtcbin, "add-ice-candidate",
                                          ice["sdpMLineIndex"].toInt(),
                                          ice["candidate"].toString().toUtf8().data());
                }
            });
    signalingSocket->open(QUrl(QString("ws://%1:8080").arg(ip)));

    gst_element_set_state(pipeline, GST_STATE_PLAYING);
}

void webrtcclient::onPadAdded(GstElement*, GstPad *pad, gpointer data) {
    auto *self = static_cast<webrtcclient*>(data);
    GstCaps *caps = gst_pad_get_current_caps(pad);
    if (!caps) return;

    // 1. Dig deeper into the caps to extract the actual media type
    GstStructure *structure = gst_caps_get_structure(caps, 0);
    const gchar *name = gst_structure_get_name(structure);
    const gchar *media = gst_structure_get_string(structure, "media");

    qDebug() << "[WebRTC] Incoming stream detected! Name:" << name << "| Media:" << media;

    if (g_strcmp0(media, "video") == 0) {
        GError *err = nullptr;

        self->decode = gst_parse_bin_from_description(
            "rtph264depay ! h264parse ! avdec_h264 ! videoconvert ! video/x-raw,format=RGBA ! "
            "appsink name=mysink emit-signals=true drop=true max-buffers=1 sync=false",
            TRUE, &err);

        if (err) {
            qDebug() << "[WebRTC] Failed to create decode bin:" << err->message;
            g_error_free(err);
            gst_caps_unref(caps);
            return;
        }

        GstElement *appsink = gst_bin_get_by_name(GST_BIN(self->decode), "mysink");
        if (appsink) {
            g_signal_connect(appsink, "new-sample", G_CALLBACK(onNewSample), self);
            gst_object_unref(appsink);
        }

        gst_bin_add(GST_BIN(self->pipeline), self->decode);
        GstPad *sinkpad = gst_element_get_static_pad(self->decode, "sink");
        gst_pad_link(pad, sinkpad);
        gst_object_unref(sinkpad);

        gst_element_sync_state_with_parent(self->decode);

        QMetaObject::invokeMethod(self, [self]() {
            self->m_isStreaming = true;
            emit self->streamConnected();
        }, Qt::QueuedConnection);

    } else if (g_strcmp0(media, "audio") == 0) {
        qDebug() << "[WebRTC] Incoming stream detected! Media: audio. Linking to laptop speakers...";

        GError *err = nullptr;
        GstElement *audiodecode = gst_parse_bin_from_description(
            "rtpopusdepay ! opusdec ! audioconvert ! autoaudiosink", TRUE, &err);

        if (!err) {
            gst_bin_add(GST_BIN(self->pipeline), audiodecode);
            GstPad *sinkpad = gst_element_get_static_pad(audiodecode, "sink");

            if (gst_pad_link(pad, sinkpad) == GST_PAD_LINK_OK) {
                qDebug() << "[WebRTC] Audio pipeline successfully linked!";
            } else {
                qDebug() << "[WebRTC] CRITICAL ERROR: Failed to link audio pad!";
            }

            gst_object_unref(sinkpad);
            gst_element_sync_state_with_parent(audiodecode);
        } else {
            qDebug() << "[WebRTC] Audio Decode Error:" << err->message;
            g_error_free(err);
        }
    }

    gst_caps_unref(caps);
}

void webrtcclient::onNegotiationNeeded(GstElement *webrtc, gpointer user_data) {
    Q_UNUSED(webrtc); Q_UNUSED(user_data);
}

void webrtcclient::onIceCandidate(GstElement*, guint mline, gchar *candidate, gpointer user_data) {
    auto *self = static_cast<webrtcclient*>(user_data);

    QJsonObject ice;
    ice["candidate"]    = QString(candidate);
    ice["sdpMLineIndex"] = (int)mline;
    QJsonObject msg;
    msg["ice"] = ice;

    QString payload = QString::fromUtf8(QJsonDocument(msg).toJson(QJsonDocument::Compact));

    QMetaObject::invokeMethod(self, [self, payload]() {
        if (self->signalingSocket && self->signalingSocket->isValid()) {
            self->signalingSocket->sendTextMessage(payload);
        }
    }, Qt::QueuedConnection);
}

void webrtcclient::onAnswerCreated(GstPromise *promise, gpointer user_data) {
    auto *self = static_cast<webrtcclient*>(user_data);

    const GstStructure *reply = gst_promise_get_reply(promise);
    GstWebRTCSessionDescription *answer = nullptr;
    gst_structure_get(reply, "answer", GST_TYPE_WEBRTC_SESSION_DESCRIPTION, &answer, nullptr);
    gst_promise_unref(promise);

    if (!answer) {
        qDebug() << "[WebRTC] ERROR: Failed to create SDP Answer!";
        return;
    }

    g_signal_emit_by_name(self->webrtcbin, "set-local-description", answer, nullptr);

    gchar *sdp_str = gst_sdp_message_as_text(answer->sdp);
    QJsonObject sdpJson;
    sdpJson["type"] = "answer";
    sdpJson["sdp"] = QString(sdp_str);

    QJsonObject msg;
    msg["sdp"] = sdpJson;

    QString payload = QString::fromUtf8(QJsonDocument(msg).toJson(QJsonDocument::Compact));

    QMetaObject::invokeMethod(self, [self, payload]() {
        if (self->signalingSocket && self->signalingSocket->isValid()) {
            self->signalingSocket->sendTextMessage(payload);
            qDebug() << "[WebRTC] Thread-safe SDP Answer sent to signaling server.";
        }
    }, Qt::QueuedConnection);

    g_free(sdp_str);
    gst_webrtc_session_description_free(answer);
}
void webrtcclient::setVideoSink(QVideoSink *sink) {
    videoSink = sink;
    if (!decode) return;

    GstElement *glsink = gst_bin_get_by_name(GST_BIN(decode), "glsink");
    if (glsink) {
        g_object_set(glsink, "video-sink", sink, nullptr);
        gst_object_unref(glsink);
    }
}

GstFlowReturn webrtcclient::onNewSample(GstElement *sink, gpointer user_data) {
    auto *self = static_cast<webrtcclient*>(user_data);

    if (!self->videoSink) return GST_FLOW_OK;

    GstSample *sample = nullptr;
    g_signal_emit_by_name(sink, "pull-sample", &sample);
    if (!sample) return GST_FLOW_OK;

    GstCaps *caps = gst_sample_get_caps(sample);
    GstStructure *s = gst_caps_get_structure(caps, 0);
    int width, height;
    gst_structure_get_int(s, "width", &width);
    gst_structure_get_int(s, "height", &height);

    GstBuffer *buffer = gst_sample_get_buffer(sample);
    GstMapInfo map;
    gst_buffer_map(buffer, &map, GST_MAP_READ);

    // qDebug() << "[WebRTC] Frame Received! Width:" << width << "Height:" << height;

    QVideoFrameFormat format(QSize(width, height), QVideoFrameFormat::Format_RGBA8888);
    QVideoFrame frame(format);

    if (frame.map(QVideoFrame::WriteOnly)) {
        int gstStride = width * 4;
        int qtStride = frame.bytesPerLine(0);

        uchar *dst = frame.bits(0);
        uchar *src = map.data;

        for (int y = 0; y < height; ++y) {
            memcpy(dst, src, gstStride);
            dst += qtStride;
            src += gstStride;
        }
        frame.unmap();

        QMetaObject::invokeMethod(self, [self, frame]() {
            if (self->videoSink) {
                self->videoSink->setVideoFrame(frame);
            }
        }, Qt::QueuedConnection);
    }

    gst_buffer_unmap(buffer, &map);
    gst_sample_unref(sample);

    return GST_FLOW_OK;
}

void webrtcclient::setPushToTalk(bool active) {
    if (pttValve) {
        g_object_set(pttValve, "drop", !active, nullptr);
        // qDebug() << "[WebRTC] Push-to-Talk Active:" << active;
    } else {
        qDebug() << "[WebRTC] ERROR: PTT Valve not found!";
    }
}
