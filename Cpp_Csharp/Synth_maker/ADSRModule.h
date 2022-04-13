#ifndef ADSR_MODULE_H
#define ADSR_MODULE_H

class ADSRModule
{
public:
	ADSRModule();

	// Call when key is pressed
	void NoteOn(double timeOn);
	// Call when key is released
	void NoteOff(double timeOff);

	// Get the correct amplitude at the requested point in time
	double GetAmplitude(double time) const;

	enum Phase
	{
		ATTACK,
		DECAY,
		SUSTAIN,
		RELEASE
	};

private:
	double m_attackTime;
	double m_decayTime;
	double m_releaseTime;

	double m_sustainAmplitude;
	double m_startAmplitude;
	double m_triggerOffTime;
	double m_triggerOnTime;
	bool   m_noteOn;

	Phase getPhase(double lifeTime) const;

};

#endif // ADSR_MODULE_H