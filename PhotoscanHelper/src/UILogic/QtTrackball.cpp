#include <cmath>

#include "QtTrackball.h"

#include <QMatrix4x4>
#include <QLineF>

QtTrackball::QtTrackball(TrackMode mode, float angularVelocity, bool retainMomentum, QVector3D axis) {
    initMembers(mode, angularVelocity, retainMomentum, axis);
}

QtTrackball::QtTrackball(float angularVelocity, bool retainMomentum, QVector3D axis) {
    initMembers(TRACKMODE_SPHERE, angularVelocity, retainMomentum, axis);
}

void QtTrackball::initMembers(TrackMode mode, float angularVelocity, bool retainMomentum, QVector3D axis) {
    m_angularVelocity = angularVelocity;
    m_retainMomentum = retainMomentum;
    m_paused = false;
    m_pressed = false;
    m_mode = mode;
    m_axis = axis;
    m_rotation = QQuaternion();
    m_lastTime = QTime::currentTime();
}

void QtTrackball::push(const QPointF& p) { //, const QQuaternion& transformation) {
    m_rotation = rotation();
    m_pressed = true;
    m_lastTime = QTime::currentTime();
    m_lastPos = p;
    m_angularVelocity = 0.0;
}

void QtTrackball::move(const QPointF& p, const QQuaternion& transformation) {
    if (!m_pressed) { return; }

    QTime currentTime = QTime::currentTime();
    int msecs = m_lastTime.msecsTo(currentTime);
    if (msecs <= 20) { return; }

    switch (m_mode)
    {
        case TRACKMODE_PLANE:
        {
            QLineF delta(m_lastPos, p);
            m_angularVelocity = 180*delta.length() / (M_PI*msecs);
            m_axis = QVector3D(-delta.dy(), delta.dx(), 0.0f).normalized();
            m_axis = transformation.rotatedVector(m_axis);
            m_rotation *= QQuaternion::fromAxisAndAngle(m_axis, 180 / M_PI * delta.length());
        }
        break;

        case TRACKMODE_SPHERE:
        {
            QVector3D lastPos3D(m_lastPos.x(), m_lastPos.y(), 0.0f);
            double sqrZ = 1 - QVector3D::dotProduct(lastPos3D, lastPos3D);
            if (sqrZ > 0) { lastPos3D.setZ(sqrt(sqrZ)); }
            else { lastPos3D.normalize(); }

            QVector3D currentPos3D(p.x(), p.y(), 0.0f);
            sqrZ = 1 - QVector3D::dotProduct(currentPos3D, currentPos3D);
            if (sqrZ > 0) { currentPos3D.setZ(sqrt(sqrZ)); }
            else { currentPos3D.normalize(); }

            m_axis = QVector3D::crossProduct(lastPos3D, currentPos3D);
            double angle = 180 / M_PI * asin(sqrt(QVector3D::dotProduct(m_axis, m_axis)));

            m_angularVelocity = angle / msecs;
            m_axis.normalize();
            m_axis = transformation.rotatedVector(m_axis);
            m_rotation *= QQuaternion::fromAxisAndAngle(m_axis, angle);
        }
        break;
    }

    m_lastPos = p;
    m_lastTime = currentTime;
}

void QtTrackball::move(const QPointF& p, const QMatrix4x4& transformation) {
    if (!m_pressed) { return; }

    QTime currentTime = QTime::currentTime();
    int msecs = m_lastTime.msecsTo(currentTime);
    if (msecs <= 20) { return; }

    switch (m_mode)
    {
        case TRACKMODE_PLANE:
        {
            QLineF delta(m_lastPos, p);
            m_angularVelocity = 180*delta.length() / (M_PI*msecs);
            m_axis = QVector3D(-delta.dy(), delta.dx(), 0.0f).normalized();
            m_axis = transformation.map(m_axis);
            m_rotation *= QQuaternion::fromAxisAndAngle(m_axis, 180 / M_PI * delta.length());
        }
        break;

        case TRACKMODE_SPHERE:
        {
            QVector3D lastPos3D(m_lastPos.x(), m_lastPos.y(), 0.0f);
            double sqrZ = 1 - QVector3D::dotProduct(lastPos3D, lastPos3D);
            if (sqrZ > 0) { lastPos3D.setZ(sqrt(sqrZ)); }
            else { lastPos3D.normalize(); }

            QVector3D currentPos3D(p.x(), p.y(), 0.0f);
            sqrZ = 1 - QVector3D::dotProduct(currentPos3D, currentPos3D);
            if (sqrZ > 0) { currentPos3D.setZ(sqrt(sqrZ)); }
            else { currentPos3D.normalize(); }

            m_axis = QVector3D::crossProduct(lastPos3D, currentPos3D);
            double angle = 180 / M_PI * asin(sqrt(QVector3D::dotProduct(m_axis, m_axis)));

            m_angularVelocity = angle / msecs;
            m_axis.normalize();
            m_axis = transformation.map(m_axis);
            m_rotation *= QQuaternion::fromAxisAndAngle(m_axis, angle);
        }
        break;
    }

    m_lastPos = p;
    m_lastTime = currentTime;
}

