#include "pch.h"
#include ".\Soulworker Packet\SWPacket.h"
#include ".\Damage Meter\Damage Meter.h"
#include ".\Soulworker Packet\SWPacketDamage.h"
#include ".\PacketInfo.h"



SWPacketDamage::SWPacketDamage(SWHEADER* swheader, BYTE* data) : SWPacket(swheader, data) {

}

VOID SWPacketDamage::AttackOfMonster()
{
	BYTE playerNum = *(_data + sizeof(SWHEADER));
	SWPACKETDAMAGE_PLAYER* monster = (SWPACKETDAMAGE_PLAYER*)((_data + sizeof(_SWPACKETDAMAGE_MONSTER) * playerNum) + sizeof(SWHEADER) + 1);
	//LogInstance.MyLog(_T("skill ID : %u\n"), monster->_skillID);


	for (int i = 0; i < playerNum; i++) {
		SWPACKETDAMAGE_MONSTER* player = (SWPACKETDAMAGE_MONSTER*)((_data + sizeof(SWPACKETDAMAGE_MONSTER) * i) + sizeof(SWHEADER) + 1);
		//LogInstance.MyLog(_T("hitted Player : %08x / damageType : %d\n"), player->_monsterID, player->_damageType);
		if (DAMAGEMETER.CheckPlayer(player->_monsterID)) {
			//LogInstance.MyLog(_T("hitted Player : %08x / damage : %u\n"), player->_monsterID, player->_totalDMG);
		}
		
		DAMAGEMETER.AddPlayerGetDamage(player->_monsterID, player->_totalDMG, player->_damageType, monster->_playerID, monster->_skillID);
	}
}

VOID SWPacketDamage::Do() {

	BYTE monsterNum = *(_data + sizeof(SWHEADER));
	SWPACKETDAMAGE_PLAYER* player = (SWPACKETDAMAGE_PLAYER*)((_data + sizeof(_SWPACKETDAMAGE_MONSTER) * monsterNum) + sizeof(SWHEADER) + 1);
	if (player->_playerID == NULL) {
		player->_playerID = 0;
	}

	// If attacker is not a player, then check it is summoned object and get summoner's id for checking is summoner a player
	// 플레이어가 아닌 개체의 데미지이면 플레이어가 소환한 개체인지 확인 및 플레이어가 누군지 받아옴
	if (!DAMAGEMETER.CheckPlayer(player->_playerID)) {
		UINT32 owner_id = 0xffffffff;
		if ((owner_id = DAMAGEMETER.GetOwnerID(player->_playerID)) == 0xffffffff) {
			AttackOfMonster();
			return;
		}
		else {
			if (!DAMAGEMETER.CheckPlayer(owner_id)) {
				AttackOfMonster();
				return;
			}
			else {
				player->_playerID = owner_id;
			}
		}
	}

	for (int i = 0; i < monsterNum; i++) {
		SWPACKETDAMAGE_MONSTER* monster = (SWPACKETDAMAGE_MONSTER*)((_data + sizeof(SWPACKETDAMAGE_MONSTER) * i) + sizeof(SWHEADER) + 1);

		//	LogInstance.WriteLog(const_cast<LPTSTR>(_T("[PLAYER] [DamageType = %d]")), monster->_damageType);
			//DAMAGEMETER.AddDamage(player->_playerID, monster->_totalDMG, monster->_criticalDMG, player->_maxCombo != 0 ? 1 : 0, player->_maxCombo != 0 ? CheckCritical(monster->_damageType) : 0, player->_maxCombo, monster->_monsterID, player->_skillID);

	//		LogInstance.WriteLogA(const_cast<CHAR*>("[DEBUG] [MonsterNum = %d] [PlayerID = %08x] [CheckPlayer = %d] [GetOwnerID = %08x] [OwnerCheckPlayer = %d]"), monsterNum, player->_playerID, DAMAGEMETER.CheckPlayer(player->_playerID), DAMAGEMETER.GetOwnerID(player->_playerID), DAMAGEMETER.CheckPlayer(DAMAGEMETER.GetOwnerID(player->_playerID)));


		DAMAGEMETER.AddDamage(player->_playerID, monster->_totalDMG, monster->_soulstoneDMG, (SWPACKETDAMAGE_DAMAGETYPE)(monster->_damageType), player->_maxCombo, monster->_monsterID, player->_skillID);


		SW_DB2_STRUCT* db = DAMAGEMETER.GetMonsterDB(monster->_monsterID);
		UINT32 db2 = 0;
		if (db != nullptr) {
			db2 = db->_db2;
		}

		// 특정 몹 사망시 일시정지
		if (monster->_remainHP <= 0 && pauseIdList.find(db2) != pauseIdList.end()) {
			DAMAGEMETER.Suspend();
		}

		// 특정 몹 사망시 메이즈 종료 처리
		else if (monster->_remainHP <= 0 && endIdList.find(db2) != endIdList.end()) {
			DAMAGEMETER.SetMazeState(TRUE);
			DAMAGEMETER.Suspend();
		}
	}
}

