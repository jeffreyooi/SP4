#pragma once
#include "Entity.h"
#include "TextureImage.h"

class Powerup : public Entity
{
private:
	int Cost;
	/*int BaseHealthIncrease;
	int Shield;
	float FireRateMultiply;
	float DamageMultiply;*/
	bool Ready;
	float Value;
	float MaxDuration;
	float Duration;
	float Cooldown;

public:
	enum POWER_TYPE
	{
		POWER_NONE,
		POWER_INCREASEBASEHEALTH,
		POWER_SHIELD,
		POWER_FIRERATEMULTIPLY,
		POWER_DAMAGEMULTIPLY,
		POWER_TANKBACKUP,
		POWER_TOTAL,
	};

	POWER_TYPE type;
	
	Powerup(POWER_TYPE type);
	~Powerup();

	int GetCost();
	/*int GetBaseHealthIncrease();
	int GetShield();
	float GetFireRateMultiply();
	float GetDamageMultiply();*/
	float GetValue();
	bool GetReady();

	void SetCost(int Cost);
	//void SetBaseHealthIncrease(int BaseHealthIncrease);
	//void SetShield(int Shield);
	//void SetFireRateMultiply(float FireRateMultiply);
	//void SetDamageMultiply(float DamageMultiply);
	void SetValue(float Value);

	void Update(float dt);
	float GetDuration();

	void RenderDurationBar(int x, int y);
};

