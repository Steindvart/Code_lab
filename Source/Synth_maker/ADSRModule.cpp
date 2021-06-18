#include "Shared/stdafx.h"

#include "ADSRModule.h"

ADSRModule::ADSRModule()
	: m_attackTime(0.1)
	, m_decayTime(0.01)
	, m_releaseTime(0.2)
	, m_sustainAmplitude(0.8)
	, m_startAmplitude(1.0)
	, m_triggerOnTime(0.0)
	, m_triggerOffTime(0.0)
	, m_noteOn(false)
{ }

// Call when key is pressed
void ADSRModule::NoteOn(double timeOn)
{
	m_triggerOnTime = timeOn;
	m_noteOn = true;
}

// Call when key is released
void ADSRModule::NoteOff(double timeOff)
{
	m_triggerOffTime = timeOff;
	m_noteOn = false;
}

// Get the correct amplitude at the requested point in time
double ADSRModule::GetAmplitude(double time) const
{
	double amplitude = 0.0;
	double lifeTime = time - m_triggerOnTime;

	if (m_noteOn)
	{
		switch (getPhase(lifeTime))
		{
		case ADSRModule::ATTACK:
			amplitude = (lifeTime / m_attackTime) * m_startAmplitude;
			break;
		case ADSRModule::DECAY:
			amplitude = ((lifeTime - m_attackTime) / m_decayTime) * (m_sustainAmplitude - m_startAmplitude) + m_startAmplitude;
			break;
		case ADSRModule::SUSTAIN:
			amplitude = m_sustainAmplitude;
			break;
		default:
			break;
		}
	}
	else
	{
		// Note has been released, so in release phase
		amplitude = ((time - m_triggerOffTime) / m_releaseTime) * (0.0 - m_sustainAmplitude) + m_sustainAmplitude;
	}

	// Amplitude should not be negative
	if (amplitude <= 0.0001)
		amplitude = 0.0;

	return amplitude;
}

ADSRModule::Phase ADSRModule::getPhase(double lifeTime) const
{
	if (lifeTime <= m_attackTime)
		return ATTACK;
	else if (lifeTime > m_attackTime && lifeTime <= (m_attackTime + m_decayTime))
		return DECAY;
	else if (lifeTime > (m_attackTime + m_decayTime))
		return SUSTAIN;
	else
		return RELEASE;
}
