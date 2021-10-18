#include "Source.h"


std::vector<Source::SoundID> Source::sonidos{
	{ Battle			, "../muestras/Battle.wav"	,"Battle"},
	{ FootStep			, "../muestras/footstep.wav","FootStep"},
	{ Gun1				, "../muestras/Gun1.wav"	,"Gun1"},
	{ Motor				, "../muestras/motor.wav"	,"Motor"},
	{ Gun2				, "../muestras/Gun2.wav"	,"Gun2"},
	{ Piano				, "../muestras/piano.ogg"	,"Piano"},
	{ Rifle				, "../muestras/rifle.wav"	,"Rifle"},
	{ RifleMod01		, "../muestras/rifle_mod/rifle_01.wav"	,"Rifle01"},
	{ RifleMod02		, "../muestras/rifle_mod/rifle_02.wav"	,"Rifle02"},
	{ RifleMod03		, "../muestras/rifle_mod/rifle_03.wav"	,"Rifle03"},
	{ RifleMod04		, "../muestras/rifle_mod/rifle_04.wav"	,"Rifle04"},
	{ Scooter			, "../muestras/scooter.wav"	,"Scooter"},
	{ Talking			, "../muestras/talking.wav"	,"Talking"},
};

std::vector<Source::Effect> Source::efectos{
	{ Play_Pause	,"Play/Pause_Sound"},
	{ Stop			,"Stop_Sound"},
	{ Pitch			,"Pitch"},
	{ Volumen		,"Volumen"},
	{ Movement		,"Movimiento"},
	{ Posicional	,"Posicional"},
	{ FadeIn		,"FadeIn"},
	{ FadeOut		,"FadeOut"},
};
