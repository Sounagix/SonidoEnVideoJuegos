#pragma once
#include <vector>
#include <string>

class Source
{
public:
	enum RutaId : int {
		Battle = 0,
		FootStep,
		Gun1,
		Gun2,
		Motor,
		Piano,
		Rifle,
		RifleMod01,
		RifleMod02,
		RifleMod03,
		RifleMod04,
		Scooter,
		Talking,
		Siren,
		Size
	};

	enum EffectId : int
	{
		Play_Pause = 0,
		Stop,
		Pitch,
		Movement,
		Volumen,
		Posicional,
		FadeIn,
		FadeOut,
		Move3DElemt
	};

	enum EffectIdSubMenu : int {
		ConeIn = 0,
		ConeOut,
		ConeOrientation,
		MinDistance3D,
		MaxDistance3D,
		Doppler,
		SubSize
	};


	enum PosEffects : int
	{
		MinDistance = 0,
		MaxDistance,
		ConeI,
		ConeO,
		SizeEf
	};

	struct SoundID
	{
		public:
			RutaId tipo;
			std::string ruta;
			std::string nombre;
	};

	struct Effect
	{
	public:
		EffectId effect;
		std::string name;
	};

	struct PosEffectStruct
	{
		PosEffects f;
		std::string name;
	};

	struct SubMenuEffects
	{
		EffectIdSubMenu f;
		std::string name;
	};

	static std::vector<SoundID> sonidos;
	static std::vector<Effect> efectos;
	static std::vector<PosEffectStruct> posEfectos;
	static std::vector<SubMenuEffects> subMenu;
};

