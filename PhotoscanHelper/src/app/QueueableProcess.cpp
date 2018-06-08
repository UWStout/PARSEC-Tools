package edu.uwstout.berriers.PSHelper.app;

import com.trolltech.qt.core.QFuture;

public interface QueueableProcess<T> {
	
	String describeProcess();
	QFuture<T> runProcess();

}
