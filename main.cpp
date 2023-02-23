#include <iostream>
#include "sound_Interface.h"
using namespace std;

namespace synth {
	double w(double dHertz)
	{
		return dHertz * 2.0 * PI;
	}

	struct note
	{
		int id;			// dcale position
		double on;		// time note was activated
		double off;		// time note was deactivated
		bool active;
		int channel;

		note()
		{
			id = 0;
			on = 0.0;
			off = 0.0;
			active = false;
			channel = 0;
		}
	};

	double osc(double dHertz, double dTime, int nType, double dLFOHertz = 0.0, double dLFOAmplitude = 0.0)
	{
		double dFreq = w(dHertz) * dTime + dLFOAmplitude * dHertz * sin(w(dLFOHertz) * dTime);

		switch (nType)
		{
			// sin wave
		case 0:
			return sin(dFreq);

			// square wave
		case 1:
			return sin(dFreq) > 0.0 ? 0.1 : -0.1;

			// triangle wave
		case 2:
			return asin(sin(dFreq)) * 2.0 / PI;

			// sawtooth wave (warmer/slower/analogue)
		case 3:
		{
			double dOutput = 0.0;

			for (double n = 1.0; n < 100.0; n++)
				dOutput += (sin(n * w(dHertz) * dTime)) / n;

			return dOutput * (2.0 / PI);
		}

		// pseudo-random noise
		case 4:
			return 2.0 * ((double)rand() / (double)RAND_MAX) - 1.0;


		default:
			return 0.0;
		}
	}

	// Scale to frequency conversion
	const int SCALE_DEFAULT = 0;
	double scale(const int nNoteID, const int nScaleID = SCALE_DEFAULT)
	{
		switch (nScaleID)
		{
		case SCALE_DEFAULT: default:
			return 128 * pow(1.0594630943592952645618252949463, nNoteID);
		}
	}

	// Envelopes
	struct sEnvelopeADSR
	{
		double dAttackTime;
		double dDecayTime;
		double dReleaseTime;
		double dSustainAmplitude;
		double dStartAmplitude;

		double dTriggerOnTime;
		double dTriggerOffTime;

		bool bNoteOn;

		sEnvelopeADSR()
		{
			dAttackTime = 0.2;
			dDecayTime = 1.0;
			dStartAmplitude = 1.0;
			dSustainAmplitude = 0.5;
			dReleaseTime = 1.0;
			bNoteOn = false;
			dTriggerOffTime = 0.0;
			dTriggerOnTime = 0.0;
		}

		double getAmplitude(double dTime, const double dTimeOn, const double dTimeOff)
		{
			double dAmplitude = 0.0;
			double dReleaseAmplitude = 0.0;

			if (dTimeOn > dTimeOff) // Note is on
			{
				double dLifeTime = dTime - dTimeOn;

				if (dLifeTime <= dAttackTime)
					dAmplitude = (dLifeTime / dAttackTime) * dStartAmplitude;

				if (dLifeTime > dAttackTime && dLifeTime <= (dAttackTime + dDecayTime))
					dAmplitude = ((dLifeTime - dAttackTime) / dDecayTime) * (dSustainAmplitude - dStartAmplitude) + dStartAmplitude;

				if (dLifeTime > (dAttackTime + dDecayTime))
					dAmplitude = dSustainAmplitude;
			}
			else // Note is off
			{
				double dLifeTime = dTimeOff - dTimeOn;

				if (dLifeTime <= dAttackTime)
					dReleaseAmplitude = (dLifeTime / dAttackTime) * dStartAmplitude;

				if (dLifeTime > dAttackTime && dLifeTime <= (dAttackTime + dDecayTime))
					dReleaseAmplitude = ((dLifeTime - dAttackTime) / dDecayTime) * (dSustainAmplitude - dStartAmplitude) + dStartAmplitude;

				if (dLifeTime > (dAttackTime + dDecayTime))
					dReleaseAmplitude = dSustainAmplitude;

				dAmplitude = ((dTime - dTimeOff) / dReleaseTime) * (0.0 - dReleaseAmplitude) + dReleaseAmplitude;
			}

			// Amplitude should not be negative
			if (dAmplitude <= 0.000)
				dAmplitude = 0.0;

			return dAmplitude;
		}

		void NoteOn(double dTimeOn)
		{
			dTriggerOnTime = dTimeOn;
			bNoteOn = true;
		}

		void NoteOff(double dTimeOff)
		{
			dTriggerOffTime = dTimeOff;
			bNoteOn = false;
		}
	};

	double env(const double dTime, sEnvelopeADSR& env, const double dTimeOn, const double dTimeOff)
	{
		return env.getAmplitude(dTime, dTimeOn, dTimeOff);
	}


	struct instrument {
		double dVolume;
		sEnvelopeADSR env;

		virtual double sound(double dTime, synth::note n, bool& bNoteFinished) = 0;
	};

	struct bell : public instrument {

