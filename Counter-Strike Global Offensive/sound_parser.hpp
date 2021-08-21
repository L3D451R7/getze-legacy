#pragma once

#include "source.hpp"

class c_dormant_esp
{
public:
	// Call before and after ESP.
	void start();
	void finish();

	void draw_sound(C_BasePlayer* player);
	//void adjustplayer_finish();
	void setup_adjust(C_BasePlayer* player, SndInfo_t& sound);

	bool valid_sound(SndInfo_t& sound);

	struct SoundPlayer
	{
		void Override(SndInfo_t& sound) {
			m_iIndex = sound.m_nSoundSource;
			m_vecOrigin = *sound.m_pOrigin;
			m_iReceiveTime = Source::m_pGlobalVars->realtime;
		}

		int m_iIndex = 0;
		float m_iReceiveTime = 0;
		Vector m_vecOrigin = Vector(0, 0, 0);
		/* Restore data */
		int m_nFlags = 0;
		C_BasePlayer* player = nullptr;
		Vector m_vecAbsOrigin = Vector(0, 0, 0);
		bool m_bDormant = false;
	} m_cSoundPlayers[64];
	CUtlVector<SndInfo_t> m_utlvecSoundBuffer;
	CUtlVector<SndInfo_t> m_utlCurSoundList;
	std::vector<SoundPlayer> m_arRestorePlayers;
};