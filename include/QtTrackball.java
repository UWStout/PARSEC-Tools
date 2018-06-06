package edu.uwstout.berriers.PSHelper.UILogic;

import com.trolltech.qt.core.QPointF;
import com.trolltech.qt.core.QTime;
import com.trolltech.qt.gui.QLineF;
import com.trolltech.qt.gui.QMatrix4x4;
import com.trolltech.qt.gui.QQuaternion;
import com.trolltech.qt.gui.QVector3D;

public class QtTrackball
{
    public enum TrackMode
    {
        TRACKMODE_PLANE,
        TRACKMODE_SPHERE
    }

    public QtTrackball() { this(TrackMode.TRACKMODE_SPHERE); }
    public QtTrackball(TrackMode mode) { this(0, true, new QVector3D(0.0f, 1.0f, 0.0f), mode); }
    public QtTrackball(float angularVelocity, boolean retainMomentum, QVector3D axis)
    {
    	this(angularVelocity, retainMomentum, axis, TrackMode.TRACKMODE_SPHERE);
    }
    
    public QtTrackball(float angularVelocity, boolean retainMomentum, QVector3D axis, TrackMode mode)
    {
        m_angularVelocity = angularVelocity;
        m_retainMomentum = retainMomentum;
        m_paused = false;
        m_pressed = false;
        m_mode = mode;
        m_axis = axis;
        m_rotation = new QQuaternion();
        m_lastTime = QTime.currentTime();    	
    }

    public void push(QPointF p, QQuaternion transformation)
    {
		m_rotation = rotation();
		m_pressed = true;
		m_lastTime = QTime.currentTime();
		m_lastPos = p;
		m_angularVelocity = 0.0;
    }
    	
    public void push(QPointF p, QMatrix4x4 transformation)
    {
		m_rotation = rotation();
		m_pressed = true;
		m_lastTime = QTime.currentTime();
		m_lastPos = p;
		m_angularVelocity = 0.0;
    }
    
    public void move(QPointF p, QQuaternion transformation)
    {
        if (!m_pressed) { return; }

        QTime currentTime = QTime.currentTime();
        int msecs = m_lastTime.msecsTo(currentTime);
        if (msecs <= 20) { return; }

        switch (m_mode)
        {
	        case TRACKMODE_PLANE:
	        {
	            QLineF delta = new QLineF(m_lastPos, p);
	            m_angularVelocity = 180*delta.length() / (Math.PI*msecs);
	            m_axis = (new QVector3D(-delta.dy(), delta.dx(), 0.0f)).normalized();
	            m_axis = transformation.rotatedVector(m_axis);
	            m_rotation.operator_multiply_assign(QQuaternion.fromAxisAndAngle(m_axis, 180 / Math.PI * delta.length()));
	        }
	        break;
	        
	        case TRACKMODE_SPHERE:
	        {
	            QVector3D lastPos3D = new QVector3D(m_lastPos.x(), m_lastPos.y(), 0.0f);
	            double sqrZ = 1 - QVector3D.dotProduct(lastPos3D, lastPos3D);
	            if (sqrZ > 0) { lastPos3D.setZ(Math.sqrt(sqrZ)); }
	            else { lastPos3D.normalize(); }
	
	            QVector3D currentPos3D = new QVector3D(p.x(), p.y(), 0.0f);
	            sqrZ = 1 - QVector3D.dotProduct(currentPos3D, currentPos3D);
	            if (sqrZ > 0) { currentPos3D.setZ(Math.sqrt(sqrZ)); }
	            else { currentPos3D.normalize(); }
	
	            m_axis = QVector3D.crossProduct(lastPos3D, currentPos3D);
	            double angle = 180 / Math.PI * Math.asin(Math.sqrt(QVector3D.dotProduct(m_axis, m_axis)));
	
	            m_angularVelocity = angle / msecs;
	            m_axis.normalize();
	            m_axis = transformation.rotatedVector(m_axis);
	            m_rotation.operator_multiply_assign(QQuaternion.fromAxisAndAngle(m_axis, angle));
	        }
	        break;
        }

        m_lastPos = p;
        m_lastTime = currentTime;      
    }
    
