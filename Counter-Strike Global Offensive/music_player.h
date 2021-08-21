#pragma once

class ConVar;

class c_music_player {
	using sound_files_t = std::vector< std::string >;
public:
	void init(void);

	void play(std::size_t file_idx);

	void stop(void);

	void run(void);

	void load_sound_files(void);

	sound_files_t	m_sound_files;
	bool			m_playing = false;
private:
	std::string		m_sound_files_path;
	ConVar			*m_voice_loopback = nullptr;
};
