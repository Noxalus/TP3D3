#pragma once

class Timer
{

public:
	Timer(void);
	~Timer(void);

	void Start();
	void Stop();
	void Update();

	bool Stopped;
	float FPS;
	float ElapsedTime;
	float RunningTime;

private:
	long m_ticksPerSecond; 
	long m_currentTime; 
	long m_lastTime; 
	long m_lastFPSUpdate; 
	long m_FPSUpdateInterval; 
	unsigned int m_numFrames; 
	float m_runningTime; 
	float m_timeElapsed; 
	float m_fps; 
	bool m_timerStopped; 
};

