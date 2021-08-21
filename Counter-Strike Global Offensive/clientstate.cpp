#include "hooked.hpp"
#include "displacement.hpp"
#include "player.hpp"
#include "weapon.hpp"
#include "prediction.hpp"
#include "movement.hpp"
#include "aimbot.hpp"
#include "anti_aimbot.hpp"
#include "menu.h"
#include "game_movement.h"
#include "lag_compensation.hpp"
#include <intrin.h>
#include "rmenu.hpp"

namespace Hooked
{
	void __fastcall PacketStart(void* ecx, void* edx, int incoming_sequence, int outgoing_acknowledged)
	{
		using Fn = void(__thiscall*)(void*, int, int);
		static auto ofunc = Source::m_pClientStateSwap->VCall<Fn>(5);

		if (!cheat::main::local() || cheat::main::local()->IsDead() || !cheat::Cvars.RageBot_Enable.GetValue() || !Source::m_pEngine->IsInGame() || cheat::main::command_numbers.empty()
			/*|| game_rules->is_freeze_period() || !c_events::is_active_round*/)
			return ofunc(ecx, incoming_sequence, outgoing_acknowledged);

		for (auto it = cheat::main::command_numbers.begin(); it != cheat::main::command_numbers.end();) 
		{
			if (*it == outgoing_acknowledged)
			{
				cheat::main::command_numbers.erase(it);
				return ofunc(ecx, incoming_sequence, outgoing_acknowledged);
			}

			++it;
		}
	}

	//void __fastcall PacketEnd(pPastaState* cl_state, void* EDX)
	//{
	//	using Fn = void(__thiscall*)(void*);
	//	static auto ofunc = Source::m_pClientStateSwap->VCall<Fn>(6);

	//	if (!cheat::main::local() || cheat::main::local()->IsDead() || !Source::m_pEngine->IsInGame() || cl_state == nullptr
	//		/*|| game_rules->is_freeze_period() || !c_events::is_active_round*/)
	//		return ofunc(cl_state);

	//	if (cl_state->m_iDeltaTick == cl_state->m_iServerTick/**(int*)(uintptr_t(cl_state) + 0x164) == *(int*)(uintptr_t(cl_state) + 0x16C)*/) {
	//		const auto ack_cmd = cl_state->m_iLastCommandAck;//*(int*)(uintptr_t(cl_state) + 0x4D2C);
	//		auto correct = std::find_if(cheat::main::m_corrections_data.begin(), cheat::main::m_corrections_data.end(), [ack_cmd](const int& a) {
	//			return a == ack_cmd;
	//			});

	//		if (correct != cheat::main::m_corrections_data.begin()) {
	//			if (cheat::main::last_velocity_modifier > (cheat::main::local()->m_flVelocityModifier() + 0.1f))
	//			{
	//				auto weapon = cheat::main::local()->get_weapon();
	//				if (!weapon || weapon && weapon->m_iItemDefinitionIndex() != 64 && !weapon->IsGrenade())
	//				{
	//					for (auto i = 0; i < cheat::main::command_numbers.size(); i++)
	//					{
	//						const auto c_cmd = cheat::main::command_numbers[i];

	//						auto cmd = csgo.m_input()->GetUserCmd(c_cmd.command_number);
	//						auto pcmd = (i == 0 || feature::usercmd.command_numbers.size() <= 1 ? nullptr : csgo.m_input()->GetUserCmd(cheat::main::command_numbers[i - 1].command_number));
	//						auto vcmd = csgo.m_input()->GetVerifiedUserCmd(c_cmd.command_number);

	//						if (!cmd || !vcmd || !(cmd->buttons & IN_ATTACK || cmd->buttons & IN_ATTACK2))
	//							continue;

	//						cmd->buttons &= ~IN_ATTACK;
	//						cmd->buttons &= ~IN_ATTACK2;

	//						cmd->viewangles.x = 89.f;

	//						if (i != 0 && cheat::main::command_numbers.size() > 1)
	//						{
	//							cmd->viewangles.y += 180.f;

	//							cmd->forwardmove = cheat::main::command_numbers[i - 1].move_data.x;
	//							cmd->sidemove = cheat::main::command_numbers[i - 1].move_data.y;
	//							cmd->upmove = cheat::main::command_numbers[i - 1].move_data.z;

	//							Engine::Movement::Instance()->circle_strafe(cmd, cheat::main::command_numbers[i - 1].original_angles);
	//						}

	//						cmd->viewangles.Clamp();

	//						vcmd->m_cmd = *cmd;
	//						vcmd->m_crc = cmd->GetChecksum();
	//					}
	//				}
	//			}

	//			ctx.last_velocity_modifier = ctx.m_local()->m_flVelocityModifier();
	//			ctx.last_velocity_modifier_tick = ack_cmd;
	//		}
	//	}

	//	return ofunc(cl_state);
	//}
}