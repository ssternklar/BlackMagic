#pragma once
#include "ContentClasses.h"
#include "Material.h"
#include "Entity.h"
#include "GenericJob.h"
#include "WAVFile.h"
class RacingStartJob : public BlackMagic::GenericJob
{
public:
	BlackMagic::AssetPointer<BlackMagic::Material>* lightMats;
	BlackMagic::Entity** lights;
	BlackMagic::AssetPointer<BlackMagic::WAVFile>* sounds;
	bool* toChangeOnceDone;
	RacingStartJob(
		BlackMagic::AssetPointer<BlackMagic::Material>* lightMats,
		BlackMagic::Entity** lights, 
		BlackMagic::AssetPointer<BlackMagic::WAVFile>* sounds,
		bool* toChangeOnceDone
	);
	virtual void Run() override;
	virtual ~RacingStartJob();
};