    public void move(QPointF p, QMatrix4x4 transformation)
    {
    	
        if (!m_pressed) { return; }

        QTime currentTime = QTime.currentTime();
        int msecs = m_lastTime.msecsTo(currentTime);
        if (msecs <= 20) { return; }

        switch (m_mode)
        {
	        case TRACKMODE_PLANE:
            {
                QLineF delta = new QLineF(m_lastPos, p);
                m_angularVelocity = 180*delta.length() / (Math.PI*msecs);
                m_axis = new QVector3D(-delta.dy(), delta.dx(), 0.0f).normalized();
                m_axis = transformation.map(m_axis);
                m_rotation.operator_multiply_assign(QQuaternion.fromAxisAndAngle(m_axis, 180 / Math.PI * delta.length()));
            }
            break;
            
	        case TRACKMODE_SPHERE:
            {
                QVector3D lastPos3D = new QVector3D(m_lastPos.x(), m_lastPos.y(), 0.0f);
                double sqrZ = 1 - QVector3D.dotProduct(lastPos3D, lastPos3D);
                if (sqrZ > 0) { lastPos3D.setZ(Math.sqrt(sqrZ)); }
                else { lastPos3D.normalize(); }

                QVector3D currentPos3D = new QVector3D(p.x(), p.y(), 0.0f);
                sqrZ = 1 - QVector3D.dotProduct(currentPos3D, currentPos3D);
                if (sqrZ > 0) { currentPos3D.setZ(Math.sqrt(sqrZ)); }
                else { currentPos3D.normalize(); } 

                m_axis = QVector3D.crossProduct(lastPos3D, currentPos3D);
                double angle = 180 / Math.PI * Math.asin(Math.sqrt(QVector3D.dotProduct(m_axis, m_axis)));

                m_angularVelocity = angle / msecs;
                m_axis.normalize();
                m_axis = transformation.map(m_axis);
                m_rotation.operator_multiply_assign(QQuaternion.fromAxisAndAngle(m_axis, angle));
            }
            break;
        }

        m_lastPos = p;
        m_lastTime = currentTime;
    }
    
    public void release(QPointF p, QQuaternion transformation)
    {
        // Calling move() caused the rotation to stop if the framerate was too low.
        move(p, transformation);
        if (!m_retainMomentum) { m_angularVelocity = 0.0f; }
        m_pressed = false;
    }
    
    public void release(QPointF p, QMatrix4x4 transformation)
    {
        // Calling move() caused the rotation to stop if the framerate was too low.
        move(p, transformation);
        if (!m_retainMomentum) m_angularVelocity = 0.0f;
        m_pressed = false;
    }

    // Remove accumulated rotation (also freezes)
    void clearRotation()
    {
        m_angularVelocity = 0.0f;
        m_pressed = false;
        m_axis = new QVector3D(0, 1, 0);
        m_rotation = new QQuaternion();
        m_lastPos = new QPointF();    	
    }
    
    // Reset to a specific rotation (also freezes)
    void setRotation(QVector3D pAxis, float pAngle)
    {
        m_angularVelocity = 0.0f;
        m_axis = pAxis;
        m_rotation = QQuaternion.fromAxisAndAngle(m_axis, pAngle);    	
    }

    // Add a specific rotation (also freezes)    
    void addRotation(QVector3D pAxis, float pAngle)
    {
        m_angularVelocity = 0.0f;
        m_axis = pAxis;
        m_rotation.operator_multiply_assign(QQuaternion.fromAxisAndAngle(m_axis, pAngle));
    }
    
    void setRotation(QtTrackball trackball)
    {
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
    void nudgeX(float delta)
    {
        boolean lOldPressed = m_pressed;
        QPointF lOldPos = m_lastPos;
        QTime lOldTime = m_lastTime;
        double lOldVel = m_angularVelocity;

        m_pressed = true;
        m_lastPos = new QPointF(0.0, 0.0);
        m_lastTime = m_lastTime.addMSecs(100); // bypass silly time check

        move(new QPointF(delta, 0), new QQuaternion());

        m_angularVelocity = lOldVel;
        m_pressed = lOldPressed;
        m_lastPos = lOldPos;
        m_lastTime = lOldTime;
    }

    // simulate a small plus/minus movement of mouse in y axis
    void nudgeY(float delta)
    {
        boolean lOldPressed = m_pressed;
        QPointF lOldPos = m_lastPos;
        QTime lOldTime = m_lastTime;
        double lOldVel = m_angularVelocity;

        m_pressed = true;
        m_lastPos = new QPointF(0.0, 0.0);
        m_lastTime = m_lastTime.addMSecs(100); // bypass silly time check
        move(new QPointF(0, delta), new QQuaternion());

        m_angularVelocity = lOldVel;
        m_pressed = lOldPressed;
        m_lastPos = lOldPos;
        m_lastTime = lOldTime;
    }

    // Set's angular velocity to zero
    void freeze()
    {
        m_angularVelocity = 0.0f;    	
    }
    
    // starts clock
    void start()
    {
        m_lastTime = QTime.currentTime();
        m_paused = false;    	
    }

    // stops clock
    void stop()
    {
        m_rotation = rotation();
        m_paused = true;    	
    }

    QQuaternion rotation()
    {
        if (m_paused || m_pressed) { return m_rotation; }

        QTime currentTime = QTime.currentTime();
        double angle = m_angularVelocity * m_lastTime.msecsTo(currentTime);
        
        QQuaternion result = m_rotation.clone();
        result.operator_multiply_assign(QQuaternion.fromAxisAndAngle(m_axis, angle));
        return result;
    }

    private QQuaternion m_rotation;
    private QVector3D m_axis;
    private double m_angularVelocity;
    private boolean m_retainMomentum;

    private QPointF m_lastPos;
    private QTime m_lastTime;
    private boolean m_paused;
    private boolean m_pressed;
    private TrackMode m_mode;
}
