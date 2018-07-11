#ifndef QT_TRACKBALL_H
#define QT_TRACKBALL_H

#include <QPointF>
#include <QTime>
#include <QQuaternion>
#include <QVector3D>

class QtTrackball {
public:
    enum TrackMode {
        TRACKMODE_PLANE,
        TRACKMODE_SPHERE
    };

    QtTrackball(TrackMode mode = TRACKMODE_SPHERE, float angularVelocity = 0,
                bool retainMomentum = true, QVector3D axis = QVector3D(0.0f, 1.0f, 0.0f));
    QtTrackball(float angularVelocity, bool retainMomentum = true, QVector3D axis = QVector3D(0.0f, 1.0f, 0.0f));

    void push(const QPointF& p);
    void move(const QPointF& p, const QQuaternion& transformation);
    void move(const QPointF& p, const QMatrix4x4& transformation);
    void release(const QPointF& p, const QQuaternion& transformation);
    void release(const QPointF& p, const QMatrix4x4& transformation);

    // Remove accumulated rotation (also freezes)
    void clearRotation();
    
    // Reset to a specific rotation (also freezes)
    void setRotation(const QVector3D& pAxis, float pAngle);

    // Add a specific rotation (also freezes)    
    void addRotation(const QVector3D& pAxis, float pAngle);
    
    void setRotation(QtTrackball trackball);

    // simulate a small plus/minus movement of mouse in x axis
    void nudgeX(float delta);

    // simulate a small plus/minus movement of mouse in y axis
    void nudgeY(float delta);

    // Set's angular velocity to zero
    void freeze();
    
    // starts clock
    void start();

    // stops clock
    void stop();

    QQuaternion rotation();

private:
    void initMembers(TrackMode mode, float angularVelocity, bool retainMomentum, QVector3D axis);

    QQuaternion m_rotation;
    QVector3D m_axis;
    double m_angularVelocity;
    bool m_retainMomentum;

    QPointF m_lastPos;
    QTime m_lastTime;
    bool m_paused;
    bool m_pressed;
    TrackMode m_mode;
};

#endif
