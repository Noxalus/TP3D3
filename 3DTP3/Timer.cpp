#include "stdafx.h"
#include "Timer.h"


/// <summary>Creates a new Timer</summary> 
Timer::Timer()
{
	// Find the frequency, or amount of ticks per second 
	NativeMethods.QueryPerformanceFrequency( ref m_ticksPerSecond ); 

	m_timerStopped = true; 
	// Update the FPS every half second. 
	m_FPSUpdateInterval = m_ticksPerSecond >> 1; 
}


/// <summary>Starts the timer.</summary> 
void Timer::Start()
{
	if ( !Stopped ) 
	{
		return; 
	} 
	NativeMethods.QueryPerformanceCounter( ref m_lastTime ); 
	m_timerStopped = false; 
}


/// <summary>Stops the timer.</summary> 
void Timer::Stop()
{
	if ( Stopped ) 
	{
		return; 
	} 
	long stopTime = 0; 
	NativeMethods.QueryPerformanceCounter( ref stopTime ); 
	m_runningTime += (float)(stopTime – m_lastTime) / (float)m_ticksPerSecond; 
	m_timerStopped = true; 
}


/// <summary>Updates the timer.</summary> 
void Timer::Update()
{
	if ( Stopped ) 
	{
		return; 
	} 

	// Get the current time 
	NativeMethods.QueryPerformanceCounter( ref m_currentTime ); 

	// Update time elapsed since last frame 
	m_timeElapsed = (float)(m_currentTime – m_lastTime) / (float)m_ticksPerSecond; 
	m_runningTime += m_timeElapsed; 

	// Update FPS 
	m_numFrames++; 
	if ( m_currentTime – m_lastFPSUpdate >= m_FPSUpdateInterval ) 
	{
		float currentTime = (float)m_currentTime / (float)m_ticksPerSecond; 
		float lastTime = (float)m_lastFPSUpdate / (float)m_ticksPerSecond; 
		m_fps = (float)m_numFrames / (currentTime – lastTime); 

		m_lastFPSUpdate = m_currentTime; 
		m_numFrames = 0; 
	} 

	m_lastTime = m_currentTime; 
}


/// <summary>Is the timer stopped?</summary> 
bool Timer::Stopped 
{
	get { return m_timerStopped; } 
}


/// <summary>Frames per second</summary> 
float Timer::FPS 
{
	get { return m_fps; } 
}


/// <summary>Elapsed time since last update. If the timer is stopped, returns 0.</summary> 
float Timer::ElapsedTime 
{
	get 
	{
		if ( Stopped ) 
		{
			return 0; 
		} 
		return m_timeElapsed; 
	} 
}

/// <summary>Total running time.</summary> 
float Timer::RunningTime 
{
	get { return m_runningTime; } 
}