void QtTrackball::release(const QPointF& p, const QQuaternion& transformation) {
    // Calling move() caused the rotation to stop if the framerate was too low.
    move(p, transformation);
    if (!m_retainMomentum) { m_angularVelocity = 0.0f; }
    m_pressed = false;
}

void QtTrackball::release(const QPointF& p, const QMatrix4x4& transformation) {
    // Calling move() caused the rotation to stop if the framerate was too low.
    move(p, transformation);
    if (!m_retainMomentum) m_angularVelocity = 0.0f;
    m_pressed = false;
}

// Remove accumulated rotation (also freezes)
void QtTrackball::clearRotation() {
    m_angularVelocity = 0.0f;
    m_pressed = false;
    m_axis = QVector3D(0, 1, 0);
    m_rotation = QQuaternion();
    m_lastPos = QPointF();
}

// Reset to a specific rotation (also freezes)
void QtTrackball::setRotation(const QVector3D& pAxis, float pAngle) {
    m_angularVelocity = 0.0f;
    m_axis = pAxis;
    m_rotation = QQuaternion::fromAxisAndAngle(m_axis, pAngle);
}

// Add a specific rotation (also freezes)
void QtTrackball::addRotation(const QVector3D& pAxis, float pAngle) {
    m_angularVelocity = 0.0f;
    m_axis = pAxis;
    m_rotation *= QQuaternion::fromAxisAndAngle(m_axis, pAngle);
}

void QtTrackball::setRotation(QtTrackball trackball) {
    m_angularVelocity = trackball.m_angularVelocity;
    m_axis = trackball.m_axis;
    m_rotation = trackball.m_rotation;
    m_retainMomentum = trackball.m_retainMomentum;
    m_lastPos = trackball.m_lastPos;
    m_lastTime = trackball.m_lastTime;
    m_paused = trackball.m_paused;
    m_pressed = trackball.m_pressed;
    m_mode = trackball.m_mode;
}

// simulate a small plus/minus movement of mouse in x axis
void QtTrackball::nudgeX(float delta)
{
    bool lOldPressed = m_pressed;
    QPointF lOldPos = m_lastPos;
    QTime lOldTime = m_lastTime;
    double lOldVel = m_angularVelocity;

    m_pressed = true;
    m_lastPos = QPointF(0.0, 0.0);
    m_lastTime = m_lastTime.addMSecs(100); // bypass silly time check

    move(QPointF(delta, 0), QQuaternion());

    m_angularVelocity = lOldVel;
    m_pressed = lOldPressed;
    m_lastPos = lOldPos;
    m_lastTime = lOldTime;
}

// simulate a small plus/minus movement of mouse in y axis
void QtTrackball::nudgeY(float delta)
{
    bool lOldPressed = m_pressed;
    QPointF lOldPos = m_lastPos;
    QTime lOldTime = m_lastTime;
    double lOldVel = m_angularVelocity;

    m_pressed = true;
    m_lastPos = QPointF(0.0, 0.0);
    m_lastTime = m_lastTime.addMSecs(100); // bypass silly time check
    move(QPointF(0, delta), QQuaternion());

    m_angularVelocity = lOldVel;
    m_pressed = lOldPressed;
    m_lastPos = lOldPos;
    m_lastTime = lOldTime;
}

// Set's angular velocity to zero
void QtTrackball::freeze()
{
    m_angularVelocity = 0.0f;
}

// starts clock
void QtTrackball::start()
{
    m_lastTime = QTime::currentTime();
    m_paused = false;
}

// stops clock
void QtTrackball::stop()
{
    m_rotation = rotation();
    m_paused = true;
}

QQuaternion QtTrackball::rotation()
{
    if (m_paused || m_pressed) { return m_rotation; }

    QTime currentTime = QTime::currentTime();
    double angle = m_angularVelocity * m_lastTime.msecsTo(currentTime);

    QQuaternion result = m_rotation;
    result *= QQuaternion::fromAxisAndAngle(m_axis, angle);
    return result;
}
