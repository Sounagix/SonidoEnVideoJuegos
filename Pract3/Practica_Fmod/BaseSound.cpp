#include <fmod.hpp>
#include <fmod_errors.h>


//	Sonido base para cualquier sonido que se quiera crear
class BaseSound {
protected:
	//	Sonido a reproducir
	Sound* sonido;
	//	Canal para reproducir
	Channel* canal;
	//	frame rate
	float timeRate = 0.0f;
	//	tiempo actual
	float currTime = 0.0f;

public:
	//	Constructor generico
	BaseSound() {};

	//	Destructor generico
	~BaseSound() {
		std::cout << "Base\n";
		sonido->release();
	}

	//	Reproduce un sound 
	void play() {
		FMOD_RESULT res = syst->playSound(sonido, 0, false, &canal);
		ERRCHECK(res);
	}

	// Reproduce un sound y si loop es -1 se reproduce en loop
	void playLoop(int loop = -1) {
		canal->setLoopCount(loop);
		FMOD_RESULT res = syst->playSound(sonido, 0, false, &canal);
		ERRCHECK(res);
	}

	//	Setea a un canal como pausa o no en función de un booleano
	void setPause(bool status) {
		canal->setPaused(status);
	}

	//	Cambia el volumen de un sonido *TODO
	void setVolume(float v) {
		canal->setVolume(v);
	}

	//	Cambia el estado de un canal a mute en función de un bool
	void setMute(bool status) {
		canal->setMute(status);
	}

	//	Cambia el pitch de un canal en función de un valor(float)
	void setPitch(float value) {
		canal->setPitch(value);
	}

	//	Cambia el frame rate de un sonido
	void setFrameRate(float value) {
		timeRate = value;
	}

	virtual void update(float deltaTime) = 0;

	//	Determina si un sonido se está reproduciendo
	bool isPlaying() {
		bool active;
		canal->isPlaying(&active);
		return active;
	}

};