/*
The MIT License (MIT)
Copyright © 2022 Matt Wells

Permission is hereby granted, free of charge, to any person obtaining a copy of this
software and associated documentation files (the “Software”), to deal in the
Software without restriction, including without limitation the rights to use, copy,
modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so, subject to the
following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once


struct DAMAGEstats
          {
	       DWORD normal;//0x04
	       DWORD laser;//0x08
	       DWORD fire;//0x0C
	       DWORD plasma;//0x10
	       DWORD electrical;//0x14
	       DWORD emp;//0x18
	       DWORD explosive;//0x1C
          };

struct PRIMEStats
          {
	       DWORD Strength;//0x24        0
	       DWORD Perception;//0x28      1
	       DWORD Endurance;//0x2C       2
	       DWORD Charisma;//0x30        3
	       DWORD Intelligence;//0x34    4
	       DWORD Agility;//0x38         5
	       DWORD Luck;//0x3C            6
          };

struct SECONDStats
          {
	       DWORD HitPoints;//0x40               7
	       DWORD ActionPoints;//0x44            8
	       DWORD ArmorClass;//0x48              9
	       DWORD Unused4C;//0x4C                A
	       DWORD MeleeDamage;//0x50             B
	       DWORD CarryWeight;//0x54             C
	       DWORD Sequence;//0x58                D
	       DWORD HealingRate;//0x5C             E
	       DWORD CriticalChance;//0x60          F
	       DWORD CriticalHitModifier;//0x64     10
	       DAMAGEstats damageDT;//
	       //DWORD DTnormal;//0x68 //DT section 11
	       //DWORD DTlaser;//0x6C               12
	       //DWORD DTfire;//0x70                13
	       //DWORD DTplasma;//0x74              14
	       //DWORD DTelectrical;//0x78          15
	       //DWORD DTemp;//0x7C                 16
	       //DWORD DTexplosive;//0x80           17
	       DAMAGEstats damageDR;
	       //DWORD DRnormal;//0x84 //DR section 18
	       //DWORD DRlaser;//0x88               19
	       //DWORD DRfire;//0x8C                1A
	       //DWORD DRplasma;//0x90              1B
	       //DWORD DRelectrical;//0x94          1C
	       //DWORD DRemp;//0x98                 1D
	       //DWORD DRexplosive;//0x9C           1E
	       DWORD RadiationResistance;//0xA0     1F
	       DWORD PoisonResistance;//0xA4        20
	       DWORD Age;//0xA8                     21
	       DWORD Gender;//0xAC                  22
          };

struct SKILLS
          {
	       DWORD smallGuns ;//0x13C
	       DWORD bigGuns;//0x140
	       DWORD energyWeapons;//0x144
	       DWORD unarmed;//0x148
	       DWORD melee;//0x14C
	       DWORD throwing;//0x150
	       DWORD firstAid;//0x154
	       DWORD doctor ;//0x158
	       DWORD sneak;//0x15C
	       DWORD lockpick;//0x160
	       DWORD steal;//0x164
	       DWORD traps;//0x168
	       DWORD science;//0x16C
	       DWORD repair;//0x170
	       DWORD speech;//0x174
	       DWORD barter;//0x178
	       DWORD gambling;//0x17C
	       DWORD outdoorsman;//0x180
          };


struct PROTOall
          {
	       DWORD objID;//0x00
	       DWORD txtID;//0x04
	       DWORD frmID;//0x08
	       DWORD lightRadius;//0x0C
	       DWORD lightIntensity;//0x10
	       DWORD lightFlags;//0x14

	       ///DWORD actionFlags;//0x18
	       ///DWORD scrtptID;//0x1C
	       ///DWORD crittFlags;//0x20
           ///PRIMEStats primeStats;//0x24
           ///SECONDStats secondStats;//0x40
           ///PRIMEStats primeStatsBonus;//0xB0
           ///SECONDStats secondStatsBonus;//0xCC
	       ///SKILLS skills;//0x13C
	       ///DWORD bodyType;//0x184
	       ///DWORD expVal;//0x188
	       ///DWORD killType;//0x18C
	       ///DWORD damageType;//0x190
	       ///DWORD frmIDHead; //0x194
	       ///DWORD aiPacket; //0x198
	       ///DWORD teamNum; //0x19C

	      };





struct ITEMTYPEdata
          {
           //	                    //0 armor           1 Containers    2 Drugs
           DWORD data01;//0x24       AC                  MaxSize
           DWORD data02;//0x28       DR DAMAGEstats      OpenFlags
           DWORD data03;//0x2C       --                  //
           DWORD data04;//0x30       --                  //
           DWORD data05;//0x34       --                  //
           DWORD data06;//0x38       --                  //
           DWORD data07;//0x3C       DT DAMAGEstats      //
           DWORD data08;//0x40       --                  //
           DWORD data09;//0x44       --                  //
           DWORD data10;//0x48       --                  //
           DWORD data11;//0x4C       --                  //
           DWORD data12;//0x50       --                  //
           DWORD data13;//0x54       --                  //
           DWORD data14;//0x58       Perk                //
           DWORD data15;//0x5C       MaleFID             //
           DWORD data16;//0x60       FemaleFID           //
           DWORD data17;//0x64       //
           DWORD data18;//0x68       //
          };

struct PROTOitem
          {
	       DWORD objID;//0x00
	       DWORD txtID;//0x04
	       DWORD frmID;//0x08
	       DWORD lightRadius;//0x0C
	       DWORD lightIntensity;//0x10
	       DWORD lightFlags;//0x14
	       DWORD actionFlags;//0x18
	       DWORD scriptID;//0x1C

	       DWORD itemType;//0x20
	       ITEMTYPEdata itemTypeData;//0x24
	       DWORD materialID;//0x6C
	       DWORD size;//0x70
	       DWORD weight;//0x74
	       DWORD cost;//0x78
	       DWORD invFrmID;//0x7C
	       BYTE soundID;//80
	      };


struct PROTOcritter
          {
	       DWORD objID;//0x00
	       DWORD txtID;//0x04
	       DWORD frmID;//0x08
	       DWORD lightRadius;//0x0C
	       DWORD lightIntensity;//0x10
	       DWORD lightFlags;//0x14
	       DWORD actionFlags;//0x18
	       DWORD scriptID;//0x1C

	       DWORD crittFlags;//0x20
           PRIMEStats primeStats;//0x24
           SECONDStats secondStats;//0x40
           PRIMEStats primeStatsBonus;//0xB0
           SECONDStats secondStatsBonus;//0xCC
	       SKILLS skills;//0x13C
	       DWORD bodyType;//0x184
	       DWORD expVal;//0x188
	       DWORD killType;//0x18C
	       DWORD damageType;//0x190
	       DWORD frmIDHead; //0x194
	       DWORD aiPacket; //0x198
	       DWORD teamNum; //0x19C
	      };


struct SCENERYTYPEdata
          {
           DWORD data01;//0x24  //generic == 0xCCCCCCCC, doors == 0x0, ladders & stairs == 0xFFFFFFFF
           DWORD data02;//0x28

          };

struct PROTOscenery
          {
	       DWORD objID;//0x00
	       DWORD txtID;//0x04
	       DWORD frmID;//0x08
	       DWORD lightRadius;//0x0C
	       DWORD lightIntensity;//0x10
	       DWORD lightFlags;//0x14
	       DWORD actionFlags;//0x18
	       DWORD scriptID;//0x1C

	       DWORD sceneryType;//0x20
	       SCENERYTYPEdata sceneryTypeData;//0x24
	       DWORD materialID;//0x2C
           DWORD unknown; //30
	       BYTE soundID;//34
	      };


struct PROTOwall
          {
	       DWORD objID;//0x00
	       DWORD txtID;//0x04
	       DWORD frmID;//0x08
	       DWORD lightRadius;//0x0C
	       DWORD lightIntensity;//0x10
	       DWORD lightFlags;//0x14
	       DWORD actionFlags;//0x18
	       DWORD scriptID;//0x1C

	       DWORD materialID;//0x20
	      };

struct PROTOtile // same as misc
          {
	       DWORD objID;//0x00
	       DWORD txtID;//0x04
	       DWORD frmID;//0x08
	       DWORD lightRadius;//0x0C
	       DWORD lightIntensity;//0x10
	       DWORD lightFlags;//0x14
	       DWORD materialID;//0x18  //misc = unknown
	      };



union PROTO {
   PROTOall *all;
   PROTOitem *item;
   PROTOcritter *critter;
   PROTOscenery *scenery;
   PROTOwall *wall;
   PROTOtile *tile;
   PROTOtile *misc;
};



struct OBJStruct;

struct ITEMnode {
      OBJStruct *obj;
	  DWORD num;
};

struct COMBAT_DATA {
   OBJStruct *who_hit_me;
   LONG currentAP;
   LONG results;
   LONG damage_last_turn;
   LONG aiPacket;
   LONG teamNum;
   DWORD unknown01;
};

struct PUD_CRITTER {
   LONG inv_size;              //0x00
   LONG inv_max;               //0x04
   ITEMnode* item;              //0x08
   LONG reaction_to_pc;        //0x0C
   COMBAT_DATA combat_data;     //0x10
   LONG current_hp;            //0x2C
   LONG current_rad;           //0x30
   LONG current_poison;        //0x34
};

struct PUD_WEAPON {
   LONG cur_ammo_quantity;
   LONG cur_ammo_type_pid;
};
struct PUD_AMMO {
   LONG cur_ammo_quantity;
   LONG none;
};
struct PUD_MISC_ITEM {
   LONG curr_charges;
   LONG none;
};
struct PUD_KEY_ITEM {
   LONG cur_key_code;
   LONG none;
};
struct PUD_PORTAL {
   LONG cur_open_flags;
   LONG none;
};
struct PUD_ELEVATOR {
   LONG elev_type;
   LONG elev_level;
};
struct PUD_STAIRS {
   LONG destMap;
   LONG destBuiltTile;
};


union PUDS {
   PUD_WEAPON weapon;
   PUD_AMMO ammo;
   PUD_MISC_ITEM misc_item;
   PUD_KEY_ITEM key_item;
   PUD_PORTAL portal;
   PUD_ELEVATOR elevator;
   PUD_STAIRS stairs;
};

struct PUD_GENERAL {
   LONG inv_size;              //0x00
   LONG inv_max;               //0x04
   ITEMnode* item;              //0x08
   LONG updated_flags;         //0x0C
   PUDS pud;                    //0x10
   DWORD none01;               //0x18
   DWORD none02;               //0x1C
   DWORD none03;               //0x20
   DWORD none04;               //0x24
   DWORD none05;               //0x28
   DWORD none06;               //0x2C
   DWORD none07;               //0x30
   DWORD none08;               //0x34
};

union PUD {
   PUD_CRITTER critter;
   PUD_GENERAL general;
};

struct OBJStruct {
	  DWORD objID;//0x00 //pc = PF00
	  LONG hexNum;//0x04
	  LONG xShift;//0x08
	  LONG yShift;//0x0C
	  LONG viewScrnX;//0x10
	  LONG viewScrnY;//0x14
	  DWORD frameNum;//0x18
	  DWORD ori;//0x1C
	  DWORD frmID;//0x20
	  DWORD flags;//0x24 //critter 24240060 //inv 0x000000FF = 1=item1, 2 = item2 4 = armor
	  LONG level;//0x28
	  PUD pud;
	  DWORD proID;//0x64  01
	  DWORD cID;//0x68  05
	  DWORD light_dist;//0x6C 04 //Light strength of this object?   lightRadius
	  DWORD light_intensity;//0x70 0100 //Something to do with radiation?  lightIntensity
	  DWORD combatFlags;//0x74 set to =0   //only valid in combat //read and written but set to 0 on load.
	  DWORD scriptID1;//0x78   50460004  34000004 related to load time  /map scrip ID ?
	  DWORD unknown7C;//0x7C set to =0  //not read but written but set to 0 on load.
	  DWORD scriptID2;//0x80  //objScriptID?
};


struct OBJNode {
      OBJStruct*obj;
      OBJNode*next;
};


///OBJStruct.flags-------
#define FLG_Disabled       0x00000001 //???
#define FLG_Flat           0x00000008
#define FLG_NoBlock        0x00000010
#define FLG_MultiHex       0x00000800
#define FLG_NoHighlight    0x00001000
#define FLG_TransRed       0x00004000
#define FLG_TransNone      0x00008000
#define FLG_TransWall      0x00010000
#define FLG_TransGlass     0x00020000
#define FLG_TransSteam     0x00040000
#define FLG_TransEnergy    0x00080000
#define FLG_LightThru      0x20000000
#define FLG_ShootThru      0x80000000
#define FLG_WallTransEnd   0x10000000

#define FLG_MarkedByPC  0x40000000
//items
#define FLG_IsHeldSlot1 0x01000000
#define FLG_IsHeldSlot2 0x02000000
#define FLG_IsWornArmor 0x04000000


///OBJStruct.combatFlags-------
#define FLG_NotVisByPC           0x00000020
#define FLG_PCTeamMem            0x00000008
#define FLG_NonPCTeamMem         0x00000001
#define FLG_IsNotPC              0x00FFFFFF //check if any above flags set
#define FLG_IsPC                 0x00000000
#define FLG_IsNotFightable       0x80000000


///Scenery Types--------
#define FLG_Portal           0x00000000
#define FLG_Stairs           0x00000001
#define FLG_Elevators        0x00000002
#define FLG_LadderBottom     0x00000003
#define FLG_LadderTop        0x00000004
#define FLG_Generic          0x00000005


extern OBJStruct **lpObj_PC;
extern OBJStruct **lpObj_ActiveCritter;
extern OBJStruct **lpObj_Mouse2;
extern OBJStruct **lpObj_Mouse;

LONG GetProListSize(LONG proType);

DWORD GetProID(LONG proType, LONG listNum);
LONG F_GetPro(DWORD proID, PROTOall **proto);

OBJStruct* FGetMapObjUnderMouse(int type, DWORD flag, int level);

LONG F_MapObj_Create(OBJStruct **lpObj, DWORD frmID, DWORD proID);
LONG F_MapObj_Move(OBJStruct *obj, DWORD hexPos, LONG level, RECT *pRect);
LONG F_MapObj_Destroy(OBJStruct *obj, RECT *pRect);

int F_Obj_ClearAnimation(OBJStruct *obj);

LONG F_Obj_SetFrmId(OBJStruct *obj, DWORD frmID, RECT *rcOut);

void F_ObjectsSetup();