		bell()
		{
			env.dAttackTime = 0.01;
			env.dDecayTime = 1.0;
			env.dStartAmplitude = 1.0;
			env.dSustainAmplitude = 0.5;
			env.dReleaseTime = 1.0;

			dVolume = 1.0;
		}

		virtual double sound(double dTime, synth::note n, bool& bNoteFinished)
		{
			double dAmplitude = synth::env(dTime, env, n.on, n.off);
			if (dAmplitude <= 0.0) bNoteFinished = true;

			double dSound =
				(
					+ 1.0   * synth::osc(synth::scale(n.id + 12), n.on - dTime, 0, 5.0, 0.001)
					+ 0.5   * synth::osc(synth::scale(n.id + 24), n.on - dTime, 0)
					+ 0.25  * synth::osc(synth::scale(n.id + 36), n.on - dTime, 0)
				);

			return dAmplitude * dSound * dVolume;
		}
	};

	struct harmonica : public instrument {

		harmonica()
		{
			env.dAttackTime = 0.2;
			env.dDecayTime = 1.0;
			env.dStartAmplitude = 1.0;
			env.dSustainAmplitude = 1.0;
			env.dReleaseTime = 1.0;

			dVolume = 1.0;
		}

		virtual double sound(double dTime, synth::note n, bool &bNoteFinished)
		{
			double dAmplitude = synth::env(dTime, env, n.on, n.off);
			if (dAmplitude <= 0.0) bNoteFinished = true;

			double dSound =
				(
					+ 1.00 * synth::osc(synth::scale(n.id), n.on - dTime, 1, 5.0, 0.001)
					+ 0.50 * synth::osc(synth::scale(n.id), n.on - dTime, 1)
					+ 0.25 * synth::osc(synth::scale(n.id - 12), n.on - dTime, 1)
				);

			return dAmplitude * dSound * dVolume;
		}
	};
}


// Global synth variables
atomic<double> dFrequencyOutput = 0.0;
double dOctaveBaseFreq = 110.0;
double d12thRootof2 = pow(2.0, 1.0 / 12.0);
//sEnvelopeADSR envelope;

//instrument *voice = nullptr;

vector<synth::note> vecNotes;
mutex muxNotes;
synth::bell instBell;
synth::harmonica instHarm;


double makeNoice(int nChannel, double dTime)
{
	unique_lock<mutex> lm(muxNotes);
	double dMixedOutput = 0.0;

	for (auto& n : vecNotes)
	{
		bool bNoteFinished = false;
		double dSound = 0;
		if (n.channel == 2)
			dSound = instBell.sound(dTime, n, bNoteFinished);
		if (n.channel == 1)
			dSound = instHarm.sound(dTime, n, bNoteFinished) * 0.5;
		dMixedOutput += dSound;

		if (bNoteFinished && n.off > n.on)
			n.active = false;
	}

	// remove unactive notes from the vector
	auto n = vecNotes.begin();
	while (n != vecNotes.end())
	{
		if (n->active == false)
			n = vecNotes.erase(n);
		else
			++n;
	}

	return dMixedOutput * 0.2;
}

int main()
{
	vector<wstring> devices = sound_Interface<short>::Enumerate();

	for (auto d : devices) wcout << "[+] Found Output Device: " << d << endl;

	// create sound interface
	wcout << "Using Device: " << devices[0] << endl;
	sound_Interface<short> sound(devices[0], 44100, 2, 8, 512);


	// link noiceMaker function to the sound machine
	sound.SetUserFunction(makeNoice);

	while (1)
	{
		for (int k = 0; k < 16; k++)
		{
			short nKeyState = GetAsyncKeyState((unsigned char)("ZSXCFVGBNJMK\xbcL\xbe\xbf"[k]));

			double dTimeNow = sound.GetTime();

			// Check if note already exists in currently playing notes
			// muxNotes is a mutex 
			muxNotes.lock();
			auto noteFound = find_if(vecNotes.begin(), vecNotes.end(), [&k](synth::note const& item) { return item.id == k; });
			if (noteFound == vecNotes.end())
			{
				// Note not found in vector

				if (nKeyState & 0x8000)
				{
					// Key has been pressed so create a new note
					synth::note n;
					n.id = k;
					n.on = dTimeNow;
					n.channel = 2;
					n.active = true;

					// Add note to vector
					vecNotes.emplace_back(n);
				}
				else
				{
					// Note not in vector, but key has been released...
					// ...nothing to do
				}
			}
			else
			{
				// Note exists in vector
				if (nKeyState & 0x8000)
				{
					// Key is still held, so do nothing
					if (noteFound->off > noteFound->on)
					{
						// Key has been pressed again during release phase
						noteFound->on = dTimeNow;
						noteFound->active = true;
					}
				}
				else
				{
					// Key has been released, so switch off
					if (noteFound->off < noteFound->on)
					{
						noteFound->off = dTimeNow;
					}
				}
			}
			// muxNotes is a mutex 
			muxNotes.unlock();
		}
	}

	return 0;
}