VOID SWPacketDamage::Debug() {

	BYTE monsterNum = *(_data + sizeof(SWHEADER));
	SWPACKETDAMAGE_PLAYER* player = (SWPACKETDAMAGE_PLAYER*)((_data + sizeof(_SWPACKETDAMAGE_MONSTER) * monsterNum) + sizeof(SWHEADER) + 1);

	//LogInstance.WriteLogA(const_cast<CHAR*>("[DEBUG] [MonsterNum = %d] [PlayerID = %08x] [CheckPlayer = %d] [GetOwnerID = %08x] [OwnerCheckPlayer = %d]"), monsterNum, player->_playerID, DAMAGEMETER.CheckPlayer(player->_playerID), DAMAGEMETER.GetOwnerID(player->_playerID), DAMAGEMETER.CheckPlayer(DAMAGEMETER.GetOwnerID(player->_playerID)));
	//LogInstance.MyLog("1234");

	//if (DAMAGEMETER.CheckPlayer(player->_playerID)) {
	//	return;
	//}

	if (!DAMAGEMETER.CheckPlayer(player->_playerID)) {
		UINT32 owner_id = 0xffffffff;
		if ((owner_id = DAMAGEMETER.GetOwnerID(player->_playerID)) == 0xffffffff) {
			return;
		}
		else {
			if (!DAMAGEMETER.CheckPlayer(owner_id)) {
				return;
			}
			else {
				player->_playerID = owner_id;
			}
		}
	}

	for (int i = 0; i < monsterNum; i++) {
		SWPACKETDAMAGE_MONSTER* monster = (SWPACKETDAMAGE_MONSTER*)((_data + sizeof(SWPACKETDAMAGE_MONSTER) * i) + sizeof(SWHEADER) + 1);

		//if (monster->_monsterID != 0x0034eee4) {
		//	return;
		//}

		/*LogInstance.MyLog(_T("unknown02\n"));
		for (int i = 0; i < 14; i++)
			LogInstance.MyLog(_T("%02x "), monster->_unknown02[i]);
		LogInstance.MyLog(_T("\n"));

		LogInstance.MyLog(_T("Unknown 01 : %02x"), monster->_unknown01);

		LogInstance.WriteLogA(const_cast<CHAR*>("\t[unknown01 = %02x] [UnknownF1 = %f] [UnknownF2 = %f] [UnknownF3 = %f]"),
		monster->_unknown01,monster->_unknownF1, monster->_unknownF2, monster->_unknownF3);

		LogInstance.WriteLogA(const_cast<CHAR*>("\t[PlayerID = %08x] [MonsterID = %08x] [SkillID = %08x] [Damage = %d] [remainHP = %d]"),
			player->_playerID, monster->_monsterID, player->_skillID, monster->_totalDMG, monster->_remainHP);*/
	}
}